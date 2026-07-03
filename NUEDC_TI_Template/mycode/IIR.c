#include "IIR.h"

/**
 * 初始化二阶 IIR 滤波器
 * @param S 结构体指针
 * @param pNum 分子系数数组指针 [b0, b1, b2]
 * @param pDen 分母系数数组指针 [a1, a2] (对应 Matlab 中的 a(2) 和 a(3))
 */
void IIR2nd_Init(IIRStructure *S, const float *pNum, const float *pDen)
{
    // 拷贝系数
    S->Num[0] = pNum[0]; // b0
    S->Num[1] = pNum[1]; // b1
    S->Num[2] = pNum[2]; // b2

    S->Den[0] = pDen[0]; // a1
    S->Den[1] = pDen[1]; // a2

    // 初始化缓冲区为 0
    IIR2nd_Clear(S);
}

/**
 * 二阶 IIR 计算函数 (差分方程实现)
 * y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] - a1*y[n-1] - a2*y[n-2]
 */
float IIR2nd_Calc(IIRStructure *S, float input)
{
    float out;

    // 计算差分方程
    out = (S->Num[0] * input) + (S->Num[1] * S->inBuf[0]) + (S->Num[2] * S->inBuf[1]) - (S->Den[0] * S->outBuf[0]) -
          (S->Den[1] * S->outBuf[1]);

    // 更新输入缓冲区 [n-2] = [n-1], [n-1] = current
    S->inBuf[1] = S->inBuf[0];
    S->inBuf[0] = input;

    // 更新输出缓冲区 [n-2] = [n-1], [n-1] = current_out
    S->outBuf[1] = S->outBuf[0];
    S->outBuf[0] = out;

    return out;
}

/**
 * 清除滤波器内部状态 (用于系统重启或复位)
 */
void IIR2nd_Clear(IIRStructure *S)
{
    S->inBuf[0] = 0.0f;
    S->inBuf[1] = 0.0f;
    S->outBuf[0] = 0.0f;
    S->outBuf[1] = 0.0f;
}
