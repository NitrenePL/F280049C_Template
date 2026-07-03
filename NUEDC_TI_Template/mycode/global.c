#include "global.h"

// ==============================================================================
// 常用变量定义并初始化
// ==============================================================================
int32_t period = 0; // 假设默认周期值
int32_t Open = 0;   // 默认关闭
int32_t MODE = 0;   // 默认模式0
int32_t LAST_MODE = 0;
// ==============================================================================
// 函数实现
// ==============================================================================

/**
 * 整数限幅
 */
int32_t Limit_Int(int32_t input, int32_t MAX, int32_t MIN)
{
    if (input >= MAX)
        return MAX;
    if (input <= MIN)
        return MIN;
    return input;
}

/**
 * 浮点限幅
 */
float Limit_Float(float input, float MAX, float MIN)
{
    if (input >= MAX)
        return MAX;
    if (input <= MIN)
        return MIN;
    return input;
}
