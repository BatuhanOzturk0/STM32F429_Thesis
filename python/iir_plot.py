import serial
import matplotlib.pyplot as plt

PORT = '/dev/tty.usbmodem112103'
BAUD = 115200

def read_one_iir_window(ser):
    indices = []
    raw_vals = []
    filtered_vals = []
    while True:
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if line == "IIR_START":
            indices = []
            raw_vals = []
            filtered_vals = []
            continue
        if line == "IIR_END":
            return indices, raw_vals, filtered_vals
        parts = line.split(',')
        if len(parts) == 3:
            try:
                idx, raw, filt = parts
                indices.append(int(idx))
                raw_vals.append(float(raw))
                filtered_vals.append(float(filt))
            except ValueError:
                continue

def main():
    ser = serial.Serial(PORT, BAUD, timeout=2)
    print("Waiting for one IIR window...")
    indices, raw_vals, filtered_vals = read_one_iir_window(ser)
    ser.close()

    plt.figure(figsize=(10, 5))
    plt.plot(indices, raw_vals, label="Raw (unfiltered)", alpha=0.6)
    plt.plot(indices, filtered_vals, label="Filtered (IIR Butterworth, 4th order, 20Hz cutoff)", linewidth=2)
    plt.xlabel("Sample index")
    plt.ylabel("Accel magnitude (g)")
    plt.title("IIR Low-Pass Filter: Raw vs Filtered Accelerometer Signal")
    plt.legend()
    plt.grid(True)
    plt.savefig("iir_before_after.png", dpi=150)
    plt.show()
    print("Saved: iir_before_after.png")

if __name__ == "__main__":
    main()
