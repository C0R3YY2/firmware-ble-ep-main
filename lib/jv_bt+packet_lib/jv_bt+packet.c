/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_bt+packet.c
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Library functions for creating BLE packets for backscatter
 * @date 2022-10-13
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#include <stdio.h>
#include <string.h>
#include "jv_bt+packet.h"
#include "crc.h"

#define BIT0 0x01
#define BIT1 0x02
#define BIT2 0x04
#define BIT3 0x08
#define BIT4 0x10
#define BIT5 0x20
#define BIT6 0x40
#define BIT7 0x80

/**
 * @brief Lookup table for reverse() function
 */
static const uint8_t lookup[16] = {
    0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
    0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf};

static uint8_t reverse(uint8_t n)
{
    return (lookup[n & 0b1111] << 4) | lookup[n >> 4];
}

static void generate_whitening_lookup(uint8_t *lookup, uint8_t ch, uint8_t len)
{
    uint8_t LFSR = (reverse(ch) >> 1) | BIT0;
    uint8_t feedback, sum, data_out;
    for (uint8_t i = 0; i < len; i++)
    {
        data_out = 0;
        for (int j = 0; j < 8; j++)
        {
            feedback = (LFSR & BIT6) >> 6;                       /* feedback = bit 6 */
            sum = feedback ^ ((LFSR & BIT3) >> 3);               /* sum = feedback + bit 3 */
            LFSR = feedback | ((LFSR << 1) & 0x6e) | (sum << 4); /* shift the LFSR for next bit */
            data_out |= feedback << (7 - j);
        }
        lookup[i] = data_out;
    }
}

static void whiten(uint8_t *data_in, uint8_t *whitening_lookup, uint8_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        data_in[i] ^= whitening_lookup[i];
    }
}

enum fec_block_t
{
    FEC_BLOCK_1,
    FEC_BLOCK_2
};

typedef enum fec_block_t fec_block_t;

static const uint8_t FEC_ENCODE_S2_LOOKUP[] = {0b00, 0b10, 0b01, 0b11};
static const uint8_t FEC_ENCODE_S8_LOOKUP[] = {0x33, 0xc3, 0x3c, 0xcc};

/**
 * @brief Do forward error correction and pattern mapping on an input buffer
 *
 * @param dst Destination buffer to store output data
 * @param src Source buffer
 * @param len Length of source buffer in Bytes
 * @param S Coding scheme, either 2 or 8
 * @param block FEC block 1 or 2
 * @param CI CI bits
 * @return int -1 if invalid argument, or 0 if successful
 *
 * @warning No bounds checking on destination buffer
 */
static uint8_t *fec_encode(uint8_t *dst, uint8_t *src, size_t len, jv_packet_encoding_t S, fec_block_t block, uint8_t CI)
{
    size_t i;
    uint8_t j, encoder1, encoder2, encoder3, byte, bit, a;
    uint16_t temp;
    encoder1 = 0;
    encoder2 = 0;
    encoder3 = 0;

    for (i = 0; i < len; i++)
    {
        byte = src[i];
        for (j = 0; j < 8; j++) // iterate over every bit in each byte
        {
            bit = (byte & (0b10000000 >> j)) >> (7 - j);
            a = (bit ^ encoder1 ^ encoder2 ^ encoder3) | ((bit ^ encoder2 ^ encoder3) << 1);
            encoder3 = encoder2;
            encoder2 = encoder1;
            encoder1 = bit;
            if (S == CODED_S2)
                temp = (temp << 2) | FEC_ENCODE_S2_LOOKUP[a];
            else
                *(dst++) = FEC_ENCODE_S8_LOOKUP[a];
        }

        if (S == CODED_S2)
        {
            *(dst++) = (uint8_t)((temp & 0xff00) >> 8);
            *(dst++) = (uint8_t)temp;
        }
    }

    /* Encode CI, TERM1, and TERM2 */
    if (block == FEC_BLOCK_1)
    {
        byte = 0x00 | (CI << 3); // CI and TERM1
        for (j = 0; j < 5; j++)
        {
            bit = (byte & (0b00010000 >> j)) >> (4 - j);
            a = (bit ^ encoder1 ^ encoder2 ^ encoder3) | ((bit ^ encoder2 ^ encoder3) << 1);
            encoder3 = encoder2;
            encoder2 = encoder1;
            encoder1 = bit;
            *(dst++) = FEC_ENCODE_S8_LOOKUP[a];
        }
    }
    else if (block == FEC_BLOCK_2)
    {
        byte = 0x00;       // TERM2
        if (S == CODED_S8) // S = 8: we encode all 3 bits of TERM2 -> 3 Bytes
        {
            for (j = 0; j < 3; j++)
            {
                bit = (byte & (0b00010000 >> j)) >> (2 - j);
                a = (bit ^ encoder1 ^ encoder2 ^ encoder3) | ((bit ^ encoder2 ^ encoder3) << 1);
                encoder3 = encoder2;
                encoder2 = encoder1;
                encoder1 = bit;
                *(dst++) = FEC_ENCODE_S8_LOOKUP[a];
            }
        }
        else if (S == CODED_S2) // S = 2: can't encode 3 bits -> 0.75 Bytes, so extend to 4 bits -> 1 Byte
        {
            for (j = 0; j < 4; j++)
            {
                bit = (byte & (0b00010000 >> j)) >> (3 - j);
                a = (bit ^ encoder1 ^ encoder2 ^ encoder3) | ((bit ^ encoder2 ^ encoder3) << 1);
                encoder3 = encoder2;
                encoder2 = encoder1;
                encoder1 = bit;
                temp = (temp << 2) | FEC_ENCODE_S2_LOOKUP[a];
            }
            *(dst++) = (uint8_t)temp;
        }
    }

    return dst;
}

int create_legacy_advertising_pdu(jv_ble_pdu *pdu, uint8_t *AdvA, uint8_t AdvA_len, uint8_t *AdvData, uint8_t AdvData_len)
{
    if (AdvA_len != ADVERTISING_ADDRESS_SIZE || AdvData_len > MAX_ADVERTISING_DATA_SIZE)
    {
        return -1;
    }

    uint8_t i = 0;

    /* Create and format header */
    pdu_type_t pdu_type = ADV_NONCONN_IND;
    pdu->pdu[i++] = (reverse((uint8_t)pdu_type) >> 4) << 4;
    pdu->pdu[i++] = reverse((uint8_t)(AdvA_len + AdvData_len));

    /* Format advertising address */
    for (i = 0; i < AdvA_len; i++)
    {
        pdu->pdu[i + 2] = reverse(AdvA[AdvA_len - i - 1]);
    }

    /* Format advertising data */
    for (i = 0; i < AdvData_len; i++)
    {
        pdu->pdu[AdvA_len + 2 + i] = reverse(AdvData[AdvData_len - i - 1]);
    }

    pdu->pdu_len = (HEADER_SIZE + AdvA_len + AdvData_len);

    return 0;
}

int init_packet(jv_ble_packet *packet, uint8_t ch, jv_ble_pdu *pdu, jv_packet_encoding_t encoding)
{
    if (ch > 39 || pdu->pdu_len < ADVERTISING_ADDRESS_SIZE || pdu->pdu_len > MAX_PDU_SIZE)
    {
        return -1;
    }

    size_t i = 0;
    packet->encoding = encoding;

    switch (encoding)
    {
    case CODED_S2:
    case CODED_S8:
        /* Preamble: 10 repititions of 0x3c */
        while (i < 10)
            packet->whitened_packet[i++] = 0x3c;
        break;

    case UNCODED_1MBPS:
        /* Preamble: 0xaa */
        packet->whitened_packet[i++] = reverse(0xaa);
        break;

    case UNCODED_2MBPS:
        /* Preamble: 0xaaaa */
        packet->whitened_packet[i++] = reverse(0xaa);
        packet->whitened_packet[i++] = reverse(0xaa);
        break;

    default:
        break;
    }

    /* Populate access address: 0x8e89bed6 */
    packet->whitened_packet[i++] = reverse(0xd6);
    packet->whitened_packet[i++] = reverse(0xbe);
    packet->whitened_packet[i++] = reverse(0x89);
    packet->whitened_packet[i++] = reverse(0x8e);

    /* Shift through whitening LFSR to generate whitening lookup table */
    generate_whitening_lookup(packet->whitening_lookup_table, ch, WHITENING_SIZE);

    /* Finish the ret of the packet and whiten */
    update_advertising_packet(packet, pdu);

    return 0;
}

void update_advertising_packet(jv_ble_packet *packet, jv_ble_pdu *pdu)
{

    /* Whitening starts after access address, which is a different index for different encodings */
    uint8_t whitening_start = ACCESS_ADDRESS_SIZE;
    switch (packet->encoding)
    {
    case CODED_S2:
    case CODED_S8:
        whitening_start += CODED_PREAMBLE_SIZE;
        break;
    case UNCODED_1MBPS:
        whitening_start += UNCODED_PREAMBLE_SIZE_1MBPS;
        break;
    case UNCODED_2MBPS:
        whitening_start += UNCODED_PREAMBLE_SIZE_2MBPS;
        break;

    default:
        break;
    }

    uint8_t i = whitening_start;

    /* Copy PDU */
    memcpy(&(packet->whitened_packet[i]), pdu->pdu, pdu->pdu_len);
    i += pdu->pdu_len;

    /* Generate CRC */
    crc_t crc = crc_init();
    crc = crc_update(crc, pdu->pdu, pdu->pdu_len);
    packet->whitened_packet[i++] = crc >> 16;
    packet->whitened_packet[i++] = crc >> 8;
    packet->whitened_packet[i++] = crc;
    packet->packet_len = i;

    /* Whiten the packet */
    whiten(&(packet->whitened_packet[whitening_start]), packet->whitening_lookup_table, (packet->packet_len - whitening_start));
}
size_t encode_packet(uint8_t *dst, jv_ble_packet *packet)
{
    // if (packet->encoding != CODED_S2 && packet->encoding != CODED_S8)
    // {
    //     return -1;
    // }

    uint8_t *packet_start = dst;
    uint8_t CI = (packet->encoding == CODED_S8) ? 0x00 : 0x02;
    size_t block_1_size = ACCESS_ADDRESS_SIZE;
    size_t block_2_size = packet->packet_len - (CODED_PREAMBLE_SIZE + ACCESS_ADDRESS_SIZE);
    uint8_t *block_1_start = packet->whitened_packet + CODED_PREAMBLE_SIZE;
    uint8_t *block_2_start = packet->whitened_packet + CODED_PREAMBLE_SIZE + ACCESS_ADDRESS_SIZE;

    memcpy(dst, packet->whitened_packet, CODED_PREAMBLE_SIZE);
    dst += CODED_PREAMBLE_SIZE;

    dst = fec_encode(dst, block_1_start, block_1_size, CODED_S8, FEC_BLOCK_1, CI);
    dst = fec_encode(dst, block_2_start, block_2_size, packet->encoding, FEC_BLOCK_2, 0x00);

    return (size_t) (dst - packet_start);
}
