#include "FFT_APP.h"

/** 抽取后的 FFT 有效采样率 */
#define FFT_SAMPLE_RATE (ISR_FREQ / (float32_t)FFT_DECIMATION)
/** 用于谐波谱线定位的电网基波频率 */
#define FFT_BASE_FREQ 50.0f
/** Hamming 窗相干增益 */
#define FFT_HAMMING_GAIN 0.54f
/** 单边 FFT 幅值归一化系数 */
#define FFT_AMP_SCALE (2.0f / ((float32_t)FFT_RFFT_SIZE * FFT_HAMMING_GAIN))

#pragma SET_DATA_SECTION("logVariables")
/** RFFT 输入帧缓冲区 */
static float32_t FFT_Input_Data[FFT_RFFT_SIZE];
/** RFFT 输出缓冲区 */
static float32_t FFT_Output_Data[FFT_RFFT_SIZE];
/** RFFT 原始幅值缓冲区 */
static float32_t FFT_Mag_Data[FFT_RFFT_SIZE / 2U];
/** 谐波幅值结果数组，下标对应谐波次数 */
float32_t FFT_HarmonicAmp[FFT_MAX_HARMONIC + 1U];
/** RFFT 相位缓冲区，预留给后续相位分析使用 */
static float32_t FFT_Phase_Data[FFT_RFFT_SIZE / 2U];

/** TI RFFT 加窗函数使用的半波 Hamming 窗表 */
static const float32_t FFT_Window_Data[FFT_RFFT_SIZE / 2U] = HAMMING1024;

/** FFT_Input_Buf 当前写入下标 */
static volatile uint16_t FFT_WriteIndex = 0U;
/** 中断分频计数器 */
static volatile uint16_t FFT_DecimationCnt = 0U;
/** 一帧 FFT 数据采满标志位 */
static volatile uint16_t FFT_ReadyFlag = 0U;

/** SysConfig RFFT 输入指针 */
float32_t *FFT_Input_Buf = FFT_Input_Data;
/** SysConfig RFFT 输出指针 */
float32_t *FFT_Output_Buf = FFT_Output_Data;
/** SysConfig RFFT 幅值指针 */
float32_t *FFT_Mag_Buf = FFT_Mag_Data;
/** SysConfig RFFT 相位指针 */
float32_t *FFT_Phase_Buf = FFT_Phase_Data;
#pragma SET_DATA_SECTION()

/**
 * @brief 将一个 ADC/控制采样值写入抽取后的 FFT 帧缓冲区
 * @param[in] sample 抽取前的输入采样值
 * @return 无
 */
RAMFUNC void FFT_APP_SamplingTask(float32_t sample)
{
    if (++FFT_DecimationCnt < FFT_DECIMATION)
    {
        return;
    }

    FFT_DecimationCnt = 0U;

    if (FFT_ReadyFlag == 0U)
    {
        FFT_Input_Buf[FFT_WriteIndex++] = sample;
        if (FFT_WriteIndex >= FFT_RFFT_SIZE)
        {
            FFT_WriteIndex = 0U;
            FFT_ReadyFlag = 1U;
        }
    }
}

/**
 * @brief 当一帧数据就绪时执行一次 RFFT，并更新谐波幅值。
 * @param 无
 * @return 无
 */
void FFT_APP_ProcessTask(void)
{
    uint16_t h, k, startBin, endBin, bin;
    float32_t binFloat, magMax;

    if (FFT_ReadyFlag == 0U)
    {
        return;
    }

    RFFT_f32_win(FFT_Input_Buf, FFT_Window_Data, FFT_RFFT_SIZE);
    RFFT_f32u(myRFFT0_handle);
    RFFT_f32_mag_TMU0(myRFFT0_handle);

    FFT_HarmonicAmp[0U] = 0.0f;
    for (h = 1U; h <= FFT_MAX_HARMONIC; h++)
    {
        binFloat = ((float32_t)h * FFT_BASE_FREQ * (float32_t)FFT_RFFT_SIZE) / FFT_SAMPLE_RATE;
        bin = (uint16_t)(binFloat + 0.5f);

        startBin = (bin > 0U) ? (uint16_t)(bin - 1U) : 0U;
        endBin = (uint16_t)(bin + 1U);
        if (endBin >= (FFT_RFFT_SIZE / 2U))
        {
            endBin = (uint16_t)((FFT_RFFT_SIZE / 2U) - 1U);
        }

        magMax = 0.0f;
        for (k = startBin; k <= endBin; k++)
        {
            if (FFT_Mag_Buf[k] > magMax)
            {
                magMax = FFT_Mag_Buf[k];
            }
        }

        FFT_HarmonicAmp[h] = magMax * FFT_AMP_SCALE;
    }

    FFT_ReadyFlag = 0U;
}
