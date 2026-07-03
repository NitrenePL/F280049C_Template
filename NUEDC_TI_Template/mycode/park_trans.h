#ifndef PARK_TRANS_H_
#define PARK_TRANS_H_

#include <math.h>
#include <stdint.h>

#ifndef PI
#define PI 3.14159265358979f
#endif

// 函数声明
void ab_to_dq(float Alpha, float Beta, float wt, float *d, float *q);
void dq_to_AlphaBeta(float *Alpha, float *Beta, float wt, float d, float q);
void abc_to_dqz(float a, float b, float c, float wt, float *d, float *q, float *z);
void dqz_to_abc(float *a, float *b, float *c, float wt, float d, float q, float z);

#endif /* PARK_TRANS_H_ */
