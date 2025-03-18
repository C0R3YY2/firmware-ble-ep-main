/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_bt+packet_test.c
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Helper function for testing JV BT+ packets
 * @date 2022-10-13
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#include <stdio.h>
#include "jv_bt+packet_test.h"

void print_buffer(uint8_t *buffer, size_t len)
{
    printf("0x");
    for (size_t i = 0; i < len; i++)
    {
        printf("%02x", *(buffer + i));
    }
    printf("\n");
}

void test_case(uint8_t *AdvA, size_t AdvA_len, uint8_t *AdvData, size_t ADvData_len, uint8_t ble_channel)
{
    jv_packet_encoding_t packet_type = CODED_S8;
    jv_ble_pdu pdu;
    jv_ble_packet packet;

    /* Create and print whitened BLE packet */
    create_legacy_advertising_pdu(&pdu, AdvA, AdvA_len, AdvData, ADvData_len);
    init_packet(&packet, ble_channel, &pdu, packet_type);

    /* Update the BLE packet */
    uint8_t AdvA_new[AdvA_len];
    uint8_t AdvData_new[ADvData_len];
    create_legacy_advertising_pdu(&pdu, AdvA_new, AdvA_len, AdvData_new, ADvData_len);
    update_advertising_packet(&packet, &pdu);

    /* Update the BLE packet to the original data and print it */
    create_legacy_advertising_pdu(&pdu, AdvA, AdvA_len, AdvData, ADvData_len);
    update_advertising_packet(&packet, &pdu);
    print_buffer(packet.whitened_packet, packet.packet_len);

    uint8_t coded_buf[1000];
    size_t coded_len;
    coded_len = encode_packet(coded_buf, &packet);
    print_buffer(coded_buf, coded_len);

    /* Upscale the packet for backscatter and print it */
    uint32_t upscaled_buffer[CODED_MAX_PACKET_SIZE * 16];
    uint32_t upscaled_length = jv_bsc_upscale_1Mbps(upscaled_buffer, packet.whitened_packet, packet.packet_len);
    // print_buffer((uint8_t *)upscaled_buffer, (size_t)upscaled_length);
}
