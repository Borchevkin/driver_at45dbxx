/*
 * utilities.h
 *
 *  Created on: 20 сент. 2017 г.
 *      Author: user
 */

#ifndef INC_UTILITIES_H_
#define INC_UTILITIES_H_

#define DEBUG_BREAK		__asm__("BKPT #0");

void Delay(uint32_t dlyTicks);
void SysTick_Handler(void);

void SetupUtilities(void);
void SPIDRV_Setup(void);

void SPI1_Transfer(uint8_t *tx, uint8_t *rx, uint16_t num);

void FLASH_SetWP(void);
void FLASH_ClearWP(void);

void FLASH_SetRESET(void);
void FLASH_ClearRESET(void);

#endif /* INC_UTILITIES_H_ */
