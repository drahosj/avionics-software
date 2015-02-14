/* Tasks.c */
#include <stdint.h>
#include <stm32f4xx.h>
#include <tasks.h>

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
	led_position = 0;
	led_time = 0;
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
}

void prepareStateChange()
{
	led_position = 0;
	led_time = 0;
	arming_time = 0;
}

static void doUnarmedTick()
{
	uint16_t tmp;
	
	/* State transition stuff */
	tmp = GPIOC->IDR & 0x0010;
	if (tmp == 0)
	{
		prepareStateChange();
		state = STATE_ARMING;
	}
}

static void doArmingTick()
{
	/* LEDs stuff */
	led_time++;
	if (led_time >= led_pattern_arming_period[led_position])
	{
		led_position++;
		if (led_position >= led_pattern_arming_length)
			led_position = 0;
		led_time = 0;
	}
	
	uint16_t tmp = GPIOC->ODR;
	tmp = (tmp & 0xfff0) | (led_pattern_arming[led_position] & 0x000f);
	GPIOC->ODR = tmp;
	
	/* State transition stuff */
	arming_time++;
	tmp = GPIOC->IDR & 0x0010;
	if (tmp)
	{
		prepareStateChange();
		state = STATE_UNARMED;
	}
	else if(arming_time > ARM_DELAY)
	{
		prepareStateChange();
		state = STATE_ARMED;
	}
}

static void doArmedTick()
{
	/* LEDs stuff */
	led_time++;
	if (led_time >= led_pattern_armed_period[led_position])
	{
		led_position++;
		if (led_position >= led_pattern_armed_length)
			led_position = 0;
		led_time = 0;
	}
	
	uint16_t tmp = GPIOC->ODR;
	uint16_t tmp2 = GPIOB->IDR;
	tmp2 = (tmp2 & 0xf000) >> 12;
	
	tmp = (tmp & 0xfff0) | (led_pattern_armed[led_position] & 0x000f);
	
	if (led_position == 3)
	{
		tmp = (tmp & 0xfff0) | ((~tmp2) & 0x000f);
	}
	
	GPIOC->ODR = tmp;
	
	/* State transition stuff */
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