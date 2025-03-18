/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file main.c
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @editor Stiliyan Dimanov (stiliyan.dimanov@jeevawireless.com)
 * @brief Main file for BLE backscatter
 * @date 2022-05-17
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#include <stdint.h>
#include "rf_driver_hal.h"
#include "main.h"
#include "../lib/jv_BlueNRG-LP_lib/jv_spi_bsc.h"
#include "../lib/jv_BlueNRG-LP_lib/jv_rtc.h"
#include "../lib/jv_BlueNRG-LP_lib/jv_gpio.h"
#include "../lib/jv_LSM6DSO32_lib/jv_imu.h"
#include "../lib/jv_bt+packet_lib/jv_bt+packet.h"
#include "../lib/jv_bt+packet_lib/jv_bt+bsc.h"

/* IMU SPI communication */
SPI_HandleTypeDef hspiMaster;
__attribute((aligned(4))) uint8_t aTxBuffer[32];                // Buffer used for transmission;
uint8_t ubNbDataToTransmit = sizeof(aTxBuffer);                 // Data length to transmit
__attribute((aligned(4))) uint8_t aRxBuffer[sizeof(aTxBuffer)]; // Buffer used for reception

/* flags for SPI, RTC, IMU interrupts */
volatile uint8_t data_ready = 0;

/* BLE packet buffers */
#ifdef USE_IMU
uint8_t AdvA[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
uint8_t AdvData[24];
#else
uint8_t AdvA[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t AdvData[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif
jv_ble_pdu pdu;
jv_ble_packet packet;

__attribute((aligned(4))) uint8_t coded_buf[1000];
size_t coded_len;

uint32_t packet_upscaled[2500];
uint32_t upscaled_length;

/**
 * @brief Main function
 *
 */
int main(void)
{
    /* System initialization function */
    while (SystemInit(SYSCLK_DIRECT_HSE, RADIO_SYSCLK_NONE) != SUCCESS)
        ;

    /* system config */
    GPIO_Init();
    CLEAR_BIT(RCC->CR, RCC_CR_HSEPLLBUFON);               // turn off HSEPLLBUF
    MODIFY_REG_FIELD(RCC->CFGR, RCC_CFGR_CLKSYSDIV, 0x1); // set system clock to 32 MHz

    /* IMU SPI config */
#ifdef USE_IMU
    HAL_Init();
    MX_SPI_MASTER_Init();
    stmdev_ctx_t dev_ctx;
    int32_t result = 0;
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    dev_ctx.handle = &hspiMaster;
    result |= imu_init(&dev_ctx);
    result |= imu_get_packet_data(&dev_ctx, (uint16_t *)(&AdvData[4]));
#ifdef IMU_POWER_OFF
    imu_interrupt_init();
    imu_interrupt_enable();
#endif
#endif

    /* BLE packet / BSC SPI init */
    AdvData[1] = 0x00;
    AdvData[2] = 0x00;
    create_legacy_advertising_pdu(&pdu, AdvA, sizeof(AdvA) / sizeof(AdvA[0]), AdvData, sizeof(AdvData) / sizeof(AdvData[0]));
    init_packet(&packet, BLE_CHANNEL, &pdu, BLE_PACKET_TYPE);

#ifdef BLE_CODED
    coded_len = encode_packet(coded_buf, &packet);
    upscaled_length = jv_bsc_upscale(packet_upscaled, coded_buf, coded_len);
#else
    upscaled_length = jv_bsc_upscale(packet_upscaled, packet.whitened_packet, packet.packet_len);
#endif

    SPI_DMA_Init((uint32_t)packet_upscaled, upscaled_length);

#ifdef ENTER_DEEPSTOP
    uint8_t ret_val;
    WakeupSourceConfig_TypeDef wakeupIO;
    PowerSaveLevels stopLevel;
    wakeupIO.RTC_enable = 1;
#endif

    /* start timer */
    RTC_WakeupInit();
    SetRTC_WakeupTimeout(RTC_DELAY);

    /* infinite program loop */
    uint16_t count = 0;

    while (1)
    {
#if defined USE_IMU && defined IMU_POWER_OFF
        /* power on IMU */
        result |= imu_power_on(&dev_ctx);
        imu_interrupt_enable();
        data_ready = 0;
#endif
        /* start transmission */
        SPI_DMA_Reinit((uint32_t)packet_upscaled, upscaled_length);
        SPI_DMA_Activate();

        /* update sequence number */
        AdvData[1] = (uint8_t)count;
        AdvData[2] = (uint8_t)(count >> 8);

        /* increment sequence number */
        count++;

        /* blink LED */
#ifdef LED_BLINK
        if (count % LED_ON_THRESHOLD == 0)
            led_on();
        else if (count % LED_ON_THRESHOLD == LED_OFF_THRESHOLD)
            led_off();
#endif

#ifdef USE_IMU
#ifdef IMU_POWER_OFF
        /* wait for data ready interrupt */
        while (data_ready != 1)
        {
            __WFE();
        }
        imu_interrupt_disable();

        /* get IMU data */
        result |= imu_get_packet_data(&dev_ctx, (uint16_t *)(&AdvData[4]));

        /* power off IMU */
        result |= imu_power_off(&dev_ctx);
#else
        /* get IMU data */
        result |= imu_get_packet_data(&dev_ctx, (uint16_t *)(&AdvData[4]));
#endif
#endif

        /* update payload */
        create_legacy_advertising_pdu(&pdu, AdvA, sizeof(AdvA) / sizeof(AdvA[0]), AdvData, sizeof(AdvData) / sizeof(AdvData[0]));

        /* wait for transmission to be complete */
        while (!DMA_SPI_TransmitCompleted())
        {
            __WFE();
        }
        SPI_DMA_Uninit();

        /* update packet */
        update_advertising_packet(&packet, &pdu);
#ifdef BLE_CODED
        coded_len = encode_packet(coded_buf, &packet);
        upscaled_length = jv_bsc_upscale(packet_upscaled, coded_buf, coded_len);
#else
        upscaled_length = jv_bsc_upscale(packet_upscaled, packet.whitened_packet, packet.packet_len);
#endif

        /* wait for timer to be complete */
#ifdef ENTER_DEEPSTOP
        ret_val = HAL_PWR_MNGR_Request(POWER_SAVE_LEVEL_STOP_WITH_TIMER, wakeupIO, &stopLevel);
        if (ret_val != SUCCESS)
            while (1)
                ;
        SPI_DMA_Init((uint32_t)packet_upscaled, upscaled_length);
#else
        while (!RTC_WakeupTimeout_Expired())
        {
            __WFE();
        }
#endif
    }
}
