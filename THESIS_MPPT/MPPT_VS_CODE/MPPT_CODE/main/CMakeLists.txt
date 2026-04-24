idf_component_register(SRCS ./app_driver.c ./app_main.c
                       INCLUDE_DIRS "include"
                       REQUIRES Status_leds ADC pwm esp_rainmaker esp_driver_gpio button led_indicator rmaker_app_reset rmaker_app_network rmaker_app_insights nvs_flash protocomm driver espressif__rmaker_common espressif__rmaker_app_network  mbedtls Led MPPT  GP_Timers isr tasks Run_PI_Controller Protection_Features  ) 
