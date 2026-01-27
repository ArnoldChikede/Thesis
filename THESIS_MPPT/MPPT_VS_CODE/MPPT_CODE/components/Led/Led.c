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


static const char *TAG = "Led.c";  //Just the nae thats gonna be used at Log level to say explain where the log is coming from
//Device handles for Switch and Light
esp_rmaker_device_t *light_device;


// Callback to handle commands received from the RainMaker cloud

static esp_err_t write_cb_for_light_device(const esp_rmaker_device_t *device, const esp_rmaker_param_t *param,
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



void create_led_device(void)
{

//Create a device.
 light_device= esp_rmaker_device_create("Light ANO", ESP_RMAKER_DEVICE_LIGHT , NULL);

 /* Add the write callback for the device. We aren't registering any read callback yet as
  * it is for future use.
 */

esp_rmaker_device_add_cb(light_device, write_cb_for_light_device, NULL);     //To change this after it works 

 /* Add the standard name parameter (type: esp.param.name), which allows setting a persistent,
   * user friendly custom name from the phone apps. All devices are recommended to have this
  * parameter.
  */

esp_rmaker_device_add_param(light_device, esp_rmaker_name_param_create(ESP_RMAKER_DEF_NAME_PARAM, "Light Ano"));

    /* Add the standard power parameter (type: esp.param.power), which adds a boolean param
     * with a toggle switch ui-type.
     You create the parameter then you have to add it !!!!
      */
    esp_rmaker_param_t *power_param = esp_rmaker_power_param_create(ESP_RMAKER_DEF_POWER_NAME, DEFAULT_POWER);
     esp_rmaker_device_add_param(light_device, power_param);


     //Add  another parameeter to control brightness
     esp_rmaker_param_t *brightness_param = esp_rmaker_brightness_param_create(ESP_RMAKER_DEF_BRIGHTNESS_NAME,0);
     esp_rmaker_device_add_param(light_device, brightness_param);

    /* Assign the power parameter as the primary, so that it can be controlled from the
     * home screen of the phone apps.
     You choose what youwant to be the dafault parameter of your device though at day end */
    
    esp_rmaker_device_assign_primary_param(light_device, power_param);

    /* Add this device  created to the node 
    So to remember we simply need to go to main and add the device to the node */
    //esp_rmaker_node_add_device(node,  light_device);





}
