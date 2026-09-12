import numpy as np
from scipy.signal import firwin

# Filter design parameters
NUM_TAPS = 31
CUTOFF_HZ = 20.0
SAMPLE_RATE_HZ = 512.0

coeffs = firwin(NUM_TAPS, CUTOFF_HZ, fs=SAMPLE_RATE_HZ)

# Print as a C array, ready to paste into a header file
print(f"/* FIR low-pass filter coefficients")
print(f" * Design: {NUM_TAPS}-tap, cutoff = {CUTOFF_HZ} Hz, fs = {SAMPLE_RATE_HZ} Hz")
print(f" * Generated with scipy.signal.firwin (Hamming window, default) */")
print(f"#define FIR_NUM_TAPS {NUM_TAPS}")
print("static const float32_t fir_coeffs[FIR_NUM_TAPS] = {")
for i, c in enumerate(coeffs):
    end = "," if i < len(coeffs) - 1 else ""
    print(f"    {c:.8f}f{end}")
print("};")
