/* flash.h */

#ifndef FLASH_H
#define FLASH_H
#include <stdint.h>

void FLASH_Clear();
void FLASH_Initialize();
void FLASH_PutData(uint8_t * data, uint16_t length);
void FLASH_PutPacket(uint8_t type, uint32_t time, uint32_t data);
void FLASH_Dump();

#define BAROMETER_PRESSURE 0x10
#define BAROMETER_TEMPERATURE 0x11

#define EVENT_LAUNCH 0x20
#define EVENT_BURNOUT 0x21

#define LAUNCH_SOURCE_PRESSURE 0x01
#define LAUNCH_SOURCE_TEMPERATURE 0x02
#define LAUNCH_SOURCE_MANUAL 0x03

#endif