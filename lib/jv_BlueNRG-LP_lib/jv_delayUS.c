/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_delayUS.c
 * @author Stiliyan Dimanov (stiliyan.dimanov@jeevawireless.com)
 * @brief Library for Delay in microseconds functions
 * @date 2022-10-11
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */
#include "rf_driver_hal.h"
#include <stdbool.h>

TIM_HandleTypeDef htimx;
volatile bool time_elapsed = false;


int jv_delayUS_Init(void)
{
	TIM_ClockConfigTypeDef sClockSourceConfig;

	htimx.Instance = TIM1;
	htimx.Init.Prescaler = (uint32_t)((HAL_TIM_GetPeriphClock(htimx.Instance) / 1000000) - 1);
	htimx.Init.CounterMode = TIM_COUNTERMODE_DOWN;
	htimx.Init.Period = 0xFFFF;
	htimx.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htimx.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htimx) != HAL_OK)
	{
		return -1;
	}

	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htimx, &sClockSourceConfig) != HAL_OK)
	{
		return -1;
	}
	return 0;
}

void jv_delayUS(uint16_t us)
{
	// compensation
    if (us >= 5) us -= 5;

    htimx.Instance->ARR = us;
    htimx.Instance->CNT = us;

//    HAL_TIM_Base_Start(&htimx);
//    while (htimx.Instance->CNT)
//    {
//    	__NOP();
//    }
//    HAL_TIM_Base_Stop(&htimx);

	time_elapsed = false;
	/* Clear the TMR ISR flag first */
    __HAL_TIM_CLEAR_IT(&htimx, TIM_IT_UPDATE);

	HAL_TIM_Base_Start_IT(&htimx);
	while (time_elapsed != 1)
	{
		__WFE();
	}
	HAL_TIM_Base_Stop_IT(&htimx);
}

/**
* @brief TIM_Base MSP Initialization
* @param htim_base: TIM_Base handle pointer
* @retval None
*/
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
	if(htim_base->Instance == TIM1)
	{
		/* Peripheral clock enable */
		__HAL_RCC_TIM1_CLK_ENABLE();
		/* TIM1 interrupt Init */
		HAL_NVIC_SetPriority(TIM1_IRQn, IRQ_HIGH_PRIORITY);
		HAL_NVIC_EnableIRQ(TIM1_IRQn);
	}
}

/**
* @brief  Period elapsed callback in non blocking mode
* @param  htim : TIM handle
* @retval None
*/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	time_elapsed = true;
}

