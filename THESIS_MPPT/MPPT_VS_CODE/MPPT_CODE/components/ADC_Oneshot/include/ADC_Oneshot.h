#ifndef ADC_ONESHOT_H
#define ADC_ONESHOT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_err.h"

extern volatile double I_PV;
extern volatile int I_PV_RAW;
extern volatile float I_PV_ADC_VOLTS;

void ADC_CurrentOneshot_Init(void);
esp_err_t ADC_ReadCurrentSample(void);

#ifdef __cplusplus
}
#endif

#endif /* ADC_ONESHOT_H */