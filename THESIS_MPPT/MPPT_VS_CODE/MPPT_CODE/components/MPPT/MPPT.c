#include <stdio.h>
#include <math.h>
#include "MPPT.h"


 double V_PV ; 
 double I_PV ;
 double P_PV ;

           //This is the equivalent of the duty delta just neeed to put the delta that be equivalent for a certain step ==0.00108313 dty cycle
double MPPT_Ts; 


double delta_Iref = 0.01 ; //0.02;
static double Iref = 2;
static double P_PV_PREV = 0.0 ;
static double V_PV_PREV = 0.0 ;
 
double delta_P; 
double delta_V ;

double IREF_MAX = 20; 
//int I_PV_Init  ;





void  MPPT(void)
{




    
}
