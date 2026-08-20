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
#define MPU9250_I2C_ADDR        (0x68 << 1)  /* HAL expects an 8-bit address, so shift left by 1 bit */

/* MPU9250 Register Addresses */
#define MPU9250_REG_WHO_AM_I    0x75

/* Expected WHO_AM_I values (may vary by chip variant) */
#define MPU9250_WHO_AM_I_DEFAULT  0x71
#define MPU9250_WHO_AM_I_ALT1     0x70
#define MPU9250_WHO_AM_I_ALT2     0x73

/* Function Prototypes */
HAL_StatusTypeDef IMU_ReadWhoAmI(I2C_HandleTypeDef *hi2c, uint8_t *who_am_i_value);
uint8_t IMU_CheckConnection(I2C_HandleTypeDef *hi2c);

#endif /* INC_IMU_DRIVER_H_ */
