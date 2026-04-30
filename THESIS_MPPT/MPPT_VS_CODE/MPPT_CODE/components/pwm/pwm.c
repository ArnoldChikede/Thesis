#include <stdio.h>
#include <stdbool.h>

#include "pwm.h"
#include "esp_log.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"

#define HOLD_ON true

static const char *PWM_TAG = "PWM_COMPONENT";

// Short period -> high frequency, low resolution
// Long period  -> low frequency, high resolution

int Resolution_hz = 80000000;
int Period_ticks = 1000;          // must stay within valid MCPWM timer range
int Compare_value = 0;            // duty compare for first generator
int Compare_value_2 = 0;          // duty compare for second generator
float Duty_Ratio;
float Duty_Ratio_2;
int PWM_Frequency;
int control_pwm_signal = 0;

mcpwm_cmpr_handle_t ret_cmpr = NULL;
mcpwm_cmpr_handle_t ret_cmpr_2 = NULL;

mcpwm_gen_handle_t ret_gen = NULL;
mcpwm_gen_handle_t ret_gen_2 = NULL;

void duty_ratio_calculation(void)
{
    Duty_Ratio = ((float)Compare_value / Period_ticks) * 100.0f;
    Duty_Ratio_2 = ((float)Compare_value_2 / Period_ticks) * 100.0f;

    printf("Duty cycle for motor 1 is %0.2f %%\n", Duty_Ratio);
    printf("Duty cycle for motor 2 is %0.2f %%\n", Duty_Ratio_2);
}

void pwm_frequency_calculation(void)
{
    if (Period_ticks > Resolution_hz) {
        printf("Error: Period ticks cannot be greater than resolution, increase resolution or lower Period_ticks\n");
        return;
    }

    if (Period_ticks > 0) {
        duty_ratio_calculation();
        PWM_Frequency = Resolution_hz / Period_ticks;
        printf("PWM Frequency is now %d Hz\n", PWM_Frequency);
    } else {
        printf("Error: Period ticks cannot be zero\n");
    }
}

void pwm_configuration(void)
{
    // TIMER CONFIG
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .intr_priority = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = Resolution_hz,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = Period_ticks
    };

    // OPERATOR CONFIG
    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
        .intr_priority = 0,
        .flags = {
            .update_dead_time_on_sync = 0,
            .update_dead_time_on_tep = 0,
            .update_dead_time_on_tez = 0,
            .update_gen_action_on_sync = 0,
            .update_gen_action_on_tep = 0,
            .update_gen_action_on_tez = 0,
        }
    };

    // COMPARATOR CONFIG
    // update_cmp_on_tez = 1 makes duty updates cleaner at the period boundary
    mcpwm_comparator_config_t comparator_config = {
        .intr_priority = 0,
        .flags = {
            .update_cmp_on_sync = 0,
            .update_cmp_on_tep = 0,
            .update_cmp_on_tez = 1,
        }
    };

    mcpwm_comparator_config_t comparator_config_2 = {
        .intr_priority = 0,
        .flags = {
            .update_cmp_on_sync = 0,
            .update_cmp_on_tep = 0,
            .update_cmp_on_tez = 1,
        }
    };

    // GENERATOR CONFIG
    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = GEN_PIN,
        .flags = {
            .invert_pwm = 0,
            .io_loop_back = 0,
            .io_od_mode = 0,
            .pull_down = 0,
            .pull_up = 0,
        }
    };

    mcpwm_generator_config_t generator_config_2 = {
        .gen_gpio_num = GEN_PIN_2,
        .flags = {
            .invert_pwm = 0,
            .io_loop_back = 0,
            .io_od_mode = 0,
            .pull_down = 0,
            .pull_up = 0,
        }
    };

    // CREATE TIMER
    mcpwm_timer_handle_t ret_timer = NULL;
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &ret_timer));

    // CREATE OPERATOR
    mcpwm_oper_handle_t ret_oper = NULL;
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &ret_oper));

    // CONNECT OPERATOR TO TIMER
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(ret_oper, ret_timer));

    // CREATE COMPARATORS
    ESP_ERROR_CHECK(mcpwm_new_comparator(ret_oper, &comparator_config, &ret_cmpr));
    ESP_ERROR_CHECK(mcpwm_new_comparator(ret_oper, &comparator_config_2, &ret_cmpr_2));

    // Clamp initial values
    if (Compare_value < 0) {
        Compare_value = 0;
    }
    if (Compare_value > Period_ticks) {
        Compare_value = Period_ticks;
    }

    if (Compare_value_2 < 0) {
        Compare_value_2 = 0;
    }
    if (Compare_value_2 > Period_ticks) {
        Compare_value_2 = Period_ticks;
    }

    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(ret_cmpr, Compare_value));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(ret_cmpr_2, Compare_value_2));

    // COMPARE ACTIONS
    mcpwm_gen_compare_event_action_t event_action_config = {
        .direction = MCPWM_TIMER_DIRECTION_UP,
        .comparator = ret_cmpr,
        .action = MCPWM_GEN_ACTION_LOW,
    };

    mcpwm_gen_compare_event_action_t event_action_config_2 = {
        .direction = MCPWM_TIMER_DIRECTION_UP,
        .comparator = ret_cmpr_2,
        .action = MCPWM_GEN_ACTION_LOW,
    };

    // TIMER ACTIONS
    mcpwm_gen_timer_event_action_t timer_action_config = {
        .direction = MCPWM_TIMER_DIRECTION_UP,
        .event = MCPWM_TIMER_EVENT_EMPTY,
        .action = MCPWM_GEN_ACTION_HIGH,
    };

    // CREATE GENERATORS
    ESP_ERROR_CHECK(mcpwm_new_generator(ret_oper, &generator_config, &ret_gen));
    ESP_ERROR_CHECK(mcpwm_new_generator(ret_oper, &generator_config_2, &ret_gen_2));

    // ATTACH ACTIONS
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(ret_gen, timer_action_config));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(ret_gen_2, timer_action_config));

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(ret_gen, event_action_config));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(ret_gen_2, event_action_config_2));

    // Force outputs low until commanded on
    power_on_off_pwm(0);

    // ENABLE + START TIMER LAST
    ESP_ERROR_CHECK(mcpwm_timer_enable(ret_timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(ret_timer, MCPWM_TIMER_START_NO_STOP));

    printf("done config pwm\n");
    ESP_LOGI(PWM_TAG, "Finished PWM configuration");
}

void update_compare_value(int Compare_Value)
{

     int max_compare = (MAX_DUTY_PERCENT * Period_ticks) / 100;
     
    if (Compare_Value < 0) {
        Compare_Value = 0;
    }


    if (Compare_Value > max_compare) {
        Compare_Value = max_compare;
    }

    //if (Compare_Value > Period_ticks) {
    //    Compare_Value = Period_ticks;
   // }

    Compare_value = Compare_Value;

    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(ret_cmpr, Compare_value));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(ret_cmpr_2, Compare_value_2));
}

void power_on_off_pwm(int force_level)
{
    if (force_level == -1) {
        printf("Powering ON the PWM signal by removing force level\n");
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(ret_gen, force_level, HOLD_ON));
        // ESP_ERROR_CHECK(mcpwm_generator_set_force_level(ret_gen_2, force_level, HOLD_ON));
    }

    if (force_level == 0) {
        printf("Powering OFF the PWM signal by forcing generator output LOW\n");
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(ret_gen, force_level, HOLD_ON));
        // ESP_ERROR_CHECK(mcpwm_generator_set_force_level(ret_gen_2, force_level, HOLD_ON));
    }
}














/*
#include <stdio.h>
#include <stdbool.h>

#include "pwm.h"
#include "esp_log.h"
#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"

#include "isr.h"
#include "freertos/semphr.h"

#define HOLD_ON true

static const char *PWM_TAG = "PWM_COMPONENT";

int Resolution_hz = 80000000;
int Period_ticks = 4000;
int Compare_value = 0;
int Compare_value_2 = 0;
float Duty_Ratio;
float Duty_Ratio_2;
int PWM_Frequency;
int control_pwm_signal = 0;

volatile uint32_t pwm_sample_trigger_count = 0;

mcpwm_cmpr_handle_t ret_cmpr = NULL;
mcpwm_cmpr_handle_t ret_cmpr_2 = NULL;

mcpwm_gen_handle_t ret_gen = NULL;
mcpwm_gen_handle_t ret_gen_2 = NULL;






static bool pwm_sample_on_reach_cb(mcpwm_cmpr_handle_t cmpr,
                                   const mcpwm_compare_event_data_t *edata,
                                   void *user_ctx)
{
    BaseType_t high_task_wakeup = pdFALSE;
    static uint32_t pwm_div_counter = 0;

    pwm_sample_trigger_count++;

    pwm_div_counter++;

    if (pwm_div_counter >= 1) {   // 80 kHz / 4 = 20 kHz
        pwm_div_counter = 0;

        if (xSemaphore_control_PI_loop_logic != NULL) {
            xSemaphoreGiveFromISR(xSemaphore_control_PI_loop_logic, &high_task_wakeup);
        }
    }

    return (high_task_wakeup == pdTRUE);
}//////





void duty_ratio_calculation(void)
{
    Duty_Ratio = ((float)Compare_value / Period_ticks) * 100.0f;
    Duty_Ratio_2 = ((float)Compare_value_2 / Period_ticks) * 100.0f;

    printf("Duty cycle for motor 1 is %0.2f %%\n", Duty_Ratio);
    printf("Duty cycle for motor 2 is %0.2f %%\n", Duty_Ratio_2);
}

void pwm_frequency_calculation(void)
{
    if (Period_ticks > Resolution_hz) {
        printf("Error: Period ticks cannot be greater than resolution, increase resolution or lower Period_ticks\n");
        return;
    }

    if (Period_ticks > 0) {
        duty_ratio_calculation();
        PWM_Frequency = Resolution_hz / Period_ticks;
        printf("PWM Frequency is now %d Hz\n", PWM_Frequency);
    } else {
        printf("Error: Period ticks cannot be zero\n");
    }
}

void pwm_configuration(void)
{
    mcpwm_timer_config_t timer_config = {
        .group_id = 0,
        .intr_priority = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = Resolution_hz,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = Period_ticks
    };

    mcpwm_operator_config_t operator_config = {
        .group_id = 0,
        .intr_priority = 0,
        .flags = {
            .update_dead_time_on_sync = 0,
            .update_dead_time_on_tep = 0,
            .update_dead_time_on_tez = 0,
            .update_gen_action_on_sync = 0,
            .update_gen_action_on_tep = 0,
            .update_gen_action_on_tez = 0,
        }
    };

    mcpwm_comparator_config_t comparator_config = {
        .intr_priority = 0,
        .flags = {
            .update_cmp_on_sync = 0,
            .update_cmp_on_tep = 0,
            .update_cmp_on_tez = 1,
        }
    };

    mcpwm_comparator_config_t comparator_config_2 = {
        .intr_priority = 0,
        .flags = {
            .update_cmp_on_sync = 0,
            .update_cmp_on_tep = 0,
            .update_cmp_on_tez = 1,
        }
    };

    mcpwm_generator_config_t generator_config = {
        .gen_gpio_num = GEN_PIN,
        .flags = {
            .invert_pwm = 0,
            .io_loop_back = 0,
            .io_od_mode = 0,
            .pull_down = 0,
            .pull_up = 0,
        }
    };

    mcpwm_generator_config_t generator_config_2 = {
        .gen_gpio_num = GEN_PIN_2,
        .flags = {
            .invert_pwm = 0,
            .io_loop_back = 0,
            .io_od_mode = 0,
            .pull_down = 0,
            .pull_up = 0,
        }
    };

    mcpwm_timer_handle_t ret_timer = NULL;
    ESP_ERROR_CHECK(mcpwm_new_timer(&timer_config, &ret_timer));

    mcpwm_oper_handle_t ret_oper = NULL;
    ESP_ERROR_CHECK(mcpwm_new_operator(&operator_config, &ret_oper));

    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(ret_oper, ret_timer));

    ESP_ERROR_CHECK(mcpwm_new_comparator(ret_oper, &comparator_config, &ret_cmpr));
    ESP_ERROR_CHECK(mcpwm_new_comparator(ret_oper, &comparator_config_2, &ret_cmpr_2));

    mcpwm_comparator_event_callbacks_t sample_cb = {
        .on_reach = pwm_sample_on_reach_cb,
    };

   
    ESP_ERROR_CHECK(mcpwm_comparator_register_event_callbacks(ret_cmpr_2, &sample_cb, NULL));

    if (Compare_value < 0) {
        Compare_value = 0;
    }
    if (Compare_value > Period_ticks) {
        Compare_value = Period_ticks;
    }

    if (Compare_value_2 < 0) {
        Compare_value_2 = 0;
    }
    if (Compare_value_2 > Period_ticks) {
        Compare_value_2 = Period_ticks;
    }

    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(ret_cmpr, Compare_value));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(ret_cmpr_2, Compare_value_2));

    mcpwm_gen_compare_event_action_t event_action_config = {
        .direction = MCPWM_TIMER_DIRECTION_UP,
        .comparator = ret_cmpr,
        .action = MCPWM_GEN_ACTION_LOW,
    };

    mcpwm_gen_timer_event_action_t timer_action_config = {
        .direction = MCPWM_TIMER_DIRECTION_UP,
        .event = MCPWM_TIMER_EVENT_EMPTY,
        .action = MCPWM_GEN_ACTION_HIGH,
    };

    ESP_ERROR_CHECK(mcpwm_new_generator(ret_oper, &generator_config, &ret_gen));
    ESP_ERROR_CHECK(mcpwm_new_generator(ret_oper, &generator_config_2, &ret_gen_2));

    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(ret_gen, timer_action_config));
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(ret_gen, event_action_config));

    power_on_off_pwm(0);

    ESP_ERROR_CHECK(mcpwm_timer_enable(ret_timer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(ret_timer, MCPWM_TIMER_START_NO_STOP));

    printf("done config pwm\n");
    ESP_LOGI(PWM_TAG, "Finished PWM configuration");
}

void update_compare_value(int Compare_Value)
{
    if (Compare_Value < 0) {
        Compare_Value = 0;
    }
    if (Compare_Value > Period_ticks) {
        Compare_Value = Period_ticks;
    }

    Compare_value = Compare_Value;

    Compare_value_2 = Compare_value/2;

    if (Compare_value_2 < 1) {
        Compare_value_2 = 1;
    }
    if (Compare_value_2 >= Period_ticks) {
    Compare_value_2 = Period_ticks - 1;
     }

   
     //Compare_value_2 = Compare_value + 200;


    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(ret_cmpr, Compare_value));
    ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(ret_cmpr_2, Compare_value_2));
}

void power_on_off_pwm(int force_level)
{
    if (force_level == -1) {
        printf("Powering ON the PWM signal by removing force level\n");
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(ret_gen, force_level, HOLD_ON));
    }

    if (force_level == 0) {
        printf("Powering OFF the PWM signal by forcing generator output LOW\n");
        ESP_ERROR_CHECK(mcpwm_generator_set_force_level(ret_gen, force_level, HOLD_ON));
    }
}

*/