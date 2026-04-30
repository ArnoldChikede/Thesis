#include <stdio.h>
#include <math.h>
#include "freertos/FreeRTOS.h"

#include "Run_PI_Controller.h"
#include "MPPT.h"
#include "ADC.h"
#include "pwm.h"
//#include "Led.h"

static volatile bool pi_tuning_reset_pending = false;

const double fs = 80000.0;
const double Pi_Ts =0.001; // 1.0 / fs;      //should be updaste freq       // 

/* tunable gains */
static double kp = 0.07;    // 0.1;
static double ki = 10;            //10.0;

/* protect gain updates */
static portMUX_TYPE pi_gain_mux = portMUX_INITIALIZER_UNLOCKED;

double Error = 0.0;
double Error_Prev = 0.0;

double Kp_contribution = 0.0;
double Ki_contribution = 0.0;
static double Ki_contribution_prev = 0.0;

const double Upper_saturation_Limit = MAX_DUTY_RATIO;
const double Lower_saturation_Limit = 0.0;


double PI_Controller_Output = 0.0;
double PI_Controller_Output_Saturation_Block = 0.0;

double duty_control_signal = 0.0;

static double Satuaration_block_state_2_input = 0.0;

void TEST(void)
{
    printf(" Entering PI control function \n");
}

/* ===== gain setters/getters ===== */

void PI_set_kp(double new_kp)
{
    if (!isfinite(new_kp) || new_kp < 0.0) {
        new_kp = 0.0;
    }

    taskENTER_CRITICAL(&pi_gain_mux);
    kp = new_kp;
    taskEXIT_CRITICAL(&pi_gain_mux);
}

void PI_set_ki(double new_ki)
{
    if (!isfinite(new_ki) || new_ki < 0.0) {
        new_ki = 0.0;
    }

    taskENTER_CRITICAL(&pi_gain_mux);
    ki = new_ki;
    taskEXIT_CRITICAL(&pi_gain_mux);
}

double PI_get_kp(void)
{
    double value;
    taskENTER_CRITICAL(&pi_gain_mux);
    value = kp;
    taskEXIT_CRITICAL(&pi_gain_mux);
    return value;
}

double PI_get_ki(void)
{
    double value;
    taskENTER_CRITICAL(&pi_gain_mux);
    value = ki;
    taskEXIT_CRITICAL(&pi_gain_mux);
    return value;
}

/* soft reset: clears PI memory only */
void PI_tuning_state_reset(void)
{
   // Error = 0.0;
   // Error_Prev = 0.0;
   // Kp_contribution = 0.0;
   // Ki_contribution = 0.0;
   // Ki_contribution_prev = 0.0;
    //PI_Controller_Output = 0.0;
    //PI_Controller_Output_Saturation_Block = 0.0;
   // Satuaration_block_state_2_input = 0.0;

    pi_tuning_reset_pending = true;
}

void PI_control(void)
{
   // printf("PI_control is running\n");

    double kp_local;
    double ki_local;

    taskENTER_CRITICAL(&pi_gain_mux);
    kp_local = kp;
    ki_local = ki;
    taskEXIT_CRITICAL(&pi_gain_mux);


     if (pi_tuning_reset_pending) {
        Error = 0.0;
        Error_Prev = 0.0;
        Kp_contribution = 0.0;
        Ki_contribution = 0.0;
        Ki_contribution_prev = 0.0;
        PI_Controller_Output = 0.0;
        PI_Controller_Output_Saturation_Block = 0.0;
        Satuaration_block_state_2_input = 0.0;
        pi_tuning_reset_pending = false;
    }



    if (control_pwm_signal != 0) {
        Error = Iref - I_PV;

        Kp_contribution = kp_local * Error;
        Ki_contribution = (Error_Prev) * ki_local * Pi_Ts + Ki_contribution_prev;


  if ( Ki_contribution < Lower_saturation_Limit) {
            Ki_contribution = Lower_saturation_Limit;
        } else if (Ki_contribution > Upper_saturation_Limit) {
            Ki_contribution = Upper_saturation_Limit;
        } else {
           Ki_contribution = Ki_contribution;
        }

 

        PI_Controller_Output = Kp_contribution + Ki_contribution;

        if (PI_Controller_Output < Lower_saturation_Limit) {
            PI_Controller_Output = Lower_saturation_Limit;
        } else if (PI_Controller_Output > Upper_saturation_Limit) {
            PI_Controller_Output= Upper_saturation_Limit;
        } else {
          PI_Controller_Output = PI_Controller_Output;
        }






        duty_control_signal = PI_Controller_Output;

        if (control_mode == CONTROL_MODE_AUTOMATIC) {
            Compare_value = (uint32_t)(duty_control_signal * Period_ticks);
            update_compare_value(Compare_value);
        }

        if (kp_local > 0.0) {
            Satuaration_block_state_2_input =
                (PI_Controller_Output - PI_Controller_Output_Saturation_Block) / kp_local;
        } else {
            Satuaration_block_state_2_input = 0.0;
        }

        Error_Prev = Error;
        Ki_contribution_prev = Ki_contribution;
    }
else {
    duty_control_signal = 0.0;
    Error = 0.0;
    Error_Prev = 0.0;
    Kp_contribution = 0.0;
    Ki_contribution = 0.0;
    Ki_contribution_prev = 0.0;
    PI_Controller_Output = 0.0;
    PI_Controller_Output_Saturation_Block = 0.0;
    Satuaration_block_state_2_input = 0.0;
    Compare_value = 0;
    update_compare_value(Compare_value);
}
}

void PI_control_reset(void)
{
    Error = 0.0;
    Error_Prev = 0.0;
    Kp_contribution = 0.0;
    Ki_contribution = 0.0;
    Ki_contribution_prev = 0.0;
    PI_Controller_Output = 0.0;
    PI_Controller_Output_Saturation_Block = 0.0;
    Satuaration_block_state_2_input = 0.0;
    duty_control_signal = 0.0;
    Compare_value = 0;
    update_compare_value(Compare_value);
}