#ifndef PID_H_
#define PID_H_

#include <stdint.h>

typedef struct
{
    float ref; // 设定值 (Reference)
    float fdb; // 反馈值 (Feedback)
    float kp;  // 比例系数
    float ki;  // 积分系数
    float Ts;  // 采样周期 (单位：秒)

    float pOut; // 比例输出
    float iOut; // 积分累加值
    float out;  // PID最终输出

    float outMax; // 输出最大限制 (例如占空比 0.95)
    float outMin; // 输出最小限制 (例如占空比 0.05)
    float intMax; // 积分限幅 (抗饱和)
} PIDStructure;

// 函数声明
void PID_Init(PIDStructure *sptr, const float kp, const float ki, const float Ts, const float outMin,
              const float outMax, const float intMax);
float PID_Calc(PIDStructure *sptr, const float fdb);
void PID_Clear(PIDStructure *sptr);

#endif /* PID_H_ */
