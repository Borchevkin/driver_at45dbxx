
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
#include "driver_at45dbxx.h"
#include "utilities.h"

at45dbxx_t at45dbxx;

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

	SetupUtilities();

	Delay(100);

	uint8_t result[PAGE_SIZE];
	uint8_t tx_data[PAGE_SIZE];

	memset(tx_data,0x00,PAGE_SIZE);
	memset(result,0x00,PAGE_SIZE);

	AT45DBXX_CheckID(&at45dbxx);

	// ================ TEST FIELD

	Delay(1000);

	/*
	AT45DBXX_ChipErase(&at45dbxx);

	memset(tx_data,0xAC,PAGE_SIZE);

	AT45DBXX_WriteMemory(&at45dbxx, 1, tx_data, PAGE_SIZE);

	memset(tx_data,0xBA,PAGE_SIZE);

	AT45DBXX_WriteMemory(&at45dbxx, 2, tx_data, PAGE_SIZE);

	memset(tx_data,0xCC,PAGE_SIZE);

	AT45DBXX_WriteMemory(&at45dbxx, 3, tx_data, PAGE_SIZE);

	memset(tx_data,0x88,PAGE_SIZE);

	AT45DBXX_WriteMemory(&at45dbxx, 4, tx_data, PAGE_SIZE);
	*/
	while (1){
		AT45DBXX_ReadMemory(&at45dbxx, 1, result);
		Delay(1000);
	}
}
