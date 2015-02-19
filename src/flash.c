/* flash.c */
#include <flash.h>
#include <stm32f4xx_flash.h>
#include <usart.h>
#include <stdio.h>

/* Use sectors 5, 6, and 7 for flash. Leaves 128k for program */
#define FLASH_ADDRESS_BASE 0x08020000
#define FLASH_ADDRESS_MAX 0x0807FFFF

static uint32_t currentDataPtr = FLASH_ADDRESS_BASE;

extern volatile uint32_t FlightTime;
static void wait(uint16_t msec)
{
	uint32_t startTime = FlightTime;
	while (FlightTime < startTime + msec) {}
}

void FLASH_Clear()
{
	FLASH_Status status;
	
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	
	status = FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3);
	while (status != FLASH_COMPLETE) {}
	
	status = FLASH_EraseSector(FLASH_Sector_6, VoltageRange_3);
	while (status != FLASH_COMPLETE) {}
	
	status = FLASH_EraseSector(FLASH_Sector_7, VoltageRange_3);
	while (status != FLASH_COMPLETE) {}
	
	FLASH_Lock();
	
	currentDataPtr = FLASH_ADDRESS_BASE;
}

void FLASH_Dump()
{
	uint8_t * ptr = (uint8_t *) FLASH_ADDRESS_BASE;
	uint32_t value = 0;
	char printfBuffer[42];
	
	/* Currently print everything until a 0xFFFFFFFF */
	while (value != 0xFFFFFFFF)
	{
		value = 0;
		value |= *(ptr++) << 24;
		value |= *(ptr++) << 16;
		value |= *(ptr++) << 8;
		value |= *(ptr++);
		sprintf(printfBuffer, "%d\r\n", value);
		usart_puts(USART2, printfBuffer);
		wait(10);
	}
}

void FLASH_PutData(uint8_t * data, uint16_t length)
{
	FLASH_Status status;
	uint16_t i;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR); 
	for (i = 0; i < length; i++)
	{
		status = FLASH_ProgramByte(currentDataPtr++, data[i]);
		while (status != FLASH_COMPLETE) {};
	}
	FLASH_Lock();
}