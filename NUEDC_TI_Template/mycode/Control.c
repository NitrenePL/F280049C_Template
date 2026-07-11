#include "Control.h"
#include "DCLF32.h"
#include "FFT_APP.h"
#include "KeyBoard.h"
#include "LPF_COEFF.h"
#include "OLED_Display.h"
#include "PLL.h"
#include "QPR.h"
#include "RMS.h"
#include "global.h"
#include "myEpwm.h"

// OLED屏幕刷新率, 当前15Hz
#define OLED_REFRESH_RATE_HZ 15U
#define SLOW_TASK_RATE_HZ    1000U

#pragma SET_DATA_SECTION("controlVariables")
// 高速RAM变量放在这里, 例如电流环、滤波器等

float32_t UF_inst, ILoad_inst, IF_inst; // 电网电压, 负载电流, APF电流

float32_t Duty = 0.05f; // SPWM调制用占空比

float32_t theta_ref;

float32_t error, output, error2, output2;

#pragma SET_DATA_SECTION()

#pragma SET_DATA_SECTION("logVariables")
// 低速RAM变量放在这里, 例如RMS数组、状态数据等

uint16_t PWM_PRD = 0;
uint8_t Open = 0; // 默认关闭
uint8_t MODE = 0; // 默认模式0
uint8_t LAST_MODE = 0;

RMS_Obj Uan_RMS;
float32_t Uan_rms;

#pragma SET_DATA_SECTION()

static inline RAMFUNC void MyProtect(void)
{
}

static inline RAMFUNC void ADC_DataProcess(void)
{
    float32_t UF_inst_H, UF_inst_L, ILoad_inst_H, ILoad_inst_L, IF_inst_H, IF_inst_L;

    UF_inst_H = (float32_t)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0); // 40 clks
    UF_inst_L = (float32_t)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER1);

    ILoad_inst_H = (float32_t)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER2);
    ILoad_inst_L = (float32_t)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER3);

    IF_inst_H = (float32_t)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER4);
    IF_inst_L = (float32_t)ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER5);

    UF_inst = UF_inst_H - UF_inst_L;
    ILoad_inst = ILoad_inst_H - ILoad_inst_L;
    IF_inst = IF_inst_H - IF_inst_L;

    MyProtect();
}

// Timer0 中断服务函数 慢速任务 1kHz
__interrupt void INT_myCPUTIMER0_ISR(void)
{
    static uint16_t ledTaskCnt = 0;
    static uint16_t oledRefreshAcc = 0;

    oledRefreshAcc = (uint16_t)(oledRefreshAcc + OLED_REFRESH_RATE_HZ);
    if (oledRefreshAcc >= SLOW_TASK_RATE_HZ)
    {
        oledRefreshAcc = (uint16_t)(oledRefreshAcc - SLOW_TASK_RATE_HZ);
        OLED_Display_RequestRefresh();
    }

    if (++ledTaskCnt >= 400U)
    {
        ledTaskCnt = 0U;
        LED_TOGGLE();
    }

    CPUTimer_clearOverflowFlag(CPUTIMER0_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

// ADC_INT 中断服务函数 主控中断 50kHz
RAMFUNC __interrupt void ADC_SamplingISR(void)
{
    ADC_DataProcess();

    DCL_runRefgen(&theta_REFGEN);                   // 360 clks
    theta_ref = DCL_getRefgenPhaseA(&theta_REFGEN); // 24 clks

    UF_inst = 20.f * __cos(CONST_2PI_32 * theta_ref) + 3.f * __cos(3.f * CONST_2PI_32 * theta_ref - CONST_PI_32 / 6.f);
    UF_inst += 2.f * __cos(5.f * CONST_2PI_32 * theta_ref - CONST_PI_32 / 6.f);

    DCL_fwriteLog(&myLOGGER0, UF_inst);

    FFT_APP_SamplingTask(UF_inst);

    // output = DCL_runDF22_C1(&QPR_Ctrl1, error); // 37 clks

    // output2 = DCL_runDF11_C1(&Zv_LPF1, error2);

    // Ua_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref);
    // Ub_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref - CONST_2PI_32 / 3.f);
    // Uc_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref + CONST_2PI_32 / 3.f);

    // PLL_Calc(Uab_inst); // 650 clks

    // Uan_rms = RMS_Calc(&Uan_RMS, Uab_inst);

    // CB_SVPWM_3Ph(Ua_pu, Ub_pu, Uc_pu); // 332 clks

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

void Loop(void)
{
    FFT_APP_ProcessTask();
    OLED_Display_Task(); // 10ms刷新周期 由CPUTIMER0调度
}
void Setup(void)
{
    PWM_PRD = EPWM_getTimeBasePeriod(EPWM_A_BASE);
    EPWM_Stop();

    // 控制器参数计算&初始化
    computeDF22_PRcontrollerCoeff(&QPR_Ctrl1, 3.f, 40.f, CONST_2PI_32 * 50.f, ISR_FREQ, CONST_2PI_32 * 2.f);
    computeDF22_PRcontrollerCoeff(&QPR_Ctrl2, 3.f, 40.f, CONST_2PI_32 * 50.f, ISR_FREQ, CONST_2PI_32 * 2.f);
    computeDF22_PRcontrollerCoeff(&QPR_Ctrl3, 3.f, 40.f, CONST_2PI_32 * 50.f, ISR_FREQ, CONST_2PI_32 * 2.f);

    computeDF11_LPFCoeff(&Zv_LPF1, 0.001f, 0.01f, ISR_FREQ);
    computeDF11_LPFCoeff(&Zv_LPF2, 0.001f, 0.01f, ISR_FREQ);

    DCL_resetPI(&myPID_Uout);

    // 锁相环初始化
    PLL_Init();

    // RMS 有效值计算初始化
    RMS_Init(&Uan_RMS);

    Keyboard_Init();       // 初始化键盘GPIO
    KEYBOARD_TIMER_Init(); // 初始化键盘CPUTIMER1中断

    Interrupt_enable(INT_ADCA1); // 使能主控中断

    OLED_Display_Init();
}
