/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_imu.h
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Header for working with the IMU
 * @date 2022-05-20
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#ifndef INC_IMU_H_
#define INC_IMU_H_

#include <main.h>
#include "LSM6DSO32/lsm6dso32_reg.h"
#include "rf_driver_hal.h"
#include "rf_driver_hal_power_manager.h"
#include "rf_device_hal_conf.h"

extern SPI_HandleTypeDef hspiMaster;
extern uint8_t aTxBuffer[];
extern uint8_t ubNbDataToTransmit;
extern uint8_t aRxBuffer[];

void imu_interrupt_init(void);
void imu_interrupt_enable(void);
void imu_interrupt_disable(void);
void imu_EXTI_callback(uint32_t Line);

/**
 * @brief Initalize the IMU for backscatter demo
 *
 * @param dev_ctx Device handle
 * @return int32_t 0 if successful, else -1
 */
int32_t imu_init(stmdev_ctx_t *dev_ctx);

/**
 * @brief Read data from the IMU and store it in a buffer for transmission
 *
 * @param dev_ctx Device handle
 * @param buf Buffer to store 18 bytes of IMU data
 * @return int32_t 0 if successful, else -1
 */
int32_t imu_get_packet_data(stmdev_ctx_t *dev_ctx, uint16_t *buf);

/**
 * @brief Power off the XL and GY independently
 *
 * @param dev_ctx Device handle
 * @return int32_t 0 if successful, else -1
 */
int32_t imu_power_off(stmdev_ctx_t *dev_ctx);

/**
 * @brief Power on the XL and GY independently
 *
 * @param dev_ctx Device handle
 * @return int32_t 0 if successful, else -1
 */
int32_t imu_power_on(stmdev_ctx_t *dev_ctx);

/**
 * @brief Platform-specific SPI write function for IMU
 *
 * @param handle Device handle
 * @param Reg Reg to write to
 * @param Bufp Data to write to reg
 * @param len Length of data
 * @return int32_t 0 if successful, else -1
 */
int32_t platform_write(void *handle, uint8_t Reg, const uint8_t *Bufp, uint16_t len);

/**
 * @brief Platform-specific SPI read function for IMU
 *
 * @param handle Device handle
 * @param Reg Reg to read from
 * @param Bufp Buffer to store read data
 * @param len Length of data to read
 * @return int32_t 0 if successful, else -1
 */
int32_t platform_read(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len);

/**
 * @brief SPI_MASTER Initialization Function
 *
 */
void MX_SPI_MASTER_Init(void);

/**
 * @brief SPI MSP Initialization
 * This function configures the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi);

/**
 * @brief SPI MSP De-Initialization
 * This function freeze the hardware resources used in this example
 * @param hspi: SPI handle pointer
 * @retval None
 */
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi);

#endif /* INC_IMU_H_ */
