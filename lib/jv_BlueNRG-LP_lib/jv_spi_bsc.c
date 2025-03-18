/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_spi_bsc.c
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Functions for using SPI/DMA for backscatter on BlueNRG-LP
 * @date 2022-05-18
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#include "jv_spi_bsc.h"
#include "rf_driver_ll_bus.h"
#include "rf_driver_ll_spi.h"
#include "rf_driver_ll_dma.h"
#include "rf_driver_ll_gpio.h"


void SPI_TransferError_Callback(void);
void DMA_IRQHandler(void);
void DMA1_TransmitComplete_Callback(void);

static volatile bool ubTransmissionComplete = false;


void SPI_DMA_Init(uint32_t tx_buffer, uint16_t size)
{
    LL_SPI_InitTypeDef SPI_InitStruct = {0};
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Peripheral clock enable */
    LL_APB1_EnableClock(LL_APB1_PERIPH_SPI1);
    LL_AHB_EnableClock(LL_AHB_PERIPH_DMA);

    /* SPI GPIO pin init */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_14;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_0;
    LL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* SPI configuration */
    SPI_InitStruct.TransferDirection = LL_SPI_HALF_DUPLEX_TX;
    SPI_InitStruct.Mode = LL_SPI_MODE_MASTER;
    SPI_InitStruct.DataWidth = LL_SPI_DATAWIDTH_8BIT;
    SPI_InitStruct.ClockPolarity = LL_SPI_POLARITY_HIGH;
    SPI_InitStruct.ClockPhase = LL_SPI_PHASE_2EDGE;
    SPI_InitStruct.NSS = LL_SPI_NSS_SOFT;
    SPI_InitStruct.BaudRate = LL_SPI_BAUDRATEPRESCALER_DIV2;
    SPI_InitStruct.BitOrder = LL_SPI_MSB_FIRST;
    SPI_InitStruct.CRCCalculation = LL_SPI_CRCCALCULATION_DISABLE;
    SPI_InitStruct.CRCPoly = 7;
    LL_SPI_Init(SPI1, &SPI_InitStruct);
    LL_SPI_SetStandard(SPI1, LL_SPI_PROTOCOL_MOTOROLA);
    LL_SPI_DisableNSSPulseMgt(SPI1);
    LL_SPI_SetRxFIFOThreshold(SPI1, LL_SPI_RX_FIFO_TH_QUARTER);
    LL_SPI_EnableDMAReq_TX(SPI1);

    /* DMA configuration */
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMAMUX_REQ_SPI1_TX);
    LL_DMA_SetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
    LL_DMA_SetChannelPriorityLevel(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PRIORITY_LOW);
    LL_DMA_SetMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MODE_NORMAL);
    LL_DMA_SetPeriphIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PERIPH_NOINCREMENT);
    LL_DMA_SetMemoryIncMode(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MEMORY_INCREMENT);
    LL_DMA_SetPeriphSize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_PDATAALIGN_BYTE);
    LL_DMA_SetMemorySize(DMA1, LL_DMA_CHANNEL_3, LL_DMA_MDATAALIGN_BYTE);
    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3, tx_buffer, LL_SPI_DMA_GetRegAddr(SPI1), LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, size);
    LL_DMA_SetPeriphRequest(DMA1, LL_DMA_CHANNEL_3, LL_DMAMUX_REQ_SPI1_TX);
    LL_DMA_EnableIT_TC(DMA1, LL_DMA_CHANNEL_3);
    LL_DMA_EnableIT_TE(DMA1, LL_DMA_CHANNEL_3);

    /* DMA interrupt init */
    NVIC_SetPriority(DMA_IRQn, 0);
    NVIC_EnableIRQ(DMA_IRQn);
}

void SPI_DMA_Reinit(uint32_t tx_buffer, uint16_t size)
{
    LL_APB1_EnableClock(LL_APB1_PERIPH_SPI1);
    LL_AHB_EnableClock(LL_AHB_PERIPH_DMA);
    LL_DMA_ConfigAddresses(DMA1, LL_DMA_CHANNEL_3, tx_buffer, LL_SPI_DMA_GetRegAddr(SPI1), LL_DMA_GetDataTransferDirection(DMA1, LL_DMA_CHANNEL_3));
    LL_DMA_SetDataLength(DMA1, LL_DMA_CHANNEL_3, size);
}

void SPI_DMA_Uninit(void)
{
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    LL_APB1_DisableClock(LL_APB1_PERIPH_SPI1);
    LL_AHB_DisableClock(LL_AHB_PERIPH_DMA);
}

void SPI_DMA_Activate(void)
{
	ubTransmissionComplete = false;
    /* Enable SPI */
    LL_SPI_Enable(SPI1);
    /* Enable DMA Channels Tx */
    LL_DMA_EnableChannel(DMA1, LL_DMA_CHANNEL_3);
}

bool DMA_SPI_TransmitCompleted(void)
{
	if (ubTransmissionComplete)
	{
		return true;
	}
	return false;
}

void SPI_TransferError_Callback(void)
{
    /* Disable DMA1 Tx Channel */
    LL_DMA_DisableChannel(DMA1, LL_DMA_CHANNEL_3);
    while (1)
        ;
}

void DMA_IRQHandler(void)
{
    if (LL_DMA_IsActiveFlag_TC3(DMA1))
    {
        LL_DMA_ClearFlag_GI3(DMA1);
        /* Call function Tranmission complete Callback */
        DMA1_TransmitComplete_Callback();
    }
    else if (LL_DMA_IsActiveFlag_TE3(DMA1))
    {
        /* Call Error function */
        SPI_TransferError_Callback();
    }

    if (LL_DMA_IsActiveFlag_TC1(DMA1))
    {
        LL_DMA_ClearFlag_GI1(DMA1);
        /* Call function Reception complete Callback */
    }
    else if (LL_DMA_IsActiveFlag_TE1(DMA1))
    {
        /* Call Error function */
        SPI_TransferError_Callback();
    }
}

void DMA1_TransmitComplete_Callback(void)
{
    /* DMA Tx transfer completed */
    ubTransmissionComplete = true;
}
