/* leds.c */
#include <tasks.h>
#include <stm32f4xx.h>
#include <stdint.h>
#include <leds.h>
static uint8_t led_pattern_unarmed[] = {0x03, 0x0c};
static uint8_t led_pattern_unarmed_period[] = {10, 10};
static uint8_t led_pattern_unarmed_length = 2;

static uint8_t led_pattern_arming[] = {0x01, 0x02, 0x04, 0x08};
static uint8_t led_pattern_arming_period[] = {1, 1, 1, 1};
static uint8_t led_pattern_arming_length = 4;

static uint8_t led_pattern_armed[] = {0x00, 0x0f, 0x00, 0x00};
static uint8_t led_pattern_armed_period[] = {1, 2, 1, 35};
static uint8_t led_pattern_armed_length = 4;

static uint8_t led_pattern_testfire[] = {0x00, 0x0f};
static uint8_t led_pattern_testfire_period[] = {1, 1};
static uint8_t led_pattern_testfire_length = 2;

static uint8_t led_pattern_power[] = {0x00, 0x0f};
static uint8_t led_pattern_power_period[] = {0, 1};
static uint8_t led_pattern_power_length = 2;

static uint8_t led_pattern_deploy[] = {0x00, 0x0f};
static uint8_t led_pattern_deploy_period[] = {1, 3};
static uint8_t led_pattern_deploy_length = 4;

static uint8_t led_pattern_idle[] = {0x00, 0x0f};
static uint8_t led_pattern_idle_period[] = {3, 1};
static uint8_t led_pattern_idle_length = 4;

#define LED_MASK 0x000f

static uint8_t led_position = 0;
static uint8_t led_time = 0;

void LEDS_Update (State_t state)
{
	uint16_t tmp;
	uint8_t led_length;
	uint8_t * led_pattern;
	uint8_t * led_period;
	
	switch(state)
	{
	case STATE_UNARMED:
		led_pattern = led_pattern_unarmed;
		led_length = led_pattern_unarmed_length;
		led_period = led_pattern_unarmed_period;
		break;
	case STATE_ARMING:
		led_pattern = led_pattern_arming;
		led_length = led_pattern_arming_length;
		led_period = led_pattern_arming_period;
		break;
    case STATE_ARMED:
        led_pattern = led_pattern_armed;
        led_length = led_pattern_armed_length;
        led_period = led_pattern_armed_period;
        break;
	case STATE_POWER:
		led_pattern = led_pattern_power;
		led_length = led_pattern_power_length;
		led_period = led_pattern_power_period;
		break;
    case STATE_COAST:
    case STATE_DESCEND:
        led_pattern = led_pattern_idle;
        led_length = led_pattern_idle_length;
        led_period = led_pattern_idle_period;
        break;
    case STATE_DEPLOY_1:
    case STATE_DEPLOY_2:
    case STATE_DEPLOY_3:
    case STATE_DEPLOY_4:
        led_pattern = led_pattern_deploy;
        led_length = led_pattern_deploy_length;
        led_period = led_pattern_deploy_period;
        break;
	case STATE_TEST_FIRE:
		led_pattern = led_pattern_testfire;
		led_length = led_pattern_testfire_length;
		led_period = led_pattern_testfire_period;
		break;
	}
	
	led_time++;
	if (led_time >= led_period[led_position])
	{
		led_position++;
		if (led_position >= led_length)
			led_position = 0;
		led_time = 0;
	}
	
	tmp = GPIOC->ODR;
	tmp = (tmp & ~LED_MASK) | (led_pattern[led_position] & LED_MASK);
  if ((state == STATE_ARMED) && (led_position == 3))
  {
    tmp |= ((~GPIOB->IDR) >> 12) & 0x000f;
  }
	GPIOC->ODR = tmp;
}
void LEDS_Reset()
{
	led_position = 0;
	led_time = 0;
}