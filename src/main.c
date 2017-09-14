
#include <stdio.h>
#include <stdarg.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_usart.h"
#include "em_int.h"
#include "spidrv.h"
#include "InitDevice.h"

#define DEBUG_BREAK		__asm__("BKPT #0");
#define JEDEC_ID_CMD	0x9F

/* Counts 1ms timeTicks */
volatile uint32_t msTicks = 0;

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

void usart_setup()
{
	// Set up the necessary peripheral clocks
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(cmuClock_USART1, true);

	GPIO_DriveModeSet(gpioPortD, gpioDriveModeLow);

	// Enable the GPIO pins for the USART, starting with CS
	// This is to avoid clocking the flash chip when we set CLK high
	GPIO_PinModeSet(USART1_CS_PORT, USART1_CS_PIN, gpioModePushPullDrive, 1);		// CS
	GPIO_PinModeSet(USART1_TX_PORT, USART1_TX_PIN, gpioModePushPullDrive, 0);		// MOSI
	GPIO_PinModeSet(USART1_RX_PORT, USART1_RX_PIN, gpioModeInput, 0);				// MISO
	GPIO_PinModeSet(USART1_CLK_PORT, USART1_CLK_PIN, gpioModePushPullDrive, 1);		// CLK

	// Enable the GPIO pins for the misc signals, leave pulled high
	GPIO_PinModeSet(gpioPortD, 4, gpioModePushPullDrive, 1);		// WP#
	GPIO_PinModeSet(gpioPortD, 5, gpioModePushPullDrive, 1);		// HOLD#

	// Initialize and enable the USART
	USART_InitSync_TypeDef init = USART_INITSYNC_DEFAULT;
	init.clockMode = usartClockMode3;
	init.msbf = true;

	USART_InitSync(USART1, &init);

	// Connect the USART signals to the GPIO peripheral
	USART1->ROUTE = USART_ROUTE_RXPEN | USART_ROUTE_TXPEN |
			USART_ROUTE_CLKPEN | USART_ROUTE_LOCATION_LOC1;
}


/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	CHIP_Init();

	usart_setup();

	if (SysTick_Config(CMU_ClockFreqGet(cmuClock_CORE) / 1000))
	{
		DEBUG_BREAK;
	}

	USART_IntClear(USART1, USART_IF_TXC);
	USART_IntEnable(USART1, USART_IF_TXC);
	NVIC_ClearPendingIRQ(USART0_TX_IRQn);
	NVIC_EnableIRQ(USART0_TX_IRQn);

	USART_IntClear(USART1, USART_IF_RXDATAV);
	USART_IntEnable(USART1, USART_IF_RXDATAV);
	NVIC_ClearPendingIRQ(USART0_RX_IRQn);
	NVIC_EnableIRQ(USART0_RX_IRQn);

	Delay(100);

	uint8_t result[3];
	uint8_t index = 0;

	GPIO_PinModeSet(USART1_CS_PORT, USART1_CS_PIN, gpioModePushPullDrive, 0);

	// Send the command, discard the first response
	USART_SpiTransfer(USART1, JEDEC_ID_CMD);

	// Now send garbage, but keep the results
	result[index++] = USART_SpiTransfer(USART1, 1);
	result[index++] = USART_SpiTransfer(USART1, 2);
	result[index++] = USART_SpiTransfer(USART1, 3);

	GPIO_PinModeSet(USART1_CS_PORT, USART1_CS_PIN, gpioModePushPullDrive, 1);

	// Check the result for what is expected from the Spansion spec
	if (result[0] != 1 || result[1] != 0x40 || result[2] != 0x13)
	{
		DEBUG_BREAK
	}

	while (1)
		;
}
