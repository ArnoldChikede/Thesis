#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gptimer.h"
#include "esp_log.h"
#include "GP_Timers.h"
#include "isr.h"


//GP_Timer for MPPT
gptimer_handle_t gptimer = NULL;
gptimer_config_t timer_config = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
    .direction = GPTIMER_COUNT_UP,      // Counting direction is up
    .resolution_hz = 1 * 1000 * 1000,   // Resolution is 1 MHz, i.e., 1 tick equals 1 microsecond
};




//GP_Timer for PI
gptimer_handle_t gptimer_PI = NULL;
gptimer_config_t timer_config_PI = {
    .clk_src = GPTIMER_CLK_SRC_DEFAULT, // Select the default clock source
    .direction = GPTIMER_COUNT_UP,      // Counting direction is up
    .resolution_hz = 1 * 1000 * 1000,   // Resolution is 1 MHz, i.e., 1 tick equals 1 microsecond
};







//Here we have the callback function for the MPPT loop logic, which will be called when the timer alarm event occurs. In this example, we simply give a semaphore to unblock the MPPT loop task, but in a real application, you can perform more complex operations here.
static bool example_timer_on_alarm_cb_MPPT(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)

{

     xSemaphoreGive( xSemaphore_control_MPPT_loop_logic);
    // General process for handling event callbacks:

    // 1. Retrieve user context data from user_ctx (passed in from gptimer_register_event_callbacks)

    // 2. Get alarm event data from edata, such as edata->count_value

    // 3. Perform user-defined operations

    // 4. Return whether a high-priority task was awakened during the above operations to notify the scheduler to switch tasks

    return false;

}





//Here we have the callback function for the PI loop logic, which will be called when the timer alarm event occurs.
static bool example_timer_on_alarm_cb_PI(gptimer_handle_t timer, const gptimer_alarm_event_data_t *edata, void *user_ctx)

{

     xSemaphoreGive( xSemaphore_control_PI_loop_logic);
    // General process for handling event callbacks:

    

    return false;

}





gptimer_alarm_config_t alarm_config = {

    .reload_count = 0,      // When the alarm event occurs, the timer will automatically reload to 0

    .alarm_count = 1000000, // Set the actual alarm period, since the resolution is 1us, 1000000 represents 1s

//period_seconds = alarm_count / resolution_hz

    .flags.auto_reload_on_alarm = true, // Enable auto-reload function

};



gptimer_alarm_config_t alarm_config_PI = {

    .reload_count = 0,      // When the alarm event occurs, the timer will automatically reload to 0

    .alarm_count = 1000000, // Set the actual alarm period, since the resolution is 1us, 1000000 represents 1s

//period_seconds = alarm_count / resolution_hz

    .flags.auto_reload_on_alarm = true, // Enable auto-reload function

};










gptimer_event_callbacks_t cb_MPPTs = {

    .on_alarm = example_timer_on_alarm_cb_MPPT, // Call the user callback function when the alarm event occurs

};


gptimer_event_callbacks_t cb_PI = {

    .on_alarm = example_timer_on_alarm_cb_PI, // Call the user callback function when the alarm event occurs

};







void intialise_and_start_gptimer(void)
{

    printf("Initialising and Starting GPTimerS \n");
    // Create a timer instance
    ESP_ERROR_CHECK(gptimer_new_timer(&timer_config, &gptimer));
     ESP_ERROR_CHECK(gptimer_new_timer(&timer_config_PI, &gptimer_PI));

    // Set the timer's alarm action
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer, &alarm_config));
    ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer_PI, &alarm_config_PI));

    // Register timer event callback functions, allowing user context to be carried
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer, &cb_MPPTs, NULL));
    ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer_PI, &cb_PI, NULL));


    // Enable the timer
    ESP_ERROR_CHECK(gptimer_enable(gptimer));
    ESP_ERROR_CHECK(gptimer_enable(gptimer_PI));

    // Start the timer
    ESP_ERROR_CHECK(gptimer_start(gptimer));
    ESP_ERROR_CHECK(gptimer_start(gptimer_PI));
}