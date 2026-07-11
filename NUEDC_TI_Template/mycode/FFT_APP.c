#include "FFT_APP.h"

/** Effective FFT sampling rate after ISR decimation. */
#define APF_FFT_SAMPLE_RATE  (ISR_FREQ / (float32_t)APF_FFT_DECIMATION)
/** Nominal grid fundamental frequency used for harmonic bin lookup. */
#define APF_FFT_BASE_FREQ    50.0f
/** Hamming window coherent gain. */
#define APF_FFT_HAMMING_GAIN 0.54f
/** Single-sided FFT magnitude normalization scale. */
#define APF_FFT_AMP_SCALE    (2.0f / ((float32_t)APF_RFFT_SIZE * APF_FFT_HAMMING_GAIN))

#pragma SET_DATA_SECTION("logVariables")
/** RFFT input frame buffer. */
static float32_t FFT_Input_Data[APF_RFFT_SIZE];
/** RFFT output buffer. */
static float32_t FFT_Output_Data[APF_RFFT_SIZE];
/** Raw RFFT magnitude buffer. */
static float32_t FFT_Mag_Data[APF_RFFT_SIZE / 2U];
/** Harmonic amplitude result array, indexed by harmonic order. */
float32_t FFT_HarmonicAmp[APF_FFT_MAX_HARMONIC + 1U];
/** RFFT phase buffer, reserved for future phase analysis. */
static float32_t FFT_Phase_Data[APF_RFFT_SIZE / 2U];
/** Half-wave Hamming window table used by TI RFFT window routine. */
static const float32_t FFT_Window_Data[APF_RFFT_SIZE / 2U] = HAMMING1024;

/** Current write index in FFT_Input_Buf. */
static volatile uint16_t FFT_WriteIndex = 0U;
/** ISR decimation counter. */
static volatile uint16_t FFT_DecimationCnt = 0U;
/** Set when one complete FFT frame is ready for processing. */
static volatile uint16_t FFT_ReadyFlag = 0U;

/** SysConfig RFFT input pointer. */
float32_t *FFT_Input_Buf = FFT_Input_Data;
/** SysConfig RFFT output pointer. */
float32_t *FFT_Output_Buf = FFT_Output_Data;
/** SysConfig RFFT magnitude pointer. */
float32_t *FFT_Mag_Buf = FFT_Mag_Data;
/** SysConfig RFFT phase pointer. */
float32_t *FFT_Phase_Buf = FFT_Phase_Data;
#pragma SET_DATA_SECTION()

/**
 * @brief Push one ADC/control sample into the decimated FFT frame buffer.
 * @param[in] sample Input sample before decimation.
 * @return None.
 */
RAMFUNC void FFT_APP_SamplingTask(float32_t sample)
{
    if (++FFT_DecimationCnt < APF_FFT_DECIMATION)
    {
        return;
    }

    FFT_DecimationCnt = 0U;

    if (FFT_ReadyFlag == 0U)
    {
        FFT_Input_Buf[FFT_WriteIndex++] = sample;
        if (FFT_WriteIndex >= APF_RFFT_SIZE)
        {
            FFT_WriteIndex = 0U;
            FFT_ReadyFlag = 1U;
        }
    }
}

/**
 * @brief Run one RFFT frame and update harmonic amplitudes when a frame is ready.
 * @param None.
 * @return None.
 */
void FFT_APP_ProcessTask(void)
{
    uint16_t h, k, startBin, endBin, bin;
    float32_t binFloat, magMax;

    if (FFT_ReadyFlag == 0U)
    {
        return;
    }

    RFFT_f32_win(FFT_Input_Buf, FFT_Window_Data, APF_RFFT_SIZE);
    RFFT_f32u(myRFFT0_handle);
    RFFT_f32_mag_TMU0(myRFFT0_handle);

    FFT_HarmonicAmp[0U] = 0.0f;
    for (h = 1U; h <= APF_FFT_MAX_HARMONIC; h++)
    {
        binFloat = ((float32_t)h * APF_FFT_BASE_FREQ * (float32_t)APF_RFFT_SIZE) / APF_FFT_SAMPLE_RATE;
        bin = (uint16_t)(binFloat + 0.5f);

        startBin = (bin > 0U) ? (uint16_t)(bin - 1U) : 0U;
        endBin = (uint16_t)(bin + 1U);
        if (endBin >= (APF_RFFT_SIZE / 2U))
        {
            endBin = (uint16_t)((APF_RFFT_SIZE / 2U) - 1U);
        }

        magMax = 0.0f;
        for (k = startBin; k <= endBin; k++)
        {
            if (FFT_Mag_Buf[k] > magMax)
            {
                magMax = FFT_Mag_Buf[k];
            }
        }

        FFT_HarmonicAmp[h] = magMax * APF_FFT_AMP_SCALE;
    }

    FFT_ReadyFlag = 0U;
}
