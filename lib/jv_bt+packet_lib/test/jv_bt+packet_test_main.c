/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_bt+packet_test_main.c
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Test file for JV BT+ packets
 * @date 2022-10-13
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */


#include <stdio.h>
#include "jv_bt+packet_test.h"

int main(int argc, char **argv)
{
    uint8_t AdvA_0[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    uint8_t AdvData_0[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};

    uint8_t ble_channel = 37;

    test_case(AdvA_0, sizeof(AdvA_0) / sizeof(AdvA_0[0]),
              AdvData_0, sizeof(AdvData_0) / sizeof(AdvData_0[0]),
              ble_channel);

    ble_channel = 0;
    test_case(AdvA_0, sizeof(AdvA_0) / sizeof(AdvA_0[0]),
              AdvData_0, sizeof(AdvData_0) / sizeof(AdvData_0[0]),
              ble_channel);

    uint8_t AdvA_1[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    uint8_t AdvData_1[] = {0x00};
    test_case(AdvA_1, sizeof(AdvA_1) / sizeof(AdvA_1[0]),
              AdvData_1, sizeof(AdvData_1) / sizeof(AdvData_1[0]),
              ble_channel);

    uint8_t AdvA_2[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    uint8_t AdvData_2[1];
    test_case(AdvA_2, sizeof(AdvA_2) / sizeof(AdvA_2[0]),
              AdvData_2, 0,
              ble_channel);

    uint8_t AdvA_3[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    uint8_t AdvData_3[] = {
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
        0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd};
    test_case(AdvA_3, sizeof(AdvA_3) / sizeof(AdvA_3[0]),
              AdvData_3, sizeof(AdvData_3) / sizeof(AdvData_3[0]),
              ble_channel);

    return 0;
}
