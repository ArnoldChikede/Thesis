#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h" // Include the FreeRTOS semaphore header file to use SemaphoreHandle_t

#define test_code 12


extern SemaphoreHandle_t xSemaphore_control_MPPT_loop_logic;
extern SemaphoreHandle_t xSemaphore_control_PI_loop_logic;


void create_semaphores_isr(void);
void isr_handler_test_code(void* arg);
//void initialise_interrupts(void);





