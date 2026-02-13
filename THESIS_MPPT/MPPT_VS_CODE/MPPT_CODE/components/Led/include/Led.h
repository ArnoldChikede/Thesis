#pragma once
                                      //iT MAKES THE FILE BE INCLUDED ONLY ONCE IN A SINGLE COMPILATION
#include <esp_rmaker_core.h>

//Create The Structures for the parameters that we want to add to the device so that we can update them from anotther scope of the function!!
//And place it in the header and it has to be before the extern command for the struct instance 
typedef struct {
    esp_rmaker_param_t *duty;
    esp_rmaker_param_t *voltage;
    esp_rmaker_param_t *current;
    esp_rmaker_param_t *inductor_current;
    esp_rmaker_param_t *power;
} mppt_rmaker_params_t;             




extern esp_rmaker_device_t *MPPT_device;
extern mppt_rmaker_params_t mppt_params;


void  create_MPPT_device(mppt_rmaker_params_t *params);
void Update_Parameters(mppt_rmaker_params_t *params);
