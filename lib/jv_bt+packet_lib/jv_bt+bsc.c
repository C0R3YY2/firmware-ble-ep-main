/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_bt+bsc.c
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Library functions for preparing a BLE packet to be backscattered
 * @date 2022-10-14
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#include "jv_bt+bsc.h"

#define CONCAT(a, b, c) a##b##c

#ifndef BLE_OFFSET
#define BLE_OFFSET 35
#endif

#if BLE_OFFSET == -15
/* -1.5 MHz offset */
/*                                      Even  Odd  */
#define ZERO_ENCODING_1Mbps  CONCAT(0x, 00ff, 00ff)
#define ONE_ENCODING_1Mbps   CONCAT(0x, f0f0, 00ff)
#define TWO_ENCODING_1Mbps   CONCAT(0X, 00ff, f0f0)
#define THREE_ENCODING_1Mbps CONCAT(0X, f0f0, f0f0)

#elif BLE_OFFSET == -30
/* -3 MHz offset */
/*                                      Even  Odd  */
#define ZERO_ENCODING_1Mbps  CONCAT(0x, 711c, 8ee3)
#define ONE_ENCODING_1Mbps   CONCAT(0x, 9831, 8ee3)
#define TWO_ENCODING_1Mbps   CONCAT(0X, 711c, 67ce)
#define THREE_ENCODING_1Mbps CONCAT(0X, 9831, 67ce)

#elif BLE_OFFSET == -35
/* -3.5 MHz offset */
/*                                      Even  Odd  */
#define ZERO_ENCODING_1Mbps  CONCAT(0x, 38E7, 38E7)
#define ONE_ENCODING_1Mbps   CONCAT(0x, CCCC, 38E7)
#define TWO_ENCODING_1Mbps   CONCAT(0X, 38E7, CCCC)
#define THREE_ENCODING_1Mbps CONCAT(0X, CCCC, CCCC)

#elif BLE_OFFSET == -40
/* -4 MHz offset */
/*                                      Even  Odd  */
#define ZERO_ENCODING_1Mbps  CONCAT(0x, 9831, 67ce)
#define ONE_ENCODING_1Mbps   CONCAT(0x, 6c26, 67ce)
#define TWO_ENCODING_1Mbps   CONCAT(0X, 9831, 93d9)
#define THREE_ENCODING_1Mbps CONCAT(0X, 6c26, 93d9)

#elif BLE_OFFSET == -45
/* -4.5 MHz offset */
/*                                      Even  Odd  */
#define ZERO_ENCODING_1Mbps  CONCAT(0x, 3333, 3333)
#define ONE_ENCODING_1Mbps   CONCAT(0x, db24, 3333)
#define TWO_ENCODING_1Mbps   CONCAT(0X, 3333, db24)
#define THREE_ENCODING_1Mbps CONCAT(0X, db24, db24)

/**************************************************/
/**************** Positive offsets ****************/
/**************************************************/

#elif BLE_OFFSET == 15
/* 1.5 MHz offset */
/*                                      Even  Odd  */
#define THREE_ENCODING_1Mbps CONCAT(0x, 00ff, 00ff)
#define TWO_ENCODING_1Mbps   CONCAT(0x, f0f0, 00ff)
#define ONE_ENCODING_1Mbps   CONCAT(0X, 00ff, f0f0)
#define ZERO_ENCODING_1Mbps  CONCAT(0X, f0f0, f0f0)

#elif BLE_OFFSET == 30
/* 3 MHz offset */
/*                                      Even  Odd  */
#define THREE_ENCODING_1Mbps CONCAT(0x, 711c, 8ee3)
#define TWO_ENCODING_1Mbps   CONCAT(0x, 9831, 8ee3)
#define ONE_ENCODING_1Mbps   CONCAT(0X, 711c, 67ce)
#define ZERO_ENCODING_1Mbps  CONCAT(0X, 9831, 67ce)

#elif BLE_OFFSET == 35
/* 3.5 MHz offset */
/*                                      Even  Odd  */
#define THREE_ENCODING_1Mbps CONCAT(0x, 38E7, 38E7)
#define TWO_ENCODING_1Mbps   CONCAT(0x, CCCC, 38E7)
#define ONE_ENCODING_1Mbps   CONCAT(0X, 38E7, CCCC)
#define ZERO_ENCODING_1Mbps  CONCAT(0X, CCCC, CCCC)

#elif BLE_OFFSET == 40
/* 4 MHz offset */
/*                                      Even  Odd  */
#define THREE_ENCODING_1Mbps CONCAT(0x, 9831, 67ce)
#define TWO_ENCODING_1Mbps   CONCAT(0x, 6c26, 67ce)
#define ONE_ENCODING_1Mbps   CONCAT(0X, 9831, 93d9)
#define ZERO_ENCODING_1Mbps  CONCAT(0X, 6c26, 93d9)

#elif BLE_OFFSET == 45
/* 4.5 MHz offset */
/*                                      Even  Odd  */
#define THREE_ENCODING_1Mbps CONCAT(0x, 3333, 3333)
#define TWO_ENCODING_1Mbps   CONCAT(0x, db24, 3333)
#define ONE_ENCODING_1Mbps   CONCAT(0X, 3333, db24)
#define ZERO_ENCODING_1Mbps  CONCAT(0X, db24, db24)
#else
#error "Invalid BLE_OFFSET"
#endif

/* -3 MHz offset */
/*                                         EvenOdd  */
#define ZERO_ENCODING_2Mbps  CONCAT(0x, 0000, F0F0)
#define ONE_ENCODING_2Mbps   CONCAT(0x, 0000, CCF0)
#define TWO_ENCODING_2Mbps   CONCAT(0X, 0000, F0CC)
#define THREE_ENCODING_2Mbps CONCAT(0X, 0000, CCCC)

const uint32_t upscale_lookup_1Mbps[4] = {
    ZERO_ENCODING_1Mbps,
    ONE_ENCODING_1Mbps,
    TWO_ENCODING_1Mbps,
    THREE_ENCODING_1Mbps};

const uint32_t upscale_lookup_2Mbps[4] = {
    ZERO_ENCODING_2Mbps,
    ONE_ENCODING_2Mbps,
    TWO_ENCODING_2Mbps,
    THREE_ENCODING_2Mbps};

uint32_t jv_bsc_upscale_1Mbps(uint32_t *dst, uint8_t *packet, size_t packet_len)
{

    /* We are writing 2 bytes per bit = 16 bytes per byte = 4 uint32s per byte
       So we stop after writing (4 * packet_len) uint32s
     */
    const uint32_t *stopping_point = dst + (packet_len << 2);
    while (dst < stopping_point)
    {
        *(dst++) = upscale_lookup_1Mbps[(((*packet) & 0xc0) >> 6)];
        *(dst++) = upscale_lookup_1Mbps[(((*packet) & 0x30) >> 4)];
        *(dst++) = upscale_lookup_1Mbps[(((*packet) & 0x0c) >> 2)];
        *(dst++) = upscale_lookup_1Mbps[((*packet) & 0x03)];
        packet++;
    }
    return packet_len << 4;
}

uint32_t jv_bsc_upscale_2Mbps(uint32_t *dst, uint8_t *packet, size_t packet_len)
{

    /* We are writing 1 byte per bit = 8 bytes per byte = 2 uint32s per byte
       So we stop after writing (2 * packet_len) uint32s
     */
    const uint32_t *stopping_point = dst + (packet_len << 1);
    while (dst < stopping_point)
    {
        *(dst++) = upscale_lookup_2Mbps[(((*packet) & 0xc0) >> 6)] | (upscale_lookup_2Mbps[(((*packet) & 0x30) >> 4)] << 16);
        *(dst++) = upscale_lookup_2Mbps[(((*packet) & 0x0c) >> 2)] | (upscale_lookup_2Mbps[((*packet) & 0x03)] << 16);
        packet++;
    }
    return packet_len << 3;
}