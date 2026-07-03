#include "park_trans.h"

/**
 * Park 变换 (Alpha-Beta 到 d-q)
 * 采用用户原有的旋转逻辑
 */
void ab_to_dq(float Alpha, float Beta, float wt, float *d, float *q)
{
    float s, c;
    // TI C2000 在开启 TMU 后，sinf/cosf 是极速的硬件指令
    s = sinf(wt);
    c = cosf(wt);

    *d = Beta * s + Alpha * c;
    *q = Beta * c - Alpha * s;
}

/**
 * 逆 Park 变换 (d-q 到 Alpha-Beta)
 */
void dq_to_AlphaBeta(float *Alpha, float *Beta, float wt, float d, float q)
{
    float s, c;
    s = sinf(wt);
    c = cosf(wt);

    *Alpha = d * c - q * s;
    *Beta = d * s + q * c;
}

/**
 * 三相 abc 到 dq0 变换 (幅值不变)
 */
void abc_to_dqz(float a, float b, float c, float wt, float *d, float *q, float *z)
{
    float theta[3];
    float c0, c1, c2, s0, s1, s2;

    theta[0] = wt;
    theta[1] = wt + (4.0f * PI / 3.0f);
    theta[2] = wt + (2.0f * PI / 3.0f);

    // 角度归一化到 [0, 2PI]
    int i;
    for (i = 1; i < 3; i++)
    {
        if (theta[i] >= 2.0f * PI)
            theta[i] -= 2.0f * PI;
    }

    c0 = cosf(theta[0]);
    c1 = cosf(theta[1]);
    c2 = cosf(theta[2]);
    s0 = sinf(theta[0]);
    s1 = sinf(theta[1]);
    s2 = sinf(theta[2]);

    *d = (2.0f / 3.0f) * (a * c0 + b * c1 + c * c2);
    *q = (-2.0f / 3.0f) * (a * s0 + b * s1 + c * s2);
    *z = (1.0f / 3.0f) * (a + b + c);
}

/**
 * 逆 dq0 到 abc 变换
 */
void dqz_to_abc(float *a, float *b, float *c, float wt, float d, float q, float z)
{
    float theta[3];
    theta[0] = wt;
    theta[1] = wt + (4.0f * PI / 3.0f);
    theta[2] = wt + (2.0f * PI / 3.0f);

    int i;
    for (i = 1; i < 3; i++)
    {
        if (theta[i] >= 2.0f * PI)
            theta[i] -= 2.0f * PI;
    }

    *a = d * cosf(theta[0]) - q * sinf(theta[0]) + z;
    *b = d * cosf(theta[1]) - q * sinf(theta[1]) + z;
    *c = d * cosf(theta[2]) - q * sinf(theta[2]) + z;
}
