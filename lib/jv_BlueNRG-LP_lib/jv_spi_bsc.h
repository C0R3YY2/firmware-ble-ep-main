/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_spi_bsc.h
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Header file for using SPI/DMA for backscatter on BlueNRG-LP
 * @date 2022-05-18
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#ifndef INC_SPI_DMA_H_
#define INC_SPI_DMA_H_

#include <stdio.h>
#include <stdbool.h>
#include <main.h>


/**
 * @brief SPI + DMA init function
 *
 * @param tx_buffer location of buffer to transmit, casted to uint32_t
 * @param size size of buffer to transmit
 */
void SPI_DMA_Init(uint32_t tx_buffer, uint16_t size);

/**
 * @brief Reinit SPI + DMA after calling SPI_DMA_Uninit()
 *
 * @param tx_buffer location of buffer to transmit, casted to uint32_t
 * @param size size of buffer to transmit
 */
void SPI_DMA_Reinit(uint32_t tx_buffer, uint16_t size);

/**
 * @brief Disables the SPI and DMA channel
 *
 */
void SPI_DMA_Uninit(void);

/**
 * @brief Start transfer on SPI
 *
 */
void SPI_DMA_Activate(void);

/**
 * @brief Check if SPI transfer is completed
 *
 */
bool DMA_SPI_TransmitCompleted(void);

#endif /* INC_SPI_DMA_H_ */
