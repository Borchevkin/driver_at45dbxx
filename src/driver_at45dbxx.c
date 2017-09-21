/*
 * driver_at45dbxx.c
 *
 *  Created on: 20 сент. 2017 г.
 *      Author: user
 */

#include <stdint.h>
#include <stdbool.h>
#include "em_device.h"
#include "em_chip.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "em_usart.h"
#include "em_int.h"
#include "spidrv.h"
#include "driver_at45dbxx.h"
#include "utilities.h"

SPIDRV_HandleData_t handleData;
SPIDRV_Handle_t handle = &handleData;

void spidrv_setup()
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

void AT45DBXX_ChipErase(at45dbxx_t * at45dbxx)
{
	uint8_t status;
	uint8_t tx_data[4];
	memset(tx_data,0x00,4);
	uint8_t rx_data[4];
	memset(rx_data,0x00,4);

	tx_data[0] = 0xC7;
	tx_data[1] = 0x94;
	tx_data[2] = 0x80;
	tx_data[3] = 0x9A;

	SPIDRV_MTransferB( handle, &tx_data, &rx_data, 4);

	do status = AT45DBXX_ReadStatus(at45dbxx);
	while (!(status & STATUS_RDY));
}

void AT45DBXX_ReadMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t result[], uint32_t num_of_bytes)
{
	uint8_t tx_data[SPI_TRANSFER_SIZE];
	memset(tx_data,0x00,SPI_TRANSFER_SIZE);
	uint8_t rx_data[SPI_TRANSFER_SIZE];
	memset(rx_data,0x00,SPI_TRANSFER_SIZE);

	tx_data[0] = READ_DATA;
	tx_data[1] = (address >> 16);
	tx_data[2] = (address >> 8);
	tx_data[3] = address;

	SPIDRV_MTransferB( handle, &tx_data, &rx_data, SPI_TRANSFER_SIZE);

	// Fill the result from the right index
	memcpy(result,rx_data + 4,PAGE_SIZE);
}

void AT45DBXX_WriteMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t data_buffer[], uint32_t num_of_bytes)
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

	SPIDRV_MTransferB( handle, &tx_data, &dummy_rx, SPI_TRANSFER_SIZE);

	do status = AT45DBXX_ReadStatus(at45dbxx);
	while (!(status & STATUS_RDY));
}

void AT45DBXX_ConfigWrite(at45dbxx_t * at45dbxx, uint8_t command)
{
	uint8_t result[1];
	uint8_t tx_data[1];
	memset(tx_data,0x00,1);
	memset(result,0x00,1);

	tx_data[0] = command;

	SPIDRV_MTransferB( handle, &tx_data, &result, 1);
}

void AT45DBXX_CheckID(at45dbxx_t * at45dbxx)
{
	uint8_t result[4];
	uint8_t tx_data[4];

	memset(tx_data,0x00,4);
	memset(result,0x00,4);

	tx_data[0] = JEDEC_ID_CMD;

	SPIDRV_MTransferB( handle, &tx_data, &result, 4);

	// Check the result for what is expected from the Spansion spec
	if (result[1] != 0x1F || result[2] != 0x24 || result[3] != 0x00)
	{
		DEBUG_BREAK
	}
}

uint8_t AT45DBXX_ReadStatus(at45dbxx_t * at45dbxx)
{
	uint8_t result[2];
	uint8_t tx_data[2];

	memset(tx_data,0x00,2);
	memset(result,0x00,2);

	tx_data[0] = STATUS;

	SPIDRV_MTransferB( handle, &tx_data, &result, 2);

	return result[1];
}
