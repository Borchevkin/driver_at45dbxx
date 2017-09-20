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

#endif /* INC_UTILITIES_H_ */
