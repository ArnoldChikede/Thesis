idf_component_register(SRCS "pwm.c"
                    REQUIRES driver  ADC isr
                    INCLUDE_DIRS "include")
