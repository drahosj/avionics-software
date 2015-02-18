/* Tasks.c */
#include <stdint.h>
#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <tasks.h>
#include <flash.h>
#include <spi.h>
#include <usart.h>
#include <stdio.h>

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

static void prepareStateChange();

void initializeTasks()
{
	LEDS_Reset();
	arming_time = 0;
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
	uint8_t txBuffer[10];
	uint8_t rxBuffer[10];
	uint8_t sprintfBuffer[32];
	uint16_t UT = 0;
	uint16_t UP = 0;
	uint32_t startTime = 0;
	
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
	txBuffer[0] = 0xF6;
	ASSERT_CS()
	SPI1_Transfer(txBuffer, rxBuffer, 3);
	DEASSERT_CS();
	UT = rxBuffer[2]; /* LSB */
	UT |= (rxBuffer[1] << 8); /* MSB */
	
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
	
	/* Read UT */
	txBuffer[0] = 0xF6;
	ASSERT_CS()
	SPI1_Transfer(txBuffer, rxBuffer, 3);
	DEASSERT_CS();
	UP = rxBuffer[2]; /* LSB */
	UP |= (rxBuffer[1] << 8); /* MSB */
	
	sprintf(sprintfBuffer, "UP:%d\r\n", UT);
	usart_puts(USART2, sprintfBuffer);
	
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