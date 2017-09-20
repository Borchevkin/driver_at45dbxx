/*
 * utilities.c
 *
 *  Created on: 20 сент. 2017 г.
 *      Author: user
 */



#include <stdint.h>
#include "em_usart.h"
#include "em_gpio.h"
#include "InitDevice.h"
#include "utilities.h"
#include "driver_at45dbxx.h"

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
      /* Increment counter necessary in Delay()*/
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
