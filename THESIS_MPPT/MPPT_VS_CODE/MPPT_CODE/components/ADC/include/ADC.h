#ifndef ADC_H
#define ADC_H

void Initialise_and_measure_ADC(void);

void ADC_MarkPwmPeriodBoundary(void);
void ADC_MarkPwmPeriodBoundaryFromISR(void);

extern volatile double V_PV;
extern volatile double V_BOOST;
extern volatile double I_PV;

#endif