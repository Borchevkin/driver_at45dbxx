/*
 * driver_at45dbxx.h
 *
 *  Created on: 20 сент. 2017 г.
 *      Author: user
 */

#ifndef INC_DRIVER_AT45DBXX_H_
#define INC_DRIVER_AT45DBXX_H_

#include <stdint.h>

/* ==========DEFINE========== */

#define AT45DBXX_OK (0)
#define AT45DBXX_NG (-1)

//Commands
#define AT45DBXX_JEDEC_ID_CMD			(0x9F)
#define AT45DBXX_PAGE_PROGRAM_CMD		(0x82)
#define AT45DBXX_PAGE_ERASE_CMD			(0x81)
#define AT45DBXX_CHIP_ERASE_1_CMD		(0xC7)
#define AT45DBXX_CHIP_ERASE_2_CMD		(0x94)
#define AT45DBXX_CHIP_ERASE_3_CMD		(0x80)
#define AT45DBXX_CHIP_ERASE_4_CMD		(0x9A)
#define AT45DBXX_READ_DATA_CMD			(0xD2)
#define AT45DBXX_STATUS_CMD				(0xD7)
#define AT45DBXX_BUF_1_READ_CMD			(0xD4)
#define AT45DBXX_BUF_2_READ_CMD			(0xD6)
#define AT45DBXX_BUF_1_WRITE_CMD		(0x84)
#define AT45DBXX_BUF_2_WRITE_CMD		(0x87)
#define AT45DBXX_BUF_1_PAGE_ER_CMD		(0x83)
#define AT45DBXX_BUF_2_PAGE_ER_CMD		(0x86)
#define AT45DBXX_BUF_1_PAGE_NER_CMD		(0x88)
#define AT45DBXX_BUF_2_PAGE_NER_CMD		(0x89)
#define AT45DBXX_PAGE_TO_BUF_1_CMD		(0x53)
#define AT45DBXX_PAGE_TO_BUF_2_CMD		(0x55)
#define AT45DBXX_BUF_1_COMP_CMD			(0x60)
#define AT45DBXX_BUF_2_COMP_CMD			(0x61)
#define AT45DBXX_EN_SEC_PROTECT_1_CMD	(0x3D)
#define AT45DBXX_EN_SEC_PROTECT_2_CMD	(0x2A)
#define AT45DBXX_EN_SEC_PROTECT_3_CMD	(0x7F)
#define AT45DBXX_EN_SEC_PROTECT_4_CMD	(0xA9)
#define AT45DBXX_DIS_SEC_PROTECT_1_CMD	(0x3D)
#define AT45DBXX_DIS_SEC_PROTECT_2_CMD	(0x2A)
#define AT45DBXX_DIS_SEC_PROTECT_3_CMD	(0x7F)
#define AT45DBXX_DIS_SEC_PROTECT_4_CMD	(0x9A)
#define AT45DBXX_ER_SEC_PROTECT_1_CMD	(0x3D)
#define AT45DBXX_ER_SEC_PROTECT_2_CMD	(0x2A)
#define AT45DBXX_ER_SEC_PROTECT_3_CMD	(0x7F)
#define AT45DBXX_ER_SEC_PROTECT_4_CMD	(0xCF)
#define AT45DBXX_PROG_SEC_PROTECT_1_CMD	(0x3D)
#define AT45DBXX_PROG_SEC_PROTECT_2_CMD	(0x2A)
#define AT45DBXX_PROG_SEC_PROTECT_3_CMD	(0x7F)
#define AT45DBXX_PROG_SEC_PROTECT_4_CMD	(0xFC)
#define AT45DBXX_READ_SEC_PROTECT_CMD	(0x32)

#define AT45DBXX_SEC_0				(0)
#define AT45DBXX_SEC_1				(1)
#define AT45DBXX_SEC_2				(2)
#define AT45DBXX_SEC_3				(3)
#define AT45DBXX_SEC_4				(4)
#define AT45DBXX_SEC_5				(5)
#define AT45DBXX_SEC_6				(6)
#define AT45DBXX_SEC_7				(7)
#define AT45DBXX_BUF_1				(1)
#define AT45DBXX_BUF_2				(2)
#define AT45DBXX_ID_1 				(0x1F)
#define AT45DBXX_ID_2 				(0x24)
#define AT45DBXX_ID_3 				(0x00)

#define AT45DBXX_STATUS_RDY		 (1 << 7) /* Bit 7: RDY/ Not BUSY */
#define AT45DBXX_STATUS_COMP     (1 << 6) /* Bit 6: COMP */
#define AT45DBXX_STATUS_PROTECT  (1 << 1) /* Bit 1: PROTECT */
#define AT45DBXX_STATUS_PGSIZE   (1 << 0) /* Bit 0: PAGE_SIZE */

#define AT45DBXX_SEC_PROTECTION		(0xFF)
#define AT45DBXX_SEC_CMD_SIZE 		(12)
#define AT45DBXX_CMD_SIZE			(4)
#define AT45DBXX_PAGE_SIZE			(264)
#define AT45DBXX_SPI_TRANSFER_SIZE 	(AT45DBXX_PAGE_SIZE + 4)  // Account for SPI header

/* ==========TYPES========== */

typedef struct
{
	void (*SetWP)(void);
	void (*ClearWP)(void);
	void (*SetRESET)(void);
	void (*ClearRESET)(void);
	void (*SetupSPI)(void);
	void (*Transfer)(uint8_t *tx, uint8_t *rx, uint16_t num);
} at45dbxx_t;

/* ==========PROTOTYPES========== */

void AT45DBXX_Init(at45dbxx_t * at45dbxx);

void AT45DBXX_ChipErase(at45dbxx_t * at45dbxx);

uint16_t AT45DBXX_BufferRead(at45dbxx_t * at45dbxx, uint8_t number, uint8_t result[]);
uint16_t AT45DBXX_BufferWrite(at45dbxx_t * at45dbxx, uint8_t number, uint8_t dataBuffer[], uint32_t numOfBytes);
uint16_t AT45DBXX_BufferToPageER(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address);
uint16_t AT45DBXX_BufferToPageNER(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address);
uint16_t AT45DBXX_PageToBufferTransfer(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address);
uint16_t AT45DBXX_PageToBufferCompare(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address);

void AT45DBXX_PageErase(at45dbxx_t * at45dbxx, uint32_t address);
void AT45DBXX_ReadMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t result[]);
uint16_t AT45DBXX_WriteMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t dataBuffer[], uint32_t numOfBytes);
void AT45DBXX_ConfigWrite(at45dbxx_t * at45dbxx, uint8_t command);
uint16_t AT45DBXX_CheckID(at45dbxx_t * at45dbxx);
uint8_t AT45DBXX_ReadStatus(at45dbxx_t * at45dbxx);

void AT45DBXX_EnableSectorProtection(at45dbxx_t * at45dbxx);
void AT45DBXX_DisableSectorProtection(at45dbxx_t * at45dbxx);
void AT45DBXX_EraseSectorProtection(at45dbxx_t * at45dbxx);
void AT45DBXX_ReadSectorProtection(at45dbxx_t * at45dbxx);
uint16_t AT45DBXX_ProgramSectorProtection(at45dbxx_t * at45dbxx, uint8_t sector);

#endif /* INC_DRIVER_AT45DBXX_H_ */
