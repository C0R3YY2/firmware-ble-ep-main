/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_delayUS.h
 * @author Stiliyan Dimanov (stiliyan.dimanov@jeevawireless.com)
 * @brief Library for Delay in microseconds functions
 * @date 2022-10-11
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#ifndef INC_JV_DELAYUS_H
#define INC_JV_DELAYUS_H

/**
* @brief jv_delayUS Initialization Function
* @param None
* @retval success = 0, error = -1
*/
int jv_delayUS_Init(void);

/**
* @brief  jv_delayUS Function
* @param  us : time in microseconds
* @retval None
*/
void jv_delayUS(uint16_t us);

#endif /* INC_JV_DELAYUS_H */
