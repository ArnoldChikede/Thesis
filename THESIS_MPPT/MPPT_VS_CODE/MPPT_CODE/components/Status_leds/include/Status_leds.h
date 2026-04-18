#pragma once

#include <stdbool.h>

void status_leds_init(void);

void status_leds_set_power_ok(bool on);
void status_leds_set_wifi_connected(bool on);
void status_leds_set_fault(bool on);
void status_leds_set_mppt_active(bool on);