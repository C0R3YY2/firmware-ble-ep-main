/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_bt+packet_test.h
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Header file for testing JV BT+ packets
 * @date 2022-10-13
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#ifndef JV_BT_TEST_H
#define JV_BT_TEST_H

#include <stddef.h>
#include <stdint.h>
#include "../jv_bt+packet.h"
#include "../jv_bt+bsc.h"

void print_buffer(uint8_t *buffer, size_t len);
void test_case(uint8_t *AdvA, size_t AdvA_len, uint8_t *AdvData, size_t ADvData_len, uint8_t ble_channel);

#endif
