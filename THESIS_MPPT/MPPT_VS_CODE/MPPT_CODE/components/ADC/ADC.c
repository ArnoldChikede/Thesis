#include <stdio.h>
#include <string.h>

#include "ADC.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_err.h"
#include "soc/soc_caps.h"
#include "esp_log.h"

/*
 * Output format helpers
 */
#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S2
#define EXAMPLE_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE1
#define EXAMPLE_ADC_GET_CHANNEL(p_data)     ((p_data)->type1.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)        ((p_data)->type1.data)
#else
#define EXAMPLE_ADC_OUTPUT_TYPE             ADC_DIGI_OUTPUT_FORMAT_TYPE2
#define EXAMPLE_ADC_GET_CHANNEL(p_data)     ((p_data)->type2.channel)
#define EXAMPLE_ADC_GET_DATA(p_data)        ((p_data)->type2.data)
#endif

/*
 * ADC channel configuration
 */
#define ADC_UNIT                            ADC_UNIT_1
#define ADC_ATTEN                           ADC_ATTEN_DB_12
#define ADC_BITWIDTH                        ADC_BITWIDTH_12

/* Keep only current channel */
#define ADC_CH_I_INDUCTOR                   ADC_CHANNEL_6   // GPIO34
#define NUMBER_OF_PATTERNS                  1

/*
 * Sampling setup
 *
 * PWM reference = 80 kHz
 * 10 ADC samples per PWM period
 * 1 channel
 *
 * Total ADC sample rate = 80k * 10 * 1 = 800 kS/s
 *
 * NOTE:
 * SAMPLE_FREQUENCY_KZ is an old name kept to avoid breaking other code.
 * Its value is in Hz.
 */
#define PWM_FREQ_HZ                         80000
#define SAMPLES_PER_PWM_PER_CH              10
#define PWM_BOUNDARY_DIVIDER                32

#define SAMPLE_FREQUENCY_KZ                 (PWM_FREQ_HZ * SAMPLES_PER_PWM_PER_CH * NUMBER_OF_PATTERNS)
#define SAMPLES_TOTAL_PER_PWM_PERIOD        (NUMBER_OF_PATTERNS * SAMPLES_PER_PWM_PER_CH)
#define SAMPLES_TOTAL_PER_BUCKET            (SAMPLES_TOTAL_PER_PWM_PERIOD * PWM_BOUNDARY_DIVIDER)

#define EXAMPLE_READ_LEN                    (SAMPLES_TOTAL_PER_BUCKET * SOC_ADC_DIGI_RESULT_BYTES)
#define MAX_BUF_SIZE                        (EXAMPLE_READ_LEN * 8)
#define TIMEOUT                             5
#define TAG                                 "ADC"

#define MV_TO_V                             0.001f

/*
 * Current sensor scaling
 */
#define I_SENSOR_OFFSET_V                   0.0f
#define INA240_GAIN_V_PER_V                 200.0f
#define SHUNT_RESISTOR_OHMS                 0.003f
#define I_SENSOR_V_PER_A                    0.67f//(INA240_GAIN_V_PER_V * SHUNT_RESISTOR_OHMS)   // 0.6 V/A

/*
 * Existing globals kept
 */
uint32_t ret_num_bytes_read = 0;
static uint8_t adc_read_buf[EXAMPLE_READ_LEN];
float scaled_data = 0.0f;
int adc_measurement = 0;

volatile double V_PV = 0.0;
volatile double V_BOOST = 0.0;
volatile double I_PV = 0.0;

/*
 * Bucketed averaging state
 * Keep only current channel in the bucket
 */
typedef struct {
    uint64_t sum_I_PV_adc;
    uint32_t count_I_PV;
} adc_bucket_t;

/*
 * Continuous mode driver objects
 */
adc_continuous_handle_t handle = NULL;
adc_continuous_handle_cfg_t adc_config = {
    .max_store_buf_size = MAX_BUF_SIZE,
    .conv_frame_size = EXAMPLE_READ_LEN,
};

static adc_digi_pattern_config_t adc_patterns[NUMBER_OF_PATTERNS] = {
    {
        .atten = ADC_ATTEN,
        .channel = ADC_CH_I_INDUCTOR,
        .unit = ADC_UNIT,
        .bit_width = ADC_BITWIDTH,
    },
};

adc_continuous_config_t continuous_config = {
    .sample_freq_hz = SAMPLE_FREQUENCY_KZ,
    .conv_mode = ADC_CONV_SINGLE_UNIT_1,
    .format = EXAMPLE_ADC_OUTPUT_TYPE,
    .pattern_num = NUMBER_OF_PATTERNS,
    .adc_pattern = adc_patterns,
};

/*
 * Calibration handle/config
 */
adc_cali_handle_t cali_handle_Unit1;
adc_cali_line_fitting_config_t cali_config_Unit1 = {
    .unit_id = ADC_UNIT_1,
    .atten = ADC_ATTEN,
    .bitwidth = ADC_BITWIDTH,
};

/*
 * These are kept so other files do not break
 */
void ADC_MarkPwmPeriodBoundary(void)
{
    // no-op
}

void ADC_MarkPwmPeriodBoundaryFromISR(void)
{
    // no-op
}

static inline void adc_bucket_clear(adc_bucket_t *b)
{
    memset(b, 0, sizeof(*b));
}

static inline uint32_t adc_bucket_total_samples(const adc_bucket_t *b)
{
    return b->count_I_PV;
}

/*
 * Publish one completed bucket into old public variables.
 * Only current is active now.
 */
static void adc_publish_bucket(const adc_bucket_t *bucket, adc_cali_handle_t cali_handle)
{
    int local_adc_measurement = 0;

    /* Keep unused outputs pinned low/zero */
    V_PV = 0.0;
    V_BOOST = 0.0;

    if (bucket->count_I_PV > 0) {
        int avg_raw_I_PV = (int)(bucket->sum_I_PV_adc / bucket->count_I_PV);

        if (adc_cali_raw_to_voltage(cali_handle, avg_raw_I_PV, &local_adc_measurement) == ESP_OK) {
            float avg_I_PV_adc = local_adc_measurement * MV_TO_V;
            I_PV = (avg_I_PV_adc - I_SENSOR_OFFSET_V) / I_SENSOR_V_PER_A;

            if (I_PV < 0.0f) {
                I_PV = 0.0f;
            }
        }
    }
}

void Initialise_and_measure_ADC(void)
{
    esp_err_t ret;
    int channels_num[NUMBER_OF_PATTERNS] = {0};

    adc_bucket_t active_bucket;
    adc_bucket_clear(&active_bucket);

    V_PV = 0.0;
    V_BOOST = 0.0;
    I_PV = 0.0;

    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &handle));
    ESP_ERROR_CHECK(adc_continuous_config(handle, &continuous_config));
    ESP_ERROR_CHECK(adc_continuous_start(handle));

    ESP_ERROR_CHECK(adc_cali_create_scheme_line_fitting(&cali_config_Unit1, &cali_handle_Unit1));

    adc_continuous_channel_to_io(ADC_UNIT_1, ADC_CH_I_INDUCTOR, &channels_num[0]);

    printf("ADC Channel %d is mapped to GPIO %d\n", ADC_CH_I_INDUCTOR, channels_num[0]);

    /*
     * GPIO34-39 do not support internal pulls.
     */
    for (int i = 0; i < NUMBER_OF_PATTERNS; i++) {
        int current_gpio = channels_num[i];

        if (current_gpio >= 34 && current_gpio <= 39) {
            ESP_LOGW(TAG, "GPIO %d has no internal pull-down/pull-up, use external resistor if needed", current_gpio);
            continue;
        }

        ret = gpio_pulldown_en(current_gpio);

        if (ret == ESP_OK) {
            ESP_LOGI(TAG, "Pulldown enabled on GPIO %d", current_gpio);
        } else {
            ESP_LOGW(TAG, "Could not enable pulldown on GPIO %d", current_gpio);
        }
    }

    while (1) {
        ret = adc_continuous_read(handle, adc_read_buf, EXAMPLE_READ_LEN, &ret_num_bytes_read, TIMEOUT);

        if (ret == ESP_ERR_TIMEOUT) {
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        } else if (ret == ESP_ERR_INVALID_STATE) {
            ESP_LOGE(TAG, "ADC pool overflow / invalid state");
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        } else if (ret != ESP_OK) {
            ESP_LOGE(TAG, "ADC read error");
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        for (int i = 0; i + SOC_ADC_DIGI_RESULT_BYTES <= ret_num_bytes_read; i += SOC_ADC_DIGI_RESULT_BYTES) {
            adc_digi_output_data_t *p = (adc_digi_output_data_t *)&adc_read_buf[i];
            uint32_t chan_num = EXAMPLE_ADC_GET_CHANNEL(p);
            uint32_t data = EXAMPLE_ADC_GET_DATA(p);

            if (chan_num >= SOC_ADC_CHANNEL_NUM(ADC_UNIT)) {
                continue;
            }

            if (chan_num == ADC_CH_I_INDUCTOR) {
                active_bucket.sum_I_PV_adc += data;
                active_bucket.count_I_PV++;
            }

            if (adc_bucket_total_samples(&active_bucket) >= SAMPLES_TOTAL_PER_BUCKET) {
                adc_publish_bucket(&active_bucket, cali_handle_Unit1);
                adc_bucket_clear(&active_bucket);
            }
        }

        /*
         * Do NOT add vTaskDelay(2) here.
         * At 800 kS/s it can overflow the ADC DMA pool.
         * If you want to be polite to the scheduler, use taskYIELD().
         */
         vTaskDelay(2);
    }
}