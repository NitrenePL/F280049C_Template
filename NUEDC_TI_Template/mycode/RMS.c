#include "RMS.h"

/**
 * RMS 结构体初始化
 */
void RMS_Init(RMS_Obj *sprt)
{
    uint16_t i;
    for (i = 0; i < RMS_WINDOW_SIZE; i++)
    {
        sprt->buffer[i] = 0.0f;
    }
    sprt->sum = 0.0f;
    sprt->out = 0.0f;
    sprt->index = 0;
    sprt->flag = 0;
}

/**
 * 滑动窗口 RMS 计算
 * 原理：Sum = Sum + (新值的平方) - (最老值的平方)
 */
float RMS_Calc(RMS_Obj *sprt, float input)
{
    float new_sq;
    float old_sq;

    // 1. 计算当前输入的平方
    new_sq = input * input;

    // 2. 如果缓冲区已满，开始执行滑动窗口逻辑
    if (sprt->flag == 1)
    {
        // 找到最老的值并计算它的平方
        old_sq = sprt->buffer[sprt->index] * sprt->buffer[sprt->index];

        // 更新平方和：加上新的，减去最老的
        sprt->sum = sprt->sum + new_sq - old_sq;
    }
    else
    {
        // 缓冲区未满时，只累加
        sprt->sum += new_sq;
    }

    // 3. 更新缓冲区中的值为当前原值
    sprt->buffer[sprt->index] = input;

    // 4. 计算 RMS 输出
    if (sprt->flag == 1)
    {
        // 均值 = 总和 / 窗口大小
        float mean_sq = sprt->sum / (float)RMS_WINDOW_SIZE;

        // TI C2000 如果开启了 TMU 优化，sqrtf 会被自动编译为一条硬件指令
        sprt->out = sqrtf(mean_sq);
    }
    else
    {
        // 在填满第一个窗口前，输出可以选为0或者当前均值的根号
        sprt->out = 0.0f;
    }

    // 5. 更新索引
    sprt->index++;
    if (sprt->index >= RMS_WINDOW_SIZE)
    {
        sprt->index = 0;
        sprt->flag = 1;
    }

    return sprt->out;
}
