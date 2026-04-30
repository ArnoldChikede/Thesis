#pragma once
#include <stdbool.h>

extern double Iref ;   //Here we Advertise the Iref variable to be used in the PI controller component as an external variable
extern double  P_PV ;

void  MPPT(void);

void MPPT_set_iref(double new_iref);
double MPPT_get_iref(void);

void MPPT_set_manual_ref_mode(bool enable);
bool MPPT_get_manual_ref_mode(void);