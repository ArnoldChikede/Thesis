#include "status_leds.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define LED_POWER_GPIO   27
#define LED_FAULT_GPIO   14
#define LED_WIFI_GPIO    12
#define LED_MPPT_GPIO    13

//static void status_led_write(gpio_num_t gpio, bool on)
//{
   // gpio_set_level(gpio, on ? 1 : 0);
//}


static void status_led_write(gpio_num_t gpio, bool on)
{
    bool active_low = false;

    if (gpio == LED_POWER_GPIO) {
        active_low = true;   // only green LED inverted
    }

    gpio_set_level(gpio, active_low ? !on : on);
}

void status_leds_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << LED_POWER_GPIO) |
                        (1ULL << LED_FAULT_GPIO) |
                        (1ULL << LED_WIFI_GPIO)  |
                        (1ULL << LED_MPPT_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    gpio_config(&io_conf);

    status_leds_set_power_ok(false);
    status_leds_set_fault(false);
    status_leds_set_wifi_connected(false);
    status_leds_set_mppt_active(false);
}

void status_leds_set_power_ok(bool on)
{
    status_led_write(LED_POWER_GPIO, on);
}

void status_leds_set_wifi_connected(bool on)
{
    status_led_write(LED_WIFI_GPIO, on);
}

void status_leds_set_fault(bool on)
{
    status_led_write(LED_FAULT_GPIO, on);
}

void status_leds_set_mppt_active(bool on)
{
    status_led_write(LED_MPPT_GPIO, on);
}