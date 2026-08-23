import numpy as np
from scipy.signal import butter, sos2tf, tf2sos

# Filter design parameters
ORDER = 4  # 4th order = 2 biquad (second-order) sections
CUTOFF_HZ = 20.0
SAMPLE_RATE_HZ = 512.0

# Design a Butterworth low-pass filter, output as second-order sections (SOS)
sos = butter(ORDER, CUTOFF_HZ, btype='low', fs=SAMPLE_RATE_HZ, output='sos')

num_stages = sos.shape[0]
print(f"/* IIR low-pass filter coefficients (Butterworth)")
print(f" * Design: order = {ORDER}, cutoff = {CUTOFF_HZ} Hz, fs = {SAMPLE_RATE_HZ} Hz")
print(f" * Generated with scipy.signal.butter (SOS / biquad cascade form) */")
print(f"#define IIR_NUM_STAGES {num_stages}")
print("/* CMSIS-DSP biquad coefficient order per stage: {b0, b1, b2, a1, a2} */")
print("/* Note: CMSIS-DSP expects NEGATED a1, a2 (it implements y[n] = b0*x[n] + b1*x[n-1] + b2*x[n-2] + a1*y[n-1] + a2*y[n-2]) */")
print(f"static const float32_t iir_coeffs[{num_stages} * 5] = {{")
for i, section in enumerate(sos):
    b0, b1, b2, a0, a1, a2 = section
    # scipy sos rows are [b0, b1, b2, a0, a1, a2] with a0 = 1 always
    # CMSIS-DSP wants: b0, b1, b2, -a1, -a2
    print(f"    /* Stage {i} */")
    print(f"    {b0:.8f}f, {b1:.8f}f, {b2:.8f}f, {-a1:.8f}f, {-a2:.8f}f,")
print("};")

