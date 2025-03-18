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
#define XSTR(x) STR(x)
#define STR(x) #x
/*
 * Application parameters
 *
 * Packet Rate:
 *     To configure output packet rate, configure RTC_DELAY
 *
 * BLE Bit Rate:
 *     Configure BLE_PHY to 125, 500, 1000, or 2000 for
 *     bit rates of 125 kbps, 500 kbps, 1 Mbps, or 2 Mbps
 *
 * BLE Channel
 *     Each packet is whitened for a particular channel, configured by BLE_CHANNEL
 *     Jeeva usually whitens packets for channel 0
 *     Beacons must be whitened for an advertising channel (37, 38, 39)
 *
 *
 * LED Blink:
 *     Enable periodic blinking of onboard LED by uncommenting LED_BLINK
 *     LED turns on every LED_ON_THRESHOLD packets
 *     LED then turns off after LED_OFF_THRESHOLD packets
 *
 * Power Saving Options:
 *     IMU_POWER_OFF: turns off the IMU between packets
 *         Saves power, but takes time. Not possible for high packet rates.
 *     ENTER_DEEPSTOP: CPU enters DEEPSTOP mode between packets (otherwise, enters CPU HALT)
 *         Saves power, but takes time. Not possible for high packet rates.
 *
 */

/**********************/
/* User configuration */
/**********************/
#define RTC_DELAY            RTC_DELAY_120HZ
#define BLE_CHANNEL   0
#define BLE_PHY 1000 // 125, 500, 1000, or 2000
#define LED_BLINK            true
#define LED_ON_THRESHOLD     220
#define LED_OFF_THRESHOLD    2
//#define USE_IMU              true
// #define IMU_POWER_OFF        true
// #define ENTER_DEEPSTOP       true

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
#define RTC_DELAY_2000HZ  8
#define RTC_DELAY_300kbps 13 // define for 300 kbps benchmarking

#if defined IMU_POWER_OFF && RTC_DELAY <= RTC_DELAY_480HZ
#error "Max data rate in IMU_POWER_OFF is 360 Hz"
#endif

#ifdef IMU_POWER_OFF
#define XL_ODR LSM6DSO32_XL_ODR_6667Hz_HIGH_PERF
#define GY_ODR LSM6DSO32_GY_ODR_6667Hz_HIGH_PERF
#else
#if RTC_DELAY == RTC_DELAY_1HZ || RTC_DELAY == RTC_DELAY_10HZ
#define XL_ODR LSM6DSO32_XL_ODR_52Hz_LOW_PW
#define GY_ODR LSM6DSO32_GY_ODR_52Hz_LOW_PW
#elif RTC_DELAY == RTC_DELAY_60HZ || RTC_DELAY == RTC_DELAY_90HZ
#define XL_ODR LSM6DSO32_XL_ODR_104Hz_NORMAL_MD
#define GY_ODR LSM6DSO32_GY_ODR_104Hz_NORMAL_MD
#elif RTC_DELAY == RTC_DELAY_120HZ || RTC_DELAY == RTC_DELAY_144HZ || RTC_DELAY == RTC_DELAY_200HZ
#define XL_ODR LSM6DSO32_XL_ODR_208Hz_NORMAL_MD
#define GY_ODR LSM6DSO32_GY_ODR_208Hz_NORMAL_MD
#elif RTC_DELAY == RTC_DELAY_240HZ || RTC_DELAY == RTC_DELAY_360HZ
#define XL_ODR LSM6DSO32_XL_ODR_417Hz_HIGH_PERF
#define GY_ODR LSM6DSO32_GY_ODR_417Hz_HIGH_PERF
#elif RTC_DELAY == RTC_DELAY_480HZ || RTC_DELAY == RTC_DELAY_720HZ
#define XL_ODR LSM6DSO32_XL_ODR_833Hz_HIGH_PERF
#define GY_ODR LSM6DSO32_GY_ODR_833Hz_HIGH_PERF
#else
#define XL_ODR LSM6DSO32_XL_ODR_3333Hz_HIGH_PERF
#define GY_ODR LSM6DSO32_GY_ODR_3333Hz_HIGH_PERF
#endif
#endif

#pragma message "Whitened for channel number " XSTR(BLE_CHANNEL)
#if BLE_PHY == 125
#pragma message "BLE datarate is 125 kbps"
#define BLE_PACKET_TYPE CODED_S8
#define BLE_CODED
#elif BLE_PHY == 500
#pragma message "BLE datarate is 500 kbps"
#define BLE_PACKET_TYPE CODED_S2
#define BLE_CODED
#elif BLE_PHY == 1000
#pragma message "BLE datarate is 1 Mbps"
#define BLE_PACKET_TYPE UNCODED_1MBPS
#elif BLE_PHY == 2000
#define STR(x) #x
#pragma message "BLE datarate is 2 Mbps"
#define BLE_PACKET_TYPE UNCODED_2MBPS
#else
#error "Invalid BLE_PHY" XSTR(BLE_PHY)
#endif

#if BLE_PHY < 2000
#define jv_bsc_upscale(x, y, z) jv_bsc_upscale_1Mbps(x, y, z)
#else
#define jv_bsc_upscale(x, y, z) jv_bsc_upscale_2Mbps(x, y, z)
#endif

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
