/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_imu.c
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @brief Helper functions for working with the IMU
 * @date 2022-05-20
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#include "jv_imu.h"

/* IMU SPI defines */
#define IMU_SPI_MASTER SPI2
#define IMU_GPIO_PORT_MASTER_SCK GPIOB
#define IMU_GPIO_PORT_MASTER_MISO GPIOB
#define IMU_GPIO_PORT_MASTER_MOSI GPIOB
#define IMU_GPIO_PORT_MASTER_CS GPIOB
#define IMU_GPIO_PIN_SPI_MASTER_SCK GPIO_PIN_7
#define IMU_GPIO_PIN_SPI_MASTER_MISO GPIO_PIN_4
#define IMU_GPIO_PIN_SPI_MASTER_MOSI GPIO_PIN_5
#define IMU_GPIO_PIN_SPI_MASTER_CS GPIO_PIN_6
#define IMU_GPIO_AF_SPI_MASTER_SCK GPIO_AF1_SPI2
#define IMU_GPIO_AF_SPI_MASTER_MISO GPIO_AF1_SPI2
#define IMU_GPIO_AF_SPI_MASTER_MOSI GPIO_AF1_SPI2
#define IMU_GPIO_AF_SPI_MASTER_CS GPIO_AF1_SPI2
#define __HAL_RCC_SPI_MASTER_CLK_ENABLE() __HAL_RCC_SPI2_CLK_ENABLE()
#define __HAL_RCC_SPI_MASTER_FORCE_RESET() __HAL_RCC_SPI2_FORCE_RESET()
#define __HAL_RCC_SPI_MASTER_RELEASE_RESET() __HAL_RCC_SPI2_RELEASE_RESET()
#define __HAL_RCC_SPI_MASTER_CLK_DISABLE __HAL_RCC_SPI2_CLK_DISABLE
#define IMU_SPI_MASTER_IRQn SPI2_IRQn
#define IMU_SPI_MASTER_IRQHandler SPI2_IRQHandler
#define IMU_INT1_PIN GPIO_PIN_3
#define IMU_INT2_PIN GPIO_PIN_15
#define IMU_INT1_PORT GPIOB
#define IMU_INT2_PORT GPIOB
#define IMU_INT1_EXTI_LINE EXTI_LINE_PB3
#define IMU_INT2_EXTI_LINE EXTI_LINE_PB15
#define IMU_INT_EXTI_IRQn GPIOB_IRQn
#define IMU_INT_IRQHANDLER GPIOA_IRQHandler

EXTI_HandleTypeDef HEXTI_InitStructure;

extern volatile uint8_t data_ready;

void imu_interrupt_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_ConfigTypeDef EXTI_Config_InitStructure = {0};

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_SYSCFG_CLK_ENABLE();

    /* Configure pin as input pulldown */
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLDOWN;
    GPIO_InitStructure.Pin = IMU_INT1_PIN;
    HAL_GPIO_Init(IMU_INT1_PORT, &GPIO_InitStructure);

    /* Configure pin as input pulldown */
    GPIO_InitStructure.Mode = GPIO_MODE_INPUT;
    GPIO_InitStructure.Pull = GPIO_PULLDOWN;
    GPIO_InitStructure.Pin = IMU_INT2_PIN;
    HAL_GPIO_Init(IMU_INT2_PORT, &GPIO_InitStructure);

    EXTI_Config_InitStructure.Line = IMU_INT1_EXTI_LINE;
    EXTI_Config_InitStructure.Trigger = EXTI_TRIGGER_RISING_EDGE;
    EXTI_Config_InitStructure.Type = EXTI_TYPE_EDGE;
    HAL_EXTI_SetConfigLine(&HEXTI_InitStructure, &EXTI_Config_InitStructure);
    HAL_EXTI_RegisterCallback(&HEXTI_InitStructure, HAL_EXTI_COMMON_CB_ID, imu_EXTI_callback);
    HAL_EXTI_Cmd(&HEXTI_InitStructure, ENABLE);
    HAL_EXTI_ClearPending(&HEXTI_InitStructure);

    HAL_NVIC_SetPriority(IMU_INT_EXTI_IRQn, IRQ_HIGH_PRIORITY);
    imu_interrupt_enable();
}

void imu_interrupt_enable(void)
{
    HAL_NVIC_EnableIRQ(IMU_INT_EXTI_IRQn);
}

void imu_interrupt_disable(void)
{
    HAL_NVIC_DisableIRQ(IMU_INT_EXTI_IRQn);
}

void imu_EXTI_callback(uint32_t Line)
{
	(void)Line;
    data_ready = 1;
}

static void test_delay(void)
{
    for (volatile uint32_t i = 0; i < 100000; i++)
        ;
}

void GPIOB_IRQHandler(void)
{
    HAL_EXTI_IRQHandler(&HEXTI_InitStructure);
}

int32_t imu_init(stmdev_ctx_t *dev_ctx)
{
    int32_t result = 0;
    uint8_t whoamI = 0;
    lsm6dso32_i3c_disable_t i3c_disable = 0;
    lsm6dso32_pin_int1_route_t int1_ctrl = {0};
    lsm6dso32_pin_int2_route_t int2_ctrl = {0};

    do
    {
        result |= lsm6dso32_device_id_get(dev_ctx, &whoamI);                 // get device id
    } while (whoamI != LSM6DSO32_ID);                                        // until valid device id
    lsm6dso32_boot_set(dev_ctx, PROPERTY_ENABLE);                            // Reboot
    test_delay();                                                            // Wait a bit
    result |= lsm6dso32_reset_set(dev_ctx, PROPERTY_ENABLE);                 // Restore default configuration
    test_delay();                                                            // Wait a bit
    do                                                                       //
    {                                                                        //
        result |= lsm6dso32_i3c_disable_set(dev_ctx, LSM6DSO32_I3C_DISABLE); // Disable I3C interface
        result |= lsm6dso32_i3c_disable_get(dev_ctx, &i3c_disable);          //
    } while (i3c_disable != LSM6DSO32_I3C_DISABLE);                          // // check I3C disabled
    result |= lsm6dso32_block_data_update_set(dev_ctx, PROPERTY_ENABLE);     // Enable Block Data Update
    result |= lsm6dso32_fifo_mode_set(dev_ctx, LSM6DSO32_BYPASS_MODE);       // Bypass FIFO
    result |= lsm6dso32_xl_data_rate_set(dev_ctx, XL_ODR);                   // Set XL Output Data Rate
    result |= lsm6dso32_xl_full_scale_set(dev_ctx, LSM6DSO32_4g);            // Set XL full scale
    result |= lsm6dso32_gy_data_rate_set(dev_ctx, GY_ODR);                   // Set G Output Data Rate
    result |= lsm6dso32_gy_full_scale_set(dev_ctx, LSM6DSO32_2000dps);       // Set G full scale
    result |= lsm6dso32_timestamp_set(dev_ctx, PROPERTY_ENABLE);             // Enable timestamp
#ifdef IMU_POWER_OFF                                                         //
    int1_ctrl.int1_ctrl.int1_drdy_xl = PROPERTY_ENABLE;                      // xl data ready enable
    int2_ctrl.int2_ctrl.int2_drdy_g = PROPERTY_ENABLE;                       // g data ready enable
    result |= lsm6dso32_data_ready_mode_set(dev_ctx, LSM6DSO32_DRDY_PULSED); // set data ready to pulsed
    result |= lsm6dso32_pin_int1_route_set(dev_ctx, &int1_ctrl);             // xl data ready on int1
    result |= lsm6dso32_pin_int2_route_set(dev_ctx, &int2_ctrl);             // g data ready on int2
#endif
    return result;
}

int32_t imu_get_packet_data(stmdev_ctx_t *dev_ctx, uint16_t *buf)
{
    int32_t result = 0;
    uint8_t whoamI = 0;
    uint8_t drdy = 0;
    lsm6dso32_reg_t reg;

    do
    {
        lsm6dso32_status_reg_get(dev_ctx, &reg.status_reg); // get data ready flags
    } while (!reg.status_reg.gda | !reg.status_reg.xlda);   // until xl/g data is ready

    result |= lsm6dso32_timestamp_raw_get(dev_ctx, (uint32_t *)buf); // read timestamp, 4 bytes

    if (reg.status_reg.tda)                                        // if temperature data ready
        result |= lsm6dso32_temperature_raw_get(dev_ctx, buf + 2); // read temperature, 2 bytes
    // else                                                           // else
    //     *(buf + 2) = (uint16_t)0;                                  // set to zero

    if (reg.status_reg.gda)                                         // if g data ready
        result |= lsm6dso32_angular_rate_raw_get(dev_ctx, buf + 3); // read angular rate, 6 bytes
    // else                                                            // else
    // {                                                               //
    //     *(buf + 3) = (uint16_t)0;                                   // set to zero
    //     *(buf + 4) = (uint16_t)0;
    //     *(buf + 5) = (uint16_t)0;
    // }
    if (reg.status_reg.xlda)                                        // if xl data ready
        result |= lsm6dso32_acceleration_raw_get(dev_ctx, buf + 6); // read acceleration, 6 bytes
    // else                                                            // else
    // {                                                               //
    //     *(buf + 6) = (uint16_t)0;                                   // set to zero
    //     *(buf + 7) = (uint16_t)0;
    //     *(buf + 8) = (uint16_t)0;
    // }

    return result;
}

int32_t imu_power_off(stmdev_ctx_t *dev_ctx)
{
    int32_t result = 0;
    result |= lsm6dso32_xl_data_rate_set(dev_ctx, LSM6DSO32_XL_ODR_OFF); // xl power down
    result |= lsm6dso32_gy_data_rate_set(dev_ctx, LSM6DSO32_GY_ODR_OFF); // g power down
    return result;
}

int32_t imu_power_on(stmdev_ctx_t *dev_ctx)
{
    int32_t result = 0;
    result |= lsm6dso32_xl_data_rate_set(dev_ctx, XL_ODR); // xl power up
    result |= lsm6dso32_gy_data_rate_set(dev_ctx, GY_ODR); // g power up
    return 0;
}

int32_t platform_write(void *handle, uint8_t Reg, const uint8_t *Bufp, uint16_t len)
{
    HAL_GPIO_WritePin(IMU_GPIO_PORT_MASTER_CS, IMU_GPIO_PIN_SPI_MASTER_CS, GPIO_PIN_RESET); // NSS low
    for (uint16_t i = 1; i <= len; i++)                                                     // iterate over aTxBuffer
        aTxBuffer[i] = Bufp[i - 1];                                                         // set aTxBuffer to zero
    aTxBuffer[0] = Reg;                                                                     // add Reg to aTxBuffer
    ubNbDataToTransmit = len + 1;                                                           // set data to transmit
    uint8_t success = HAL_SPI_Transmit(                                                     //
        &hspiMaster, (uint8_t *)aTxBuffer, ubNbDataToTransmit, 10);                         // transmit
    HAL_GPIO_WritePin(IMU_GPIO_PORT_MASTER_CS, IMU_GPIO_PIN_SPI_MASTER_CS, GPIO_PIN_SET);   // NSS high
    if (success != HAL_OK)                                                                  // check for success
        return -1;
    else
        return 0;
}

int32_t platform_read(void *handle, uint8_t Reg, uint8_t *Bufp, uint16_t len)
{
    HAL_GPIO_WritePin(IMU_GPIO_PORT_MASTER_CS, IMU_GPIO_PIN_SPI_MASTER_CS, GPIO_PIN_RESET); // NSS low
    for (uint16_t i = 0; i <= len; i++)                                                     // iterate over buffers
    {                                                                                       //
        aTxBuffer[i] = 0;                                                                   // set aTxBuffer to zero
        aRxBuffer[i] = 0;                                                                   // set aRxBuffer to zero
    }                                                                                       //
    aTxBuffer[0] = Reg | 0x80;                                                              // add Reg to aTxbuffer
    ubNbDataToTransmit = len + 1;                                                           // set data to transmit
    uint8_t success = HAL_SPI_TransmitReceive(                                              //
        &hspiMaster, (uint8_t *)aTxBuffer, (uint8_t *)aRxBuffer, ubNbDataToTransmit, 10);   // transmit
    HAL_GPIO_WritePin(IMU_GPIO_PORT_MASTER_CS, IMU_GPIO_PIN_SPI_MASTER_CS, GPIO_PIN_SET);   // NSS high
    for (uint16_t i = 0; i < len; i++)                                                      // copy results to Bufp
        Bufp[i] = aRxBuffer[i + 1];                                                         //
    if (success != HAL_OK)                                                                  // check for success
        return -1;
    else
        return 0;
}

void MX_SPI_MASTER_Init(void)
{
    /* GPIO Ports Clock Enable */
    __HAL_RCC_GPIOB_CLK_ENABLE();
    /* SPI_MASTER parameter configuration*/
    hspiMaster.Instance = IMU_SPI_MASTER;
    hspiMaster.Init.Mode = SPI_MODE_MASTER;
    hspiMaster.Init.Direction = SPI_DIRECTION_2LINES;
    hspiMaster.Init.CLKPolarity = SPI_POLARITY_HIGH;
    hspiMaster.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspiMaster.Init.DataSize = SPI_DATASIZE_8BIT;
    hspiMaster.Init.NSS = SPI_NSS_SOFT;
    hspiMaster.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspiMaster.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspiMaster.Init.TIMode = SPI_TIMODE_DISABLE;
    hspiMaster.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspiMaster.Init.CRCPolynomial = 7;
    hspiMaster.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    hspiMaster.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
    if (HAL_SPI_Init(&hspiMaster) != HAL_OK)
    {
        while (1)
            ;
    }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == IMU_SPI_MASTER)
    {
        /* Peripheral clock enable */
        __HAL_RCC_SPI_MASTER_CLK_ENABLE();

        /* SCK */
        GPIO_InitStruct.Pin = IMU_GPIO_PIN_SPI_MASTER_SCK;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = IMU_GPIO_AF_SPI_MASTER_SCK;
        HAL_GPIO_Init(IMU_GPIO_PORT_MASTER_SCK, &GPIO_InitStruct);

        /* MOSI */
        GPIO_InitStruct.Pin = IMU_GPIO_PIN_SPI_MASTER_MOSI;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = IMU_GPIO_AF_SPI_MASTER_MOSI;
        HAL_GPIO_Init(IMU_GPIO_PORT_MASTER_MOSI, &GPIO_InitStruct);

        /* MISO */
        GPIO_InitStruct.Pin = IMU_GPIO_PIN_SPI_MASTER_MISO;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_InitStruct.Alternate = IMU_GPIO_AF_SPI_MASTER_MISO;
        HAL_GPIO_Init(IMU_GPIO_PORT_MASTER_MISO, &GPIO_InitStruct);

        /* NSS */
        GPIO_InitStruct.Pin = IMU_GPIO_PIN_SPI_MASTER_CS;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_WritePin(IMU_GPIO_PORT_MASTER_CS, IMU_GPIO_PIN_SPI_MASTER_CS, GPIO_PIN_SET);
        HAL_GPIO_Init(IMU_GPIO_PORT_MASTER_CS, &GPIO_InitStruct);
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == IMU_SPI_MASTER)
    {
        /* Reset peripherals */
        __HAL_RCC_SPI_MASTER_FORCE_RESET();
        __HAL_RCC_SPI_MASTER_RELEASE_RESET();

        /* Peripheral clock disable */
        __HAL_RCC_SPI_MASTER_CLK_DISABLE();

        HAL_GPIO_DeInit(IMU_GPIO_PORT_MASTER_SCK, IMU_GPIO_PIN_SPI_MASTER_SCK);
        HAL_GPIO_DeInit(IMU_GPIO_PORT_MASTER_MOSI, IMU_GPIO_PIN_SPI_MASTER_MOSI);
        HAL_GPIO_DeInit(IMU_GPIO_PORT_MASTER_MISO, IMU_GPIO_PIN_SPI_MASTER_MISO);

        /* SPI_MASTER interrupt DeInit */
        HAL_NVIC_DisableIRQ(IMU_SPI_MASTER_IRQn);
    }
}
