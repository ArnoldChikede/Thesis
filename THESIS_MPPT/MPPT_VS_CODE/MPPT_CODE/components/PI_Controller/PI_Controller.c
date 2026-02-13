#include <stdio.h>
#include "PI_Controller.h"
#include "MPPT.h"
#include "ADC.h"
//We might However change to Double 

const double fs=25000.0; //In this case made Const because as scope level all valaues should  be constants 
const double Ts=1.0/fs;
double kp =0.05;
double ki = 200;  

//double Iref = 0.0;  //Would like to have type checling 						//So thats why i dont use Preprocessor Macros eg  #define
//double IL= 0.0;
double Error = 0.0 ; 
double Error_Prev = 0.0 ; 

double Kp_contribution =  0.0 ;
double Ki_contribution = 0.0; 
static double Ki_contribution_prev = 0.0; 

const int Upper_saturation_Limit = 1;
const int Lower_saturation_Limit = 0;

double PI_Controller_Output;
double PI_Controller_Output_Saturation_Block ;


double duty_control_signal = 0.0 ; 

static double Satuaration_block_state_2_input = 0.0;
 

void PI(void)
{
 

Error = Iref - IL ; 

printf("Errror is %0.2f \n", Error);



//CODE FOR THE PI CONTROLLER
//SPLIT IN TWO AND THEN ADD EACH CONTRUBUTION 
//FORWARD EULER APPROXIMATION USED FOR DISCRETIZING THE INTERGRATOR

Kp_contribution = kp * Error ;


Ki_contribution = (Error_Prev)*ki*Ts + Ki_contribution_prev ;  //Always remember the Integral works on the Previos value of the error
//Ki_contribution = (Error-Satuaration_block_state_2_input)*ki*Ts + Ki_contribution ;


PI_Controller_Output = Kp_contribution + Ki_contribution ;

 // INCLUDE SATURAION BLOCK 
 
 if(PI_Controller_Output <  Lower_saturation_Limit) 
 {
	PI_Controller_Output_Saturation_Block  = Lower_saturation_Limit;
 }
 
 
 else if(PI_Controller_Output > Upper_saturation_Limit) 
 {
	 PI_Controller_Output_Saturation_Block  = Upper_saturation_Limit;
 }
  
else { PI_Controller_Output_Saturation_Block = PI_Controller_Output; }

 duty_control_signal =  PI_Controller_Output_Saturation_Block ;
  
   
 Satuaration_block_state_2_input  =  (PI_Controller_Output  - PI_Controller_Output_Saturation_Block)* 1/kp ;

Error_Prev  = Error;
Ki_contribution_prev = Ki_contribution;


printf("Duty cycle control signal is %0.2f \n", duty_control_signal );

}
