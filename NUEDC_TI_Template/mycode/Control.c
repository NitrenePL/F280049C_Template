#include "Control.h"

#pragma SET_DATA_SECTION("controlVariables")
// 高速RAM变量放在这里, 例如电流环、滤波器等
float32_t Uout, Iout;
float32_t Duty = 0.05f;
float32_t theta_ref = 0.f; // 参考相位
float32_t Ua_pu, Ub_pu, Uc_pu;

#pragma SET_DATA_SECTION()

#pragma SET_DATA_SECTION("logVariables")
// 低速RAM变量放在这里, 例如RMS数组、状态数据等
float32_t Set_Uout = 16.0f;

uint16_t PWM_PRD = 0; // 假设默认周期值
uint8_t Open = 0;     // 默认关闭
uint8_t MODE = 0;     // 默认模式0
uint8_t LAST_MODE = 0;
#pragma SET_DATA_SECTION()

// Timer0 中断服务函数 慢速任务 1ms周期
__interrupt void INT_myCPUTIMER0_ISR(void)
{
    static uint16_t ledTaskCnt = 0;
    static uint16_t oledTaskCnt = 0;

    if (++oledTaskCnt >= 10U)
    {
        oledTaskCnt = 0U;
        OLED_Display_RequestRefresh();
    }

    if (++ledTaskCnt >= 200U)
    {
        ledTaskCnt = 0U;
        LED_TOGGLE();
    }

    CPUTimer_clearOverflowFlag(CPUTIMER0_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

__attribute__((section(".TI.ramfunc"))) __interrupt void ADC_SamplingISR(void)
{
    /*
    static float filter = 0;
    static float last_filter = 0;

    filter = 0.01f * ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0) + 0.99f * last_filter;
    last_filter = filter;
    Uout = filter * 0.01254755f + 0.002526192f;


    if (MODE == 0)
    {
        EPWM_Stop();
        DCL_resetPI(&myPID_Uout);
    }
    else if (MODE == 1)
    {
        if (LAST_MODE == 0)
        {
            EPWM_Start();
        }
        // Duty = DCL_runPI_C3(&myPID_Uout, Set_Uout, Uout);
        // EPWM_OUTPUTA_duty(Duty);
    }
    LAST_MODE = MODE;
    */

    DCL_runRefgen(&theta_REFGEN);
    theta_ref = DCL_getRefgenPhaseA(&theta_REFGEN);
    Ua_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref);
    Ub_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref - CONST_2PI_32 / 3.f);
    Uc_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref + CONST_2PI_32 / 3.f);

    CB_SVPWM_3Ph(Ua_pu, Ub_pu, Uc_pu); // 385 clks

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
    DCL_resetPI(&myPID_Uout); // 复位PI输出函数

    Keyboard_Init();       // 初始化键盘GPIO
    KEYBOARD_TIMER_Init(); // 初始化键盘CPUTIMER1中断

    EPWM_Stop();
    
    Interrupt_enable(INT_ADCA1);    // 使能主控中断

    OLED_Display_Init();
}

void MyProtect(void)
{
}
