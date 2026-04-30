#pragma once

#define GEN_PIN 25
#define GEN_PIN_2 23

#define MAX_DUTY_PERCENT 80
#define MAX_DUTY_RATIO   (MAX_DUTY_PERCENT / 100.0)

extern float Duty_Ratio;
extern float Duty_Ratio_2;
extern int PWM_Frequency;
extern int Period_ticks;
extern int Compare_value ;     
extern int Compare_value_2 ;  
extern int Resolution_hz;
extern int control_pwm_signal;

//extern volatile uint32_t pwm_sample_trigger_count;

void pwm_configuration(void);
void duty_ratio_calculation(void);
void pwm_frequency_calculation(void);
void init_Resolution_and_presiod_ticks_to_step_size_of_each(void);
void update_compare_value(int Compare_Value);
void power_on_off_pwm(int force_level) ;