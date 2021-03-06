/* Tasks.c */

/* BSD License blah blah blah for the whole thing
 * 
 * Barometer code adapted from Adafruit BMP183 library (www.adafruit.com)
 * by Limor Fried/Ladyada
 --Barometer License--
  *************************************************** 
  This is a library for the Adafruit BMP183 Barometric Pressure + Temp sensor

  Designed specifically to work with the Adafruit BMP183 Breakout 
  ----> http://www.adafruit.com/products/1900

  These sensors use SPI to communicate, 4 pins are required to  
  interface
  Adafruit invests time and resources providing this open source code, 
  please support Adafruit and open-source hardware by purchasing 
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.  
  BSD license, all text above must be included in any redistribution
 */
#include <stdint.h>
#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <tasks.h>
#include <flash.h>
#include <spi.h>
#include <usart.h>
#include <stdio.h>
#include <math.h>

#define ARM_DELAY 25

extern volatile uint32_t FlightTime;
uint32_t LaunchTime;

static uint16_t arming_time = 0;

static uint8_t state = STATE_UNARMED;

static void doUnarmedTick();
static void doArmingTick();
static void doArmedTick();
static void doCoastTick();
static void doPowerTick();
static void doTestFireTick();

static void prepareStateChange();

void initializeTasks()
{
	LEDS_Reset();
	arming_time = 0;
    BAROMETER_Initialize();
}

void Task_1s()
{
}

void Task_10s()
{
    if (state == STATE_ARMED)
    {
        BAROMETER_ReadToFlash();
    }
}

void Task_100ms()
{
	switch (state)
	{
	case STATE_UNARMED:
		doUnarmedTick();
		break;
	case STATE_ARMING:
		doArmingTick();
		break;
	case STATE_ARMED:
		doArmedTick();
		break;
    case STATE_POWER:
        doPowerTick();
        break;
    case STATE_COAST:
        doCoastTick();
        break;
	case STATE_TEST_FIRE:
		doTestFireTick();
		break;
	}
	LEDS_Update(state);
}

static void prepareStateChange()
{
	LEDS_Reset();
	arming_time = 0;
}

static void doUnarmedTick()
{
	uint16_t tmp;
	
	tmp = GPIOC->IDR & 0x0010;
	if (tmp == 0)
	{
		prepareStateChange();
		state = STATE_ARMING;
	}
}

static void doArmingTick()
{
	uint16_t tmp;
	
	arming_time++;
	tmp = GPIOC->IDR & 0x0010;
	if (tmp)
	{
		prepareStateChange();
		state = STATE_UNARMED;
	}
	else if(arming_time > ARM_DELAY)
	{
		state = STATE_ARMED;
		LEDS_Update(state);
		prepareStateChange();
		FLASH_Clear();
	}
}

static void doArmedTick()
{
	uint16_t tmp;
	
	BAROMETER_ReadToBuffer();
	
	tmp = GPIOC->IDR & 0x0010;
	if (tmp == 0)
	{
		arming_time++;
	}
	else
	{
		arming_time = 0;
	}
	
	tmp = BAROMETER_CheckLaunch();
    if (tmp)
    {
        prepareStateChange();
        FLASH_PutPacket(EVENT_LAUNCH, FlightTime, tmp);
        BAROMETER_BufferToFlash();
        BAROMETER_ClearBuffer();
        
        LaunchTime = FlightTime;
        
        state = STATE_POWER;
    }
	
	if (arming_time > ARM_DELAY)
	{
		prepareStateChange();
        FLASH_PutPacket(EVENT_LAUNCH, LAUNCH_SOURCE_MANUAL, tmp);
        BAROMETER_BufferToFlash();
        BAROMETER_ClearBuffer();
		state = STATE_TEST_FIRE;
	}
} 

static void doPowerTick()
{
    uint16_t tmp;
    
    BAROMETER_ReadToFlash();
    
    arming_time++;
    
    if (arming_time > 30) /* Three seconds of power */
    {
        FLASH_PutPacket(EVENT_BURNOUT, FlightTime, 0);
        prepareStateChange();
        state = STATE_COAST;
    }
} 

static void doCoastTick()
{
    uint16_t tmp;
    
    BAROMETER_ReadToFlash();
    
    if (0) /* Detect apogee */
    {
        prepareStateChange();
        state = STATE_COAST;
    }
    if (FlightTime - LaunchTime > 600000)
    {
        NVIC_SystemReset();
    }
}

static void doTestFireTick()
{
	uint8_t tmp = GPIOC->ODR;
	arming_time++;
	tmp |= 0x03c0;
	GPIOC->ODR = tmp;
    
    BAROMETER_ReadToFlash();
	
	if (arming_time > 25)
	{
		tmp = GPIOC->ODR;
		tmp = tmp & ~0x3c0;
		GPIOC->ODR = tmp;
		
		prepareStateChange();
		state = STATE_UNARMED;
	}
}