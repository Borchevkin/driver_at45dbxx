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

void AT45DBXX_Init(at45dbxx_t * at45dbxx)
{
	at45dbxx->SetupSPI = spidrv_setup;
	at45dbxx->Transfer = SPI1_Transfer;
	at45dbxx->SetWP = FLASH_SetWP;
	at45dbxx->ClearWP = FLASH_ClearWP;
	at45dbxx->SetRESET = FLASH_SetRESET;
	at45dbxx->ClearRESET = FLASH_ClearRESET;
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

	at45dbxx->Transfer(&tx_data, &rx_data, 4);

	//TODO Make non-block
	do status = AT45DBXX_ReadStatus(at45dbxx);
	while (!(status & STATUS_RDY));
}

void AT45DBXX_PageErase(at45dbxx_t * at45dbxx, uint32_t address)
{
	uint8_t status;

	uint8_t tx_data[4];
	memset(tx_data,0x00,4);
	uint8_t rx_data[4];
	memset(rx_data,0x00,4);

	uint32_t page = address << 1;

	tx_data[0] = PAGE_ERASE;
	tx_data[1] = (page >> 8)	& 0xff;
	tx_data[2] = (page)			& 0xff;
	tx_data[3] = 0; //page			& 0xff;

	at45dbxx->Transfer(&tx_data, &rx_data, 4);

	//TODO Make non-block
	do status = AT45DBXX_ReadStatus(at45dbxx);
	while (!(status & STATUS_RDY));
}

void AT45DBXX_ReadMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t result[])
{
	uint8_t tx_data[SPI_TRANSFER_SIZE];
	memset(tx_data,0x00,SPI_TRANSFER_SIZE);
	uint8_t rx_data[SPI_TRANSFER_SIZE];
	memset(rx_data,0x00,SPI_TRANSFER_SIZE);

	uint32_t page = address << 1;

	tx_data[0] = READ_DATA;
	tx_data[1] = (page >> 8)	& 0xff;
	tx_data[2] = (page)			& 0xff;
	tx_data[3] = 0; //page			& 0xff;

	at45dbxx->Transfer(&tx_data, &rx_data, SPI_TRANSFER_SIZE);

	// Fill the result from the right index
	memcpy(result,rx_data+4,PAGE_SIZE);
}

void AT45DBXX_WriteMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t data_buffer[], uint32_t num_of_bytes)
{
	if (num_of_bytes > PAGE_SIZE) DEBUG_BREAK

	uint8_t status;
	uint8_t dummy_rx[SPI_TRANSFER_SIZE];
	uint8_t tx_data[SPI_TRANSFER_SIZE];  // Need room for cmd + three address bytes

	memset(tx_data,0x00,SPI_TRANSFER_SIZE);
	memset(dummy_rx,0x00,SPI_TRANSFER_SIZE);

	uint32_t page = address << 1;

	tx_data[0] = PAGE_PROGRAM;
	tx_data[1] = (page >> 8)	& 0xff;
	tx_data[2] = (page)			& 0xff;
	tx_data[3] = 0; //page			& 0xff;

	for (int i=0; i < PAGE_SIZE; i++)
	{
		if (i >= num_of_bytes) break;
		tx_data[i+4] = data_buffer[i];
	}

	at45dbxx->Transfer(&tx_data, &dummy_rx, SPI_TRANSFER_SIZE);

	//TODO Make non-block
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

	at45dbxx->Transfer(&tx_data, &result, 1);
}

void AT45DBXX_CheckID(at45dbxx_t * at45dbxx)
{
	uint8_t result[4];
	uint8_t tx_data[4];

	memset(tx_data,0x00,4);
	memset(result,0x00,4);

	tx_data[0] = JEDEC_ID_CMD;

	at45dbxx->Transfer(&tx_data, &result, 4);

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

	at45dbxx->Transfer(&tx_data, &result, 2);

	return result[1];
}

void AT45DBXX_BufferRead(at45dbxx_t * at45dbxx, uint8_t number, uint8_t result[])
{
	uint8_t tx_data[SPI_TRANSFER_SIZE];
	memset(tx_data,0x00,SPI_TRANSFER_SIZE);
	uint8_t rx_data[SPI_TRANSFER_SIZE];
	memset(rx_data,0x00,SPI_TRANSFER_SIZE);

	uint8_t buff = 0;
	if (number == 1)buff = BUF_1_READ;
	if (number == 2)buff = BUF_2_READ;
	if (number < 1 || number > 2) DEBUG_BREAK

	tx_data[0] = buff;
	tx_data[1] = 0;
	tx_data[2] = 0;
	tx_data[3] = 0;

	at45dbxx->Transfer(&tx_data, &rx_data, SPI_TRANSFER_SIZE);

	// Fill the result from the right index
	memcpy(result,rx_data+4,PAGE_SIZE);
}

void AT45DBXX_BufferWrite(at45dbxx_t * at45dbxx, uint8_t number, uint8_t data_buffer[], uint32_t num_of_bytes)
{
	if (num_of_bytes > PAGE_SIZE) DEBUG_BREAK

	uint8_t dummy_rx[SPI_TRANSFER_SIZE];
	uint8_t tx_data[SPI_TRANSFER_SIZE];  // Need room for cmd + three address bytes

	memset(tx_data,0x00,SPI_TRANSFER_SIZE);
	memset(dummy_rx,0x00,SPI_TRANSFER_SIZE);

	uint8_t buff = 0;
	if (number == 1)buff = BUF_1_WRITE;
	if (number == 2)buff = BUF_2_WRITE;
	if (number < 1 || number > 2) DEBUG_BREAK

	tx_data[0] = buff;
	tx_data[1] = 0;
	tx_data[2] = 0;
	tx_data[3] = 0;

	for (int i=0; i < PAGE_SIZE; i++)
	{
		if (i >= num_of_bytes) break;
		tx_data[i+4] = data_buffer[i];
	}

	at45dbxx->Transfer(&tx_data, &dummy_rx, SPI_TRANSFER_SIZE);
}

void AT45DBXX_BufferToPageER(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address)
{
	uint8_t status;

	uint8_t tx_data[4];
	memset(tx_data,0x00,4);
	uint8_t rx_data[4];
	memset(rx_data,0x00,4);

	uint8_t buff = 0;
	if (number == 1)buff = BUF_1_PAGE_ER;
	if (number == 2)buff = BUF_2_PAGE_ER;
	if (number < 1 || number > 2) DEBUG_BREAK

	uint32_t page = address << 1;

	tx_data[0] = buff;
	tx_data[1] = (page >> 8)	& 0xff;
	tx_data[2] = (page)			& 0xff;
	tx_data[3] = 0; //page			& 0xff;

	at45dbxx->Transfer(&tx_data, &rx_data, 4);

	//TODO Make non-block
	do status = AT45DBXX_ReadStatus(at45dbxx);
	while (!(status & STATUS_RDY));
}

void AT45DBXX_BufferToPageNER(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address)
{
	uint8_t status;

	uint8_t tx_data[4];
	memset(tx_data,0x00,4);
	uint8_t rx_data[4];
	memset(rx_data,0x00,4);

	uint8_t buff = 0;
	if (number == 1)buff = BUF_1_PAGE_NER;
	if (number == 2)buff = BUF_2_PAGE_NER;
	if (number < 1 || number > 2) DEBUG_BREAK

	uint32_t page = address << 1;

	tx_data[0] = buff;
	tx_data[1] = (page >> 8)	& 0xff;
	tx_data[2] = (page)			& 0xff;
	tx_data[3] = 0; //page			& 0xff;

	at45dbxx->Transfer(&tx_data, &rx_data, 4);

	//TODO Make non-block
	do status = AT45DBXX_ReadStatus(at45dbxx);
	while (!(status & STATUS_RDY));
}

void AT45DBXX_PageToBufferTransfer(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address)
{
	uint8_t tx_data[4];
	memset(tx_data,0x00,4);
	uint8_t rx_data[4];
	memset(rx_data,0x00,4);

	uint8_t buff = 0;
	if (number == 1)buff = PAGE_TO_BUF_1;
	if (number == 2)buff = PAGE_TO_BUF_2;
	if (number < 1 || number > 2) DEBUG_BREAK

	uint32_t page = address << 1;

	tx_data[0] = buff;
	tx_data[1] = (page >> 8)	& 0xff;
	tx_data[2] = (page)			& 0xff;
	tx_data[3] = 0; //page			& 0xff;

	at45dbxx->Transfer(&tx_data, &rx_data, 4);
}

void AT45DBXX_PageToBufferCompare(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address)
{
	uint8_t status;

	uint8_t tx_data[4];
	memset(tx_data,0x00,4);
	uint8_t rx_data[4];
	memset(rx_data,0x00,4);

	uint8_t buff = 0;
	if (number == 1)buff = BUF_1_COMP;
	if (number == 2)buff = BUF_2_COMP;
	if (number < 1 || number > 2) DEBUG_BREAK

	uint32_t page = address << 1;

	tx_data[0] = buff;
	tx_data[1] = (page >> 8)	& 0xff;
	tx_data[2] = (page)			& 0xff;
	tx_data[3] = 0; //page			& 0xff;

	at45dbxx->Transfer(&tx_data, &rx_data, 4);

	//TODO Make non-block
	do status = AT45DBXX_ReadStatus(at45dbxx);
	while (!(status & STATUS_RDY));
}

void AT45DBXX_EnableSectorProtection(at45dbxx_t * at45dbxx)
{
	uint8_t tx_data[4];
	memset(tx_data,0x00,4);
	uint8_t rx_data[4];
	memset(rx_data,0x00,4);

	tx_data[0] = 0x3D;
	tx_data[1] = 0x2A;
	tx_data[2] = 0x7F;
	tx_data[3] = 0xA9;

	at45dbxx->Transfer(&tx_data, &rx_data, 4);
}

void AT45DBXX_DisableSectorProtection(at45dbxx_t * at45dbxx)
{
	uint8_t tx_data[4];
	memset(tx_data,0x00,4);
	uint8_t rx_data[4];
	memset(rx_data,0x00,4);

	tx_data[0] = 0x3D;
	tx_data[1] = 0x2A;
	tx_data[2] = 0x7F;
	tx_data[3] = 0x9A;

	at45dbxx->Transfer(&tx_data, &rx_data, 4);
}

void AT45DBXX_EraseSectorProtection(at45dbxx_t * at45dbxx)
{
	uint8_t status;

	uint8_t tx_data[4];
	memset(tx_data,0x00,4);
	uint8_t rx_data[4];
	memset(rx_data,0x00,4);

	tx_data[0] = 0x3D;
	tx_data[1] = 0x2A;
	tx_data[2] = 0x7F;
	tx_data[3] = 0xCF;

	at45dbxx->Transfer(&tx_data, &rx_data, 4);

	//TODO Make non-block
	do status = AT45DBXX_ReadStatus(at45dbxx);
	while (!(status & STATUS_RDY));
}

void AT45DBXX_ProgramSectorProtection(at45dbxx_t * at45dbxx, uint8_t sector)
{
	uint8_t status;

	uint8_t tx_data[12];
	memset(tx_data,0x00,12);
	uint8_t rx_data[12];
	memset(rx_data,0x00,12);

	tx_data[0] = 0x3D;
	tx_data[1] = 0x2A;
	tx_data[2] = 0x7F;
	tx_data[3] = 0xFC;

	if (sector == 0)
	tx_data[4] = 0xF0;
	if (sector == 1)
	tx_data[5] = 0xFF;
	if (sector == 2)
	tx_data[6] = 0xFF;
	if (sector == 3)
	tx_data[7] = 0xFF;
	if (sector == 4)
	tx_data[8] = 0xFF;
	if (sector == 5)
	tx_data[9] = 0xFF;
	if (sector == 6)
	tx_data[10] = 0xFF;
	if (sector == 7)
	tx_data[11] = 0xFF;
	if (sector < 0 || sector > 7) DEBUG_BREAK

	at45dbxx->Transfer(&tx_data, &rx_data, 12);

	//TODO Make non-block
	do status = AT45DBXX_ReadStatus(at45dbxx);
	while (!(status & STATUS_RDY));
}

void AT45DBXX_ReadSectorProtection(at45dbxx_t * at45dbxx)
{
	uint8_t tx_data[12];
	memset(tx_data,0x00,12);
	uint8_t rx_data[12];
	memset(rx_data,0x00,12);

	tx_data[0] = 0x32;
	tx_data[1] = 0x00;
	tx_data[2] = 0x00;
	tx_data[3] = 0x00;

	at45dbxx->Transfer(&tx_data, &rx_data, 12);
}
