/**
 *       _                       __        ___          _
 *      | | ___  _____   ____ _  \ \      / (_)_ __ ___| | ___  ___ ___
 *   _  | |/ _ \/ _ \ \ / / _` |  \ \ /\ / /| | '__/ _ \ |/ _ \/ __/ __|
 *  | |_| |  __/  __/\ V / (_| |   \ V  V / | | | |  __/ |  __/\__ \__ \
 *   \___/ \___|\___| \_/ \__,_|    \_/\_/  |_|_|  \___|_|\___||___/___/
 *
 * @file jv_gpio.c
 * @author Jerrold Erickson (jerrold.erickson@jeevawireless.com)
 * @editor Stiliyan Dimanov (stiliyan.dimanov@jeevawireless.com)
 * @brief Library for GPIO functions
 * @date 2022-09-13
 *
 * @copyright Copyright (c) 2022 Jeeva Wireless
 *
 */

#include "jv_gpio.h"
#include <main.h>

/* Port A */
#define PUPD_IMU_SPI_EN   LL_PWR_PUPD_IO0
#define PUPD_SWCLK        LL_PWR_PUPD_IO3
#define PUPD_SWDIO        LL_PWR_PUPD_IO2
#define PUPD_BOOT         LL_PWR_PUPD_IO10
#define PUPD_IO1          LL_PWR_PUPD_IO11
#define PUPD_IO2          LL_PWR_PUPD_IO1
#define PUPD_IO3          LL_PWR_PUPD_IO8
#define PUPD_IO4          LL_PWR_PUPD_IO9

/* Port B */
#define PUPD_IMU_SPI_CLK  LL_PWR_PUPD_IO7
#define PUPD_IMU_SPI_CS   LL_PWR_PUPD_IO6
#define PUPD_IMU_SPI_MOSI LL_PWR_PUPD_IO5
#define PUPD_IMU_SPI_MISO LL_PWR_PUPD_IO4
#define PUPD_IMU_SPI_I1   LL_PWR_PUPD_IO3
#define PUPD_IMU_SPI_I2   LL_PWR_PUPD_IO15
#define PUPD_MOD          LL_PWR_PUPD_IO14
#define PUPD_SW_EN        LL_PWR_PUPD_I2O
#define PUPD_LED_R        LL_PWR_PUPD_IO1
#define PUPD_LED_O        LL_PWR_PUPD_IO0

void GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
        //enables the clock for GPIO A and B
        //this must be done before configuring GPIO A and B

    /* Pull up resistors (pin is high when there's no data)*/ 
    LL_PWR_EnablePUA(PUPD_SWDIO);
        //pull-up resistor on SWDIO pin (Serial Wire Debug I/O)
        //this is used for debugging (but the em9305 will use UART GPIO7)

    /* Pull down resistors (pin is low when there's no data)*/
    LL_PWR_EnablePDA(PUPD_SWCLK); //SWCLK: Serial Wire Clock for debugging
    LL_PWR_EnablePDA(PUPD_BOOT);  //BOOT: Boot mode selection pin
    LL_PWR_EnablePDA(PUPD_IO1);   //IO1: General purpose I/O pins
    LL_PWR_EnablePDA(PUPD_IO2);   //IO2: General purpose I/O pins
    LL_PWR_EnablePDA(PUPD_IO3);   //IO3: General purpose I/O pins
    LL_PWR_EnablePDA(PUPD_IO4);   //IO4: General purpose I/O pins
    LL_PWR_EnablePDB(PUPD_LED_R); //LED_R: Pin connected to red LEDs
    LL_PWR_EnablePDB(PUPD_LED_O); //LED_O: Pin connected to orange LEDs

//if USE_IMU is not defined, then a pull-down resistor is on ALL IMU-related SPI pins gets 
#ifndef USE_IMU
    LL_PWR_EnablePDA(PUPD_IMU_SPI_EN);
    LL_PWR_EnablePDB(PUPD_IMU_SPI_CLK);
    LL_PWR_EnablePDB(PUPD_IMU_SPI_CS);
    LL_PWR_EnablePDB(PUPD_IMU_SPI_MOSI);
    LL_PWR_EnablePDB(PUPD_IMU_SPI_MISO);
    LL_PWR_EnablePDB(PUPD_IMU_SPI_I1);
    LL_PWR_EnablePDB(PUPD_IMU_SPI_I2);

//if USE_IMU is defined, then a pull-up resistor is on the IMU_SPI_EN pin 
#else
    LL_PWR_EnablePUA(PUPD_IMU_SPI_EN);
#endif

    GPIO_InitTypeDef GPIO_InitStruct = {0};
        //variable that gets reused for each pin configuration
            //here, the pins are:
                //SW_EN_PIN: Switch Enable
                //LED_GPIO_PIN: LED 
                //DBG_GPIO_PIN: 
        //note: GPIO_InitStruct is zero bc it's good practice 


    ////////////////(Switch Enable)////////////////
    GPIO_InitStruct.Pin = SW_EN_PIN;
        //Switch Enable

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        //set to "push-pull output"

    GPIO_InitStruct.Pull = GPIO_PULLUP;
        //pull-UP resistor

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
        //medium speed pin

    jv_gpioSet(SW_EN); //sets the pin high before initializing
    HAL_GPIO_Init(SW_EN_PORT, &GPIO_InitStruct); //initialize SW_EN_PIN settings

    
    ////////////////(LED)////////////////
    GPIO_InitStruct.Pin = LED_GPIO_PIN;
        //LED

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        //set to "push-pull output"

    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        //pull-DOWN resistor

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        //high speed pin

    led_off(); //turns off LED before initializing settings 
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct); //initializes LED settings


    ////////////////(Debug)//////////////
    GPIO_InitStruct.Pin = DBG_GPIO_PIN;
        //Debug

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
        //set to "push-pull output"

    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
        //pull-DOWN resistor

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        //high speed pin

    jv_gpioReset(DBG_GPIO); //set the pin low before initializing
    HAL_GPIO_Init(DBG_GPIO_PORT, &GPIO_InitStruct); //initialize debug settings
}

void led_on(void)
{
    jv_gpioSet(LED_GPIO);
}

void led_off(void)
{
    jv_gpioReset(LED_GPIO);
}

void led_toggle(void)
{
    jv_gpioToggle(LED_GPIO);
}
