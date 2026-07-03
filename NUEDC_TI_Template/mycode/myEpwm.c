#include "myEpwm.h"

// period 为 PWM 周期计数器值 (TBPRD)
// 可以在 main 中通过 EPWM_getTimeBasePeriod(EPWM_A_BASE) 获取

void EPWM_Start(void)
{
    EPWM_clearTripZoneFlag(EPWM_A_BASE, EPWM_TZ_FLAG_OST);
}

void EPWM_Stop(void)
{
    EPWM_forceTripZoneEvent(EPWM_A_BASE, EPWM_TZ_FORCE_EVENT_OST);
}

// 通用占空比设置函数 (针对增减计数模式/中心对齐)
static inline void set_epwm_duty(uint32_t base, float duty)
{
    if (duty > 0.99f)
        duty = 0.99f;
    if (duty < 0.01f)
        duty = 0.01f;

    // CMPA = (1 - D) * TBPRD
    uint16_t cmp_val = (uint16_t)(duty * (float)period);
    EPWM_setCounterCompareValue(base, EPWM_COUNTER_COMPARE_A, cmp_val);
}

// 通用相位设置函数
static inline void set_epwm_phase(uint32_t base, float phase_deg)
{
    if (phase_deg <= 0.0f)
        phase_deg = 0.0f;
    else if (phase_deg >= 360.0f)
        phase_deg = 360.0f;

    // 计算相位计数值
    uint16_t phs_val = (uint16_t)((phase_deg / 360.0f) * (float)period);
    EPWM_setPhaseShift(base, phs_val);
}

void EPWM_OUTPUTA_duty(float duty)
{
    set_epwm_duty(EPWM_A_BASE, duty);
}
void EPWM_OUTPUTB_duty(float duty)
{
    set_epwm_duty(EPWM_B_BASE, duty);
}
void EPWM_OUTPUTC_duty(float duty)
{
    set_epwm_duty(EPWM_C_BASE, duty);
}
void EPWM_OUTPUTD_duty(float duty)
{
    set_epwm_duty(EPWM_D_BASE, duty);
}
void EPWM_OUTPUTE_duty(float duty)
{
    set_epwm_duty(EPWM_E_BASE, duty);
}
void EPWM_OUTPUTF_duty(float duty)
{
    set_epwm_duty(EPWM_F_BASE, duty);
}

// 注意：C2000 的相位控制通常需要开启 PHSEN (移相使能)
void EPWM_OUTPUTB_phase(float phase)
{
    set_epwm_phase(EPWM_B_BASE, phase);
}
void EPWM_OUTPUTC_phase(float phase)
{
    set_epwm_phase(EPWM_C_BASE, phase);
}
void EPWM_OUTPUTD_phase(float phase)
{
    set_epwm_phase(EPWM_D_BASE, phase);
}
void EPWM_OUTPUTE_phase(float phase)
{
    set_epwm_phase(EPWM_E_BASE, phase);
}

void SPWM(float D)
{
    if (D >= 0.99f)
        D = 0.99f;
    else if (D <= -0.99f)
        D = -0.99f;

    EPWM_OUTPUTA_duty(0.5f + 0.5f * D);
    EPWM_OUTPUTB_duty(0.5f - 0.5f * D);
}

// -------------------------------------------------------------------------
// SVPWM 调制
// -------------------------------------------------------------------------
void SVPWMmodulation(float Ur, float Udc, float theta)
{
    float Tsv1, Tsv2, Tsv7;
    float Da, Db, Dc;
    const float sin_pi_3 = 0.8660254f; // sin(PI/3)

    // 角度归一化
    theta = fmodf(theta, 2.0f * 3.14159265f);
    if (theta < 0.0f)
        theta += 2.0f * 3.14159265f;

    // 扇区判断与时长计算 (保持原有逻辑)
    int sector = (int)(theta / (3.14159265f / 3.0f));
    float theta_rel = fmodf(theta, 3.14159265f / 3.0f);

    Tsv1 = 1.5f * Ur * sinf(3.14159265f / 3.0f - theta_rel) / (Udc * sin_pi_3);
    Tsv2 = 1.5f * Ur * sinf(theta_rel) / (Udc * sin_pi_3);
    Tsv7 = 0.5f * (1.0f - Tsv1 - Tsv2);

    switch (sector)
    {
        case 0:
            Da = Tsv7 + Tsv1 + Tsv2;
            Db = Tsv7 + Tsv2;
            Dc = Tsv7;
            break;
        case 1:
            Da = Tsv7 + Tsv1;
            Db = Tsv7 + Tsv1 + Tsv2;
            Dc = Tsv7;
            break;
        case 2:
            Da = Tsv7;
            Db = Tsv7 + Tsv1 + Tsv2;
            Dc = Tsv7 + Tsv2;
            break;
        case 3:
            Da = Tsv7;
            Db = Tsv7 + Tsv1;
            Dc = Tsv7 + Tsv1 + Tsv2;
            break;
        case 4:
            Da = Tsv7 + Tsv2;
            Db = Tsv7;
            Dc = Tsv7 + Tsv1 + Tsv2;
            break;
        case 5:
            Da = Tsv7 + Tsv1 + Tsv2;
            Db = Tsv7;
            Dc = Tsv7 + Tsv1;
            break;
        default:
            return;
    }

    EPWM_OUTPUTA_duty(Da);
    EPWM_OUTPUTB_duty(Db);
    EPWM_OUTPUTC_duty(Dc);
}

// -------------------------------------------------------------------------
// 基于 Alpha-Beta 的 SVPWM (七段式)
// -------------------------------------------------------------------------
void SVPWM1(float a, float b)
{
    int A, B, C, sec;
    float X, Y, Z;
    float k1, k2, k3, t0, t1, t2;

    X = b;
    Y = 0.8660254f * a - 0.5f * b;
    Z = -0.8660254f * a - 0.5f * b;

    A = (X > 0) ? 1 : 0;
    B = (Y > 0) ? 1 : 0;
    C = (Z > 0) ? 1 : 0;
    sec = 4 * C + 2 * B + A;

    switch (sec)
    {
        case 3: // Sector 1
            t1 = Y;
            t2 = X;
            t0 = 1.0f - t1 - t2;
            k1 = t1 + t2 + t0 * 0.5f;
            k2 = t2 + t0 * 0.5f;
            k3 = t0 * 0.5f;
            break;
        case 1: // Sector 2
            t1 = -Y;
            t2 = -Z;
            t0 = 1.0f - t1 - t2;
            k1 = t2 + t0 * 0.5f;
            k2 = t1 + t2 + 0.5f * t0;
            k3 = t0 * 0.5f;
            break;
        case 5: // Sector 3
            t1 = X;
            t2 = Z;
            t0 = 1.0f - t1 - t2;
            k1 = t0 * 0.5f;
            k2 = t1 + t2 + t0 * 0.5f;
            k3 = t2 + 0.5f * t0;
            break;
        case 4: // Sector 4
            t1 = -X;
            t2 = -Y;
            t0 = 1.0f - t1 - t2;
            k1 = t0 * 0.5f;
            k2 = t2 + t0 * 0.5f;
            k3 = t1 + t2 + 0.5f * t0;
            break;
        case 6: // Sector 5
            t1 = Z;
            t2 = Y;
            t0 = 1.0f - t1 - t2;
            k1 = t2 + t0 * 0.5f;
            k2 = t0 * 0.5f;
            k3 = t1 + t2 + 0.5f * t0;
            break;
        case 2: // Sector 6
            t1 = -Z;
            t2 = -X;
            t0 = 1.0f - t1 - t2;
            k1 = t1 + t2 + t0 * 0.5f;
            k2 = t0 * 0.5f;
            k3 = t2 + 0.5f * t0;
            break;
        default:
            return;
    }

    EPWM_OUTPUTA_duty(k1);
    EPWM_OUTPUTB_duty(k2);
    EPWM_OUTPUTC_duty(k3);
}
