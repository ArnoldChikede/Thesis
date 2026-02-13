#include <stdio.h>
#include <math.h>
#include "MPPT.h"
#include "ADC.h"

 //double V_PV ;   //SHOULD COME FROM THE ADC COMPONENT
// double I_PV ;  //SHOULD COME FROM THE ADC COMPONENT
 double P_PV ;

double MPPT_Ts; //Remeber this is supposed to be set as a Value From How the Task is going to run in FreeRTOS
                //And we Kind of not use it here 
double delta_Iref = 0.01 ; //0.02;
 double Iref = 2.0;
static double P_PV_PREV = 0.0 ;
static double V_PV_PREV = 0.0 ;
 
double delta_P; 
double delta_V ;

double IREF_MAX = 20; 
//int I_PV_Init  ;





void  MPPT(void)
{

P_PV = I_PV * V_PV;

delta_P = P_PV - P_PV_PREV;
delta_V = V_PV - V_PV_PREV;


if (fabs(delta_P) > 0.001)   // we try to put a  meaningful change  so that we dont respond to noise 
{
    if (delta_P > 0)
    {
        
        if (delta_V > 0)
            Iref -= delta_Iref;   //Try to increase Voltage and it reduces current
        else
            Iref += delta_Iref;
    }
    else
    {
        //Reverse  direction
        if (delta_V > 0)
            Iref += delta_Iref;
        else
            Iref -= delta_Iref;
    }
}

// Setting Limits so that we dont saturate our Controller however we implemented Anti winding techniques though
if (Iref < 0.0)       Iref = 0.0;
if (Iref > IREF_MAX)  Iref = IREF_MAX;



// We try to update stuff
P_PV_PREV = P_PV;
V_PV_PREV = V_PV;



//WE CAN MAKE THIS AS A STATE MACHINE TYPE OF SITUATION  and try to adapt it more if we want though 

if (fabs(delta_P) > 0.45){delta_Iref = 0.1; }

else if (fabs(delta_P) > 0.35){delta_Iref = 0.08; }

else if (fabs(delta_P) > 0.3){delta_Iref = 0.06 ; }

else if (fabs(delta_P) > 0.25){delta_Iref = 0.04 ; }

else if (fabs(delta_P) > 0.2){delta_Iref = 0.03 ; }

else if (fabs(delta_P) > 0.15){delta_Iref = 0.02 ; }

else if (fabs(delta_P) > 0.1 ){delta_Iref = 0.01 ; }

else if (fabs(delta_P) > 0.06 ){delta_Iref = 0.005 ; }

else if (fabs(delta_P) > 0.04 ){delta_Iref = 0.004 ; }

else {delta_Iref = 0.0005 ; }
   
}
