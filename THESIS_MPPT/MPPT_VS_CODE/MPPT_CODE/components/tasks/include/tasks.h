#pragma once 



void create_task_MPPT_loop_logic( void );
void create_task_PI_loop_logic( void );

void task_to_Calculate_MPPT( void * pvParameters );
void task_to_Calculate_PI( void * pvParameters );

