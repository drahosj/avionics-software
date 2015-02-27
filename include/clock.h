#ifndef CLOCK_H
#define CLOCK_H

/*
 *Public clock API
 *
 *Start the clock task, then read mission time in seconds
 *
 * Clock task should be started early in init
 *
 */
#include <stdint.h>
void startClock();
long getFlightTime();

extern uint32_t FlightTime;
void wait(uint16_t msec);


#endif
