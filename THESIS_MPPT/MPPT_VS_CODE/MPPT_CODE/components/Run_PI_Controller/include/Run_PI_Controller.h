#pragma once

typedef enum {
    CONTROL_MODE_MANUAL = 0,
    CONTROL_MODE_AUTOMATIC = 1
} control_mode_t;

extern volatile control_mode_t control_mode;

extern double duty_control_signal;

void PI_control(void);
void TEST(void);
void PI_control_reset(void);
void PI_tuning_state_reset(void);
void PI_set_kp(double new_kp);
void PI_set_ki(double new_ki);
double PI_get_kp(void);
double PI_get_ki(void);