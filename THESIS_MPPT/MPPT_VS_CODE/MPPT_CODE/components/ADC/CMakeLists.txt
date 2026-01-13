idf_component_register(SRCS "ADC.c"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_adc REQUIRES esp_driver_gpio)
