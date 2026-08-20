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
