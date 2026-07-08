#include "Control.h"

#pragma SET_DATA_SECTION("controlVariables")
// 高速RAM变量放在这里, 例如电流环、滤波器等
float32_t Uout, Iout;
float32_t Duty = 0.05f;
float32_t theta_ref = 0.f;  // 参考相位
float32_t Ua_pu, Ub_pu, Uc_pu;

#pragma SET_DATA_SECTION()

#pragma SET_DATA_SECTION("logVariables")
// 低速RAM变量放在这里, 例如RMS数组、状态数据等
float32_t Set_Uout = 16.0f;

uint16_t PWM_PRD = 0; // 假设默认周期值
uint8_t Open = 0;     // 默认关闭
uint8_t MODE = 0;     // 默认模式0
uint8_t LAST_MODE = 0;

char OLED_1[20];
char OLED_2[20];
char OLED_3[20];
char OLED_4[20];
char OLED_5[20];
char OLED_6[20];

#pragma SET_DATA_SECTION()

// Timer0 中断服务函数 慢速任务 1ms周期
__interrupt void INT_myCPUTIMER0_ISR(void)
{
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
    Ub_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref - CONST_2PI_32/3.f);
    Uc_pu = 0.7f * __cos(CONST_2PI_32 * theta_ref + CONST_2PI_32/3.f);

    CB_SVPWM_3Ph(Ua_pu, Ub_pu, Uc_pu);      // 385 clks

    ADC_clearInterruptStatus(ADCA_BASE, ADC_INT_NUMBER1);
    Interrupt_clearACKGroup(INTERRUPT_ACK_GROUP1);
}

void Show_Data(void)
{
    static int32_t integral;
    static int32_t fractional;
    sprintf(OLED_1, "MODE:%d", MODE);
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
    // Show_Data();
}
void Setup(void)
{
    PWM_PRD = EPWM_getTimeBasePeriod(EPWM_A_BASE);
    // OLED_Init();
    // MYSELF_TIMER_Init();
    Keyboard_Init();
    Interrupt_enable(INT_ADCA1);
    EPWM_Stop();

    DCL_resetPI(&myPID_Uout); // 复位PI输出函数
}

void MyProtect(void)
{
}
