/*
 * dsp_module.h
 *
 *  Created on: 23 Aug 2026
 *      Author: batuhanozturk
 */
#ifndef DSP_MODULE_H
#define DSP_MODULE_H

#include "arm_math.h"

/* FFT test function: generates a known sine wave, runs FFT, prints result */
void DSP_TestFFT_256(void);
void DSP_RunFFT_FromIMU(float32_t accel_x_g, float32_t accel_y_g, float32_t accel_z_g);

void DSP_BenchmarkFFT(uint16_t fft_size);

void DSP_RunFIR_FromIMU(float32_t accel_x_g, float32_t accel_y_g, float32_t accel_z_g);
void DSP_RunIIR_FromIMU(float32_t accel_x_g, float32_t accel_y_g, float32_t accel_z_g);
void DSP_RunRMS_FromIMU(float32_t accel_x_g, float32_t accel_y_g, float32_t accel_z_g);
void DSP_RunPeakDetect_FromIMU(float32_t accel_x_g, float32_t accel_y_g, float32_t accel_z_g);

#endif /* DSP_MODULE_H */
