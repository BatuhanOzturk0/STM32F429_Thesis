/*
 * dma_handler.h
 *
 *  Created on: 22 Aug 2026
 *      Author: batuhanozturk
 */
#ifndef INC_DMA_HANDLER_H_
#define INC_DMA_HANDLER_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* MPU9250 register map note:
 * 0x3B-0x40 Accel X,Y,Z (6 bytes)
 * 0x41-0x42 Temperature (2 bytes)
 * 0x43-0x48 Gyro X,Y,Z  (6 bytes)
 * These registers are contiguous, so a single 14-byte burst read
 * starting at ACCEL_XOUT_H captures a full sample (accel + temp + gyro)
 * in one DMA transaction. */
#define DMA_MPU9250_I2C_ADDR       (0x68 << 1)
#define DMA_MPU9250_REG_ACCEL_XOUT_H  0x3B
#define DMA_SAMPLE_SIZE_BYTES       14U

/* Circular buffer sizing.
 * NOTE: Because I2C DMA reads are transaction-based (not a single
 * continuous stream like ADC/UART), the DMA peripheral itself does not
 * provide a native "half complete" event for this usage pattern. We
 * therefore track half/full buffer positions ourselves in software,
 * inside HAL_I2C_MasterRxCpltCallback, based on the write index. */
#define DMA_SAMPLES_PER_HALF        20U
#define DMA_BUFFER_SAMPLE_COUNT     (DMA_SAMPLES_PER_HALF * 2U)   /* 40 samples total */

/* Byte offsets of each field inside one 14-byte raw sample */
#define DMA_OFFSET_ACCEL_X   0
#define DMA_OFFSET_ACCEL_Y   2
#define DMA_OFFSET_ACCEL_Z   4
#define DMA_OFFSET_TEMP      6
#define DMA_OFFSET_GYRO_X    8
#define DMA_OFFSET_GYRO_Y    10
#define DMA_OFFSET_GYRO_Z    12

typedef struct
{
    uint8_t raw_samples[DMA_BUFFER_SAMPLE_COUNT][DMA_SAMPLE_SIZE_BYTES];

    volatile uint32_t write_index;            /* next slot the DMA will write into */
    volatile uint32_t total_samples_written;  /* running total, never reset */

    volatile uint8_t  half_ready_flag;        /* set when samples [0 .. HALF-1] are ready */
    volatile uint8_t  full_ready_flag;        /* set when samples [HALF .. FULL-1] are ready */

    volatile uint32_t overflow_count;         /* incremented if a half was overwritten
                                                  before the main loop processed it */
    volatile uint8_t  half_processed;         /* main loop sets this once it has read
                                                  the half that half_ready_flag pointed to */
    volatile uint8_t  full_processed;
} DMA_CircularBuffer_t;

/* Extracted, human-readable form of one raw sample */
typedef struct
{
    int16_t accel_x_raw;
    int16_t accel_y_raw;
    int16_t accel_z_raw;
    int16_t temp_raw;
    int16_t gyro_x_raw;
    int16_t gyro_y_raw;
    int16_t gyro_z_raw;
} DMA_Sample_t;

/* Public API */
void DMA_Handler_Init(I2C_HandleTypeDef *hi2c);
void DMA_Handler_StartFirstRead(void);
void DMA_Handler_ParseSample(const uint8_t *raw, DMA_Sample_t *out);
DMA_CircularBuffer_t *DMA_Handler_GetBuffer(void);

#endif /* INC_DMA_HANDLER_H_ */
