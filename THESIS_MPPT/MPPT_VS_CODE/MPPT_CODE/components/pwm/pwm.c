#include <stdio.h>
#include "pwm.h"
#include "esp_log.h"
//#include "tasks.h"
 //#include "action_on_motors_logic.h"

#include "driver/mcpwm_oper.h"
#include "driver/mcpwm_timer.h"
#include "driver/mcpwm_cmpr.h"
#include "driver/mcpwm_gen.h"


static const char *PWM_TAG = "PWM_COMPONENT";
//extern int period_ticks_step;
//extern int resolution_step;

//Short period → high frequency, low resolution.
//Long period → low frequency, high resolution

    
int Resolution_hz =  8000000;
int Period_ticks = 50000 ;          //has to be below 65 535 //These also set the resolution we  will deal with 
int Compare_value = 0;   //For setting the duty cyle of the first generator
int Compare_value_2 = 0;  //For setting the duty cyle of the second generator
float Duty_Ratio ; //= 0.0f;
float Duty_Ratio_2; // = 0.0f;
int PWM_Frequency;   //in Hz

//mcpwm_cmpr_handle_t ret_cmpr=NULL;

//Here we create a function that initialises  the resolutyion values at startup to certain vales
//void init_Resolution_and_presiod_ticks_to_step_size_of_each(void){

 //Resolution_hz=  7900000; // resolution_step ;
// Period_ticks= 50000; //period_ticks_step;  
 // step_for_compare_value = 5000; //5k for testing onnly// period_ticks_step /50 ; //This makes us have a #num of steps for the duty cycle control!!
//}


 mcpwm_cmpr_handle_t ret_cmpr=NULL;
mcpwm_cmpr_handle_t ret_cmpr_2=NULL;


void duty_ratio_calculation(void){
    Duty_Ratio= ((float)Compare_value/Period_ticks)*100;
     Duty_Ratio_2= ((float)Compare_value_2/Period_ticks)*100;
    printf("Duty cycle for motor 1 is %0.2f %%\n", Duty_Ratio);
    printf("Duty cycle for motor 2 is %0.2f %%\n", Duty_Ratio_2);
}



void pwm_frequency_calculation(void){
if(Period_ticks > Resolution_hz )   //avoid division by zero error    
{
 printf("Error: Period ticks cannot be greater than resolution, Increase resolution or lower  Period Ticks rather to change frequency\n");
}



if(Period_ticks > 0)   //avoid division by zero error
{
    duty_ratio_calculation();
    
PWM_Frequency = Resolution_hz / Period_ticks ;
 printf("PWM Frequency is now %d Hz \n", PWM_Frequency);}

 else {
    printf("Error: Period ticks cannot be zero, Increase period ticks rather to change frequency\n");
 }
}



void pwm_configuration(void)
{
//CONFIGURATION STRUCTURES FOR MCPWM
//TIMERS configuration structure;

mcpwm_timer_config_t timer_config = {
.group_id = 0,  
.intr_priority=0,
.clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,                       // MCPWM_TIMER_CLK_SRC_PLL160M

.resolution_hz= Resolution_hz,               // Change later to a variableyou can change anywhere                      //BASICALLY SETTING HOWM MANY TIMER TICKS HAPPEN IN A SECOND  1000000
.count_mode= MCPWM_TIMER_COUNT_MODE_UP,
.period_ticks=Period_ticks // 1000,                                      //this sets the  length of each period

};

//Operator configuration structure

mcpwm_operator_config_t operator_config = {

 .group_id=0,
 .intr_priority=0,
 .flags= {

.update_dead_time_on_sync=0,
.update_dead_time_on_tep=0,
.update_dead_time_on_tez=0,
.update_gen_action_on_sync=0,
.update_gen_action_on_tep=0,
.update_gen_action_on_tez=0,
}

};



//Comparator structure

mcpwm_comparator_config_t comparator_config = {

.intr_priority=0,
.flags = {
    .update_cmp_on_sync=0,
    .update_cmp_on_tep=0,
    .update_cmp_on_tez=0,
}
};


mcpwm_comparator_config_t comparator_config_2= {

.intr_priority=0,
.flags = {
    .update_cmp_on_sync=0,
    .update_cmp_on_tep=0,
    .update_cmp_on_tez=0,
}
};


//Generator configuration
mcpwm_generator_config_t generator_config ={

.gen_gpio_num = GEN_PIN,
.flags={
    .invert_pwm=0,
    .io_loop_back=0,
    .io_od_mode=0,
    .pull_down=0, // Set pull-down enabled as sefety measure when no signal is being generated
    .pull_up=0,
}
};


mcpwm_generator_config_t generator_config_2 ={

.gen_gpio_num = GEN_PIN_2,
.flags={
    .invert_pwm=0,
    .io_loop_back=0,
    .io_od_mode=0,
    .pull_down=0, // Set pull-down enabled as sefety measure when no signal is being generated
    .pull_up=0,
}
};


//ALLOCATING THE SUBMODULES EG TIMERS, COMPARATORS ANDF GENERTORS ETC TO MEMEORY WITHT THE CONFIGURATION SETTINGS SETTED ABOVE

mcpwm_timer_handle_t ret_timer=NULL;
 mcpwm_new_timer( &timer_config, &ret_timer);
 mcpwm_timer_enable(ret_timer);
 mcpwm_timer_start_stop(ret_timer, MCPWM_TIMER_START_NO_STOP );

//We shall need two operators for two generators
 mcpwm_oper_handle_t ret_oper=NULL; //Operato handle initialised here coz we need to pass its address to the function as it will be the output of the function
//mcpwm_oper_handle_t ret_oper_2=NULL; 
 mcpwm_new_operator(&operator_config, &ret_oper);
//mcpwm_new_operator(&operator_config, &ret_oper_2);
mcpwm_operator_connect_timer(ret_oper, ret_timer );
//mcpwm_operator_connect_timer(ret_oper_2, ret_timer );


//Now we need to create two comparators for the two generators
// mcpwm_cmpr_handle_t ret_cmpr=NULL;
//mcpwm_cmpr_handle_t ret_cmpr_2=NULL;
 mcpwm_new_comparator(ret_oper, &comparator_config, &ret_cmpr);
mcpwm_new_comparator(ret_oper, &comparator_config_2, &ret_cmpr_2);
 mcpwm_comparator_set_compare_value(ret_cmpr, Compare_value);  // (cmpr_val/period_ticks)*100  500
mcpwm_comparator_set_compare_value(ret_cmpr_2, Compare_value_2);


 //we have here the compare event action structure and the timer event action structure  for both generators
 mcpwm_gen_compare_event_action_t event_action_config={
.direction=MCPWM_TIMER_DIRECTION_UP,
.comparator=ret_cmpr,
.action=MCPWM_GEN_ACTION_LOW,

 };


 mcpwm_gen_compare_event_action_t event_action_config_2={
.direction=MCPWM_TIMER_DIRECTION_UP,
.comparator=ret_cmpr_2,
.action=MCPWM_GEN_ACTION_LOW,

}; 


 //Here we are using one timer event action structure for both generators coz we have one time connected to two generators
mcpwm_gen_timer_event_action_t  timer_action_config={
.direction=MCPWM_TIMER_DIRECTION_UP,
.event= MCPWM_TIMER_EVENT_EMPTY,
.action=MCPWM_GEN_ACTION_HIGH,

 };



mcpwm_gen_handle_t ret_gen=NULL;
mcpwm_gen_handle_t ret_gen_2=NULL;

mcpwm_new_generator(ret_oper, &generator_config, &ret_gen);
mcpwm_new_generator(ret_oper, &generator_config_2, &ret_gen_2);      
mcpwm_generator_set_action_on_timer_event(ret_gen, timer_action_config);
mcpwm_generator_set_action_on_timer_event(ret_gen_2, timer_action_config);
 mcpwm_generator_set_action_on_compare_event(ret_gen,event_action_config );
mcpwm_generator_set_action_on_compare_event(ret_gen_2,event_action_config_2 );
 

//printf("done config pwm");
ESP_LOGI(PWM_TAG,"Finished PWM configuration");
}


void update_compare_value(void) {

    mcpwm_comparator_set_compare_value(ret_cmpr, Compare_value);  //For Changing the Duty Cycle Dynamically In another Components 
    mcpwm_comparator_set_compare_value(ret_cmpr_2, Compare_value_2);

}
