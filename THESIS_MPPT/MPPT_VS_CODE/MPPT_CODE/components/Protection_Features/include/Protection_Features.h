#ifndef PROTECTION_FEATURES_H
#define PROTECTION_FEATURES_H

#include <stdbool.h>

/* Types of faults */
typedef enum
{
    FAULT_NONE = 0,
    FAULT_OVERVOLTAGE,
    FAULT_OVERCURRENT
} fault_type_t;

/* Protection settings */
typedef struct
{
    float overvoltage_limit;      // Trip threshold for output voltage
    float overcurrent_limit;      // Trip threshold for input current

    float voltage_recovery_limit; // Voltage must go below this before recovery
    float current_recovery_limit; // Current must go below this before recovery

    bool latch_fault;             // true = stay off until reset
} protection_config_t;

/* Protection status */
typedef struct
{
    fault_type_t fault;
    bool pwm_enabled;
    bool fault_active;
} protection_status_t;

/* Functions */
void Protection_Init(const protection_config_t *config);
void Protection_Check(float output_voltage, float input_current);
void Protection_Reset(void);

bool Protection_FaultActive(void);
fault_type_t Protection_GetFault(void);
protection_status_t Protection_GetStatus(void);

#endif