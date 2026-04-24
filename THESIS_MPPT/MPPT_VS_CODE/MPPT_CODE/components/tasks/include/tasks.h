#pragma once 

extern  volatile  uint32_t pi_loop_execution_count;

void create_task_MPPT_loop_logic( void );
void create_task_PI_loop_logic( void );
void create_task_LOG_loop(void);


void task_to_Calculate_MPPT( void * pvParameters );
void task_to_Calculate_PI( void * pvParameters );
void task_to_Log_System_State(void *pvParameters);


void create_task_ADC_speed_test(void);