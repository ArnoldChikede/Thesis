idf_component_register(SRCS "GP_Timers.c"
                       PRIV_REQUIRES esp_driver_gptimer isr
                       INCLUDE_DIRS "include")