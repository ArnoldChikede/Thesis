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
#include "PI_Controller.h"
#include "ADC.h"
#include "MPPT.h"




static const char *TAG = "Led.c";  //Just the nae thats gonna be used at Log level to say explain where the log is coming from
//Device handles for Switch and Light
esp_rmaker_device_t *MPPT_device;


         //Here is more like we create  a custom  data staructure and from this we can crate different instances of the same struct if say we have multiple MPPT devices in the future and we can update the parameters of each device using the respective instance of the struct

mppt_rmaker_params_t mppt_params; //create the instance of the struct to be used in the code and we can update the parameters using this instance from any scope of the code as long as we include the header file of the  component where this struct is defined and where the instance is created



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
    return ESP_OK;
}



void create_MPPT_device(mppt_rmaker_params_t *params)
{



//Create a device.
 MPPT_device= esp_rmaker_device_create("MPPT", ESP_RMAKER_DEVICE_LIGHT , NULL);

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
    esp_rmaker_param_t *power_param = esp_rmaker_power_param_create(ESP_RMAKER_DEF_POWER_NAME, DEFAULT_POWER); //Create Parameter
     esp_rmaker_device_add_param(MPPT_device, power_param); //Now add the parameter to the device


     //Add  another parameeter to control brightness
     esp_rmaker_param_t *brightness_param = esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME,0);
     esp_rmaker_device_add_param(MPPT_device, brightness_param);

    /* Assign the power parameter as the primary, so that it can be controlled from the
     * home screen of the phone apps.
     You choose what youwant to be the dafault parameter of your device though at day end */


//ADDING CUSTOM PARAMETERS TO THE DEVICE NOW

// Duty Cycle (0.0 – 100.0 %)
esp_rmaker_param_t *duty_param = esp_rmaker_param_create(
    "Duty Cycle", NULL, esp_rmaker_float(duty_control_signal), PROP_FLAG_READ | PROP_FLAG_WRITE);
esp_rmaker_device_add_param(MPPT_device, duty_param);


// Input Voltage
esp_rmaker_param_t *voltage_param = esp_rmaker_param_create(
    "Voltage", NULL, esp_rmaker_float(V_PV), PROP_FLAG_READ);
esp_rmaker_device_add_param(MPPT_device, voltage_param);

// Input Current
esp_rmaker_param_t *current_param = esp_rmaker_param_create(
    "Input Current", NULL, esp_rmaker_float(I_PV), PROP_FLAG_READ);
esp_rmaker_device_add_param(MPPT_device, current_param);


// Input Current
esp_rmaker_param_t *inductor_current_param = esp_rmaker_param_create(
    "Inductor Current", NULL, esp_rmaker_float(IL), PROP_FLAG_READ);
esp_rmaker_device_add_param(MPPT_device,inductor_current_param);



// Input Power
esp_rmaker_param_t *power_param_mppt = esp_rmaker_param_create(
    "Power", NULL, esp_rmaker_float(P_PV), PROP_FLAG_READ);
esp_rmaker_device_add_param(MPPT_device, power_param_mppt);





    
esp_rmaker_device_assign_primary_param(MPPT_device, power_param);

    /* Add this device  created to the node 
    So to remember we simply need to go to main and add the device to the node */
    //esp_rmaker_node_add_device(node,  MPPT_device);


//take params as a nickname for  whatever address is passed to the function and then we can use this nickname to update the parameters from another scope of the code using this nickname and the instance of the struct created at the top of the code
//so right here we are assigning the address of the parameters created to the respective members of the struct so that we can use this struct to update/expose the parameters from another scope of the code using the instance of the struct created at the top of the code
//So in my cases params = &mppt_params or simply the address of the instance of the struct created  
params->duty             = duty_param;
params->voltage          = voltage_param;
params->current          = current_param;
params->inductor_current = inductor_current_param;
params->power            = power_param_mppt;



}

// The Updating to be done in one of the tasks loop so far for testing ...we shall create a separate task for it later on
void Update_Parameters(mppt_rmaker_params_t *params)
{
    esp_rmaker_param_update_and_report(params->duty , esp_rmaker_float(V_PV));
esp_rmaker_param_update_and_report(params->current, esp_rmaker_float(I_PV));
esp_rmaker_param_update_and_report(params->inductor_current, esp_rmaker_float(IL));
esp_rmaker_param_update_and_report(params->power, esp_rmaker_float(P_PV));
esp_rmaker_param_update_and_report(params->duty, esp_rmaker_float(duty_control_signal));
}