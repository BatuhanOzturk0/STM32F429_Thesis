/*
 * imu_driver.c
 *
 *  Created on: 20 Aug 2026
 *      Author: batuhanozturk
 */

#include <stdio.h>
#include "imu_driver.h"

/**
 * @brief  Reads the WHO_AM_I register of the MPU9250
 * @param  hi2c: I2C handle to use (hi2c3, defined in main.c)
 * @param  who_am_i_value: pointer where the read value will be stored
 * @retval HAL_StatusTypeDef: HAL_OK on success, otherwise an error code
 */
HAL_StatusTypeDef IMU_ReadWhoAmI(I2C_HandleTypeDef *hi2c, uint8_t *who_am_i_value)
{
    HAL_StatusTypeDef status;

    status = HAL_I2C_Mem_Read(hi2c,
                               MPU9250_I2C_ADDR,
                               MPU9250_REG_WHO_AM_I,
                               I2C_MEMADD_SIZE_8BIT,
                               who_am_i_value,
                               1,
                               100);

    return status;
}

/**
 * @brief  Verifies the sensor connection and prints the result to the terminal
 * @param  hi2c: I2C handle to use
 * @retval uint8_t: 1 = connection successful, 0 = failed
 */
uint8_t IMU_CheckConnection(I2C_HandleTypeDef *hi2c)
{
    uint8_t who_am_i = 0;
    HAL_StatusTypeDef status;

    status = IMU_ReadWhoAmI(hi2c, &who_am_i);

    if (status == HAL_OK)
    {
        printf("MPU9250 WHO_AM_I read: 0x%02X\r\n", who_am_i);

        if (who_am_i == MPU9250_WHO_AM_I_DEFAULT ||
            who_am_i == MPU9250_WHO_AM_I_ALT1 ||
            who_am_i == MPU9250_WHO_AM_I_ALT2)
        {
            printf("Sensor verified: MPU9250 family (variant: 0x%02X)\r\n", who_am_i);
            return 1;
        }
        else
        {
            printf("WARNING: Unexpected WHO_AM_I value! Sensor may not be an MPU9250.\r\n");
            return 0;
        }
    }
    else
    {
        printf("ERROR: I2C read failed! HAL Status: %d\r\n", status);
        printf("Check: wiring, VCC/GND, SCL/SDA connections, AD0 pin.\r\n");
        return 0;
    }
}

/**
 * @brief  Reads raw accelerometer data (X, Y, Z) and converts to g-units
 * @param  hi2c: I2C handle to use
 * @param  accel_data: pointer to struct where results will be stored
 * @retval HAL_StatusTypeDef: HAL_OK on success, otherwise an error code
 */
HAL_StatusTypeDef IMU_ReadAccel(I2C_HandleTypeDef *hi2c, IMU_AccelData_t *accel_data)
{
    HAL_StatusTypeDef status;
    uint8_t raw_bytes[6];

    status = HAL_I2C_Mem_Read(hi2c,
                               MPU9250_I2C_ADDR,
                               MPU9250_REG_ACCEL_XOUT_H,
                               I2C_MEMADD_SIZE_8BIT,
                               raw_bytes,
                               6,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    accel_data->accel_x_raw = (int16_t)((raw_bytes[0] << 8) | raw_bytes[1]);
    accel_data->accel_y_raw = (int16_t)((raw_bytes[2] << 8) | raw_bytes[3]);
    accel_data->accel_z_raw = (int16_t)((raw_bytes[4] << 8) | raw_bytes[5]);

    accel_data->accel_x_g = accel_data->accel_x_raw / MPU9250_ACCEL_SENSITIVITY_2G;
    accel_data->accel_y_g = accel_data->accel_y_raw / MPU9250_ACCEL_SENSITIVITY_2G;
    accel_data->accel_z_g = accel_data->accel_z_raw / MPU9250_ACCEL_SENSITIVITY_2G;

    return HAL_OK;
}

/**
 * @brief  Prints raw and converted accelerometer data to the terminal
 * @param  accel_data: pointer to the accelerometer data to print
 * @retval None
 */
void IMU_PrintAccel(const IMU_AccelData_t *accel_data)
{
    printf("Accel raw [X:%6d Y:%6d Z:%6d]  ->  g [X:%+.3f Y:%+.3f Z:%+.3f]\r\n",
           accel_data->accel_x_raw, accel_data->accel_y_raw, accel_data->accel_z_raw,
           accel_data->accel_x_g, accel_data->accel_y_g, accel_data->accel_z_g);
}

/**
 * @brief  Reads raw gyroscope data (X, Y, Z) and converts to degrees/second
 * @param  hi2c: I2C handle to use
 * @param  gyro_data: pointer to struct where results will be stored
 * @retval HAL_StatusTypeDef: HAL_OK on success, otherwise an error code
 */
HAL_StatusTypeDef IMU_ReadGyro(I2C_HandleTypeDef *hi2c, IMU_GyroData_t *gyro_data)
{
    HAL_StatusTypeDef status;
    uint8_t raw_bytes[6];

    status = HAL_I2C_Mem_Read(hi2c,
                               MPU9250_I2C_ADDR,
                               MPU9250_REG_GYRO_XOUT_H,
                               I2C_MEMADD_SIZE_8BIT,
                               raw_bytes,
                               6,
                               100);

    if (status != HAL_OK)
    {
        return status;
    }

    gyro_data->gyro_x_raw = (int16_t)((raw_bytes[0] << 8) | raw_bytes[1]);
    gyro_data->gyro_y_raw = (int16_t)((raw_bytes[2] << 8) | raw_bytes[3]);
    gyro_data->gyro_z_raw = (int16_t)((raw_bytes[4] << 8) | raw_bytes[5]);

    gyro_data->gyro_x_dps = gyro_data->gyro_x_raw / MPU9250_GYRO_SENSITIVITY_250DPS;
    gyro_data->gyro_y_dps = gyro_data->gyro_y_raw / MPU9250_GYRO_SENSITIVITY_250DPS;
    gyro_data->gyro_z_dps = gyro_data->gyro_z_raw / MPU9250_GYRO_SENSITIVITY_250DPS;

    return HAL_OK;
}

/**
 * @brief  Prints raw and converted gyroscope data to the terminal
 * @param  gyro_data: pointer to the gyroscope data to print
 * @retval None
 */
void IMU_PrintGyro(const IMU_GyroData_t *gyro_data)
{
    printf("Gyro  raw [X:%6d Y:%6d Z:%6d]  -> dps [X:%+.3f Y:%+.3f Z:%+.3f]\r\n",
           gyro_data->gyro_x_raw, gyro_data->gyro_y_raw, gyro_data->gyro_z_raw,
           gyro_data->gyro_x_dps, gyro_data->gyro_y_dps, gyro_data->gyro_z_dps);
}
