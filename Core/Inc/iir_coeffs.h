/*
 * iir_coeffs.h
 *
 *  Created on: 24 Aug 2026
 *      Author: batuhanozturk
 */


#ifndef IIR_COEFFS_H
#define IIR_COEFFS_H

#include "arm_math.h"

/* IIR low-pass filter coefficients (Butterworth)
 * Design: order = 4, cutoff = 20.0 Hz, fs = 512.0 Hz
 * Generated with scipy.signal.butter (SOS / biquad cascade form) */
#define IIR_NUM_STAGES 2
/* CMSIS-DSP biquad coefficient order per stage: {b0, b1, b2, a1, a2} */
/* Note: CMSIS-DSP expects NEGATED a1, a2 (it implements y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] + a1*y[n-1] + a2*y[n-2]) */
static const float32_t iir_coeffs[IIR_NUM_STAGES * 5] = {
    /* Stage 0 */
    0.00016777f, 0.00033554f, 0.00016777f, 1.58439134f, -0.63334051f,
    /* Stage 1 */
    1.00000000f, 2.00000000f, 1.00000000f, 1.77501376f, -0.82985213f,
};

#endif /* IIR_COEFFS_H */
