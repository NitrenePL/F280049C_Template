#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#define ISR_FREQ 20000.f

#define RAMFUNC __attribute__((section(".TI.ramfunc")))

#define LED5_GPIO 23U
#define LED_TOGGLE() GPIO_togglePin(LED5_GPIO)

#include "device.h"
#include "driverlib.h"
#include <stdint.h>
#include "c2000ware_libraries.h"



#ifdef __cplusplus
extern "C"
{
#endif
    extern uint16_t PWM_PRD;
    extern uint8_t Open; // 开波标志：1-开，0-关
    extern uint8_t MODE; // 模式变量
    extern uint8_t LAST_MODE;

    extern float32_t Duty; // 占空比
    extern float32_t Set_Uout;
    extern float32_t Uout;
    extern float32_t Iout;
    extern float32_t theta_ref;
    extern float32_t Ua_pu;
    extern float32_t Ub_pu;
    extern float32_t Uc_pu;
    extern float32_t error;
    extern float32_t output;

    
#ifdef __cplusplus
}
#endif

#endif /* GLOBAL_VARS_H */
