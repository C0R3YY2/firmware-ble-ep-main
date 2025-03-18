/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_rtc.h
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @editor Stiliyan Dimanov (stiliyan.dimanov@jeevawireless.com)
 * @brief Header for using the RTC on BlueNRG-LP
 * @date 2022-10-15
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#ifndef INC_RTC_H_
#define INC_RTC_H_

#include <stdio.h>
#include <stdbool.h>

/**
 * @brief RTC init function
 *
 */
void RTC_WakeupInit(void);

/**
 * @brief Configures RTC wakeup period and starts the timer
 *
 * @param time wakeup period, in clock cycles (for 2048 Hz clock frequency)
 */
void SetRTC_WakeupTimeout(uint32_t time);

/**
 * @brief Disables the active RTC wakeup timer
 *
 */
void DisableRTC_WakeupTimeout(void);

/**
 * @brief Check if RTC_WakeupTimeout is expired
 *
 */
bool RTC_WakeupTimeout_Expired(void);

#endif /* INC_RTC_H_ */
