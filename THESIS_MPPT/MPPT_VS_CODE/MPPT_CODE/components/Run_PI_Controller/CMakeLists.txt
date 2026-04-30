idf_component_register(SRCS "Run_PI_Controller.c"
                    PRIV_REQUIRES MPPT ADC pwm freertos
                    INCLUDE_DIRS "include")