#include <stdio.h>
#include "Protection_Features.h"
#include "pwm.h"
#include "Status_leds.h"
#include "Led.h"
#include "Run_PI_Controller.h"


static protection_config_t protection_config_settings;
static protection_status_t protection_status;




static void PWM_Enable(void)
{
    power_on_off_pwm(-1); // Remove force level to allow normal operation..taken from the pwm component
    printf("PWM ENABLED\n");
}

static void PWM_Disable(void)
{
    power_on_off_pwm(0); // Force PWM off..taken from the pwm component
    printf("PWM DISABLED\n");
}




/* --------------------------------------------------------------------------
   Initialize protection system
   -------------------------------------------------------------------------- */
void Protection_Init(const protection_config_t *config)
{
    if (config == NULL)
    {
        return;
    }

    protection_config_settings = *config;

    protection_status.fault = FAULT_NONE;
    protection_status.fault_active = false;
    protection_status.pwm_enabled = true;

    
    status_leds_set_fault(false);
    // PWM_Enable();
}




/* --------------------------------------------------------------------------
   Check for fault conditions
   -------------------------------------------------------------------------- */
void Protection_Check(float output_voltage, float input_current)
{
    /* If a fault is already active */
    if (protection_status.fault_active)
    {
        /* If faults are latched, do nothing until manual reset */
        if (protection_config_settings.latch_fault) //Remember that the latch option lets you choose between the two settings, 1. either manual reset or  2. automatic reset when the values return to normal
        {
            return;
        }

        /* Automatic recovery if values return to safe region */
        if ((output_voltage < protection_config_settings.voltage_recovery_limit) &&
            (input_current < protection_config_settings.current_recovery_limit))
        {
            protection_status.fault = FAULT_NONE;
            protection_status.fault_active = false;
            protection_status.pwm_enabled = true;


            status_leds_set_fault(false);
            update_yellow_led_state();
            PWM_Enable();   //We need to check if this wont affect the continous workig of the PWM ..if not will will leave like howit ,,,if it does we can comment  out coz trhe outcome that we need here is to do nothing to the PWM signal if everything is assmend to within the limits 
        }

        return;
    }


    

    /* Check overvoltage first */
    if (output_voltage >= protection_config_settings.overvoltage_limit)
    {
        protection_status.fault = FAULT_OVERVOLTAGE;
        protection_status.fault_active = true;
        protection_status.pwm_enabled = false;


        control_pwm_signal = 0;
        Compare_value = 0;
        update_compare_value(0);

        status_leds_set_fault(true);
        status_leds_set_mppt_active(false);
        PWM_Disable();
        printf("FAULT: Overvoltage detected\n");
        return;
    }

    /* Check overcurrent */
    if (input_current >= protection_config_settings.overcurrent_limit)
    {
        protection_status.fault = FAULT_OVERCURRENT;
        protection_status.fault_active = true;
        protection_status.pwm_enabled = false;

        control_pwm_signal = 0;
        Compare_value = 0;
        update_compare_value(0);


        status_leds_set_fault(true);
        status_leds_set_mppt_active(false);
        PWM_Disable();
        printf("FAULT: Overcurrent detected %f \n", input_current);
        printf("FAULT: Duty Cycle is  %f \n", duty_control_signal);
        return;
    }
}





/* --------------------------------------------------------------------------
   Manual reset
   -------------------------------------------------------------------------- */
void Protection_Reset(void)
{
    protection_status.fault = FAULT_NONE;
    protection_status.fault_active = false;
    protection_status.pwm_enabled = true;

    status_leds_set_fault(false);
    update_yellow_led_state();
    PWM_Enable();
}





/* --------------------------------------------------------------------------
   Status functions
   -------------------------------------------------------------------------- */
bool Protection_FaultActive(void)
{
    return protection_status.fault_active;
}

fault_type_t Protection_GetFault(void)
{
    return protection_status.fault;
}


protection_status_t Protection_GetStatus(void)
{
    return protection_status;
}