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

//Commands
#define AT45DBXX_JEDEC_ID_CMD	(0x9F)
#define AT45DBXX_PAGE_PROGRAM_CMD	(0x82)
#define AT45DBXX_PAGE_ERASE_CMD		(0x81)
#define AT45DBXX_READ_DATA_CMD		(0xD2)
#define AT45DBXX_STATUS_CMD			(0xD7)
#define AT45DBXX_BUF_1_READ_CMD		(0xD4)
#define AT45DBXX_BUF_2_READ_CMD		(0xD6)
#define AT45DBXX_BUF_1_WRITE_CMD		(0x84)
#define AT45DBXX_BUF_2_WRITE_CMD		(0x87)
#define AT45DBXX_BUF_1_PAGE_ER_CMD	(0x83)
#define AT45DBXX_BUF_2_PAGE_ER_CMD	(0x86)
#define AT45DBXX_BUF_1_PAGE_NER_CMD	(0x88)
#define AT45DBXX_BUF_2_PAGE_NER_CMD	(0x89)
#define AT45DBXX_PAGE_TO_BUF_1_CMD	(0x53)
#define AT45DBXX_PAGE_TO_BUF_2_CMD	(0x55)
#define AT45DBXX_BUF_1_COMP_CMD		(0x60)
#define AT45DBXX_BUF_2_COMP_CMD		(0x61)


#define AT45DBXX_STATUS_RDY		(1 << 7) /* Bit 7: RDY/ Not BUSY */
#define AT45DBXX_STATUS_COMP     (1 << 6) /* Bit 6: COMP */
#define AT45DBXX_STATUS_PROTECT  (1 << 1) /* Bit 1: PROTECT */
#define AT45DBXX_STATUS_PGSIZE   (1 << 0) /* Bit 0: PAGE_SIZE */

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

void AT45DBXX_BufferRead(at45dbxx_t * at45dbxx, uint8_t number, uint8_t result[]);
void AT45DBXX_BufferWrite(at45dbxx_t * at45dbxx, uint8_t number, uint8_t data_buffer[], uint32_t num_of_bytes);
void AT45DBXX_BufferToPageER(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address);
void AT45DBXX_BufferToPageNER(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address);
void AT45DBXX_PageToBufferTransfer(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address);
void AT45DBXX_PageToBufferCompare(at45dbxx_t * at45dbxx, uint8_t number, uint32_t address);

void AT45DBXX_PageErase(at45dbxx_t * at45dbxx, uint32_t address);
void AT45DBXX_ReadMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t result[]);
void AT45DBXX_WriteMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t data_buffer[], uint32_t num_of_bytes);
void AT45DBXX_ConfigWrite(at45dbxx_t * at45dbxx, uint8_t command);
void AT45DBXX_CheckID(at45dbxx_t * at45dbxx);
uint8_t AT45DBXX_ReadStatus(at45dbxx_t * at45dbxx);

void AT45DBXX_EnableSectorProtection(at45dbxx_t * at45dbxx);
void AT45DBXX_DisableSectorProtection(at45dbxx_t * at45dbxx);
void AT45DBXX_EraseSectorProtection(at45dbxx_t * at45dbxx);
void AT45DBXX_ReadSectorProtection(at45dbxx_t * at45dbxx);
uint8_t AT45DBXX_ProgramSectorProtection(at45dbxx_t * at45dbxx, uint8_t sector);

#endif /* INC_DRIVER_AT45DBXX_H_ */
