#ifndef FFT_APP_H_
#define FFT_APP_H_

#include "global.h"

/** RFFT point count. */
#define APF_RFFT_SIZE        1024U
/** ADC ISR decimation ratio for FFT sampling. */
#define APF_FFT_DECIMATION   10U
/** Highest harmonic order exported by FFT_HarmonicAmp. */
#define APF_FFT_MAX_HARMONIC 19U

#ifdef __cplusplus
extern "C"
{
#endif

/** Harmonic amplitude array, index 1..APF_FFT_MAX_HARMONIC is valid. */
extern float32_t FFT_HarmonicAmp[APF_FFT_MAX_HARMONIC + 1U];

/**
 * @brief Push one ADC/control sample into the decimated FFT frame buffer.
 * @param[in] sample Input sample before decimation.
 * @return None.
 */
RAMFUNC void FFT_APP_SamplingTask(float32_t sample);

/**
 * @brief Run one RFFT frame and update harmonic amplitudes when a frame is ready.
 * @param None.
 * @return None.
 */
void FFT_APP_ProcessTask(void);

#ifdef __cplusplus
}
#endif

#endif /* FFT_APP_H_ */
