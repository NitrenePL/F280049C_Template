#include "pid.h"

/**
 * PID 初始化
 */
void PID_Init(PIDStructure *sptr, const float kp, const float ki, const float Ts, const float outMin,
              const float outMax, const float intMax)
{
    sptr->kp = kp;
    sptr->ki = ki;
    sptr->Ts = Ts;
    sptr->outMax = outMax;
    sptr->outMin = outMin;
    sptr->intMax = intMax;

    sptr->iOut = 0.0f;
    sptr->pOut = 0.0f;
    sptr->out = 0.0f;
    sptr->ref = 0.0f;
    sptr->fdb = 0.0f;
}

/**
 * PID 计算
 * 对于 Buck 变换器：fdb 是采集的输出电压，ref 是目标电压，返回的是占空比
 */
float PID_Calc(PIDStructure *sptr, const float fdb)
{
    float error;
    sptr->fdb = fdb;
    error = sptr->ref - fdb;

    // 1. 计算积分项 (后向矩形积分)
    sptr->iOut += sptr->ki * error * sptr->Ts;

    // 2. 积分限幅 (Anti-windup 抗饱和)
    if (sptr->iOut > sptr->intMax)
        sptr->iOut = sptr->intMax;
    else if (sptr->iOut < -sptr->intMax)
        sptr->iOut = -sptr->intMax;

    // 3. 计算比例项
    sptr->pOut = sptr->kp * error;

    // 4. 总输出计算
    sptr->out = sptr->pOut + sptr->iOut;

    // 5. 输出限幅 (例如占空比不能超过 1.0 且不能小于 0)
    if (sptr->out > sptr->outMax)
        sptr->out = sptr->outMax;
    if (sptr->out < sptr->outMin)
        sptr->out = sptr->outMin;

    return (sptr->out);
}

/**
 * 清除积分项 (通常在关机或发生保护时调用)
 */
void PID_Clear(PIDStructure *sptr)
{
    sptr->iOut = 0.0f;
    sptr->out = 0.0f;
}
