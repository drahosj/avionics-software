/* flash.c */
#include <flash.h>
#include <stm32f4xx_flash.h>
#include <usart.h>

/* Use sectors 5, 6, and 7 for flash. Leaves 128k for program */
#define FLASH_ADDRESS_BASE 0x08020000
#define FLASH_ADDRESS_MAX 0x0807FFFF

static uint32_t currentDataPtr = FLASH_ADDRESS_BASE;

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
	
	/* Currently print everything until a 0xFF */
	while ( *ptr != 0xff)
	{
		usart_putc(USART2, *(ptr++));
	}
}

void FLASH_PutData(uint8_t * data, uint16_t length)
{
	FLASH_Status status;
	uint16_t i;
	FLASH_Unlock();
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR); 
	for (i = 0; i < length; i++)
	{
		status = FLASH_ProgramByte(currentDataPtr++, data[i]);
		while (status != FLASH_COMPLETE) {};
	}
	FLASH_Lock();
}