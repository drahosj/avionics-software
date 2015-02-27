/* barometer.h */

#ifndef BAROMETER_H
#define BAROMETER_H
#include <stdint.h>

void BAROMETER_ReadToFlash();
void BAROMETER_ReadToBuffer();
void BAROMETER_BufferToFlash();
void BAROMETER_ClearBuffer();
void BAROMETER_Initialize();

#endif