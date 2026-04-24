
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "ADC10664.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "rom/ets_sys.h"

#define TAG "ADC10664"

/* ---------------- Pin mapping ----------------
 * S/H and RD tied together
 * CS tied permanently to GND in hardware
 * INT is active low
 *
 * Data bus:
 * DB0 -> GPIO36
 * DB1 -> GPIO39
 * DB2 -> GPIO26
 * DB3 -> GPIO27
 * DB4 -> GPIO14
 * DB5 -> GPIO13
 * DB6 -> GPIO18
 * DB7 -> GPIO19
 * DB8 -> GPIO22
 * DB9 -> GPIO23
 */

#define ADC10664_PIN_SH_RD     GPIO_NUM_16
#define ADC10664_PIN_INT       GPIO_NUM_21

#define ADC10664_PIN_DB0       GPIO_NUM_36
#define ADC10664_PIN_DB1       GPIO_NUM_39
#define ADC10664_PIN_DB2       GPIO_NUM_26
#define ADC10664_PIN_DB3       GPIO_NUM_27
#define ADC10664_PIN_DB4       GPIO_NUM_14
#define ADC10664_PIN_DB5       GPIO_NUM_13
#define ADC10664_PIN_DB6       GPIO_NUM_18
#define ADC10664_PIN_DB7       GPIO_NUM_19
#define ADC10664_PIN_DB8       GPIO_NUM_22
#define ADC10664_PIN_DB9       GPIO_NUM_23

/* Reference setup
 * If VREF+ = 5V and VREF- = 0V
 */
#define ADC10664_VREF_PLUS_V   5.0f
#define ADC10664_VREF_MINUS_V  0.0f
#define ADC10664_MAX_CODE      1023.0f

/* Public globals */
volatile uint16_t ADC10664_RAW = 0;
volatile float ADC10664_VOLTS = 0.0f;

/* Small helper: read all data bus bits */
static inline uint16_t adc10664_read_bus(void)
{
    uint16_t code = 0;

    code |= (gpio_get_level(ADC10664_PIN_DB0) ? 1U : 0U) << 0;
    code |= (gpio_get_level(ADC10664_PIN_DB1) ? 1U : 0U) << 1;
    code |= (gpio_get_level(ADC10664_PIN_DB2) ? 1U : 0U) << 2;
    code |= (gpio_get_level(ADC10664_PIN_DB3) ? 1U : 0U) << 3;
    code |= (gpio_get_level(ADC10664_PIN_DB4) ? 1U : 0U) << 4;
    code |= (gpio_get_level(ADC10664_PIN_DB5) ? 1U : 0U) << 5;
    code |= (gpio_get_level(ADC10664_PIN_DB6) ? 1U : 0U) << 6;
    code |= (gpio_get_level(ADC10664_PIN_DB7) ? 1U : 0U) << 7;
    code |= (gpio_get_level(ADC10664_PIN_DB8) ? 1U : 0U) << 8;
    code |= (gpio_get_level(ADC10664_PIN_DB9) ? 1U : 0U) << 9;

    return code;
}

float ADC10664_CodeToVoltage(uint16_t code)
{
    return ADC10664_VREF_MINUS_V +
           (((float)code) / ADC10664_MAX_CODE) *
           (ADC10664_VREF_PLUS_V - ADC10664_VREF_MINUS_V);
}

bool ADC10664_IsDataReady(void)
{
    /* INT is active low */
    return (gpio_get_level(ADC10664_PIN_INT) == 0);
}

void ADC10664_Init(void)
{
    /* S/H and RD tied together -> output pin */
    gpio_config_t io_out = {
        .pin_bit_mask = (1ULL << ADC10664_PIN_SH_RD),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_out));

    /* INT pin -> input */
    gpio_config_t io_int = {
        .pin_bit_mask = (1ULL << ADC10664_PIN_INT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_int));

    /* Data bus pins -> inputs */
    gpio_config_t io_bus = {
        .pin_bit_mask =
            (1ULL << ADC10664_PIN_DB2) |
            (1ULL << ADC10664_PIN_DB3) |
            (1ULL << ADC10664_PIN_DB4) |
            (1ULL << ADC10664_PIN_DB5) |
            (1ULL << ADC10664_PIN_DB6) |
            (1ULL << ADC10664_PIN_DB7) |
            (1ULL << ADC10664_PIN_DB8) |
            (1ULL << ADC10664_PIN_DB9),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&io_bus));

    /* GPIO36 and GPIO39 are input-only, no gpio_config needed in many cases,
       but keeping them as plain inputs is fine by default. */

    /* Idle high */
    gpio_set_level(ADC10664_PIN_SH_RD, 1);

    ESP_LOGI(TAG, "ADC10664 interface init done");
}

/* Trigger one conversion:
 * 1. Pull S/H-RD low -> sample and convert
 * 2. Return high
 * 3. Wait for INT low
 * 4. Read DB0..DB9
 */
esp_err_t ADC10664_ReadSample(void)
{
    const int timeout_loops = 1000;
    int i = 0;

    /* Trigger sample/conversion pulse */
    gpio_set_level(ADC10664_PIN_SH_RD, 0);

    /* Short pulse width; tweak if needed */
    ets_delay_us(1);

    gpio_set_level(ADC10664_PIN_SH_RD, 1);

    /* Wait for conversion complete: INT active low */
    while (!ADC10664_IsDataReady()) {
        i++;
        if (i > timeout_loops) {
            ESP_LOGW(TAG, "ADC10664 conversion timeout");
            return ESP_ERR_TIMEOUT;
        }
        ets_delay_us(1);
    }

    /* Read 10-bit output bus */
    ADC10664_RAW = adc10664_read_bus();
    ADC10664_VOLTS = ADC10664_CodeToVoltage(ADC10664_RAW);

    return ESP_OK;
}

