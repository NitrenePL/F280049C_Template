#ifndef PLL_H_
#define PLL_H_

#include "global.h"
#include "IIR.h"
#include "pid.h"

#ifndef PI
#define PI 3.14159265358979f
#endif

#define PLL_SAMPLING_FREQ ISR_FREQ
#define PLL_LOCK_HOLD_LIMIT ((uint32_t)(PLL_SAMPLING_FREQ))
#define PLL_LOCK_HOLD_ON_THRESHOLD ((uint32_t)(0.99f * PLL_SAMPLING_FREQ))
#define PLL_LOCK_HOLD_OFF_THRESHOLD ((uint32_t)(0.75f * PLL_SAMPLING_FREQ))

extern float theta, w;
extern uint16_t PLL_Locked;

extern float theta_myself, w_myself;
extern uint16_t PLL_Locked_myself;

void PLL_Init(void);
RAMFUNC void PLL_Calc(float input);

void PLL_Init_myself(void);
RAMFUNC void PLL_Calc_myself(float input);

#endif /* PLL_H_ */
