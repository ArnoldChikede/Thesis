#pragma once

#define GEN_PIN 22
#define GEN_PIN_2 23

extern float Duty_Ratio;
extern float Duty_Ratio_2;
extern int PWM_Frequency;
extern int Period_ticks;
extern int Compare_value ;     
extern int Compare_value_2 ;  
extern int Resolution_hz;

void pwm_configuration(void);
void duty_ratio_calculation(void);
void pwm_frequency_calculation(void);
void init_Resolution_and_presiod_ticks_to_step_size_of_each(void);
void update_compare_value(void);