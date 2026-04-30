#include <stdio.h>
#include "Led.h"

#include <esp_rmaker_core.h>
#include <esp_rmaker_standard_types.h>
#include <esp_rmaker_standard_params.h>
#include <esp_rmaker_standard_devices.h>
#include <esp_rmaker_schedule.h>
#include <esp_rmaker_scenes.h>
#include <esp_rmaker_console.h>
#include <esp_rmaker_ota.h>

#include <esp_rmaker_common_events.h>

#include <app_network.h>
#include <app_insights.h>


#include "driver/gpio.h"
#include <string.h>
#include "app_priv.h"
#include "esp_log.h"
#include "Run_PI_Controller.h"
#include "ADC.h"
#include "MPPT.h"
#include "pwm.h"
#include "Protection_Features.h"
#include "Status_leds.h"

#define ESP_RMAKER_DEF_START_STOP_NAME "PWM"
#define ESP_RMAKER_DEF_DUTY_CONTROL_NAME "Duty_Control"
#define ESP_RMAKER_DEF_DUTY_UPDATE_NAME "Manual Duty Cycle For Open Loop Control"
#define ESP_RMAKER_DEF_PV_INPUT_POWER_NAME  "PV Input Power"


#define ESP_RMAKER_DEF_KP_NAME "Kp"
#define ESP_RMAKER_DEF_KI_NAME "Ki"

#define ESP_RMAKER_DEF_IREF_NAME "Current Reference (A)"
#define ESP_RMAKER_DEF_MANUAL_REF_MODE_NAME "Manual Ref Mode"

#define ESP_RMAKER_DEF_CONTROL_MODE_NAME "Auto Mode For Current Control"

static esp_rmaker_param_t *duty_cycle_param_create(const char *name, int val);

static esp_rmaker_param_t *current_reference_param_create(const char *name, float val);

static esp_rmaker_param_t *controller_gain_param_create(
    const char *name, float val, float min, float max, float step);

volatile control_mode_t control_mode = CONTROL_MODE_MANUAL;

static const char *TAG = "Led.c";  //Just the nae thats gonna be used at Log level to say explain where the log is coming from
//Device handles for Switch and Light
esp_rmaker_device_t *MPPT_device;


         //Here is more like we create  a custom  data staructure and from this we can crate different instances of the same struct if say we have multiple MPPT devices in the future and we can update the parameters of each device using the respective instance of the struct

mppt_rmaker_params_t mppt_params; //create the instance of the struct to be used in the code and we can update the parameters using this instance from any scope of the code as long as we include the header file of the  component where this struct is defined and where the instance is created
//global struct variable to hold the parameters of the MPPT device and update them from any scope of the code as long as we include the header file of the  component where this struct is defined and where the instance is created


// Callback to handle commands received from the RainMaker cloud

static esp_err_t write_cb_for_MPPT_device(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
            const esp_rmaker_param_val_t val, void *priv_data, esp_rmaker_write_ctx_t *ctx)
{
    if (ctx) {
        ESP_LOGI(TAG, "Received write request via : %s", esp_rmaker_device_cb_src_to_str(ctx->src));
    }

    if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_POWER_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.b? "true" : "false", esp_rmaker_device_get_name(device),
                esp_rmaker_param_get_name(param));
        app_driver_set_state(val.val.b);
        esp_rmaker_param_update(param, val);

        if (val.val.b==true){
        printf("ndmbonzaniko \n");
        gpio_reset_pin(25);
        gpio_set_direction(25, GPIO_MODE_OUTPUT);
        gpio_set_level(25,1);
        }
        else {gpio_set_level(25,0);}
    }


        if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_BRIGHTNESS_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.i? "true" : "false", esp_rmaker_device_get_name(device),
                esp_rmaker_param_get_name(param));
        app_driver_set_state(val.val.i);
        esp_rmaker_param_update(param, val);


    }



if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_MANUAL_REF_MODE_NAME) == 0) {
    bool enable_manual_ref = val.val.b;

    MPPT_set_manual_ref_mode(enable_manual_ref);
    esp_rmaker_param_update_and_report(param, esp_rmaker_bool(enable_manual_ref));

    ESP_LOGI(TAG, "Manual reference mode = %s", enable_manual_ref ? "ON" : "OFF");

    return ESP_OK;
}



if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_IREF_NAME) == 0) {
    double new_iref = (double)val.val.f;

    if (!MPPT_get_manual_ref_mode()) {
        ESP_LOGI(TAG, "Ignoring manual Iref update because Manual Ref Mode is OFF");
        esp_rmaker_param_update_and_report(param, esp_rmaker_float((float)MPPT_get_iref()));
        return ESP_OK;
    }

    MPPT_set_iref(new_iref);

    double iref_now = MPPT_get_iref();
    ESP_LOGI(TAG, "Updated manual current reference = %.3f A", iref_now);
    esp_rmaker_param_update_and_report(param, esp_rmaker_float((float)iref_now));

    return ESP_OK;
}








if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_KP_NAME) == 0) {
    double new_kp = (double)val.val.f;

    PI_set_kp(new_kp);
    PI_tuning_state_reset();

    double kp_now = PI_get_kp();
    ESP_LOGI(TAG, "Updated Kp = %.6f", kp_now);
    esp_rmaker_param_update_and_report(param, esp_rmaker_float((float)kp_now));
    return ESP_OK;
}

if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_KI_NAME) == 0) {
    double new_ki = (double)val.val.f;

    PI_set_ki(new_ki);
    PI_tuning_state_reset();

    double ki_now = PI_get_ki();
    ESP_LOGI(TAG, "Updated Ki = %.6f", ki_now);
    esp_rmaker_param_update_and_report(param, esp_rmaker_float((float)ki_now));
    return ESP_OK;
}











    /*if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_START_STOP_NAME) == 0) {
        ESP_LOGI(TAG, "Received value = %s for %s - %s",
                val.val.b? "true" : "false", esp_rmaker_device_get_name(device),
                esp_rmaker_param_get_name(param));
        app_driver_set_state(val.val.b);
        esp_rmaker_param_update(param, val);

          
        if (val.val.b==true){
        control_pwm_signal=-1;  // setting  -1 is equivalent to removing the force level
         power_on_off_pwm(control_pwm_signal);  
        printf("start pwm\n");
        }
        else {
             control_pwm_signal=0;
             power_on_off_pwm(control_pwm_signal);  
             printf("stop pwm\n");}

             update_yellow_led_state();
        } */

    if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_START_STOP_NAME) == 0) {
    if (Protection_FaultActive()) {
        ESP_LOGW(TAG, "Ignoring PWM command because a fault is active");
        esp_rmaker_param_update(param, esp_rmaker_bool(false));
        return ESP_OK;
    }

    ESP_LOGI(TAG, "Received value = %s for %s - %s",
            val.val.b ? "true" : "false", esp_rmaker_device_get_name(device),
            esp_rmaker_param_get_name(param));
    app_driver_set_state(val.val.b);
    esp_rmaker_param_update(param, val);

    if (val.val.b == true) {
        control_pwm_signal = -1;
        power_on_off_pwm(control_pwm_signal);
        printf("start pwm\n");
    } else {
        control_pwm_signal = 0;
        power_on_off_pwm(control_pwm_signal);
        printf("stop pwm\n");
    }

    update_yellow_led_state();
}







     if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_CONTROL_MODE_NAME) == 0) {
    ESP_LOGI(TAG, "Received value = %s for %s - %s",
             val.val.b ? "true" : "false",
             esp_rmaker_device_get_name(device),
             esp_rmaker_param_get_name(param));

    control_mode_t previous_mode = control_mode;
    control_mode = val.val.b ? CONTROL_MODE_AUTOMATIC : CONTROL_MODE_MANUAL;

    esp_rmaker_param_update(param, val);

    if (previous_mode == CONTROL_MODE_AUTOMATIC &&
        control_mode == CONTROL_MODE_MANUAL) {
        PI_control_reset();
        printf("Switched to MANUAL mode, PI reset and duty cleared\n");
    } else if (control_mode == CONTROL_MODE_AUTOMATIC) {
        printf("Switched to AUTOMATIC mode\n");
    } else {
        printf("Switched to MANUAL mode\n");
    }
    update_yellow_led_state();
}



    


/*if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_DUTY_UPDATE_NAME) == 0) {
    int duty = val.val.i;

    ESP_LOGI(TAG, "Received value = %d for %s - %s",
             duty,
             esp_rmaker_device_get_name(device),
             esp_rmaker_param_get_name(param));



    Compare_value = (duty * Period_ticks) / 100;

    // printf("control_pwm_signal is %d", control_pwm_signal);

if(Compare_value >0 && control_pwm_signal== -1){
    update_compare_value(Compare_value);}

if(control_pwm_signal== 0){
    Compare_value=0;
    update_compare_value(Compare_value);}

  //  printf("compare is %d", Compare_value);
   // esp_rmaker_param_update(param, esp_rmaker_int(duty));
}*/

/*if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_DUTY_UPDATE_NAME) == 0) {
    int duty = val.val.i;

    ESP_LOGI(TAG, "Received value = %d for %s - %s",
             duty,
             esp_rmaker_device_get_name(device),
             esp_rmaker_param_get_name(param));

    esp_rmaker_param_update(param, val);

    if (control_mode == CONTROL_MODE_MANUAL) {
        Compare_value = (duty * Period_ticks) / 100;

        if (control_pwm_signal == -1) {
            update_compare_value(Compare_value);
        } else {
            Compare_value = 0;
            update_compare_value(Compare_value);
        }
    } else {
        ESP_LOGI(TAG, "Ignoring manual duty update because system is in AUTOMATIC mode");
    }
}*/


if (strcmp(esp_rmaker_param_get_name(param), ESP_RMAKER_DEF_DUTY_UPDATE_NAME) == 0) {
    int duty = val.val.i;

    if (Protection_FaultActive()) {
        ESP_LOGW(TAG, "Ignoring manual duty update because a fault is active");
        return ESP_OK;
    }

    if (duty < 0) {
        duty = 0;
    } else if (duty >MAX_DUTY_PERCENT) {    //We can limit Duty here properly to say 80 percent to avoid stressing the system too much in case of user error and also because at very high duty cycles the system might be unstable and we dont want to cause faults by allowing the user to set very high duty cycles in open loop control
        duty = MAX_DUTY_PERCENT; 
    }

    ESP_LOGI(TAG, "Received value = %d for %s - %s",
             duty,
             esp_rmaker_device_get_name(device),
             esp_rmaker_param_get_name(param));

    /* report back the clamped value so app stays in sync */
    esp_rmaker_param_update_and_report(param, esp_rmaker_int(duty));

    if (control_mode == CONTROL_MODE_MANUAL) {
        Compare_value = (duty * Period_ticks) / 100;

        if (control_pwm_signal == -1) {
            update_compare_value(Compare_value);
        } else {
            Compare_value = 0;
            update_compare_value(Compare_value);
        }
    } else {
        ESP_LOGI(TAG, "Ignoring manual duty update because system is in AUTOMATIC mode");
    }
}
















    return ESP_OK;
}



void create_MPPT_device(mppt_rmaker_params_t *params)
{



//Create a device.
 MPPT_device= esp_rmaker_device_create("MPPT", ESP_RMAKER_DEVICE_OTHER , NULL);

 /* Add the write callback for the device. We aren't registering any read callback yet as
  * it is for future use.
 */

esp_rmaker_device_add_cb(MPPT_device, write_cb_for_MPPT_device, NULL);     //To change this after it works 

 /* Add the standard name parameter (type: esp.param.name), which allows setting a persistent,
   * user friendly custom name from the phone apps. All devices are recommended to have this
  * parameter.
  */

//esp_rmaker_device_add_param(MPPT_device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, "MPPT_Duty_Cycle"));

    /* Add the standard power parameter (type: esp.param.power), which adds a boolean param
     * with a toggle switch ui-type.
     You create the parameter then you have to add it !!!!
      */
   // esp_rmaker_param_t *power_param = esp_rmaker_power_param_create(ESP_RMAKER_DEF_POWER_NAME, DEFAULT_POWER); //Create Parameter
    //esp_rmaker_device_add_param(MPPT_device, power_param); //Now add the parameter to the device

    esp_rmaker_param_t *start_stop_param = esp_rmaker_power_param_create(ESP_RMAKER_DEF_START_STOP_NAME, DEFAULT_POWER); //Create Parameter
    esp_rmaker_device_add_param(MPPT_device, start_stop_param); //Now add the parameter to the device

    esp_rmaker_param_t *control_mode_param =  esp_rmaker_power_param_create(ESP_RMAKER_DEF_CONTROL_MODE_NAME, false);
    esp_rmaker_device_add_param(MPPT_device, control_mode_param);
    


esp_rmaker_param_t *manual_ref_mode_param =
    esp_rmaker_power_param_create(ESP_RMAKER_DEF_MANUAL_REF_MODE_NAME, false);
esp_rmaker_device_add_param(MPPT_device, manual_ref_mode_param);

esp_rmaker_param_t *iref_param =
    current_reference_param_create(ESP_RMAKER_DEF_IREF_NAME, (float)MPPT_get_iref());
esp_rmaker_device_add_param(MPPT_device, iref_param);




     //Add  another parameeter to controlduty_cycle
esp_rmaker_param_t *duty_param = duty_cycle_param_create(ESP_RMAKER_DEF_DUTY_UPDATE_NAME, 0);
esp_rmaker_device_add_param(MPPT_device, duty_param);





    /* Assign the power parameter as the primary, so that it can be controlled from the
     * home screen of the phone apps.
     You choose what youwant to be the dafault parameter of your device though at day end */


//ADDING CUSTOM PARAMETERS TO THE DEVICE NOW

// Duty Cycle (0.0 – 100.0 %)
//esp_rmaker_param_t *duty_param = esp_rmaker_param_create(
   // "Duty Cycle", NULL, esp_rmaker_float(duty_control_signal), PROP_FLAG_READ | PROP_FLAG_WRITE); //Has read or write capabilites
//esp_rmaker_device_add_param(MPPT_device, duty_param);


// Input Voltage
esp_rmaker_param_t *voltage_param = esp_rmaker_param_create(
    "Voltage", NULL, esp_rmaker_float(V_PV), PROP_FLAG_READ);
esp_rmaker_device_add_param(MPPT_device, voltage_param);

// Input Current
esp_rmaker_param_t *current_param = esp_rmaker_param_create(
    "Input Current", NULL, esp_rmaker_float(I_PV), PROP_FLAG_READ);
esp_rmaker_device_add_param(MPPT_device, current_param);


// InDuctor Current
esp_rmaker_param_t *inductor_current_param = esp_rmaker_param_create(
    "Inductor Current", NULL, esp_rmaker_float(I_PV), PROP_FLAG_READ);  //I_PV = IL
esp_rmaker_device_add_param(MPPT_device,inductor_current_param);



// Input Power
esp_rmaker_param_t *power_param_mppt = esp_rmaker_param_create(
    ESP_RMAKER_DEF_PV_INPUT_POWER_NAME, NULL, esp_rmaker_float(P_PV), PROP_FLAG_READ);
esp_rmaker_device_add_param(MPPT_device, power_param_mppt);



esp_rmaker_param_t *kp_param =
    controller_gain_param_create(ESP_RMAKER_DEF_KP_NAME, (float)PI_get_kp(), 0.0f, 5.0f, 0.001f);
esp_rmaker_device_add_param(MPPT_device, kp_param);

esp_rmaker_param_t *ki_param =
    controller_gain_param_create(ESP_RMAKER_DEF_KI_NAME, (float)PI_get_ki(), 0.0f, 600.0f, 0.01f);
esp_rmaker_device_add_param(MPPT_device, ki_param);



    
//esp_rmaker_device_assign_primary_param(MPPT_device, power_param);

    /* Add this device  created to the node 
    So to remember we simply need to go to main and add the device to the node */
    //esp_rmaker_node_add_device(node,  MPPT_device);


//take params as a nickname for  whatever address is passed to the function and then we can use this nickname to update the parameters from another scope of the code using this nickname and the instance of the struct created at the top of the code
//so right here we are assigning the address of the parameters created to the respective members of the struct so that we can use this struct to update/expose the parameters from another scope of the code using the instance of the struct created at the top of the code
//So in my cases params = &mppt_params or simply the address of the instance of the struct created  
params->duty             = duty_param;
params->voltage          = voltage_param;
params->current          = current_param;
//params->inductor_current = inductor_current_param;
params->power            = power_param_mppt;


params->kp = kp_param;
params->ki = ki_param;


params->manual_ref_mode = manual_ref_mode_param;
params->iref = iref_param;


}

// The Updating to be done in one of the tasks loop so far for testing ...we shall create a separate task for it later on
void Update_Parameters(mppt_rmaker_params_t *params)
{
//esp_rmaker_param_update_and_report(params->voltage , esp_rmaker_float(V_PV));
//esp_rmaker_param_update_and_report(params->current, esp_rmaker_float(I_PV));

//esp_rmaker_param_update_and_report(params->inductor_current, esp_rmaker_float(IL));

//esp_rmaker_param_update_and_report(params->power, esp_rmaker_float(P_PV));
//esp_rmaker_param_update_and_report(params->duty, esp_rmaker_int(duty_control_signal)); we shall turn this on 
}



//CREATE A helper function for the Duty Cycle standard Parameter!!
//so that we dont have repetitions when say we want tp create second duty cycle parameter 

static esp_rmaker_param_t *duty_cycle_param_create(const char *name, int val)
{
    esp_rmaker_param_t *param = esp_rmaker_param_create(
        name,
        "esp.param.duty-cycle",
        esp_rmaker_int(val),
        PROP_FLAG_READ | PROP_FLAG_WRITE
    );

    if (param) {
        //esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_SLIDER);  changing the slider to text because slider is not working properly with the current implementation of the code and we want to test the functionality first and then we can change the ui type later on
        esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_TEXT);
        esp_rmaker_param_add_bounds(param,
                                    esp_rmaker_int(0),
                                     esp_rmaker_int(MAX_DUTY_PERCENT),  //we can revert back to 1 if we want though 
                                    esp_rmaker_int(1));
    }
    return param;
}




static esp_rmaker_param_t *current_reference_param_create(const char *name, float val)
{
    esp_rmaker_param_t *param = esp_rmaker_param_create(
        name,
        "esp.param.current",
        esp_rmaker_float(val),
        PROP_FLAG_READ | PROP_FLAG_WRITE
    );

    if (param) {
        esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_TEXT);
        esp_rmaker_param_add_bounds(param,
                                    esp_rmaker_float(0.0f),
                                    esp_rmaker_float(20.0f),
                                    esp_rmaker_float(0.1f));
    }
    return param;
}




static esp_rmaker_param_t *controller_gain_param_create(
    const char *name, float val, float min, float max, float step)
{
    esp_rmaker_param_t *param = esp_rmaker_param_create(
        name,
        "esp.param.gain",
        esp_rmaker_float(val),
        PROP_FLAG_READ | PROP_FLAG_WRITE
    );

    if (param) {
        esp_rmaker_param_add_ui_type(param, ESP_RMAKER_UI_TEXT);
        esp_rmaker_param_add_bounds(param,
                                    esp_rmaker_float(min),
                                    esp_rmaker_float(max),
                                    esp_rmaker_float(step));
    }
    return param;
}









 void update_yellow_led_state(void)
{
    bool mppt_active = (control_mode == CONTROL_MODE_AUTOMATIC) &&
                       (control_pwm_signal != 0) &&
                       (!Protection_FaultActive());

    status_leds_set_mppt_active(mppt_active);
}