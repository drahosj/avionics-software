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
    uint8_t type;
    uint32_t timestamp;
    uint32_t data;
	char printfBuffer[42];
	
	/* Currently print everything until a 0xFFFFFFFF */
    usart_puts(USART2, "\r========\r"); /* Magic token */
	for(;;)
	{
        type = *(ptr++);
        if (type == 0xff)
            break;
            
		timestamp = 0;
		timestamp |= *(ptr++) << 16;
		timestamp |= *(ptr++) << 8;
		timestamp |= *(ptr++);
        
        data = 0;
        data |= *(ptr++) << 24;
        data |= *(ptr++) << 16;
        data |= *(ptr++) << 8;
        data |= *(ptr++);
        
        sprintf(printfBuffer, "=%d:0x%x:%d\r", timestamp, type, data);
        usart_puts(USART2, printfBuffer);
        
        /* Wait for TX Buffer to empty so we don't overflow it by flooding
         * it with data */
        wait(4);
	}
}

/* Data packet format: 8 bytes
 * 8 bits type identifier, 3 bytes 24-bit timestamp (msec), 32 bit data
 * */

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

void FLASH_PutPacket(uint8_t type, uint32_t timestamp, uint32_t data)
{ 
    uint8_t flashBuffer[8]; 
    flashBuffer[0] = type;
    flashBuffer[1] = (timestamp >> 16) & 0xFF;
    flashBuffer[2] = (timestamp >> 8) & 0xFF;
    flashBuffer[3] = timestamp & 0xFF;
    
    flashBuffer[4] = (data >> 24) & 0xFF;
    flashBuffer[5] = (data >> 16) & 0xFF;
    flashBuffer[6] = (data >> 8) & 0xFF;
    flashBuffer[7] = data & 0xFF;
    FLASH_PutData(flashBuffer, 8);
    
}