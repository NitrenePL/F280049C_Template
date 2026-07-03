#ifndef IIR_H_
#define IIR_H_

#include <stdint.h>

/**
 * 二阶 IIR 滤波器结构体 (直接 I 型)
 * 传递函数: H(z) = (b0 + b1*z^-1 + b2*z^-2) / (1 + a1*z^-1 + a2*z^-2)
 */
typedef struct
{
    float inBuf[2];  // 输入状态缓冲区: x[n-1], x[n-2]
    float outBuf[2]; // 输出状态缓冲区: y[n-1], y[n-2]
    float Num[3];    // 分子系数 (b0, b1, b2)
    float Den[2];    // 分母系数 (a1, a2) -> 注意: a0 默认为 1.0
} IIRStructure;

// 函数声明
void IIR2nd_Init(IIRStructure *S, const float *pNum, const float *pDen);
float IIR2nd_Calc(IIRStructure *S, float input);
void IIR2nd_Clear(IIRStructure *S);

#endif /* IIR_H_ */
