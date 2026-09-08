# Real-Time IMU to DSP Processing Pipeline on STM32F429I-DISC1 under FreeRTOS

MSc Dissertation project (Gisma University of Applied Sciences) evaluating how **FreeRTOS task scheduling configuration** affects the latency, CPU load, and deadline behavior of a real time **IMU sensor → digital signal processing** pipeline on an ARM Cortex-M4 microcontroller.

**Author:** Batuhan Öztürk

**Supervisor:** Dr. Mohamad Hoseini

**Module:** M598 — MSc Dissertation

**Board:** STM32F429I-DISC1 (STM32F429ZIT6, Cortex-M4 @ 180 MHz)

**Sensor:** MPU9250 9 axis IMU (I2C3)

---

## Overview

This project implements a four layer, real time embedded pipeline that acquires motion data from an IMU and processes it with a CMSIS DSP filter chain (FFT, FIR, IIR, RMS, Peak detection) under FreeRTOS. Three FreeRTOS task priority configurations are benchmarked against one another to determine whether scheduling policy has a measurable effect on end to end latency, CPU utilization, and deadline compliance.

**Key finding:** the system is significantly over provisioned for its workload  DSP processing times are on the order of microseconds to milliseconds, while deadlines are on the order of seconds. As a result, scheduling configuration has **no measurable effect** on latency, CPU load, or deadline misses across all three configurations tested (latency variation <1.5%, zero deadline misses in every configuration).

## Architecture

The pipeline is organized into four layers:

| Layer | Description |
|---|---|
| **1 — Sensor Driver** | HAL based I2C3 driver for the MPU9250, reading accelerometer, gyroscope, and temperature in a single 14 byte burst transaction (registers 0x3B–0x48). |
| **2 — Buffering** | Software managed circular double buffer (half/full callback chaining), since I2C's transaction based nature is unsuited to hardware Circular DMA. |
| **3 — DSP Processing** | CMSIS DSP based signal chain: 256/512/1024-point FFT, 31 tap FIR (Hamming, 20 Hz cutoff), 4th order Butterworth IIR (biquad DF2T, 20 Hz cutoff), RMS, and peak detection. |
| **4 — Scheduling** | FreeRTOS (CMSIS-OS v1) tasks  IMU_Task, DSP_Task, UART_Task  evaluated under three distinct priority configurations. |

### Scheduling configurations evaluated

| Config | IMU_Task | DSP_Task | UART_Task |
|---|---|---|---|
| **A** | AboveNormal | Normal | BelowNormal |
| **B** | Normal | Normal | Normal |
| **C** | BelowNormal | Normal | AboveNormal |

## Repository Structure

├── Core/               Application source: drivers, DSP module, FreeRTOS tasks, main.c

├── Drivers/            ST HAL / CMSIS drivers

├── Middlewares/        FreeRTOS, CMSIS-DSP

├── USB_HOST/           USB host stack (board support)

├── docs/               Evidence and documentation (screenshots, CPU load captures)

│   └── evidence/       CPU_LOAD_{A,B,C}_{START,FINAL}.png

├── python/             Host side analysis & plotting scripts (fft_plot.py, fir_plot.py, iir_plot.py, generate_fir_coeffs.py, generate_iir_coeffs.py)

├── results/            Final measurement outputs tables and charts (300 DPI)

│   ├── TABLE_master_summary.png

│   ├── TABLE_latency_summary.png

│   ├── TABLE_cpu_load_summary.png

│   ├── CHART_latency_by_config.png

│   ├── CHART_latency_deadline_headroom.png

│   └── CHART_cpu_load_by_config.png

├── Thesis_stm32F429.ioc      STM32CubeMX project configuration

├── STM32F429ZITX_FLASH.ld    Linker script (Flash)

├── STM32F429ZITX_RAM.ld      Linker script (RAM)

└── .cproject / .project      STM32CubeIDE project files

## Hardware Setup

| Component | Detail |
|---|---|
| MCU board | STM32F429I-DISC1 |
| IMU | MPU9250 (I2C, 3 axis accel + 3 axis gyro + temp) |
| Interface | I2C3 — SCL: PA8, SDA: PC9 |
| Connection | Breadboard |

## Toolchain

- **IDE:** STM32CubeIDE 2.2.0
- **Configurator:** STM32CubeMX (standalone  CubeIDE 2.0+ removed the integrated plugin, so code is generated in CubeMX and imported separately)
- **RTOS:** FreeRTOS 10.3.1 (CMSIS-OS v1 wrapper)
- **DSP:** CMSIS-DSP
- **Host analysis:** Python 3 (pyserial, matplotlib, scipy)
- **Measurement:** DWT cycle counter (DWT->CYCCNT) for microsecond resolution latency; HAL_GetTick() based runtime statistics for CPU load (avoids the ~23.86 s DWT overflow during multi minute test runs)

## Building the Project

1. Clone the repository.
2. Open STM32CubeMX, load Thesis_stm32F429.ioc if peripheral configuration needs to be regenerated (not required to simply build/flash).
3. Open STM32CubeIDE and import the project (File → Import → Existing Projects into Workspace). Ensure the CubeIDE workspace folder does not overlap the project folder.
4. Build and flash to the STM32F429I-DISC1 via the onboard ST-LINK.
5. Connect a serial terminal to the ST-LINK VCP at 115200 baud (e.g. screen /dev/tty.usbmodemXXXXX 115200 on macOS/Linux) to view live IMU/DSP/latency output.

## Results Summary

Across all three scheduling configurations:

- **Zero deadline misses** in every configuration.
- **Latency variation < 1.5%** across configurations (FFT-256 ≈ 1.90 ms, FIR ≈ 8.85 ms, IIR ≈ 7.07 ms, RMS ≈ 6.9 ms, Peak ≈ 6.9 ms).
- **CPU load** stable across configurations (IMU_Task ≈ 17–18%, IDLE ≈ 79%, DSP_Task/UART_Task/default task ≈ 1–2% each).

Full tables and charts are available in results/, and CPU load capture evidence is in docs/evidence/.

## Notable Implementation Details

- I2C's transaction based nature makes hardware Circular DMA unsuitable; acquisition instead uses Normal DMA mode with software half/full buffer tracking via callback chaining.
- I2C bus lockup after an MCU reset mid transaction (MPU9250 holds SDA low) is resolved with a bit banging I2C3_BusRecovery() routine, called before peripheral re initialization.
- The DSP processing chain (FFT/FIR/IIR/RMS/Peak) executes inside IMU_Task; DSP_Task acts as a lightweight trigger relay. This architectural detail is a key factor in why latency is stable across scheduling configurations, and is documented and discussed as such in the dissertation rather than altered post hoc.

## Limitations

- Single board study (no hardware replication).
- I2C was deliberately chosen over SPI for sensor interfacing; this trade off is discussed in the dissertation.
- No environmental/temperature control during testing.
- Power consumption and Flash/RAM footprint were not measured.

## License

This repository accompanies an academic MSc dissertation submitted to Gisma University of Applied Sciences. Code is provided for reference and reproducibility purposes.
