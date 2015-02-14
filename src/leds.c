/* leds.c */
#include <tasks.h>
#include <stm32f4xx.h>
#include <stdint.h>
static uint8_t led_pattern_unarmed[] = {0x03, 0x0c};
static uint8_t led_pattern_unarmed_period[] = {10, 10};
static uint8_t led_pattern_unarmed_length = 2;

static uint8_t led_pattern_arming[] = {0x01, 0x02, 0x04, 0x08};
static uint8_t led_pattern_arming_period[] = {1, 1, 1, 1};
static uint8_t led_pattern_arming_length = 4;

static uint8_t led_pattern_armed[] = {0x00, 0x0f, 0x00, 0x00};
static uint8_t led_pattern_armed_period[] = {1, 2, 1, 35};
static uint8_t led_pattern_armed_length = 4;


static uint8_t led_position = 0;
static uint8_t led_time = 0;

void LEDS_update (State_t state)
{
	uint16_t tmp;
	switch(state)
	{
	case STATE_UNARMED:
		/* LEDs stuff */
		led_time++;
		if (led_time >= led_pattern_unarmed_period[led_position])
		{
			led_position++;
			if (led_position >= led_pattern_unarmed_length)
				led_position = 0;
			led_time = 0;
		}
		
		tmp = GPIOC->ODR;
		tmp = (tmp & 0xfff0) | (led_pattern_unarmed[led_position] & 0x000f);
		GPIOC->ODR = tmp;
		
		break;
	case STATE_ARMING:
		
		break;
	case STATE_ARMED:
		
		break;
	case STATE_TEST_FIRE:
		
		break;
	}
}