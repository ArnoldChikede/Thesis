idf_component_register(SRCS "isr.c"
                    REQUIRES esp_driver_gpio freertos
                    INCLUDE_DIRS "include")
