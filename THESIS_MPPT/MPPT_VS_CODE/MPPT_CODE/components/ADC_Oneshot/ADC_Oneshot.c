#include <stdio.h>
#include <string.h>

#include "ADC_Oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"

#include "hal/adc_types.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#define TAG                         "ADC_ONESHOT"

#define ADC_UNIT_USED               ADC_UNIT_1
#define ADC_ATTEN_USED              ADC_ATTEN_DB_12
#define ADC_BITWIDTH_USED           ADC_BITWIDTH_12
#define ADC_CH_I_INDUCTOR           ADC_CHANNEL_6    // GPIO34

#define MV_TO_V                     0.001f

/*
 * Current sensor scaling
 * INA240 gain = 200 V/V
 * Shunt = 3 mOhm
 * Sensor gain = 0.6 V/A
 */
#define I_SENSOR_OFFSET_V           0.00f
#define I_PV_OFFSET_A               0.0f //0.236667f
#define INA240_GAIN_V_PER_V         200.0f
#define SHUNT_RESISTOR_OHMS         0.003f
#define I_SENSOR_V_PER_A           0.67f// (INA240_GAIN_V_PER_V * SHUNT_RESISTOR_OHMS)   // 0.6 V/A

//volatile double I_PV = 0.0;
volatile int I_PV_RAW = 0;
volatile float I_PV_ADC_VOLTS = 0.0f;

/* ADC driver handle */
static adc_oneshot_unit_handle_t adc1_handle = NULL;

/* Calibration handle */
static adc_cali_handle_t adc1_cali_handle = NULL;
static bool adc1_cali_enabled = false;

/* -------------------------------------------------------------------------- */
/* Calibration helpers                                                        */
/* -------------------------------------------------------------------------- */

static bool adc_calibration_init(adc_unit_t unit, adc_atten_t atten, adc_cali_handle_t *out_handle)
{
    esp_err_t ret;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .chan = ADC_CH_I_INDUCTOR,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_USED,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, out_handle);
    if (ret == ESP_OK) {
        calibrated = true;
    }
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = unit,
        .atten = atten,
        .bitwidth = ADC_BITWIDTH_USED,
    };
    ret = adc_cali_create_scheme_line_fitting(&cali_config, out_handle);
    if (ret == ESP_OK) {
        calibrated = true;
    }
#else
    ret = ESP_FAIL;
#endif

    if (calibrated) {
        ESP_LOGI(TAG, "ADC calibration enabled");
    } else {
        ESP_LOGW(TAG, "ADC calibration not available");
    }

    return calibrated;
}

/* -------------------------------------------------------------------------- */
/* ADC init                                                                   */
/* -------------------------------------------------------------------------- */

void ADC_CurrentOneshot_Init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_USED,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc1_handle));

    adc_oneshot_chan_cfg_t chan_config = {
        .bitwidth = ADC_BITWIDTH_USED,
        .atten = ADC_ATTEN_USED,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, ADC_CH_I_INDUCTOR, &chan_config));

    adc1_cali_enabled = adc_calibration_init(ADC_UNIT_USED, ADC_ATTEN_USED, &adc1_cali_handle);

    ESP_LOGI(TAG, "ADC one-shot init done for current channel GPIO34 / ADC_CHANNEL_6");
}

/* -------------------------------------------------------------------------- */
/* Single current sample read                                                 */
/* -------------------------------------------------------------------------- */

esp_err_t ADC_ReadCurrentSample(void)
{
    int raw = 0;
    esp_err_t ret = adc_oneshot_read(adc1_handle, ADC_CH_I_INDUCTOR, &raw);
    if (ret != ESP_OK) {
        return ret;
    }

    I_PV_RAW = raw;

    int voltage_mv = 0;

    if (adc1_cali_enabled) {
        ret = adc_cali_raw_to_voltage(adc1_cali_handle, raw, &voltage_mv);
        if (ret != ESP_OK) {
            return ret;
        }
    } else {
        /* Fallback if calibration is unavailable:
         * leave voltage as raw-scaled estimate if you want, or just return raw.
         */
        voltage_mv = 0;
    }

    I_PV_ADC_VOLTS = voltage_mv * MV_TO_V;

    I_PV = (I_PV_ADC_VOLTS - I_SENSOR_OFFSET_V ) / I_SENSOR_V_PER_A;
    I_PV = I_PV - I_PV_OFFSET_A;
    if (I_PV < 0.0f) {
        I_PV = 0.0f;
    }

    return ESP_OK;
}