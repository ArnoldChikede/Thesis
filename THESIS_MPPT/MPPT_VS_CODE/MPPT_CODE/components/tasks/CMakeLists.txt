idf_component_register(SRCS "tasks.c"
                    REQUIRES isr MPPT PI_Controller Led
                    INCLUDE_DIRS "include")
