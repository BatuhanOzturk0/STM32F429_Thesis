import serial
import matplotlib.pyplot as plt

PORT = '/dev/tty.usbmodem112103'
BAUD = 115200
SAMPLE_RATE_HZ = 512.0
FFT_SIZE = 256

def read_one_fft_window(ser):
    bins = []
    mags = []
    while True:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line == "FFT_START":
            bins = []
            mags = []
            continue
        if line == "FFT_END":
            return bins, mags
        if ',' in line:
            try:
                idx, mag = line.split(',')
                bins.append(int(idx))
                mags.append(float(mag))
            except ValueError:
                continue

def main():
    ser = serial.Serial(PORT, BAUD, timeout=2)
    print("Waiting for one FFT window...")
    bins, mags = read_one_fft_window(ser)
    ser.close()

    freqs = [b * (SAMPLE_RATE_HZ / FFT_SIZE) for b in bins]

    plt.figure(figsize=(10, 5))
    plt.plot(freqs, mags)
    plt.xlabel("Frequency (Hz)")
    plt.ylabel("Magnitude")
    plt.title("256-point FFT Spectrum from IMU Accelerometer Data")
    plt.grid(True)
    plt.savefig("fft_256_spectrum.png", dpi=150)
    plt.show()
    print("Saved: fft_256_spectrum.png")

if __name__ == "__main__":
    main()
