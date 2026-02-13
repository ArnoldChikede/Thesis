idf_component_register(SRCS "Led.c"
                    INCLUDE_DIRS "include"
                   REQUIRES espressif__esp_rainmaker espressif__rmaker_app_network  espressif__rmaker_app_insights esp_driver_gpio main PI_Controller ADC  MPPT )              