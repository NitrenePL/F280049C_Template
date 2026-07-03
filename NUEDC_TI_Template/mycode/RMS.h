#ifndef RMS_H_
#define RMS_H_

#include <math.h>
#include <stdint.h>


// -------------------------------------------------------------------------
// 宏定义窗口大小，修改此处即可改变计算周期
// -------------------------------------------------------------------------
#define RMS_WINDOW_SIZE 400

typedef struct
{
    float buffer[RMS_WINDOW_SIZE]; // 采样值缓冲区 (只存原值以节省内存)
    float sum;                     // 窗口内平方和
    float out;                     // 最终RMS输出结果
    uint16_t index;                // 当前索引
    uint16_t flag;                 // 缓冲区填满标志位
} RMS_Obj;

// 函数声明
void RMS_Init(RMS_Obj *sprt);
float RMS_Calc(RMS_Obj *sprt, float input);

#endif /* RMS_H_ */
