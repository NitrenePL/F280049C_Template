#ifndef MYEPWM_H
#define MYEPWM_H

#include "device.h"
#include "driverlib.h"
#include "global.h"
#include <math.h>

// 宏定义：方便修改映射关系
#define EPWM_A_BASE EPWM1_BASE
#define EPWM_B_BASE EPWM2_BASE
#define EPWM_C_BASE EPWM3_BASE
#define EPWM_D_BASE EPWM4_BASE
#define EPWM_E_BASE EPWM5_BASE
#define EPWM_F_BASE EPWM6_BASE

// 函数声明
void EPWM_Start(void);
void EPWM_Stop(void);

void EPWM_OUTPUTA_duty(float duty);
void EPWM_OUTPUTB_duty(float duty);
void EPWM_OUTPUTB_phase(float phase);
void EPWM_OUTPUTC_duty(float duty);
void EPWM_OUTPUTC_phase(float phase);
void EPWM_OUTPUTD_duty(float duty);
void EPWM_OUTPUTD_phase(float phase);
void EPWM_OUTPUTE_duty(float duty);
void EPWM_OUTPUTE_phase(float phase);
void EPWM_OUTPUTF_duty(float duty);

void SVPWMmodulation(float Ur, float Udc, float theta);
void SVPWM1(float a, float b);
void SPWM(float D);

#endif
