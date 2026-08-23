/*
 * dsp_module.c
 *
 *  Created on: 23 Aug 2026
 *      Author: batuhanozturk
 */


#include "dsp_module.h"
#include <stdio.h>
#include <math.h>

#include "scheduler_eval.h"
#include "fir_coeffs.h"
#include "iir_coeffs.h"

#define FFT_SIZE        256
#define TEST_FREQ_HZ    10.0f
#define SAMPLE_RATE_HZ  512.0f

/* Complex FFT input/output buffer: interleaved [real, imag, real, imag, ...] */
static float32_t fft_input[FFT_SIZE * 2];
static float32_t fft_output_mag[FFT_SIZE];

void DSP_TestFFT_256(void)
{
    arm_cfft_instance_f32 fft_instance;

    /* Step 1: Generate a known 10 Hz sine wave as test signal */
    for (uint32_t i = 0; i < FFT_SIZE; i++)
    {
        float32_t t = (float32_t)i / SAMPLE_RATE_HZ;
        fft_input[2 * i]     = arm_sin_f32(2.0f * PI * TEST_FREQ_HZ * t); /* real part */
        fft_input[2 * i + 1] = 0.0f;                                      /* imaginary part */
    }

    /* Step 2: Initialize CFFT instance for 256 points */
    arm_status status = arm_cfft_init_f32(&fft_instance, FFT_SIZE);
    if (status != ARM_MATH_SUCCESS)
    {
        printf("FFT init failed! status=%d\r\n", status);
        return;
    }

    /* Step 3: Run the FFT (in-place, forward transform) */
    arm_cfft_f32(&fft_instance, fft_input, 0 /* forward */, 1 /* bit reverse */);

    /* Step 4: Compute magnitude spectrum */
    arm_cmplx_mag_f32(fft_input, fft_output_mag, FFT_SIZE);

    /* Step 5: Find peak bin (only first half is meaningful for real input) */
    float32_t max_val = 0.0f;
    uint32_t max_idx = 0;
    arm_max_f32(fft_output_mag, FFT_SIZE / 2, &max_val, &max_idx);

    float32_t freq_resolution = SAMPLE_RATE_HZ / (float32_t)FFT_SIZE;
    float32_t detected_freq = (float32_t)max_idx * freq_resolution;

    printf("\r\n=== FFT Test (256-point) ===\r\n");
    printf("Expected freq: %.2f Hz\r\n", TEST_FREQ_HZ);
    printf("Peak bin: %lu, Magnitude: %.2f\r\n", (unsigned long)max_idx, max_val);
    printf("Detected freq: %.2f Hz\r\n", detected_freq);
    printf("Freq resolution (df): %.4f Hz\r\n", freq_resolution);
    printf("=============================\r\n\r\n");
}

#define IMU_FFT_SIZE 256

static float32_t imu_fft_buffer[IMU_FFT_SIZE * 2]; /* interleaved real/imag */
static uint32_t imu_sample_count = 0;

void DSP_RunFFT_FromIMU(float32_t accel_x_g, float32_t accel_y_g, float32_t accel_z_g)
{
    /* Combined magnitude of the three axes - direction-independent activity/vibration detection */
    float32_t magnitude = sqrtf(accel_x_g * accel_x_g +
                                 accel_y_g * accel_y_g +
                                 accel_z_g * accel_z_g);

    if (imu_sample_count < IMU_FFT_SIZE)
    {
        imu_fft_buffer[2 * imu_sample_count]     = magnitude; /* real */
        imu_fft_buffer[2 * imu_sample_count + 1] = 0.0f;      /* imaginary */
        imu_sample_count++;
        return;
    }

    /* Buffer full (256 samples collected): run the FFT */
    arm_cfft_instance_f32 fft_instance;
    arm_cfft_init_f32(&fft_instance, IMU_FFT_SIZE);
    arm_cfft_f32(&fft_instance, imu_fft_buffer, 0 /* forward */, 1 /* bit reverse */);

    static float32_t mag_output[IMU_FFT_SIZE];
    arm_cmplx_mag_f32(imu_fft_buffer, mag_output, IMU_FFT_SIZE);

    /* Send over UART in CSV format: bin_index,magnitude */
    printf("FFT_START\r\n");
    for (uint32_t i = 0; i < IMU_FFT_SIZE / 2; i++)
    {
        printf("%lu,%.4f\r\n", (unsigned long)i, mag_output[i]);
    }
    printf("FFT_END\r\n");

    imu_sample_count = 0; /* reset buffer for the next window */
}

#define MAX_FFT_SIZE 1024

static float32_t bench_fft_buffer[MAX_FFT_SIZE * 2]; /* sized for the largest case (1024) */

void DSP_BenchmarkFFT(uint16_t fft_size)
{
    const float32_t test_freq_hz   = 10.0f;
    const float32_t sample_rate_hz = 512.0f;

    /* Generate the same known 10 Hz sine wave, sized to fft_size */
    for (uint16_t i = 0; i < fft_size; i++)
    {
        float32_t t = (float32_t)i / sample_rate_hz;
        bench_fft_buffer[2 * i]     = arm_sin_f32(2.0f * PI * test_freq_hz * t);
        bench_fft_buffer[2 * i + 1] = 0.0f;
    }

    arm_cfft_instance_f32 fft_instance;
    arm_status status = arm_cfft_init_f32(&fft_instance, fft_size);
    if (status != ARM_MATH_SUCCESS)
    {
        printf("FFT init failed for size %u! status=%d\r\n", fft_size, status);
        return;
    }

    /* Measure only the FFT compute step itself, not signal generation or init */
    uint32_t t_start = SCHED_EVAL_START();
    arm_cfft_f32(&fft_instance, bench_fft_buffer, 0 /* forward */, 1 /* bit reverse */);
    SCHED_EVAL_STOP_AND_PRINT(t_start, "FFT_Compute");

    /* Quick sanity check: verify the peak still lands at the expected bin */
    static float32_t bench_mag[MAX_FFT_SIZE];
    arm_cmplx_mag_f32(bench_fft_buffer, bench_mag, fft_size);

    float32_t max_val = 0.0f;
    uint32_t max_idx = 0;
    arm_max_f32(bench_mag, fft_size / 2, &max_val, &max_idx);

    float32_t freq_resolution = sample_rate_hz / (float32_t)fft_size;
    float32_t detected_freq = (float32_t)max_idx * freq_resolution;

    printf("=== FFT Benchmark (%u-point) ===\r\n", fft_size);
    printf("Detected freq: %.2f Hz (bin %lu), df: %.4f Hz\r\n",
           detected_freq, (unsigned long)max_idx, freq_resolution);
    printf("=================================\r\n\r\n");
}

#define FIR_WINDOW_SIZE 128  /* how many filtered samples to collect before sending over UART */

static arm_fir_instance_f32 fir_instance;
static float32_t fir_state[FIR_NUM_TAPS + FIR_WINDOW_SIZE - 1]; /* CMSIS-DSP requires numTaps + blockSize - 1 */
static uint8_t fir_initialized = 0;

static float32_t fir_raw_buffer[FIR_WINDOW_SIZE];
static float32_t fir_filtered_buffer[FIR_WINDOW_SIZE];
static uint32_t fir_sample_count = 0;

void DSP_RunFIR_FromIMU(float32_t accel_x_g, float32_t accel_y_g, float32_t accel_z_g)
{
    if (!fir_initialized)
    {
        arm_fir_init_f32(&fir_instance, FIR_NUM_TAPS, (float32_t *)fir_coeffs, fir_state, 1);
        fir_initialized = 1;
    }

    /* Combined magnitude, same approach as the FFT feed */
    float32_t magnitude = sqrtf(accel_x_g * accel_x_g +
                                 accel_y_g * accel_y_g +
                                 accel_z_g * accel_z_g);

    if (fir_sample_count < FIR_WINDOW_SIZE)
    {
        fir_raw_buffer[fir_sample_count] = magnitude;
        fir_sample_count++;
        return;
    }

    /* Window full: run the FIR filter over the whole block at once */
    uint32_t t_start = SCHED_EVAL_START();
    arm_fir_f32(&fir_instance, fir_raw_buffer, fir_filtered_buffer, FIR_WINDOW_SIZE);
    SCHED_EVAL_STOP_AND_PRINT(t_start, "FIR_Compute");

    /* Send raw vs filtered pairs over UART in CSV format: index,raw,filtered */
    printf("FIR_START\r\n");
    for (uint32_t i = 0; i < FIR_WINDOW_SIZE; i++)
    {
        printf("%lu,%.5f,%.5f\r\n", (unsigned long)i, fir_raw_buffer[i], fir_filtered_buffer[i]);
    }
    printf("FIR_END\r\n");

    fir_sample_count = 0; /* reset for the next window */
}


#define IIR_WINDOW_SIZE 128

static arm_biquad_cascade_df2T_instance_f32 iir_instance;
static float32_t iir_state[IIR_NUM_STAGES * 2]; /* CMSIS-DSP DF2T requires 2 state values per stage */
static uint8_t iir_initialized = 0;

static float32_t iir_raw_buffer[IIR_WINDOW_SIZE];
static float32_t iir_filtered_buffer[IIR_WINDOW_SIZE];
static uint32_t iir_sample_count = 0;

void DSP_RunIIR_FromIMU(float32_t accel_x_g, float32_t accel_y_g, float32_t accel_z_g)
{
    if (!iir_initialized)
    {
        arm_biquad_cascade_df2T_init_f32(&iir_instance, IIR_NUM_STAGES, (float32_t *)iir_coeffs, iir_state);
        iir_initialized = 1;
    }

    /* Combined magnitude, same approach as the FFT/FIR feed */
    float32_t magnitude = sqrtf(accel_x_g * accel_x_g +
                                 accel_y_g * accel_y_g +
                                 accel_z_g * accel_z_g);

    if (iir_sample_count < IIR_WINDOW_SIZE)
    {
        iir_raw_buffer[iir_sample_count] = magnitude;
        iir_sample_count++;
        return;
    }

    /* Window full: run the IIR filter over the whole block at once */
    uint32_t t_start = SCHED_EVAL_START();
    arm_biquad_cascade_df2T_f32(&iir_instance, iir_raw_buffer, iir_filtered_buffer, IIR_WINDOW_SIZE);
    SCHED_EVAL_STOP_AND_PRINT(t_start, "IIR_Compute");

    /* Send raw vs filtered pairs over UART in CSV format: index,raw,filtered */
    printf("IIR_START\r\n");
    for (uint32_t i = 0; i < IIR_WINDOW_SIZE; i++)
    {
        printf("%lu,%.5f,%.5f\r\n", (unsigned long)i, iir_raw_buffer[i], iir_filtered_buffer[i]);
    }
    printf("IIR_END\r\n");

    iir_sample_count = 0; /* reset for the next window */
}
#define RMS_WINDOW_SIZE 128

static float32_t rms_buffer[RMS_WINDOW_SIZE];
static uint32_t rms_sample_count = 0;

void DSP_RunRMS_FromIMU(float32_t accel_x_g, float32_t accel_y_g, float32_t accel_z_g)
{
    /* Combined magnitude, same approach as the FFT/FIR/IIR feed */
    float32_t magnitude = sqrtf(accel_x_g * accel_x_g +
                                 accel_y_g * accel_y_g +
                                 accel_z_g * accel_z_g);

    if (rms_sample_count < RMS_WINDOW_SIZE)
    {
        rms_buffer[rms_sample_count] = magnitude;
        rms_sample_count++;
        return;
    }

    /* Window full: compute RMS over the whole block at once */
    float32_t rms_result = 0.0f;

    uint32_t t_start = SCHED_EVAL_START();
    arm_rms_f32(rms_buffer, RMS_WINDOW_SIZE, &rms_result);
    SCHED_EVAL_STOP_AND_PRINT(t_start, "RMS_Compute");

    printf("[RMS] window_size=%u value=%.5f g\r\n", RMS_WINDOW_SIZE, rms_result);

    rms_sample_count = 0; /* reset for the next window */
}
