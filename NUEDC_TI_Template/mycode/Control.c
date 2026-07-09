#include "Control.h"
#include "DCLF32.h"
#include "KeyBoard.h"
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
float32_t Uout, Iout;
float32_t Duty = 0.05f;
float32_t theta_ref = 0.f; // 参考相位
float32_t Ua_pu, Ub_pu, Uc_pu;
float32_t error, output;
float32_t Uab_inst, Ucb_inst; // 线电压瞬时值

#pragma SET_DATA_SECTION()

#pragma SET_DATA_SECTION("logVariables")
// 低速RAM变量放在这里, 例如RMS数组、状态数据等
float32_t Set_Uout = 16.0f;

uint16_t PWM_PRD = 0; // 假设默认周期值
uint8_t Open = 0;     // 默认关闭
uint8_t MODE = 0;     // 默认模式0
uint8_t LAST_MODE = 0;

RMS_Obj Uan_RMS;
float32_t Uan_rms;

#pragma SET_DATA_SECTION()

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

/**
 * @brief ADC转换完成中断中的实时控制环。
 * @note 关键步骤耗时@100MHz: ADC读数40clks, Refgen360clks, DF22 37clks, PhaseA 24clks, PLL 650clks, SVPWM 332clks。
 */
RAMFUNC __interrupt void ADC_SamplingISR(void)
{
    Uab_inst = 3.3f / 4096.f * ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0); // 40 clks

    DCL_runRefgen(&theta_REFGEN); // 360 clks

    output = DCL_runDF22_C1(&QPR_Ctrl1, error); // 37 clks

    theta_ref = DCL_getRefgenPhaseA(&theta_REFGEN); // 24 clks
    Ua_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref);
    Ub_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref - CONST_2PI_32 / 3.f);
    Uc_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref + CONST_2PI_32 / 3.f);

    // PLL_Calc(Uab_inst); // 650 clks

    Uan_rms = RMS_Calc(&Uan_RMS, Uab_inst); 

    CB_SVPWM_3Ph(Ua_pu, Ub_pu, Uc_pu); // 332 clks

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

void Loop(void)
{
    OLED_Display_Task(); // 10ms刷新周期 由CPUTIMER0调度
}
void Setup(void)
{
    PWM_PRD = EPWM_getTimeBasePeriod(EPWM_A_BASE);
    EPWM_Stop();

    computeDF22_PRcontrollerCoeff(&QPR_Ctrl1, 3.f, 40.f, CONST_2PI_32 * 50.f, ISR_FREQ, CONST_2PI_32 * 2.f);
    computeDF22_PRcontrollerCoeff(&QPR_Ctrl2, 3.f, 40.f, CONST_2PI_32 * 50.f, ISR_FREQ, CONST_2PI_32 * 2.f);
    computeDF22_PRcontrollerCoeff(&QPR_Ctrl3, 3.f, 40.f, CONST_2PI_32 * 50.f, ISR_FREQ, CONST_2PI_32 * 2.f);

    // 控制器初始化
    DCL_resetPI(&myPID_Uout);
    DCL_resetDF22(&QPR_Ctrl1);
    DCL_resetDF22(&QPR_Ctrl2);
    DCL_resetDF22(&QPR_Ctrl3);

    // 锁相环初始化
    PLL_Init();

    // RMS 有效值计算初始化
    RMS_Init(&Uan_RMS);

    Keyboard_Init();       // 初始化键盘GPIO
    KEYBOARD_TIMER_Init(); // 初始化键盘CPUTIMER1中断

    Interrupt_enable(INT_ADCA1); // 使能主控中断

    OLED_Display_Init();
}

void MyProtect(void)
{
}
