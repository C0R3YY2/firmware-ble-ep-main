/*       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * Author: Stiliyan Dimanov (stiliyan.dimanov@jeevawireless.com)
 * Copyright: 2022 Jeeva Wireless
 * License: Proprietary
 */
#include "rf_driver_hal.h"
#include "rf_driver_hal_vtimer.h"


extern TIM_HandleTypeDef htimx;

/****************************************************************************/
/*           Cortex Processor Interruption and Exception Handlers           */ 
/****************************************************************************/
void NMI_IRQHandler(void)
{
    /* Go to infinite loop when NMI exception occurs */
    while (1)
    {}
}

void HardFault_IRQHandler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {}
}

void SVC_IRQHandler(void)
{
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_IRQHandler(void)
{
   HAL_IncTick();
   HAL_SYSTICK_IRQHandler();
}

/**
* @brief This function handles TIM1 global interrupt.
*/
void TIM1_IRQHandler(void)
{
	HAL_TIM_IRQHandler(&htimx);
}

void BLE_WKUP_IRQHandler(void)
{
    HAL_VTIMER_WakeUpCallback();
}

void CPU_WKUP_IRQHandler(void) 
{
    HAL_VTIMER_TimeoutCallback();
}

void BLE_TX_RX_IRQHandler(void)
{
    RADIO_IRQHandler();
    HAL_VTIMER_RadioTimerIsr();
}

// void BLE_ERROR_IRQHandler(void)
// {
//   volatile uint32_t debug_cmd;
//   BLUE->DEBUGCMDREG |= 1;
//   /* If the device is configured with 
//      System clock = 64 MHz and BLE clock = 16 MHz
//      a register read is necessary to end fine  
//      the clear interrupt register operation,
//      due the AHB down converter latency */ 
//   debug_cmd = BLUE->DEBUGCMDREG;
// }
