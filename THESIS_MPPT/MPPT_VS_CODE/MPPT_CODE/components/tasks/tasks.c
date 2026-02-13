#include <stdio.h>
#include "tasks.h"
#include "isr.h"
#include "freertos/idf_additions.h"

#include "tasks.h"
#include "MPPT.h"
#include "PI_Controller.h"
#include "Led.h"




//static TaskHandle_t xHandle_to_change_resolution_up = NULL;  // making xHandle for task to be Global

static TaskHandle_t xHandle_to_MPPT= NULL;
static TaskHandle_t xHandle_to_PI= NULL;










/*
void task_to_Calculate_MPPT( void * pvParameters )
{
    if( xSemaphore_control_MPPT_loop_logic == NULL) 
    {
        printf("xSemaphore_period_ticks_down creation failure \n");   
    }

        else if(  xSemaphore_control_MPPT_loop_logic!= NULL)
        {
            //printf("xSemaphore_period_ticks_down != NULL so successful creation\n");
            if(xSemaphoreTake( xSemaphore_control_MPPT_loop_logic, portMAX_DELAY) == pdTRUE )

            {
            printf("running MPPT loop logic \n");
                MPPT();  //function that Performs MPPT
                Update_Parameters(&mppt_params); //function that updates the parameters exposed to the rainmaker cloud using the instance of the struct created at the top of the code and passed as an argument to the function
                //Shall DELETE THIS LATER
                // xSemaphoreGive( xSemaphore_MPPT_loop_logic);
            create_task_MPPT_loop_logic();  //here we are recreating the task so that it can be triggered again by the interrupt
            vTaskDelete( NULL);
                      }

            else
                {
                        printf("BlockTime expired without the semaphore becoming available.");
                          //vTaskDelay(pdMS_TO_TICKS(delay_for_direction_command_checking));
                        //  xSemaphoreGive( xSemaphore_control_MPPT_loop_logic);
                          create_task_MPPT_loop_logic();
                        vTaskDelete( NULL);
             } 
        }


        else{  
          printf("BlockTime expired without the semaphore becoming available._2\n");        
           // vTaskDelay(pdMS_TO_TICKS(delay_for_direction_command_checking));
           // xSemaphoreGive( xSemaphore_control_MPPT_loop_logic);           
            create_task_MPPT_loop_logic();
            vTaskDelete( NULL ); }

}   */



void task_to_Calculate_MPPT(void *pvParameters)
{
    // Make sure the semaphore exists
    if (xSemaphore_control_MPPT_loop_logic == NULL) 
    {
        printf("xSemaphore_control_MPPT_loop_logic creation failure\n");
        vTaskDelete(NULL);  // Can't proceed without semaphore
        return;
    }

    // Main loop: task stays alive forever
    while (1)
    {
        // Wait for the semaphore (blocks task, frees CPU)
        if (xSemaphoreTake(xSemaphore_control_MPPT_loop_logic, portMAX_DELAY) == pdTRUE)
        {
            printf("running MPPT loop logic\n");

            MPPT();                       // Run MPPT algorithm
           // Update_Parameters(&mppt_params);  //  RainMaker cloud update  

            // Task automatically loops back and blocks on semaphore again
            // NO task deletion or recreation needed
        }

        // Optional small delay to allow other lower-priority tasks to run
        //vTaskDelay(pdMS_TO_TICKS(1));
    }
}
























/*
void task_to_Calculate_PI( void * pvParameters )
{
    if( xSemaphore_control_PI_loop_logic == NULL) 
    {
        printf("xSemaphore_PI creation failure \n");   
    }

        else if(  xSemaphore_control_PI_loop_logic!= NULL)
        {
            //printf("xSemaphore_period_ticks_down != NULL so successful creation\n");
            if(xSemaphoreTake( xSemaphore_control_PI_loop_logic, portMAX_DELAY) == pdTRUE )

            {
            printf("running PI loop logic \n");
                PI();  //function that Performs MPPT
           // xSemaphoreGive( xSemaphore_MPPT_loop_logic);
            create_task_PI_loop_logic();  //here we are recreating the task so that it can be triggered again by the interrupt
            vTaskDelete( NULL);
                      }

            else
                {
                        printf("BlockTime expired without the semaphore becoming available.");
                          //vTaskDelay(pdMS_TO_TICKS(delay_for_direction_command_checking));
                        //  xSemaphoreGive( xSemaphore_control_MPPT_loop_logic);
                          create_task_PI_loop_logic();
                        vTaskDelete( NULL);
             } 
        }


        else{  
          printf("BlockTime expired without the semaphore becoming available._2\n");        
           // vTaskDelay(pdMS_TO_TICKS(delay_for_direction_command_checking));
           // xSemaphoreGive( xSemaphore_control_MPPT_loop_logic);           
            create_task_PI_loop_logic();
            vTaskDelete( NULL ); }

}

*/



void task_to_Calculate_PI(void *pvParameters)
{
    // Make sure the semaphore exists
    if (xSemaphore_control_PI_loop_logic == NULL) 
    {
        printf("xSemaphore_control_PI_loop_logic creation failure\n");
        vTaskDelete(NULL);  // Can't proceed without semaphore
        return;
    }

    // Main loop: task stays alive forever
    while (1)
    {
        // Wait for the semaphore (blocks task, frees CPU)
        if (xSemaphoreTake(xSemaphore_control_PI_loop_logic, portMAX_DELAY) == pdTRUE)
        {
            printf("running PI loop logic\n");

            PI();  // Run PI control loop

            // Optional: if you want, you can rate-limit or conditionally update cloud params here
            // e.g., only if duty cycle changed significantly
        }

        // Optional tiny delay to allow other lower-priority tasks to run
        //vTaskDelay(pdMS_TO_TICKS(1));
    }
}




















void create_task_MPPT_loop_logic( void )
{   
static uint8_t ucParameterToPass;
  xTaskCreatePinnedToCore( task_to_Calculate_MPPT, "PERIOD_TICKS_DOWN TASK",2048, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle_to_MPPT, 1);   //8192 STACK SIZE IN WORDS 1 W= 4 BYTTES 2048
  configASSERT( xHandle_to_MPPT );  
if (xHandle_to_MPPT != NULL) 
{
   // printf("Task handle creation  for MPPT_Loop_Logic succeeded!\n");
           
}

else if ((xHandle_to_MPPT == NULL) ){
    
}

}   



void create_task_PI_loop_logic( void )
{   
static uint8_t ucParameterToPass;
  xTaskCreatePinnedToCore( task_to_Calculate_PI, "PI TASK",2048, &ucParameterToPass, tskIDLE_PRIORITY, &xHandle_to_PI, 1);   //8192 STACK SIZE IN WORDS 1 W= 4 BYTTES 2048
  configASSERT( xHandle_to_PI );  
if (xHandle_to_PI != NULL) 
{
   // printf("Task handle creation  for PI_Loop_Logic succeeded!\n");
           
}

else if ((xHandle_to_PI == NULL) ){
    
}

}   