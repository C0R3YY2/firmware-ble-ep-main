/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file main.c
 * @editor Stiliyan Dimanov (stiliyan.dimanov@jeevawireless.com)
 * @brief Main file for BLE backscatter on BlueNRG-LP boards
 * @date 2022-05-17
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */
#include <stdint.h>
#include "rf_driver_hal.h"
#include "rf_driver_hal_vtimer.h"
#include "rf_driver_hal_radio_2g4.h"
#include "main.h"
#include "../lib/jv_BlueNRG-LP_lib/jv_spi_bsc.h"
#include "../lib/jv_BlueNRG-LP_lib/jv_rtc.h"
#include "../lib/jv_BlueNRG-LP_lib/jv_gpio.h"
#include "../lib/jv_BlueNRG-LP_lib/jv_delayUS.h"
#include "../lib/jv_LSM6DSO32_lib/jv_imu.h"
#include "../lib/jv_bt+packet_lib/jv_bt+packet.h"
#include "../lib/jv_bt+packet_lib/jv_bt+bsc.h"


/* IMU SPI communication */
SPI_HandleTypeDef hspiMaster;
__attribute((aligned(4))) uint8_t aTxBuffer[32];                // Buffer used for transmission;
uint8_t ubNbDataToTransmit = sizeof(aTxBuffer);                 // Data length to transmit
__attribute((aligned(4))) uint8_t aRxBuffer[sizeof(aTxBuffer)]; // Buffer used for reception

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

/* Buffer used for storing received data */
__attribute((aligned(4))) uint8_t rxBuff[MAX_LL_PACKET_LENGTH] = {0};

const uint8_t linkID = EP_LINK_ID;
volatile uint8_t data_ready = 0;
// volatile uint16_t dl_seq_num = 0;
volatile bool radio_dl_received = false;
volatile bool radio_dl_timeout = false;
volatile bool radio_dl_err = false;

enum EP_state
{
	DISASSOCIATED,
	ASSOCIATED
} association_state;


uint8_t RxCallback(ActionPacket* p, ActionPacket* next)
{
	if ((p->status & BLUE_INTERRUPT1REG_DONE) != 0)
	{
		if ((p->status & BLUE_STATUSREG_PREVTRANSMIT) == 0)
		{
			if ((p->status & BLUE_INTERRUPT1REG_RCVOK) != 0)
			{
				/* Make sure the packet format matches, and we are get the right Advertising Address */
				if (rxBuff[0] == 0x02 && rxBuff[1] == 0x08 && rxBuff[2] == BLE_ADV_ADDR_5 && rxBuff[3] == BLE_ADV_ADDR_4 && \
						rxBuff[4] == BLE_ADV_ADDR_3 && rxBuff[5] == BLE_ADV_ADDR_2 && rxBuff[6] == BLE_ADV_ADDR_1 && rxBuff[7] == BLE_ADV_ADDR_0)
				{
					// dl_seq_num = (uint16_t)rxBuff[9] << 8 | rxBuff[8];
					radio_dl_received = true;
				}
				else
				{
					/* This is not us. Assert the error flag */
					radio_dl_err = true;
				}
			}
			else if ((p->status & BLUE_INTERRUPT1REG_RCVTIMEOUT) != 0)
			{
				/* DL timeout. Assert the timeout flag */
				radio_dl_timeout = true;
			}
			else if ((p->status & BLUE_INTERRUPT1REG_RCVCRCERR) != 0)
			{
				/* CRC error. Assert the error flag */
				radio_dl_err = true;
			}
		}
	}
	return TRUE;
}

void ep_send_ble_packet(void)
{
	static uint16_t count = 0;

#ifdef LED_BLINK
	led_toggle();
#endif

	/* linkID */
	AdvData[0] = linkID;
	/* update sequence number */
	// AdvData[1] = (uint8_t)dl_seq_num;
	// AdvData[2] = (uint8_t)(dl_seq_num >> 8);
	AdvData[1] = (uint8_t)count;
	AdvData[2] = (uint8_t)(count >> 8);

	/* increment sequence number */
	count++;

#ifdef USE_IMU
#ifdef IMU_POWER_OFF
	/* power on IMU */
	result |= imu_power_on(&dev_ctx);
	imu_interrupt_enable();
	data_ready = 0;

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

	/* update packet */
	update_advertising_packet(&packet, &pdu);
#ifdef BLE_CODED
	coded_len = encode_packet(coded_buf, &packet);
	upscaled_length = jv_bsc_upscale(packet_upscaled, coded_buf, coded_len);
#else
	upscaled_length = jv_bsc_upscale(packet_upscaled, packet.whitened_packet, packet.packet_len);
#endif

	/* cc26xx needs some time to prepare for receiving after downlink has been transmitted.
	 * Add also a timing slot for every endpoint depend of the linkID.
	 * */
	jv_delayUS(DL_UL_DELAY + (EP_LINK_ID * UL_PKT_DURATION));

	jv_gpioReset(DBG_GPIO);
	/* start transmission */
	SPI_DMA_Reinit((uint32_t)packet_upscaled, upscaled_length);
	SPI_DMA_Activate();
	/* wait for transmission to be complete */
	while (!DMA_SPI_TransmitCompleted())
	{
		__WFE();
	}
	SPI_DMA_Uninit();

	jv_gpioSet(DBG_GPIO);
}

/**
 * @brief Main function
 *
 */
int main(void)
{
    association_state = DISASSOCIATED;
    uint8_t assoc_dl_fail_cnt = 0;
    bool state_changed = true;
    bool RTC_set = false;

    /* System initialization function */
	while (SystemInit(SYSCLK_32M, BLE_SYSCLK_32M) != SUCCESS)
		;

//    /* System initialization function */
//    while (SystemInit(SYSCLK_DIRECT_HSE, RADIO_SYSCLK_NONE) != SUCCESS)
//    	;
//    CLEAR_BIT(RCC->CR, RCC_CR_HSEPLLBUFON);               // turn off HSEPLLBUF
//    MODIFY_REG_FIELD(RCC->CFGR, RCC_CFGR_CLKSYSDIV, 0x1); // set system clock to 32 MHz

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

    /* system config */
    GPIO_Init();
    /* Timer delay initialization */
	jv_delayUS_Init();
    /* RTC timer initialization */
    RTC_WakeupInit();

#ifdef ENTER_DEEPSTOP
    uint8_t ret;
    WakeupSourceConfig_TypeDef wakeupIO;
    PowerSaveLevels stopLevel;
    wakeupIO.RTC_enable = 1;
    wakeupIO.LPU_enable = 0;
    wakeupIO.IO_Mask_Low_polarity = NO_WAKEUP_SOURCE;
    wakeupIO.IO_Mask_High_polarity = NO_WAKEUP_SOURCE;
    uint32_t wakeupSource;
#endif

    /* Radio initialization */
	HAL_VTIMER_InitType VTIMER_InitStruct = {HS_STARTUP_TIME, INITIAL_CALIBRATION, CALIBRATION_INTERVAL};
	RADIO_Init();
	HAL_VTIMER_Init(&VTIMER_InitStruct);
	HAL_RADIO_SetNetworkID(BLE_ACCESS_ADDR);

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

    /* infinite program loop */
    while (1)
    {
    	/* Perform calibration procedure */
    	HAL_VTIMER_Tick();

    	switch (association_state)
    	{
    		case DISASSOCIATED:
    			if (state_changed)
    			{
    				state_changed = false;
    				/* Put the radio in continuous RX mode until we get DL */
    				HAL_RADIO_ReceivePacket(BLE_DL_CHANNEL, RX_WAKEUP_TIME, rxBuff, RX_DISASSOC_TOUT, MAX_LL_PACKET_LENGTH, RxCallback);
    			}

    			if (radio_dl_received)
    			{
					SetRTC_WakeupTimeout(RTC_DL_TO_RX_TIME);
    				jv_gpioSet(DBG_GPIO);
					RTC_set = true;
					radio_dl_received = false;
					association_state = ASSOCIATED;
					ep_send_ble_packet();
    			}
    			else if ( radio_dl_timeout || radio_dl_err)
    			{
    				jv_gpioSet(DBG_GPIO);
					radio_dl_timeout = false;
					radio_dl_err = false;
					HAL_RADIO_ReceivePacket(BLE_DL_CHANNEL, RX_WAKEUP_TIME, rxBuff, RX_DISASSOC_TOUT, MAX_LL_PACKET_LENGTH, RxCallback);
					jv_gpioReset(DBG_GPIO);
    			}
    			break;
    		case ASSOCIATED:
    			if (radio_dl_received)
    			{
					SetRTC_WakeupTimeout(RTC_DL_TO_RX_TIME);
    				jv_gpioSet(DBG_GPIO);
					RTC_set = true;
					radio_dl_received = false;
					assoc_dl_fail_cnt = 0;
					ep_send_ble_packet();
    			}
    			else if (radio_dl_err)
    			{
					SetRTC_WakeupTimeout(RTC_DL_TO_RX_TIME);
    				jv_gpioSet(DBG_GPIO);
					RTC_set = true;
					radio_dl_err = false;
    			}
    			else if (radio_dl_timeout)
    			{
    				if (assoc_dl_fail_cnt >= ASSOC_DISASSOC_THRESH)
    				{
    					assoc_dl_fail_cnt = 0;
    					state_changed = true;
    					association_state = DISASSOCIATED;
    				}
    				else
    				{
						SetRTC_WakeupTimeout(RTC_TOUT_TO_RX_TIME);
						jv_gpioSet(DBG_GPIO);
						RTC_set = true;
						assoc_dl_fail_cnt++;
    				}
					radio_dl_timeout = false;
    			}

    			if (RTC_set)
    			{
    				RTC_set = false;
#ifdef ENTER_DEEPSTOP
#else
    		        while (!RTC_WakeupTimeout_Expired())
    		        {
    		            __WFE();
    		        }
#endif
    		        DisableRTC_WakeupTimeout();
					HAL_RADIO_ReceivePacket(BLE_DL_CHANNEL, RX_WAKEUP_TIME, rxBuff, RX_ASSOC_TOUT, MAX_LL_PACKET_LENGTH, RxCallback);
					jv_gpioReset(DBG_GPIO);
    			}
    			break;
    		default:
    			break;
    	}
    }
}
