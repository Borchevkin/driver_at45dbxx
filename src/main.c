
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

// Commands
#define DEBUG_BREAK		__asm__("BKPT #0");
#define JEDEC_ID_CMD	0x9F
#define STATUS			0x05
#define WR_ENABLE		0x06
#define PAGE_PROGRAM	0x02
#define CHIP_ERASE		0xC7
#define READ_DATA		0x03

// Bit positions for status register
#define WIP_BIT			1 << 0
#define WEL_BIT			1 << 1

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

SPIDRV_HandleData_t handleData;
SPIDRV_Handle_t handle = &handleData;


#define PAGE_SIZE			256
#define SPI_TRANSFER_SIZE 	PAGE_SIZE + 4  // Account for SPI header

void spidrv_setup()
{
	// Set up the necessary peripheral clocks
	CMU_ClockEnable(cmuClock_GPIO, true);

	GPIO_DriveModeSet(gpioPortD, gpioDriveModeLow);

	// Enable the GPIO pins for the misc signals, leave pulled high
	GPIO_PinModeSet(gpioPortD, 4, gpioModePushPullDrive, 1);          // WP#
	GPIO_PinModeSet(gpioPortD, 5, gpioModePushPullDrive, 1);          // HOLD#

	// Initialize and enable the SPIDRV
	SPIDRV_Init_t initData = SPIDRV_MASTER_USART1;
	initData.clockMode = spidrvClockMode3;

	// Initialize a SPI driver instance
	SPIDRV_Init( handle, &initData );
}

// Writes a single byte
void config_write(uint8_t command)
{
	const int size = 1;
	uint8_t result[size];
	uint8_t tx_data[size];

	tx_data[0] = command;

	SPIDRV_MTransferB( handle, &tx_data, &result, size);
}

// Reads the status
uint8_t read_status()
{
	const int size = 2;
	uint8_t result[size];
	uint8_t tx_data[size];

	tx_data[0] = STATUS;

	SPIDRV_MTransferB( handle, &tx_data, &result, size);
	return result[1];
}

void chip_erase()
{
	uint8_t status;
	config_write(WR_ENABLE);

	status = read_status();
	if (!(status & WEL_BIT))
	{
		DEBUG_BREAK
	}

	config_write(CHIP_ERASE);

	do status = read_status();
	while (status & WIP_BIT);
}

void read_memory(uint32_t address, uint8_t result[], uint32_t num_of_bytes)
{
	uint8_t tx_data[SPI_TRANSFER_SIZE];
	uint8_t rx_data[SPI_TRANSFER_SIZE];

	tx_data[0] = READ_DATA;
	tx_data[1] = (address >> 16);
	tx_data[2] = (address >> 8);
	tx_data[3] = address;

	SPIDRV_MTransferB( handle, &tx_data, &rx_data, SPI_TRANSFER_SIZE);

	// Fill the result from the right index
	for (int i=0; i< PAGE_SIZE; i++)
	{
		result[i] = rx_data[i+4];
	}
}

void write_memory(uint32_t address, uint8_t data_buffer[], uint32_t num_of_bytes)
{
	if (num_of_bytes > PAGE_SIZE) DEBUG_BREAK

	uint8_t status;
	uint8_t dummy_rx[SPI_TRANSFER_SIZE];
	uint8_t tx_data[SPI_TRANSFER_SIZE];  // Need room for cmd + three address bytes

	tx_data[0] = PAGE_PROGRAM;
	tx_data[1] = (address >> 16);
	tx_data[2] = (address >> 8);
	tx_data[3] = address;

	for (int i=0; i < PAGE_SIZE; i++)
	{
		if (i >= num_of_bytes) break;
		tx_data[i+4] = data_buffer[i];
	}

	config_write(WR_ENABLE);
	SPIDRV_MTransferB( handle, &tx_data, &dummy_rx, SPI_TRANSFER_SIZE);

	do 	status = read_status();
	while (status & WIP_BIT);
}

/**************************************************************************//**
 * @brief  Main function
 *****************************************************************************/
int main(void)
{
	CHIP_Init();

	spidrv_setup();

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

	uint8_t result[PAGE_SIZE];
	uint8_t tx_data[PAGE_SIZE];


	tx_data[0] = JEDEC_ID_CMD;

	SPIDRV_MTransferB( handle, &tx_data, &result, 4);


	// Check the result for what is expected from the Spansion spec
	if (result[1] != 0x1F || result[2] != 0x24 || result[3] != 0x00)
	{
		DEBUG_BREAK
	}

	// Never enter here except with debugger and �Move to Line�
	if (result[1] == 0x20)
	{
		for (int i=0; i < 256; i++) tx_data[i] = i;

		chip_erase();
		read_memory(0, result, PAGE_SIZE);
		write_memory(0, tx_data, PAGE_SIZE);
		read_memory(0, result, PAGE_SIZE);
	}


	read_memory(0, result, PAGE_SIZE);

	while (1){
		read_memory(0, result, PAGE_SIZE);
		Delay(1000);
	}
}
