#pragma once


//#define POT_POWER_PIN 12             //ADC 6
//#define VOT_IN   32           //ADC 4

//extern double IL;  
extern volatile double V_PV ; 
extern volatile double V_BOOST ; 
extern volatile double I_PV;




void Initialise_and_measure_ADC(void);