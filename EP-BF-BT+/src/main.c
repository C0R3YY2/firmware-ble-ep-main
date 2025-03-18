/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file main.c
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Main file for battery-free (BF) BLE backscatter
 * @date 2022-05-17
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#include <stdint.h>
#include "rf_driver_hal.h"
#include "rf_driver_ll_flash.h"

#include "main.h"
#include "../lib/jv_BlueNRG-LP_lib/jv_spi_bsc.h"
#include "../lib/jv_BlueNRG-LP_lib/jv_gpio.h"
#include "../lib/jv_bt+packet_lib/jv_bt+packet.h"
#include "../lib/jv_bt+packet_lib/jv_bt+bsc.h"

/* flag for SPI interrupt */
volatile uint8_t data_ready = 0;

/* BLE packet buffers */
uint8_t AdvA[] = {0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc};
    //6-byte Advertising Address ("MAC Address") used for the central to identify this device

uint8_t AdvData[] = {0xb5, 0xef, 0xbe, 0x01, 0x00, 0x0f,
                     0x6d, 0x16, 0xbb, 0xa9, 0xa2, 0x7c,
                     0xa6, 0x80, 0x42, 0x60, 0x59, 0x20,
                     0x01, 0x6c, 0x41, 0x15, 0x02, 0x00,
                     0x4c, 0xff, 0x1a, 0x04, 0x01, 0x02};
    //30-byte Advertising PDU (or payload) (max pdu size is 31-bytes)


jv_ble_pdu pdu;
jv_ble_packet packet;

uint32_t packet_upscaled[200] __attribute__((section(".noinit")));
    //allocates the 200-element 'packet_upscaled' array into a memory location that
    //preserves data across system resets (memory only clears after a complete removal of power)

uint32_t upscaled_length;

#define FLASH_USER_START_ADDR (FLASH_END_ADDR - FLASH_PAGE_SIZE + 1)
    //defines the start address in flash memory for user data
    //the start address is set to the last "page" of the the flash memory by
    //subtracting the flash memory's end address (FLASH_END_ADDR) by
    //the length of one flash-memory page plus one (FLASH_PAGE_SIZE + 1)

uint32_t Address = 0;
    //will hold the flash's memory address for
    //reading and writing flash memory (aka it's a pointer variable)

__IO uint32_t flash_data = 0;
    //will hold the data for reading and writing flash memory
        //the '__IO' type is an mcu-specific type for input/output operations (maybe gpio on em9305?)


/**
 * @brief Main function
 *
 */
int main(void)
{
    ///////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////* System initialization function *//////////////////////////
    ///////////////////////////////////////////////////////////////////////////////////////
    
    //////////////////(vector table)//////////////////
    SCB->VTOR = (uint32_t)(__vector_table);
    RAM_VR.AppBase = (uint32_t)(__vector_table);
        //stores the memory addresses of exception handlers and interrupt service routines (ISRs) into
        //the Vector Table Offset Register (VTOR) which is in the System Control Block (SCB) 

    //////////////////(GPIO config)//////////////////
    GPIO_Init();
        //defined in jv_gpio.c

    //////////////////(clock config)//////////////////
    /* seems like all these functions are defined in the STMicro API */
    CLEAR_BIT(RCC->CR, RCC_CR_HSEPLLBUFON);
        //turn off HSEPLLBUF

    LL_RCC_HSE_Enable();
        //enables high-speed external (HSE) oscillator

    while (!LL_RCC_HSE_IsReady)
        ;
        //waits for the HSE oscillator to be ready

    LL_RCC_SetRC64MPLLPrescaler(LL_RCC_RC64MPLL_DIV_2);
        //"sets RC64M PLL prescaler to divide by 2"

    LL_RCC_DIRECT_HSE_Enable();
        //enables direct HSE mode

    MODIFY_REG_FIELD(RCC->CFGR, RCC_CFGR_CLKSYSDIV, 0x1);
        //sets system's clock to 32 MHz

    LL_FLASH_SetWaitStates(FLASH, LL_FLASH_WAIT_STATES_0);
        //"configures flash wait states for optimal performance"

    //////////////////(interupt config)//////////////////
    /* seems like all these functions are defined in the STMicro API */
    setInterruptPriority();
        //idk what this does

    __enable_irq();
        //enable global interupts


    while (1)
    {
        //////////////////(Creates BLE Advertising Packet)//////////////////
        create_legacy_advertising_pdu(&pdu, AdvA, sizeof(AdvA) / sizeof(AdvA[0]), AdvData, sizeof(AdvData) / sizeof(AdvData[0]));
            //buids a legacy advertising PDU with predefined AdvA (device address) and AdvData

        init_packet(&packet, BLE_CHANNEL, &pdu, BLE_PACKET_TYPE);
            //initializes the packet with BLE_CHANNEL and BLE_PACKET_TYPE

        upscaled_length = jv_bsc_upscale(packet_upscaled, packet.whitened_packet, packet.packet_len);
            //upscales the whitened packet for backscatter transmission

        //////////////////(Sets up the SPI and DMA for Bluetooth Transmission)//////////////////
        SPI_DMA_Init((uint32_t)packet_upscaled, upscaled_length);
            //sets up SPI and DMA for packet transmission
        
        SPI_DMA_Activate();
            //activates SPI and DMA to start transmission

        //waits until the the transmission is complete using DMA_SPI_TransmitCompleted()
        while (!DMA_SPI_TransmitCompleted())
        {
            __WFE(); //while waiting for a DMA interupt event, put the processor in 'wait-for-event' (WFE) mode to save power 
        }


        SPI_DMA_Uninit();
            //once the BLE transmission is finished, un-initialize the SPI and DMA

        led_on();
            //turn an LED ON (maybe for debugging)

        //go back up, and repeat this whole process again
    }
}
