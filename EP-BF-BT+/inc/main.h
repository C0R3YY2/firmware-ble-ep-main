/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file bluenrg_lp_ble_backscatter.h
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Header file for BlueNRG-LP backscatter
 * @date 2022-05-17
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#ifndef __MAIN_H
#define __MAIN_H

/*
 * Application parameters
 *
 * BLE Channel
 *     Each packet is whitened for a particular channel, configured by BLE_CHANNEL
 *     Jeeva usually whitens packets for channel 0
 *     Beacons must be whitened for an advertising channel (37, 38, 39)
 *
 */

/**********************/
/* User configuration */
/**********************/
#define BLE_CHANNEL 37

/* Other application defines */
#define RTC_DELAY_1HZ     16384
#define RTC_DELAY_10HZ    1638
#define RTC_DELAY_60HZ    273
#define RTC_DELAY_90HZ    182
#define RTC_DELAY_120HZ   137
#define RTC_DELAY_144HZ   114
#define RTC_DELAY_200HZ   82
#define RTC_DELAY_240HZ   68
#define RTC_DELAY_360HZ   46
#define RTC_DELAY_480HZ   34
#define RTC_DELAY_720HZ   23
#define RTC_DELAY_1000HZ  16
#define RTC_DELAY_300kbps 13 // define for 300 kbps benchmarking

#define XL_ODR LSM6DSO32_XL_ODR_3333Hz_HIGH_PERF
#define GY_ODR LSM6DSO32_GY_ODR_3333Hz_HIGH_PERF

#define XSTR(x) STR(x)
#define STR(x) #x
#pragma message "Whitened for channel number " XSTR(BLE_CHANNEL)

#define BLE_PACKET_TYPE UNCODED_1MBPS
#define jv_bsc_upscale(x, y, z) jv_bsc_upscale_1Mbps(x, y, z)

/* Board pin defines */
#define LED_GPIO
#define LED_GPIO_PORT GPIOB
#define LED_GPIO_PIN  GPIO_PIN_1

#define SW_EN
#define SW_EN_PORT    GPIOB
#define SW_EN_PIN     GPIO_PIN_2

#define SW_MOD
#define SW_MOD_PORT   GPIOB
#define SW_MOD_PIN    LL_GPIO_PIN_14

#define DBG_GPIO
#define DBG_GPIO_PORT GPIOA
#define DBG_GPIO_PIN  LL_GPIO_PIN_11

#endif /* __MAIN_H */
