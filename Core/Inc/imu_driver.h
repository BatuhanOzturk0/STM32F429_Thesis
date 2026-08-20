/*
 * imu_driver.h
 *
 *  Created on: 20 Aug 2026
 *      Author: batuhanozturk
 */

#ifndef INC_IMU_DRIVER_H_
#define INC_IMU_DRIVER_H_

#include "stm32f4xx_hal.h"
#include <stdint.h>

/* MPU9250 I2C 7-bit address (AD0 = GND -> 0x68) */
#define MPU9250_I2C_ADDR        (0x68 << 1)

/* MPU9250 Register Addresses */
#define MPU9250_REG_WHO_AM_I     0x75
#define MPU9250_REG_ACCEL_XOUT_H 0x3B
#define MPU9250_REG_GYRO_XOUT_H  0x43

/* Expected WHO_AM_I values (may vary by chip variant) */
#define MPU9250_WHO_AM_I_DEFAULT  0x71
#define MPU9250_WHO_AM_I_ALT1     0x70
#define MPU9250_WHO_AM_I_ALT2     0x73

/* Accelerometer full-scale range: +/-2g -> 1g corresponds to 16384 raw units */
#define MPU9250_ACCEL_SENSITIVITY_2G  16384.0f

/* Gyroscope full-scale range: +/-250 dps -> 1 dps corresponds to 131 raw units */
#define MPU9250_GYRO_SENSITIVITY_250DPS  131.0f

/* Holds raw and converted (g-unit) accelerometer readings for one sample */
typedef struct
{
    int16_t accel_x_raw;
    int16_t accel_y_raw;
    int16_t accel_z_raw;
    float   accel_x_g;
    float   accel_y_g;
    float   accel_z_g;
} IMU_AccelData_t;

/* Holds raw and converted (degrees/second) gyroscope readings for one sample */
typedef struct
{
    int16_t gyro_x_raw;
    int16_t gyro_y_raw;
    int16_t gyro_z_raw;
    float   gyro_x_dps;
    float   gyro_y_dps;
    float   gyro_z_dps;
} IMU_GyroData_t;

/* Function Prototypes */
HAL_StatusTypeDef IMU_ReadWhoAmI(I2C_HandleTypeDef *hi2c, uint8_t *who_am_i_value);
uint8_t IMU_CheckConnection(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef IMU_ReadAccel(I2C_HandleTypeDef *hi2c, IMU_AccelData_t *accel_data);
void IMU_PrintAccel(const IMU_AccelData_t *accel_data);
HAL_StatusTypeDef IMU_ReadGyro(I2C_HandleTypeDef *hi2c, IMU_GyroData_t *gyro_data);
void IMU_PrintGyro(const IMU_GyroData_t *gyro_data);

#endif /* INC_IMU_DRIVER_H_ */
