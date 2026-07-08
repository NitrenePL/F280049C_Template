#include "Control.h"
float32_t Uout, Iout;
float32_t Duty = 0.05f;
float32_t Set_Uout = 16.0f;
// Timer0 中断服务函数（必须和 SysConfig 里的名字完全一样）
__interrupt void INT_myCPUTIMER0_ISR(void)
{
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

__attribute__((section(".TI.ramfunc"))) __interrupt void INT_EPWMA_ISR(void)
{
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
    EPWM_clearEventTriggerInterruptFlag(EPWMA_BASE);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP3);
}

__interrupt void INT_A0_1_ISR(void)
{
    static float filter = 0;
    static float last_filter = 0;
    filter = 0.01f * ADC_readResult(ADCARESULT_BASE, ADC_SOC_NUMBER0) + 0.99f * last_filter;
    last_filter = filter;
    Uout = filter * 0.01254755f + 0.002526192f;

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

void Show_Data(void)
{
    int32_t integral;
    int32_t fractional;
    sprintf(OLED_1, "MODE:%ld", MODE);
    integral = (int32_t)Uout;
    fractional = (int32_t)((Uout - integral) * 100);
    sprintf(OLED_2, "Uout:%ld.%02ld", integral, (fractional < 0 ? -fractional : fractional));
    integral = (int32_t)Duty;
    fractional = (int32_t)((Duty - integral) * 100);
    sprintf(OLED_3, "Duty:%ld.%02ld", integral, (fractional < 0 ? -fractional : fractional));
    OLED_ShowString(3, 1, (uint8_t *)OLED_1, sizeof(OLED_1));
    OLED_ShowString(3, 2, (uint8_t *)OLED_2, sizeof(OLED_2));
    OLED_ShowString(3, 3, (uint8_t *)OLED_3, sizeof(OLED_3));
    OLED_ShowString(3, 4, (uint8_t *)OLED_4, sizeof(OLED_4));
    OLED_ShowString(3, 5, (uint8_t *)OLED_5, sizeof(OLED_5));
    OLED_ShowString(3, 6, (uint8_t *)OLED_6, sizeof(OLED_6));
}
void Loop(void)
{
    Show_Data();
}
void Setup(void)
{
    period = EPWM_getTimeBasePeriod(EPWM_A_BASE);
    OLED_Init();
    MYSELF_TIMER_Init();
    Keyboard_Init();
    Interrupt_enable(INT_EPWM1);
    Interrupt_enable(INT_ADCA1);
    EPWM_Stop();

    DCL_resetPI(&myPID_Uout); // 复位PI输出函数
}

void MyProtect(void)
{
}
