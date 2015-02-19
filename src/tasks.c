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

   enum
    {
      BMP183_REGISTER_CAL_AC1            = 0xAA,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_AC2            = 0xAC,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_AC3            = 0xAE,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_AC4            = 0xB0,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_AC5            = 0xB2,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_AC6            = 0xB4,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_B1             = 0xB6,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_B2             = 0xB8,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_MB             = 0xBA,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_MC             = 0xBC,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CAL_MD             = 0xBE,  // R   Calibration data (16 bits)
      BMP183_REGISTER_CHIPID             = 0xD0,
      BMP183_REGISTER_VERSION            = 0xD1,
      BMP183_REGISTER_SOFTRESET          = 0xE0,
      BMP183_REGISTER_CONTROL            = 0xF4,
      BMP183_REGISTER_TEMPDATA           = 0xF6,
      BMP183_REGISTER_PRESSUREDATA       = 0xF6,
      BMP183_REGISTER_READTEMPCMD        = 0x2E,
      BMP183_REGISTER_READPRESSURECMD    = 0x34
    };

#define ASSERT_CS() GPIO_ResetBits(GPIOA, GPIO_Pin_4);
#define DEASSERT_CS() GPIO_SetBits(GPIOA, GPIO_Pin_4);

#define ARM_DELAY 25

extern volatile uint64_t FlightTime;

static uint16_t arming_time = 0;

static uint8_t state = STATE_UNARMED;

static void doUnarmedTick();
static void doArmingTick();
static void doArmedTick();
static void doTestFireTick();

static void doBarometer();
static float calculateAltitude(uint32_t T, uint32_t P);
static uint16_t read16(uint8_t address);

static uint16_t ac1, ac2, ac3, ac4, ac5, ac6, b1, b2, mb, mc, md; /* Calibration constants */

static void prepareStateChange();

void initializeTasks()
{
	LEDS_Reset();
	arming_time = 0;
	
	
	ac1 = read16(BMP183_REGISTER_CAL_AC1);
	ac2 = read16(BMP183_REGISTER_CAL_AC2);
	ac3 = read16(BMP183_REGISTER_CAL_AC3);
	ac4 = read16(BMP183_REGISTER_CAL_AC4);
	ac5 = read16(BMP183_REGISTER_CAL_AC5);
	ac6 = read16(BMP183_REGISTER_CAL_AC6);

	b1 = read16(BMP183_REGISTER_CAL_B1);
	b2 = read16(BMP183_REGISTER_CAL_B2);

	mb = read16(BMP183_REGISTER_CAL_MB);
	mc = read16(BMP183_REGISTER_CAL_MC);
	md = read16(BMP183_REGISTER_CAL_MD);
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
	case STATE_TEST_FIRE:
		doTestFireTick();
		break;
	}
	LEDS_Update(state);
}

void prepareStateChange()
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
	
	doBarometer();
	
	tmp = GPIOC->IDR & 0x0010;
	if (tmp == 0)
	{
		arming_time++;
	}
	else
	{
		arming_time = 0;
	}
		
	if (arming_time > ARM_DELAY)
	{
		prepareStateChange();
		state = STATE_TEST_FIRE;
	}
}

static void doTestFireTick()
{
	uint8_t tmp = GPIOC->ODR;
	arming_time++;
	tmp |= 0x03c0;
	GPIOC->ODR = tmp;
	
	if (arming_time > 25)
	{
		tmp = GPIOC->ODR;
		tmp = tmp & ~0x3c0;
		GPIOC->ODR = tmp;
		
		prepareStateChange();
		state = STATE_UNARMED;
	}
}

static void doBarometer()
{
	uint8_t txBuffer[10];
	uint8_t rxBuffer[10];
	uint8_t sprintfBuffer[32];
	uint32_t startTime = 0;
	int32_t UT, UP, B3, B4, B5, B6, B7, X1, X2, X3, T, P;
	uint8_t oss = 0;
	
	/* Read barometer and write to flash */
	/* Initiate temperature read */
	txBuffer[0] = 0x74;
	txBuffer[1] = 0x2E;
	ASSERT_CS()
	SPI1_Transfer(txBuffer, rxBuffer, 2);
	DEASSERT_CS();
	startTime = FlightTime;
	while (FlightTime < startTime + 5) {} /* DELAY FOR READ - TRY TO PROPERLY ASYNC THIS! */
	
	/* Read UT */
	UT = read16(0xF6);
	
	sprintf(sprintfBuffer, "UT:%d ", UT);
	usart_puts(USART2, sprintfBuffer);
	
	/* Initiate pressure read */	
	txBuffer[0] = 0x74;
	txBuffer[1] = 0x34;
	ASSERT_CS()
	SPI1_Transfer(txBuffer, rxBuffer, 2);
	DEASSERT_CS();
	startTime = FlightTime;
	while (FlightTime < startTime + 5) {} /* DELAY FOR READ - TRY TO PROPERLY ASYNC THIS! */
	
	/* Read UP */
	UP = read16(0xF6);
	
  UT = 27898;
  UP = 23843;
  ac6 = 23153;
  ac5 = 32757;
  mc = -8711;
  md = 2868;
  b1 = 6190;
  b2 = 4;
  ac3 = -14383;
  ac2 = -72;
  ac1 = 408;
  ac4 = 32741;
	
	sprintf(sprintfBuffer, "UP:%d ", UP);
	usart_puts(USART2, sprintfBuffer);
	
	  // do temperature calculations
  X1=(UT-ac6)*(ac5)/pow(2,15);
  X2=(mc*pow(2,11))/(X1+md);
  B5=X1 + X2;
  T = (B5+8)/pow(2,4);
/*
  // do pressure calcs
  B6 = B5 - 4000;
  X1 = ((int32_t)b2 * ( (B6 * B6)>>12 )) >> 11;
  X2 = ((int32_t)ac2 * B6) >> 11;
  X3 = X1 + X2;
  B3 = ((((int32_t)ac1*4 + X3) << oversampling) + 2) / 4;

  X1 = ((int32_t)ac3 * B6) >> 13;
  X2 = ((int32_t)b1 * ((B6 * B6) >> 12)) >> 16;
  X3 = ((X1 + X2) + 2) >> 2;
  B4 = ((uint32_t)ac4 * (uint32_t)(X3 + 32768)) >> 15;
  B7 = ((uint32_t)UP - B3) * (uint32_t)( 50000UL >> oversampling );

  if (B7 < 0x80000000) {
    p = (B7 * 2) / B4;
  } else {
    p = (B7 / B4) * 2;
  }
  X1 = (p >> 8) * (p >> 8);
  X1 = (X1 * 3038) >> 16;
  X2 = (-7357 * p) >> 16;

  p = p + ((X1 + X2 + (int32_t)3791)>>4);*/
  
sprintf(sprintfBuffer, "T:%d P:%d\r\n", T, P);
usart_puts(USART1, sprintfBuffer);
}

uint16_t read16(uint8_t address)
{
	uint8_t txBuffer[3];
	uint8_t rxBuffer[3];
	uint16_t ret;
	
	txBuffer[0] = address;
	ASSERT_CS()
	SPI1_Transfer(txBuffer, rxBuffer, 3);
	DEASSERT_CS();
	ret = rxBuffer[2]; /* LSB */
	ret |= (rxBuffer[1] << 8); /* MSB */
	
	return ret;	
}