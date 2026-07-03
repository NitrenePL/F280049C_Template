#ifndef GLOBAL_VARS_H
#define GLOBAL_VARS_H

#include "device.h"
#include <stdint.h>


#ifdef __cplusplus
extern "C"
{
#endif

    // ==============================================================================
    // 常用全局变量声明 (注意：extern 只声明，不分配内存)
    // ==============================================================================
    extern int32_t period;
    extern int32_t Open; // 开波标志：1-开，0-关
    extern int32_t MODE; // 模式变量
    extern int32_t LAST_MODE;
    // ==============================================================================
    // 临时全局变量声明 (注意：extern 只声明，不分配内存)
    // ==============================================================================
    extern float32_t Duty; // 占空比
    extern float32_t Set_Uout;
    // ==============================================================================
    // 函数原型声明
    // ==============================================================================

    // 限幅函数
    int32_t Limit_Int(int32_t input, int32_t MAX, int32_t MIN);
    float Limit_Float(float input, float MAX, float MIN);

    // 框架逻辑函数（需在 main.c 或 Control.c 中实现）
    void Setup(void);
    void Loop(void);
    void ReadData(void);
    void Show_Data(void);
    void MyProtect(void);

#ifdef __cplusplus
}
#endif

#endif /* GLOBAL_VARS_H */
