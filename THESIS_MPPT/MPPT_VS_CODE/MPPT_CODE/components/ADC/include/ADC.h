#pragma once


#define POT_POWER_PIN 12             //ADC 6
#define VOT_IN   32           //ADC 4

extern double IL;  
extern double V_PV ; 
extern double I_PV ;




void Initialise_and_measure_ADC(void);