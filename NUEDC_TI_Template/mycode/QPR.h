#ifndef QPR_DF22_H_
#define QPR_DF22_H_

/*
 * qpr_df22.h
 *
 * QPR Controller Based on DF22 Structure
 *
 * Copyright (c) 2026 NitreneP
 *
 * This file integrates and adapts code and algorithms from the following sources:
 *
 * 1. QPR / PR controller coefficient calculation:
 *    Derived from Texas Instruments Digital Power SDK examples.
 *
 * 2. DF22 controller execution structure:
 *    Adapted from the Texas Instruments Digital Control Library (DCL),
 *    originally implemented in assembly for C2000 devices.
 *
 * 3. Integration, C implementation, comments, and usage example:
 *    Integrated and modified by NitreneP.
 *
 */

#include "math.h"

/*
 * 使用例
 *
 * // 实例化 QPR 控制器
 * DCL_DF22 DCL_PR_Ctrl;
 *
 * // 计算 QPR 离散化系数
 * //
 * // 函数原型：
 * // void computeDF22_PRcontrollerCoeff(DCL_DF22 *v,
 * //                                    float kp,
 * //                                    float kr,
 * //                                    float wo,
 * //                                    float fs,
 * //                                    float wrc);
 * //
 * // 参数：
 * // kp  = 3
 * // kr  = 175
 * // wo  = 2*pi*50 rad/s，谐振频率 50 Hz
 * // fs  = 20000 Hz，采样频率 20 kHz
 * // wrc = 2*pi*1 rad/s，谐振带宽 1 Hz
 * //
 * computeDF22_PRcontrollerCoeff(&DCL_PR_Ctrl,
 *                               3.0f,
 *                               175.0f,
 *                               2.0f * M_PI * 50.0f,
 *                               20000.0f,
 *                               2.0f * M_PI * 1.0f);
 *
 * // 置零 QPR 内部状态
 * DCL_resetDF22(&DCL_PR_Ctrl);
 *
 * // 在控制中断中周期调用
 * output = DCL_runDF22_C1(&DCL_PR_Ctrl, error);
 */

/*
 * DF22 二阶 IIR 控制器结构体
 *
 * 对应离散传递函数：
 *
 *              U(z)   b0 + b1 z^-1 + b2 z^-2
 *      G(z) = ------ = ------------------------
 *              E(z)    1 + a1 z^-1 + a2 z^-2
 *
 * 对应差分方程：
 *
 *      u[k] = b0 e[k] + b1 e[k-1] + b2 e[k-2]
 *             - a1 u[k-1] - a2 u[k-2]
 *
 * 其中：
 *      e[k]：当前误差输入
 *      u[k]：当前控制器输出
 */
typedef struct dcl_df22
{
    float b0; //!< 分子系数 b0
    float b1; //!< 分子系数 b1
    float b2; //!< 分子系数 b2
    float a1; //!< 分母系数 a1，对应 1 + a1 z^-1 + a2 z^-2
    float a2; //!< 分母系数 a2
    float x1; //!< DF-II Transposed 内部状态 x1
    float x2; //!< DF-II Transposed 内部状态 x2
} DCL_DF22;

//! \brief          将 DF22 控制器内部状态清零
//! \param[in] p    指向 DCL_DF22 控制器结构体的指针
//! \return         None
static inline void DCL_resetDF22(DCL_DF22 *p)
{
    p->x1 = p->x2 = 0.0f;
}

//! \brief          运行一次 DF22 控制器
//!
//!                         U(z)   b0 + b1 z^-1 + b2 z^-2
//!                 G(z) = ------ = ------------------------
//!                         E(z)    1 + a1 z^-1 + a2 z^-2
//!
//!                 对应标准差分方程：
//!
//!                 u[k] = b0 e[k] + b1 e[k-1] + b2 e[k-2]
//!                        - a1 u[k-1] - a2 u[k-2]
//!
//!                 本函数采用 DF-II Transposed 结构：
//!
//!                 u[k]      = b0 e[k] + x1[k]
//!                 x1[k+1]   = b1 e[k] + x2[k] - a1 u[k]
//!                 x2[k+1]   = b2 e[k]         - a2 u[k]
//!
//! \param[in] p    指向 DCL_DF22 控制器结构体的指针
//! \param[in] ek   当前误差输入 e[k]
//! \return         当前控制器输出 u[k]
static inline float DCL_runDF22_C1(DCL_DF22 *p, float ek)
{
    float uk;
    float x1d;
    float x2d;

    /*
     * 输出方程：
     *
     *      u[k] = b0 e[k] + x1[k]
     */
    uk = (p->b0 * ek) + p->x1;

    /*
     * 状态更新方程：
     *
     *      x1[k+1] = b1 e[k] + x2[k] - a1 u[k]
     *      x2[k+1] = b2 e[k]         - a2 u[k]
     */
    x1d = (p->b1 * ek) + p->x2 - (p->a1 * uk);
    x2d = (p->b2 * ek) - (p->a2 * uk);

    p->x1 = x1d;
    p->x2 = x2d;

    return uk;
}

//! \brief          计算 QPR 控制器的 DF22 离散系数
//!
//!                                2 Kr wc s
//!                 Gc(s) = Kp + ----------------
//!                              s^2 + 2 wc s + wo^2
//!
//!                 其中：
//!                     Kp：比例增益
//!                     Kr：谐振增益
//!                     wo：谐振角频率，单位 rad/s
//!                     wc：谐振带宽角频率，单位 rad/s
//!
//!                 使用 Tustin 双线性变换进行离散化：
//!
//!                                1 - z^-1
//!                     s = 2 fs -----------
//!                                1 + z^-1
//!
//!                 为减小双线性变换带来的频率畸变，对谐振频率 wo 做预畸变：
//!
//!                     wo_adjusted = 2 fs tan(wo / (2 fs))
//!
//!                 因此连续域中的 wo 替换为 wo_adjusted 后再进行 Tustin 离散化。
//!
//!                 注意：
//!                     1. fs 单位为 Hz。
//!                     2. wo 和 wrc 单位均为 rad/s。
//!                     3. tanf() 可替换为 C2000 FastRTS 或 TMU 加速函数。
//!                     4. 计算出的 a1、a2 采用分母 1 + a1 z^-1 + a2 z^-2 的形式。
//!
//! \param[in] v     指向 DCL_DF22 控制器结构体的指针
//! \param[in] kp    比例增益 Kp
//! \param[in] kr    谐振增益 Kr
//! \param[in] wo    谐振角频率 wo，单位 rad/s
//! \param[in] fs    采样频率 fs，单位 Hz
//! \param[in] wrc   谐振带宽角频率 wc，单位 rad/s
//! \return          None
static inline void computeDF22_PRcontrollerCoeff(DCL_DF22 *v, float kp, float kr, float wo, float fs, float wrc)
{
    float temp1;
    float temp2;
    float wo_adjusted;

    /*
     * 谐振频率预畸变：
     *
     *      wo_adjusted = 2 fs tan(wo / (2 fs))
     *
     * 用于补偿 Tustin 双线性变换中的频率压缩。
     */
    wo_adjusted = 2.0f * fs * tanf(wo / (2.0f * fs));
    temp1 = 4.0f * fs * fs + wo_adjusted * wo_adjusted + 4.0f * fs * wrc;

    /*
     * 谐振项分子系数
     *
     *      temp2 = 4 Kr wrc fs / temp1
     */
    temp2 = 4.0f * kr * wrc * fs / temp1;

    v->b0 = temp2;
    v->b1 = 0.0f;
    v->b2 = -temp2;

    v->a1 = (-8.0f * fs * fs + 2.0f * wo_adjusted * wo_adjusted) / temp1;

    v->a2 = (temp1 - 8.0f * fs * wrc) / temp1;

    /*
     * 初始化 DF22 内部状态
     */
    v->x1 = 0.0f;
    v->x2 = 0.0f;

    /*
     * 将比例项 Kp 并入分子
     */
    if (kp != 0.0f)
    {
        v->b0 += kp;
        v->b1 += kp * v->a1;
        v->b2 += kp * v->a2;
    }
}

#endif /* QPR_DF22_H_ */
