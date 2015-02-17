/* flash.h */

#ifndef FLASH_H
#define FLASH_H
#include <stdint.h>

void FLASH_Clear();
void FLASH_Initialize();
void FLASH_PutData(uint8_t * data, uint16_t length);
void FLASH_Dump();

#endif