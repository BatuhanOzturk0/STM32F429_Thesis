/*
 * fir_coeffs.h
 *
 *  Created on: 24 Aug 2026
 *      Author: batuhanozturk
 */

#ifndef FIR_COEFFS_H
#define FIR_COEFFS_H

#include "arm_math.h"

/* FIR low-pass filter coefficients
 * Design: 31-tap, cutoff = 20.0 Hz, fs = 512.0 Hz
 * Generated with scipy.signal.firwin (Hamming window, default) */
#define FIR_NUM_TAPS 31
static const float32_t fir_coeffs[FIR_NUM_TAPS] = {
    -0.00095876f,
    -0.00065290f,
    -0.00015807f,
     0.00095420f,
     0.00315588f,
     0.00687669f,
     0.01241556f,
     0.01986444f,
     0.02905853f,
     0.03956309f,
     0.05070147f,
     0.06162246f,
     0.07139821f,
     0.07913917f,
     0.08410899f,
     0.08582209f,
     0.08410899f,
     0.07913917f,
     0.07139821f,
     0.06162246f,
     0.05070147f,
     0.03956309f,
     0.02905853f,
     0.01986444f,
     0.01241556f,
     0.00687669f,
     0.00315588f,
     0.00095420f,
    -0.00015807f,
    -0.00065290f,
    -0.00095876f
};

#endif /* FIR_COEFFS_H */
