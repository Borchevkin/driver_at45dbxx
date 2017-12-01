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
	at45dbxx->SetupSPI = SPIDRV_Setup;
	at45dbxx->Transfer = SPI1_Transfer;
	at45dbxx->SetWP = FLASH_SetWP;
	at45dbxx->ClearWP = FLASH_ClearWP;
	at45dbxx->SetRESET = FLASH_SetRESET;
	at45dbxx->ClearRESET = FLASH_ClearRESET;
}

void AT45DBXX_ChipErase(at45dbxx_t * at45dbxx)
{
	uint8_t status;
	uint8_t txData[AT45DBXX_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_CMD_SIZE);
	uint8_t rxData[AT45DBXX_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_CMD_SIZE);

	txData[0] = AT45DBXX_CHIP_ERASE_1_CMD;
	txData[1] = AT45DBXX_CHIP_ERASE_2_CMD;
	txData[2] = AT45DBXX_CHIP_ERASE_3_CMD;
	txData[3] = AT45DBXX_CHIP_ERASE_4_CMD;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_CMD_SIZE);

	//TODO Make non-block
	do
	{
		status = AT45DBXX_ReadStatus(at45dbxx);
	} while (!(status & AT45DBXX_STATUS_RDY));
}

void AT45DBXX_PageErase(at45dbxx_t * at45dbxx, uint32_t address)
{
	uint8_t status;

	uint8_t txData[AT45DBXX_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_CMD_SIZE);
	uint8_t rxData[AT45DBXX_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_CMD_SIZE);

	uint32_t page = address << 1;

	txData[0] = AT45DBXX_PAGE_ERASE_CMD;
	txData[1] = (page >> 8)	& 0xff;
	txData[2] = (page)		& 0xff;
	txData[3] = 0;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_CMD_SIZE);

	//TODO Make non-block
	do
	{
		status = AT45DBXX_ReadStatus(at45dbxx);
	} while (!(status & AT45DBXX_STATUS_RDY));
}

void AT45DBXX_ReadMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t result[])
{
	uint8_t txData[AT45DBXX_SPI_TRANSFER_SIZE];
	memset(txData,0x00,AT45DBXX_SPI_TRANSFER_SIZE);
	uint8_t rxData[AT45DBXX_SPI_TRANSFER_SIZE];
	memset(rxData,0x00,AT45DBXX_SPI_TRANSFER_SIZE);

	uint32_t page = address << 1;

	txData[0] = AT45DBXX_READ_DATA_CMD;
	txData[1] = (page >> 8)	& 0xff;
	txData[2] = (page)		& 0xff;
	txData[3] = 0;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_SPI_TRANSFER_SIZE);

	// Fill the result from the right index
	memcpy(result,rxData+4,AT45DBXX_PAGE_SIZE);
}

uint16_t AT45DBXX_WriteMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t dataBuffer[], uint32_t numOfBytes)
{
	if (numOfBytes > AT45DBXX_PAGE_SIZE)
	{
		return AT45DBXX_NG;
	}

	uint8_t status;
	uint8_t dummyRX[AT45DBXX_SPI_TRANSFER_SIZE];
	uint8_t txData[AT45DBXX_SPI_TRANSFER_SIZE];  // Need room for cmd + three address bytes

	memset(txData,0x00,AT45DBXX_SPI_TRANSFER_SIZE);
	memset(dummyRX,0x00,AT45DBXX_SPI_TRANSFER_SIZE);

	uint32_t page = address << 1;

	txData[0] = AT45DBXX_PAGE_PROGRAM_CMD;
	txData[1] = (page >> 8)	& 0xff;
	txData[2] = (page)		& 0xff;
	txData[3] = 0;

	for (int i=0; i < AT45DBXX_PAGE_SIZE; i++)
	{
		if (i >= numOfBytes)
		{
			break;
		}
		txData[i+4] = dataBuffer[i];
	}

	at45dbxx->Transfer(&txData[0], &dummyRX[0], AT45DBXX_SPI_TRANSFER_SIZE);

	//TODO Make non-block
	do
	{
		status = AT45DBXX_ReadStatus(at45dbxx);
	} while (!(status & AT45DBXX_STATUS_RDY));

	return AT45DBXX_OK;
}

void AT45DBXX_ConfigWrite(at45dbxx_t * at45dbxx, uint8_t command)
{
	uint8_t result[1];
	uint8_t txData[1];
	memset(txData,0x00,1);
	memset(result,0x00,1);

	txData[0] = command;

	at45dbxx->Transfer(&txData[0], &result[0], 1);
}

uint16_t AT45DBXX_CheckID(at45dbxx_t * at45dbxx)
{
	uint8_t result[AT45DBXX_CMD_SIZE];
	uint8_t txData[AT45DBXX_CMD_SIZE];

	memset(txData,0x00,AT45DBXX_CMD_SIZE);
	memset(result,0x00,AT45DBXX_CMD_SIZE);

	txData[0] = AT45DBXX_JEDEC_ID_CMD;

	at45dbxx->Transfer(&txData[0], &result[0], AT45DBXX_CMD_SIZE);

	// Check the result for what is expected from the Spansion spec
	if (result[1] != AT45DBXX_ID_1 || result[2] != AT45DBXX_ID_2 || result[3] != AT45DBXX_ID_3)
	{
		return AT45DBXX_NG;
	}

	return AT45DBXX_OK;
}

uint8_t AT45DBXX_ReadStatus(at45dbxx_t * at45dbxx)
{
	uint8_t result[2];
	uint8_t txData[2];

	memset(txData,0x00,2);
	memset(result,0x00,2);

	txData[0] = AT45DBXX_STATUS_CMD;

	at45dbxx->Transfer(&txData[0], &result[0], 2);

	return result[1];
}

uint16_t AT45DBXX_BufferRead(at45dbxx_t * at45dbxx, uint8_t number, uint8_t result[])
{
	uint8_t txData[AT45DBXX_SPI_TRANSFER_SIZE];
	memset(txData,0x00,AT45DBXX_SPI_TRANSFER_SIZE);
	uint8_t rxData[AT45DBXX_SPI_TRANSFER_SIZE];
	memset(rxData,0x00,AT45DBXX_SPI_TRANSFER_SIZE);

	uint8_t buff = 0;

	if (number == AT45DBXX_BUF_1)
	{
		buff = AT45DBXX_BUF_1_READ_CMD;
	}

	if (number == AT45DBXX_BUF_2)
	{
		buff = AT45DBXX_BUF_2_READ_CMD;
	}

	if (number < AT45DBXX_BUF_1 || number > AT45DBXX_BUF_2)
	{
		return AT45DBXX_NG;
	}

	txData[0] = buff;
	txData[1] = 0;
	txData[2] = 0;
	txData[3] = 0;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_SPI_TRANSFER_SIZE);

	// Fill the result from the right index
	memcpy(result,rxData+4,AT45DBXX_PAGE_SIZE);

	return AT45DBXX_OK;
}

uint16_t AT45DBXX_BufferWrite(at45dbxx_t * at45dbxx, uint8_t number, uint8_t dataBuffer[], uint32_t numOfBytes)
{
	if (numOfBytes > AT45DBXX_PAGE_SIZE)
	{
		return AT45DBXX_NG;
	}

	uint8_t dummyRX[AT45DBXX_SPI_TRANSFER_SIZE];
	uint8_t txData[AT45DBXX_SPI_TRANSFER_SIZE];  // Need room for cmd + three address bytes

	memset(txData,0x00,AT45DBXX_SPI_TRANSFER_SIZE);
	memset(dummyRX,0x00,AT45DBXX_SPI_TRANSFER_SIZE);

	uint8_t buff = 0;

	if (number == AT45DBXX_BUF_1)
	{
		buff = AT45DBXX_BUF_1_WRITE_CMD;
	}

	if (number == AT45DBXX_BUF_2)
	{
		buff = AT45DBXX_BUF_2_WRITE_CMD;
	}

	if (number < AT45DBXX_BUF_1 || number > AT45DBXX_BUF_2)
	{
		return AT45DBXX_NG;
	}

	txData[0] = buff;
	txData[1] = 0;
	txData[2] = 0;
	txData[3] = 0;

	for (int i=0; i < AT45DBXX_PAGE_SIZE; i++)
	{
		if (i >= numOfBytes)
		{
			break;
		}
		txData[i+4] = dataBuffer[i];
	}

	at45dbxx->Transfer(&txData[0], &dummyRX[0], AT45DBXX_SPI_TRANSFER_SIZE);

	return AT45DBXX_OK;
}

uint16_t AT45DBXX_BufferToPageER(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address)
{
	uint8_t status;

	uint8_t txData[AT45DBXX_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_CMD_SIZE);
	uint8_t rxData[AT45DBXX_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_CMD_SIZE);

	uint8_t buff = 0;

	if (number == AT45DBXX_BUF_1)
	{
		buff = AT45DBXX_BUF_1_PAGE_ER_CMD;
	}

	if (number == AT45DBXX_BUF_2)
	{
		buff = AT45DBXX_BUF_2_PAGE_ER_CMD;
	}

	if (number < AT45DBXX_BUF_1 || number > AT45DBXX_BUF_2)
	{
		return AT45DBXX_NG;
	}

	uint32_t page = address << 1;

	txData[0] = buff;
	txData[1] = (page >> 8)	& 0xff;
	txData[2] = (page)		& 0xff;
	txData[3] = 0;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_CMD_SIZE);

	//TODO Make non-block
	do
	{
		status = AT45DBXX_ReadStatus(at45dbxx);
	} while (!(status & AT45DBXX_STATUS_RDY));

	return AT45DBXX_OK;
}

uint16_t AT45DBXX_BufferToPageNER(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address)
{
	uint8_t status;

	uint8_t txData[AT45DBXX_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_CMD_SIZE);
	uint8_t rxData[AT45DBXX_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_CMD_SIZE);

	uint8_t buff = 0;

	if (number == AT45DBXX_BUF_1)
	{
		buff = AT45DBXX_BUF_1_PAGE_NER_CMD;
	}

	if (number == AT45DBXX_BUF_2)
	{
		buff = AT45DBXX_BUF_2_PAGE_NER_CMD;
	}

	if (number < AT45DBXX_BUF_1 || number > AT45DBXX_BUF_2)
	{
		return AT45DBXX_NG;
	}

	uint32_t page = address << 1;

	txData[0] = buff;
	txData[1] = (page >> 8)	& 0xff;
	txData[2] = (page)		& 0xff;
	txData[3] = 0;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_CMD_SIZE);

	//TODO Make non-block
	do
	{
		status = AT45DBXX_ReadStatus(at45dbxx);
	} while (!(status & AT45DBXX_STATUS_RDY));

	return AT45DBXX_OK;
}

uint16_t AT45DBXX_PageToBufferTransfer(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address)
{
	uint8_t txData[AT45DBXX_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_CMD_SIZE);
	uint8_t rxData[AT45DBXX_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_CMD_SIZE);

	uint8_t buff = 0;

	if (number == AT45DBXX_BUF_1)
	{
		buff = AT45DBXX_PAGE_TO_BUF_1_CMD;
	}

	if (number == AT45DBXX_BUF_2)
	{
		buff = AT45DBXX_PAGE_TO_BUF_2_CMD;
	}

	if (number < AT45DBXX_BUF_1 || number > AT45DBXX_BUF_2)
	{
		return AT45DBXX_NG;
	}

	uint32_t page = address << 1;

	txData[0] = buff;
	txData[1] = (page >> 8)	& 0xff;
	txData[2] = (page)		& 0xff;
	txData[3] = 0;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_CMD_SIZE);

	return AT45DBXX_OK;
}

uint16_t AT45DBXX_PageToBufferCompare(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address)
{
	uint8_t status;

	uint8_t txData[AT45DBXX_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_CMD_SIZE);
	uint8_t rxData[AT45DBXX_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_CMD_SIZE);

	uint8_t buff = 0;

	if (number == AT45DBXX_BUF_1)
	{
		buff = AT45DBXX_BUF_1_COMP_CMD;
	}

	if (number == AT45DBXX_BUF_2)
	{
		buff = AT45DBXX_BUF_2_COMP_CMD;
	}

	if (number < AT45DBXX_BUF_1 || number > AT45DBXX_BUF_2)
	{
		return AT45DBXX_NG;
	}

	uint32_t page = address << 1;

	txData[0] = buff;
	txData[1] = (page >> 8)	& 0xff;
	txData[2] = (page)		& 0xff;
	txData[3] = 0;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_CMD_SIZE);

	//TODO Make non-block
	do
	{
		status = AT45DBXX_ReadStatus(at45dbxx);
	} while (!(status & AT45DBXX_STATUS_RDY));

	return AT45DBXX_OK;
}

void AT45DBXX_EnableSectorProtection(at45dbxx_t * at45dbxx)
{
	uint8_t txData[AT45DBXX_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_CMD_SIZE);
	uint8_t rxData[AT45DBXX_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_CMD_SIZE);

	txData[0] = AT45DBXX_EN_SEC_PROTECT_1_CMD;
	txData[1] = AT45DBXX_EN_SEC_PROTECT_2_CMD;
	txData[2] = AT45DBXX_EN_SEC_PROTECT_3_CMD;
	txData[3] = AT45DBXX_EN_SEC_PROTECT_4_CMD;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_CMD_SIZE);
}

void AT45DBXX_DisableSectorProtection(at45dbxx_t * at45dbxx)
{
	uint8_t txData[AT45DBXX_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_CMD_SIZE);
	uint8_t rxData[AT45DBXX_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_CMD_SIZE);

	txData[0] = AT45DBXX_DIS_SEC_PROTECT_1_CMD;
	txData[1] = AT45DBXX_DIS_SEC_PROTECT_2_CMD;
	txData[2] = AT45DBXX_DIS_SEC_PROTECT_3_CMD;
	txData[3] = AT45DBXX_DIS_SEC_PROTECT_4_CMD;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_CMD_SIZE);
}

void AT45DBXX_EraseSectorProtection(at45dbxx_t * at45dbxx)
{
	uint8_t status;

	uint8_t txData[AT45DBXX_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_CMD_SIZE);
	uint8_t rxData[AT45DBXX_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_CMD_SIZE);

	txData[0] = AT45DBXX_ER_SEC_PROTECT_1_CMD;
	txData[1] = AT45DBXX_ER_SEC_PROTECT_2_CMD;
	txData[2] = AT45DBXX_ER_SEC_PROTECT_3_CMD;
	txData[3] = AT45DBXX_ER_SEC_PROTECT_4_CMD;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_CMD_SIZE);

	//TODO Make non-block
	do
	{
		status = AT45DBXX_ReadStatus(at45dbxx);
	} while (!(status & AT45DBXX_STATUS_RDY));
}

uint16_t AT45DBXX_ProgramSectorProtection(at45dbxx_t * at45dbxx, uint8_t sector)
{
	uint8_t status;

	uint8_t txData[AT45DBXX_SEC_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_SEC_CMD_SIZE);
	uint8_t rxData[AT45DBXX_SEC_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_SEC_CMD_SIZE);

	txData[0] = AT45DBXX_PROG_SEC_PROTECT_1_CMD;
	txData[1] = AT45DBXX_PROG_SEC_PROTECT_2_CMD;
	txData[2] = AT45DBXX_PROG_SEC_PROTECT_3_CMD;
	txData[3] = AT45DBXX_PROG_SEC_PROTECT_4_CMD;

	//TODO Rework
	if (sector == AT45DBXX_SEC_0)
	txData[4] = 0xF0;
	if (sector == AT45DBXX_SEC_1)
	txData[5] = AT45DBXX_SEC_PROTECTION;
	if (sector == AT45DBXX_SEC_2)
	txData[6] = AT45DBXX_SEC_PROTECTION;
	if (sector == AT45DBXX_SEC_3)
	txData[7] = AT45DBXX_SEC_PROTECTION;
	if (sector == AT45DBXX_SEC_4)
	txData[8] = AT45DBXX_SEC_PROTECTION;
	if (sector == AT45DBXX_SEC_5)
	txData[9] = AT45DBXX_SEC_PROTECTION;
	if (sector == AT45DBXX_SEC_6)
	txData[10] = AT45DBXX_SEC_PROTECTION;
	if (sector == AT45DBXX_SEC_7)
	txData[11] = AT45DBXX_SEC_PROTECTION;
	if (sector < AT45DBXX_SEC_0 || sector > AT45DBXX_SEC_7)
	{
		return sector;
	}

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_SEC_CMD_SIZE);

	//TODO Make non-block
	do
	{
		status = AT45DBXX_ReadStatus(at45dbxx);
	} while (!(status & AT45DBXX_STATUS_RDY));

	return AT45DBXX_OK;
}

void AT45DBXX_ReadSectorProtection(at45dbxx_t * at45dbxx)
{
	uint8_t txData[AT45DBXX_SEC_CMD_SIZE];
	memset(txData,0x00,AT45DBXX_SEC_CMD_SIZE);
	uint8_t rxData[AT45DBXX_SEC_CMD_SIZE];
	memset(rxData,0x00,AT45DBXX_SEC_CMD_SIZE);

	txData[0] = AT45DBXX_READ_SEC_PROTECT_CMD;
	txData[1] = 0x00;
	txData[2] = 0x00;
	txData[3] = 0x00;

	at45dbxx->Transfer(&txData[0], &rxData[0], AT45DBXX_SEC_CMD_SIZE);
}
