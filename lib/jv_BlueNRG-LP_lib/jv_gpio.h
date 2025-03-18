/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_gpio.h
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @editor Stiliyan Dimanov (stiliyan.dimanov@jeevawireless.com)
 * @brief Library for GPIO functions
 * @date 2022-09-13
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#ifndef INC_JV_GPIO_H_
#define INC_JV_GPIO_H_

#include "rf_driver_hal.h"
#include "rf_driver_ll_gpio.h"

/**
 * @brief Init GPIOs for pull down resistors
 *
 */
void GPIO_Init(void);

/**
 * @brief Turn on LED
 *
 */
void led_on(void);

/**
 * @brief Turn off LED
 *
 */
void led_off(void);

/**
 * @brief Toggle LED
 *
 */
void led_toggle(void);

/**
 * @brief Manipulate Output
 *
 */
#define jv_gpioSet(pin)		(HAL_GPIO_WritePin(pin##_PORT, pin##_PIN, GPIO_PIN_SET))
#define jv_gpioReset(pin)	(HAL_GPIO_WritePin(pin##_PORT, pin##_PIN, GPIO_PIN_RESET))
#define jv_gpioToggle(pin)	(HAL_GPIO_TogglePin(pin##_PORT, pin##_PIN))

/**
 * @brief Manipulate Input
 *
 */
#define jv_gpioRead(pin)	(HAL_GPIO_ReadPin(pin##_PORT, pin##_PIN))

#endif /* INC_JV_GPIO_H_ */
