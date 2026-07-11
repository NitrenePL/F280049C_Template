#ifndef FFT_APP_H_
#define FFT_APP_H_

#include "global.h"

/** RFFT 计算点数 */
#define FFT_RFFT_SIZE 1024U
/** FFT 采样相对 ADC 中断的抽取倍数，50kHz 的 10 分频就是 5kHz */
#define FFT_DECIMATION 10U
/** FFT_HarmonicAmp 输出的最高谐波次数 */
#define FFT_MAX_HARMONIC 19U

#ifdef __cplusplus
extern "C"
{
#endif

    /** 谐波幅值数组，有效下标为 1..FFT_MAX_HARMONIC */
    extern float32_t FFT_HarmonicAmp[FFT_MAX_HARMONIC + 1U];

    /**
     * @brief 将一个 ADC/控制采样值写入抽取后的 FFT 帧缓冲区
     * @param[in] sample 抽取前的输入采样值
     * @return 无
     */
    RAMFUNC void FFT_APP_SamplingTask(float32_t sample);

    /**
     * @brief 当一帧数据就绪时执行一次 RFFT，并更新谐波幅值
     * @param 无
     * @return 无
     */
    void FFT_APP_ProcessTask(void);

#ifdef __cplusplus
}
#endif

#endif /* FFT_APP_H_ */
