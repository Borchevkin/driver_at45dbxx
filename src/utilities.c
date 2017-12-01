/*
 * utilities.c
 *
 *  Created on: 20 сент. 2017 г.
 *      Author: user
 */

#include <stdint.h>
#include <stdbool.h>
#include "em_usart.h"
#include "em_gpio.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_int.h"
#include "spidrv.h"
#include "InitDevice.h"
#include "utilities.h"
#include "driver_at45dbxx.h"

SPIDRV_HandleData_t handleData;
SPIDRV_Handle_t handle = &handleData;

void SPIDRV_Setup(void)
{
	// Set up the necessary peripheral clocks
	CMU_ClockEnable(cmuClock_GPIO, true);

	GPIO_DriveModeSet(gpioPortD, gpioDriveModeLow);

	// Enable the GPIO pins for the misc signals, leave pulled high
	GPIO_PinModeSet(gpioPortD, 4, gpioModePushPullDrive, 1);          // WP#
	GPIO_PinModeSet(gpioPortD, 5, gpioModePushPullDrive, 1);          // RESET#

	// Initialize and enable the SPIDRV
	SPIDRV_Init_t initData = SPIDRV_MASTER_USART1;
	initData.clockMode = spidrvClockMode3;

	// Initialize a SPI driver instance
	SPIDRV_Init( handle, &initData );
}

volatile uint32_t msTicks = 0;

void _ErrorHandler(void)
{
	DEBUG_BREAK;
}

void Delay(uint32_t dlyTicks)
{
	uint32_t curTicks;

	curTicks = msTicks;
	while ((msTicks - curTicks) < dlyTicks) ;
}

void SysTick_Handler(void)
{
      msTicks++;
}

void SetupUtilities(void)
{
	USART_IntClear(USART1, USART_IF_TXC);
	USART_IntEnable(USART1, USART_IF_TXC);
	NVIC_ClearPendingIRQ(USART0_TX_IRQn);
	NVIC_EnableIRQ(USART0_TX_IRQn);

	USART_IntClear(USART1, USART_IF_RXDATAV);
	USART_IntEnable(USART1, USART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(USART0_RX_IRQn);
	NVIC_EnableIRQ(USART0_RX_IRQn);
}

void SPI1_Transfer(uint8_t *tx, uint8_t *rx, uint16_t num)
{
	SPIDRV_MTransferB( handle, tx, rx, num);
}


void FLASH_SetWP(void)
{
	GPIO_PinOutSet(WP_PORT, WP_PIN);
}

void FLASH_ClearWP(void)
{
	GPIO_PinOutClear(WP_PORT, WP_PIN);
}

void FLASH_SetRESET(void)
{
	GPIO_PinOutSet(RESET_PORT, RESET_PIN);
}

void FLASH_ClearRESET(void)
{
	GPIO_PinOutClear(RESET_PORT, RESET_PIN);
}
