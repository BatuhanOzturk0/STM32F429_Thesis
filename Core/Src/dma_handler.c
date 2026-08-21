/*
 * dma_handler.c
 *
 *  Created on: 22 Aug 2026
 *      Author: batuhanozturk
 */

#include "dma_handler.h"

/* Module-private state. Not exposed directly - accessed via DMA_Handler_GetBuffer(). */
static I2C_HandleTypeDef   *dma_i2c_handle;
static DMA_CircularBuffer_t dma_buffer;

/**
 * @brief  Kicks off the next single-sample DMA read into the buffer slot
 *         currently pointed to by write_index. Called both at startup and
 *         at the end of every completed transfer, to keep the chain going.
 * @retval None
 */
static void DMA_Handler_StartNextRead(void)
{
    HAL_I2C_Mem_Read_DMA(dma_i2c_handle,
                          DMA_MPU9250_I2C_ADDR,
                          DMA_MPU9250_REG_ACCEL_XOUT_H,
                          I2C_MEMADD_SIZE_8BIT,
                          dma_buffer.raw_samples[dma_buffer.write_index],
                          DMA_SAMPLE_SIZE_BYTES);
}

/**
 * @brief  Initializes the circular buffer state. Must be called once
 *         before DMA_Handler_StartFirstRead().
 * @param  hi2c: I2C handle to use for all DMA reads (hi2c3 in this project)
 * @retval None
 */
void DMA_Handler_Init(I2C_HandleTypeDef *hi2c)
{
    dma_i2c_handle = hi2c;

    dma_buffer.write_index           = 0;
    dma_buffer.total_samples_written = 0;
    dma_buffer.half_ready_flag       = 0;
    dma_buffer.full_ready_flag       = 0;
    dma_buffer.overflow_count        = 0;
    dma_buffer.half_processed        = 1; /* nothing to process yet */
    dma_buffer.full_processed        = 1;
}

/**
 * @brief  Starts the very first DMA read, which then keeps re-triggering
 *         itself from inside HAL_I2C_MemRxCpltCallback below.
 * @retval None
 */
void DMA_Handler_StartFirstRead(void)
{
    DMA_Handler_StartNextRead();
}

/**
 * @brief  Returns a pointer to the module's circular buffer, so main.c
 *         can inspect flags/counters and read out sample data.
 * @retval Pointer to the internal DMA_CircularBuffer_t instance
 */
DMA_CircularBuffer_t *DMA_Handler_GetBuffer(void)
{
    return &dma_buffer;
}

/**
 * @brief  Converts one raw 14-byte sample (big-endian register pairs)
 *         into signed 16-bit fields.
 * @param  raw: pointer to a 14-byte raw sample from the circular buffer
 * @param  out: pointer to the struct where parsed values will be stored
 * @retval None
 */
void DMA_Handler_ParseSample(const uint8_t *raw, DMA_Sample_t *out)
{
    out->accel_x_raw = (int16_t)((raw[DMA_OFFSET_ACCEL_X] << 8) | raw[DMA_OFFSET_ACCEL_X + 1]);
    out->accel_y_raw = (int16_t)((raw[DMA_OFFSET_ACCEL_Y] << 8) | raw[DMA_OFFSET_ACCEL_Y + 1]);
    out->accel_z_raw = (int16_t)((raw[DMA_OFFSET_ACCEL_Z] << 8) | raw[DMA_OFFSET_ACCEL_Z + 1]);
    out->temp_raw    = (int16_t)((raw[DMA_OFFSET_TEMP]    << 8) | raw[DMA_OFFSET_TEMP + 1]);
    out->gyro_x_raw  = (int16_t)((raw[DMA_OFFSET_GYRO_X]  << 8) | raw[DMA_OFFSET_GYRO_X + 1]);
    out->gyro_y_raw  = (int16_t)((raw[DMA_OFFSET_GYRO_Y]  << 8) | raw[DMA_OFFSET_GYRO_Y + 1]);
    out->gyro_z_raw  = (int16_t)((raw[DMA_OFFSET_GYRO_Z]  << 8) | raw[DMA_OFFSET_GYRO_Z + 1]);
}

/**
 * @brief  HAL callback: fires once per completed 14-byte DMA sample read.
 *         Note: HAL_I2C_Mem_Read_DMA() completes via HAL_I2C_MemRxCpltCallback,
 *         NOT HAL_I2C_MasterRxCpltCallback (that one is only for
 *         HAL_I2C_Master_Receive_DMA). Using the wrong name means the HAL
 *         library silently never calls our handler.
 *         Advances the write index, marks half/full readiness, detects
 *         overflow (a half being overwritten before the main loop
 *         processed it), and immediately re-arms the next DMA read so
 *         the acquisition chain never stalls.
 * @param  hi2c: I2C handle that completed the transfer
 * @retval None
 */
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance != I2C3)
    {
        return;
    }

    dma_buffer.total_samples_written++;
    dma_buffer.write_index = (dma_buffer.write_index + 1) % DMA_BUFFER_SAMPLE_COUNT;

    if (dma_buffer.write_index == DMA_SAMPLES_PER_HALF)
    {
        /* We just finished writing the first half [0 .. HALF-1] */
        if (dma_buffer.half_ready_flag == 1 && dma_buffer.half_processed == 0)
        {
            dma_buffer.overflow_count++; /* main loop never read the previous half */
        }
        dma_buffer.half_ready_flag = 1;
        dma_buffer.half_processed  = 0;
    }
    else if (dma_buffer.write_index == 0)
    {
        /* We just finished writing the second half [HALF .. FULL-1] and wrapped */
        if (dma_buffer.full_ready_flag == 1 && dma_buffer.full_processed == 0)
        {
            dma_buffer.overflow_count++;
        }
        dma_buffer.full_ready_flag = 1;
        dma_buffer.full_processed  = 0;
    }

    DMA_Handler_StartNextRead();
}

/**
 * @brief  HAL callback: fires on any I2C error during a DMA transfer
 *         (e.g. NACK, bus error, arbitration lost). Without this, a
 *         single transient I2C glitch would stall the acquisition chain
 *         forever, since nothing else re-arms the next DMA read.
 * @param  hi2c: I2C handle that reported the error
 * @retval None
 */
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
    if (hi2c->Instance != I2C3)
    {
        return;
    }

    /* Attempt to recover the chain by re-arming the next read at the
     * same write_index (the failed sample slot is simply overwritten). */
    DMA_Handler_StartNextRead();
}
