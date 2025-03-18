/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file bluenrg_lp_ble_backscatter.h
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @editor Stiliyan Dimanov (stiliyan.dimanov@jeevawireless.com)
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
 * BLE Bit Rate:
 *     Configure BLE_PHY to 125, 500, 1000, or 2000 for
 *     bit rates of 125 kbps, 500 kbps, 1 Mbps, or 2 Mbps
 *
 * BLE Channel
 *     Each packet is whitened for a particular channel, configured by BLE_CHANNEL
 *     Jeeva usually whitens packets for channel 0
 *     Beacons must be whitened for an advertising channel (37, 38, 39)
 *
 * Power Saving Options:
 *     IMU_POWER_OFF: turns off the IMU between packets
 *         Saves power, but takes time. Not possible for high packet rates.
 *     ENTER_DEEPSTOP: CPU enters DEEPSTOP mode between packets (otherwise, enters CPU HALT)
 *         Saves power, but can be a bit buggy/inconsistent
 *
 */

/**********************/
/* User configuration */
/**********************/
#define EP_LINK_ID		   0
//#define LED_BLINK          true
//#define USE_IMU            true
//#define IMU_POWER_OFF      true
//#define ENTER_DEEPSTOP     true

/* BLE defines */
#define BLE_CHANNEL			0
#define BLE_PHY 125 // 125, 500, 1000, or 2000
#define BLE_DL_CHANNEL 		0
#define BLE_ADV_ADDR_5  	0x00
#define BLE_ADV_ADDR_4		0x10
#define BLE_ADV_ADDR_3  	0x00
#define BLE_ADV_ADDR_2  	0x21
#define BLE_ADV_ADDR_1  	0x0d
#define BLE_ADV_ADDR_0  	0x01
#define BLE_ACCESS_ADDR		(uint32_t)(0x8E89BED6)

/* Radio defines */
#define INITIAL_CALIBRATION     FALSE
#define CALIBRATION_INTERVAL    0
#define HS_STARTUP_TIME         (uint16_t)(1)   /* High Speed start up time min value */
#define RX_WAKEUP_TIME          300     		/* The minimum is 230us */

/* Timings */
#define DL_UL_DELAY				400
#define UL_PKT_DURATION			900
#define RX_DISASSOC_TOUT		0xFFFFFF 		/* 16.7s */
#define RX_ASSOC_TOUT			1800			/* 2ms */
#define ASSOC_DISASSOC_THRESH	10
#define POLLINIG_RATE			25000

/* RTC tick is ~61.035 us */
#define RTC_DL_TO_RX_TIME		393
#define RTC_TOUT_TO_RX_TIME		373


#if EP_LINK_ID > 2
#error "Unsupported LinkId"
#endif

#ifdef IMU_POWER_OFF
#define XL_ODR LSM6DSO32_XL_ODR_6667Hz_HIGH_PERF
#define GY_ODR LSM6DSO32_GY_ODR_6667Hz_HIGH_PERF
#else
#define XL_ODR LSM6DSO32_XL_ODR_104Hz_NORMAL_MD
#define GY_ODR LSM6DSO32_GY_ODR_104Hz_NORMAL_MD

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

#endif /* __bluenrg_lp_ble_backscatter_H */
