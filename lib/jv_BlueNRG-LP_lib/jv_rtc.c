/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_rtc.c
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @editor Stiliyan Dimanov (stiliyan.dimanov@jeevawireless.com)
 * @brief Functions for using the RTC on BlueNRG-LP
 * @date 2022-10-15
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#include "jv_rtc.h"
#include "rf_driver_ll_bus.h"
#include "rf_driver_ll_rtc.h"
#include "rf_driver_ll_rcc.h"
#include "rf_driver_ll_pwr.h"
#include "system_BlueNRG_LP.h"


static volatile bool RTC_WU_Completed;

void RTC_WakeupInit(void)
{
    LL_APB0_EnableClock(LL_APB0_PERIPH_RTC);                       // Enable Peripheral Clock
    LL_RCC_ClearFlag_RTCRSTREL();                                  // RTC peripheral reset
    LL_APB0_ForceReset(LL_APB0_PERIPH_RTC);                        //
    LL_APB0_ReleaseReset(LL_APB0_PERIPH_RTC);                      //
    while (LL_RCC_IsActiveFlag_RTCRSTREL() == 0)                   //
        ;                                                          //
    LL_RCC_ClearFlag_RTCRSTREL();                                  //
    LL_RTC_DisableWriteProtection(RTC);                            // Disable the write protection for RTC registers
    LL_RTC_EnableInitMode(RTC);                                    // Init mode setup
    while (LL_RTC_IsActiveFlag_INIT(RTC) == RESET)                 // Wait till the Init mode is active
        ;                                                          //
    LL_RTC_SetHourFormat(RTC, LL_RTC_HOURFORMAT_24HOUR);           // Configure Hour Format
    LL_RTC_SetAlarmOutEvent(RTC, LL_RTC_ALARMOUT_DISABLE);         // Output disabled
    LL_RTC_SetOutputPolarity(RTC, LL_RTC_OUTPUTPOLARITY_PIN_HIGH); // Output polarity
    LL_RTC_SetAsynchPrescaler(RTC, 0x7F);                          // Set Asynchronous prescaler factor
    LL_RTC_SetSynchPrescaler(RTC, 0x00FF);                         // Set Synchronous prescaler factor
    LL_RTC_DisableInitMode(RTC);                                   // Exit Initialization mode
    LL_RTC_WAKEUP_SetClock(RTC, LL_RTC_WAKEUPCLOCK_DIV_2); 		// Configure the clock source
    LL_RTC_EnableWriteProtection(RTC);                     		// Enable write protection
    NVIC_SetPriority(RTC_IRQn, IRQ_LOW_PRIORITY);          		// Configure NVIC for RTC
    NVIC_EnableIRQ(RTC_IRQn);                              		// Enable interrupt
}

void SetRTC_WakeupTimeout(uint32_t time)
{
    RTC_WU_Completed = false;
    LL_RTC_DisableWriteProtection(RTC);                    // Disable write protection
    LL_RTC_WAKEUP_Disable(RTC);                            // Disable Wake-up Timer
    LL_RTC_DisableIT_WUT(RTC);                             // In case of interrupt mode is used, the interrupt source must disabled
    while (LL_RTC_IsActiveFlag_WUTW(RTC) == 0)             // Wait till RTC WUTWF flag is set
        ;                                                  //
    LL_PWR_ClearWakeupSource(LL_PWR_EWS_INT);              // Clear PWR wake up Flag
    LL_RTC_ClearFlag_WUT(RTC);                             // Clear RTC Wake Up timer Flag
    LL_RTC_WAKEUP_SetAutoReload(RTC, time);                // Configure the Wake-up Timer counter
    LL_RTC_EnableIT_WUT(RTC);                              // Configure the Interrupt in the RTC_CR register
    LL_RTC_WAKEUP_Enable(RTC);                             // Enable the Wake-up Timer
    LL_RTC_EnableWriteProtection(RTC);                     // Enable write protection
}

void DisableRTC_WakeupTimeout(void)
{
    LL_RTC_DisableWriteProtection(RTC); // Disable write protection
    LL_RTC_WAKEUP_Disable(RTC);         // Disable the Wake-up Timer
    LL_RTC_EnableWriteProtection(RTC);  // Enable write protection
}

bool RTC_WakeupTimeout_Expired(void)
{
    if (RTC_WU_Completed)
    {
    	RTC_WU_Completed = false;
    	return true;
    }
    return false;
}

void RTC_IRQHandler(void)
{
    if (LL_RTC_IsActiveFlag_WUT(RTC))
    {
        RTC_WU_Completed = true;
        LL_RTC_ClearFlag_WUT(RTC);
    }
}
