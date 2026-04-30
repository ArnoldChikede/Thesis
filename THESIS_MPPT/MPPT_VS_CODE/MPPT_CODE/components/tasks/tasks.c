#include <stdio.h>
#include "tasks.h"
#include "Run_PI_Controller.h"
#include "isr.h"
#include "freertos/idf_additions.h"



#include "MPPT.h"
#include "Led.h"
#include "pwm.h"
#include "Protection_Features.h"
#include "ADC.h"

//#include "ADC_Oneshot.h"
#include "freertos/semphr.h"
#include "esp_timer.h"




//static TaskHandle_t xHandle_to_change_resolution_up = NULL;  // making xHandle for task to be Global

static TaskHandle_t xHandle_to_MPPT= NULL;
static TaskHandle_t xHandle_to_PI= NULL;
static TaskHandle_t xHandle_to_LOG = NULL;

volatile uint32_t pi_loop_execution_count = 0;


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
    printf(" Entering loop for running MPPT loop logic\n");
    pwm_frequency_calculation();
    
    while (1)
    {
        // Wait for the semaphore (blocks task, frees CPU)
        //printf("MPPT waiting...\n");
        if (xSemaphoreTake(xSemaphore_control_MPPT_loop_logic, portMAX_DELAY) == pdTRUE)
        {
           // printf("running MPPT loop logic\n");


               if (Protection_FaultActive()) {
                continue;
                }

                  if (control_mode != CONTROL_MODE_AUTOMATIC) {
                   continue;
             }


            MPPT();                       // Run MPPT algorithm

           //duty_ratio_calculation();
           //pwm_frequency_calculation();
          // Update_Parameters(&mppt_params);  //  RainMaker cloud update  ...WE CAN RATHER Have  this as a separate tasks with a separate time and semaphore structure coz when we start running this at the speed of teh mppt then its gonna be so fast
                                          // that we will deplet the MQTT budget that we have from the rainmaker  cloud . 

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
    //   printf("PI task started and waiting for semaphore\n");
    while (1)
    {
        // Wait for the semaphore (blocks task, frees CPU)
        if (xSemaphoreTake(xSemaphore_control_PI_loop_logic, portMAX_DELAY) == pdTRUE)
        {
           
      

            pi_loop_execution_count++;
            // printf("running PI loop logic\n");
           Protection_Check(V_BOOST , I_PV); // Check for faults before running PI control loop



            if (Protection_FaultActive()) {     
             continue;
                 }



             if (control_mode != CONTROL_MODE_AUTOMATIC) {
                       continue;
                }

                 
            PI_control();  // Run PI control loop

            // Optional: if you want, you can rate-limit or conditionally update cloud params here
            // e.g., only if duty cycle changed significantly
        }

        // Optional tiny delay to allow other lower-priority tasks to run
        //vTaskDelay(pdMS_TO_TICKS(1));
    }
}




void task_to_Log_System_State(void *pvParameters)
{
    if (xSemaphore_control_LOG_loop == NULL)
    {
        printf("xSemaphore_control_LOG_loop creation failure\n");
        vTaskDelete(NULL);
        return;
    }

     static uint32_t last_trigger_count = 0;
    static uint32_t last_pi_count = 0;


    while (1)
    {
        if (xSemaphoreTake(xSemaphore_control_LOG_loop, portMAX_DELAY) == pdTRUE)
        {
         printf("LOG | V_PV=%.3f V | V_BOOST=%.3f V | I_PV=%.3f A | Iref=%.3f A | ManualRef=%d\n",
          V_PV, V_BOOST, I_PV, MPPT_get_iref(), MPPT_get_manual_ref_mode());
        printf("Duty Cycle is %.6f\n", duty_control_signal);
         /*/uint32_t trigger_now = pwm_sample_trigger_count;
            uint32_t pi_now = pi_loop_execution_count;

            printf("I = %f | PWM trig/s = %lu | PI exec/s = %lu\n",
                   I_PV,
                   (unsigned long)(trigger_now - last_trigger_count),
                   (unsigned long)(pi_now - last_pi_count));

            last_trigger_count = trigger_now;
            last_pi_count = pi_now;*/





        }
    }
}




//void task_to_Test_ADC_Speed(void *pvParameters)
//{
  //  const int N = 5000;
   // int64_t t0, t1;

    //printf("Starting ADC speed test...\n");

   // t0 = esp_timer_get_time();

   // for (int i = 0; i < N; i++)
    //{
        //if (ADC_ReadCurrentSample() != ESP_OK)
        //{
          //  printf("ADC read failed at sample %d\n", i);
         //   vTaskDelete(NULL);
          //  return;
       // }
    //}

  //  t1 = esp_timer_get_time();

   // double total_us = (double)(t1 - t0);
    //double avg_us = total_us / N;
    //double rate_hz = 1000000.0 / avg_us;

    //printf("ADC speed test done\n");
    //printf("Total time: %.2f us\n", total_us);
   // printf("Average per sample: %.3f us\n", avg_us);
   // printf("Estimated sample rate: %.1f Hz\n", rate_hz);

   // vTaskDelete(NULL);
//}














void create_task_MPPT_loop_logic( void )
{   
static uint8_t ucParameterToPass;
  xTaskCreatePinnedToCore( task_to_Calculate_MPPT, "PERIOD_TICKS_DOWN TASK",2048, &ucParameterToPass, tskIDLE_PRIORITY+2  , &xHandle_to_MPPT, 1);   //8192 STACK SIZE IN WORDS 1 W= 4 BYTTES 2048
  configASSERT( xHandle_to_MPPT );  
if (xHandle_to_MPPT != NULL) 
{
    printf("Task handle creation  for MPPT_Loop_Logic succeeded!\n");
           
}

else if ((xHandle_to_MPPT == NULL) ){
    
}

}   



void create_task_PI_loop_logic( void )
{   //changestack to 4096
static uint8_t ucParameterToPass;
  xTaskCreatePinnedToCore( task_to_Calculate_PI, "PI TASK",2048, &ucParameterToPass, tskIDLE_PRIORITY+2 , &xHandle_to_PI, 1);   //8192 STACK SIZE IN WORDS 1 W= 4 BYTTES 2048
  configASSERT( xHandle_to_PI );  
if (xHandle_to_PI != NULL) 
{
   // printf("Task handle creation  for PI_Loop_Logic succeeded!\n");
           
}

else if ((xHandle_to_PI == NULL) ){
    
}

}   


void create_task_LOG_loop(void)
{
    static uint8_t ucParameterToPass;
    xTaskCreatePinnedToCore(task_to_Log_System_State,
                            "LOG TASK",
                            4096,
                            &ucParameterToPass,
                            tskIDLE_PRIORITY + 1,
                            &xHandle_to_LOG,
                            1);

    configASSERT(xHandle_to_LOG);

    if (xHandle_to_LOG != NULL)
    {
        printf("Task handle creation for LOG_TASK succeeded!\n");
    }
}


//void create_task_ADC_speed_test(void)
//{
   // xTaskCreatePinnedToCore(task_to_Test_ADC_Speed,
                      //      "ADC_SPEED_TEST",
                      //      4096,
                      //      NULL,
                      //      tskIDLE_PRIORITY + 2,
                 //           NULL,
                 //           1);
//}