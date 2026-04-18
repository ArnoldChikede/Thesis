idf_component_register(SRCS "tasks.c"
                    REQUIRES isr MPPT  Run_PI_Controller Led pwm ADC Protection_Features
                    INCLUDE_DIRS "include")
