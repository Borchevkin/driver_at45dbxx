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
#define JEDEC_ID_CMD	0x9F
#define PAGE_PROGRAM	0x82
#define PAGE_ERASE		0x81
#define READ_DATA		0xD2
#define STATUS			0xD7
#define BUF_1_READ		0xD4
#define BUF_2_READ		0xD6
#define BUF_1_WRITE		0x84
#define BUF_2_WRITE		0x87
#define BUF_1_PAGE_ER	0x83
#define BUF_2_PAGE_ER	0x86
#define BUF_1_PAGE_NER	0x88
#define BUF_2_PAGE_NER	0x89
#define PAGE_TO_BUF_1	0x53
#define PAGE_TO_BUF_2	0x55
#define BUF_1_COMP		0x60
#define BUF_2_COMP		0x61


#define STATUS_RDY		(1 << 7) /* Bit 7: RDY/ Not BUSY */
#define STATUS_COMP     (1 << 6) /* Bit 6: COMP */
#define STATUS_PROTECT  (1 << 1) /* Bit 1: PROTECT */
#define STATUS_PGSIZE   (1 << 0) /* Bit 0: PAGE_SIZE */

#define PAGE_SIZE			264
#define SPI_TRANSFER_SIZE 	PAGE_SIZE + 4  // Account for SPI header

/* ==========TYPES========== */

typedef struct {
//TODO
}at45dbxx_t;

/* ==========PROTOTYPES========== */

void AT45DBXX_ChipErase(at45dbxx_t * at45dbxx);
void AT45DBXX_BufferRead(at45dbxx_t * at45dbxx, uint8_t number, uint8_t result[]);
void AT45DBXX_BufferWrite(at45dbxx_t * at45dbxx, uint8_t number, uint8_t data_buffer[], uint32_t num_of_bytes);
void AT45DBXX_PageErase(at45dbxx_t * at45dbxx, uint32_t address);
void AT45DBXX_ReadMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t result[]);
void AT45DBXX_WriteMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t data_buffer[], uint32_t num_of_bytes);
void AT45DBXX_ConfigWrite(at45dbxx_t * at45dbxx, uint8_t command);
void AT45DBXX_CheckID(at45dbxx_t * at45dbxx);
uint8_t AT45DBXX_ReadStatus(at45dbxx_t * at45dbxx);

//Need to replace it somewhere
void spidrv_setup();

#endif /* INC_DRIVER_AT45DBXX_H_ */
