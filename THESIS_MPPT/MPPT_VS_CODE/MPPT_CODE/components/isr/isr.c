#include <stdio.h>
#include "isr.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp_intr_types.h"
#include "driver/gpio.h"





SemaphoreHandle_t xSemaphore_control_MPPT_loop_logic = NULL;  //declaring semaphore handle =NULL so its at a defined state
SemaphoreHandle_t xSemaphore_control_PI_loop_logic = NULL;  //declaring semaphore handle =NULL so its at a defined state



// ISR: very lightweight and short High Priority Function, just signals the task
//ISR can be triggered from Hardware and software as well 
// Inside an ISR, you’re only allowed to call a small set of ISR-safe FreeRTOS functions those ending with FromISR



//Right here is where we can add abutton feture so that once we  press it we can  both the MPPT AND Pi controller to strt working 
void create_semaphores_isr(void)
{
    xSemaphore_control_MPPT_loop_logic = xSemaphoreCreateBinary();  //creating binary semaphore
    xSemaphore_control_PI_loop_logic = xSemaphoreCreateBinary();  //creating binary semaphore
     
    if( (xSemaphore_control_MPPT_loop_logic != NULL)  )
      {
          printf("xSemaphore(s) MPPT successfully\n");
      }

        else
        {
            printf("xSemaphore(s) MPPT creation failure\n");
        }


if( (xSemaphore_control_PI_loop_logic != NULL)  )
      {
          printf("xSemaphore(s) PI successfully\n");
      }

        else
        {
            printf("xSemaphore(s) PI creation failure\n");
        }




}





// Resolution control isr
/*
 void isr_handler_MPPT_loop_logic(void* arg)
  {                                //At first we made the function static but its beig called in main so we had to make it global  so that it has external linkage 
    BaseType_t xHigherPriorityTaskWoken ; 
    xSemaphoreGiveFromISR( xSemaphore_test_code, &xHigherPriorityTaskWoken);
}




void initialise_interrupts(void){
//configuration of the GPIO pins for the Interrupts
//For now we will have it For Testing and Debugging PURPOSES 
//Here ist even Driven Interrupts but in the real world we will be using the GPTimer for the MPPT implementation and we will have timer driven interrupts
//We shall leave one GPIO  intururpt for testing purposes but we shall use the GPTimer for real world implementaiion of MPPT 
gpio_reset_pin(test_code);

gpio_set_direction(test_code, GPIO_MODE_INPUT);

gpio_set_pull_mode(test_code, GPIO_PULLDOWN_ONLY);  //mantaining the Pin at a defined level instaed of floating it for when we have no signal(Important Concept dealing with Digital Devices)


//GPIO_INTR_HIGH_LEVEL
gpio_set_intr_type(test_code, GPIO_INTR_POSEDGE ); 

// The handler service that takes care of per pin per pin  handlers 
//We only need done of these in  but we have to add  different handlers for different cases of interrupts
gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1 );  

//hooking up the different handlers for different pins
gpio_isr_handler_add(test_code, isr_handler_test_code , (void*)test_code);   //first Intterupt calling 

}    */