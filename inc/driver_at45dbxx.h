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
#define READ_DATA		0x03

#define PAGE_SIZE			256
#define SPI_TRANSFER_SIZE 	PAGE_SIZE + 4  // Account for SPI header

/* ==========TYPES========== */

typedef struct {
//TODO
}at45dbxx_t;

/* ==========PROTOTYPES========== */

void AT45DBXX_ChipErase(at45dbxx_t * at45dbxx);
void AT45DBXX_ReadMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t result[], uint32_t num_of_bytes);
void AT45DBXX_WriteMemory(at45dbxx_t * at45dbxx, uint32_t address, uint8_t data_buffer[], uint32_t num_of_bytes);
void AT45DBXX_ConfigWrite(at45dbxx_t * at45dbxx, uint8_t command);
void AT45DBXX_CheckID(at45dbxx_t * at45dbxx);

//Need to replace it somewhere
void spidrv_setup();

#endif /* INC_DRIVER_AT45DBXX_H_ */
