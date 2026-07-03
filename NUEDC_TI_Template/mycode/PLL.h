#ifndef PLL_H_
#define PLL_H_

#include "IIR.h" // 使用之前移植的 IIR 库
#include "device.h"
#include "driverlib.h"
#include "pid.h" // 使用之前移植的 PID 库
#include <math.h>


#ifndef PI
#define PI 3.14159265358979f
#endif

// 采样频率，需与定时器中断频率一致
#define PLL_SAMPLING_FREQ 20000.0f

// 状态 LED 定义 (请根据你的开发板实际引脚修改，例如 LaunchPad 通常是 31 和 34)
#define PLL_LED_1_GPIO 31
#define PLL_LED_2_GPIO 34

// 外部变量声明
extern float theta, w;
extern uint16_t PLL_Locked;

extern float theta_myself, w_myself;
extern uint16_t PLL_Locked_myself;

// 函数声明
void PLL_Init(void);
void PLL_Calc(float input);

void PLL_Init_myself(void);
void PLL_Calc_myself(float input);

#endif
