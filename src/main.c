
/* Standard includes. */

/* Scheduler includes. */
#include <system.h>
#include <stdio.h>

/* Hardware Includes */
#include <stm32f4xx.h>
#include <stm32f4xx_gpio.h>
#include <stm32f4xx_rcc.h>

/* Application includes */
#include <tasks.h>
#include <flash.h>
#include <usart.h>
#include <spi.h>

/*Address Defines*/

/*Init system config*/

volatile uint32_t FlightTime = 0;

static volatile uint8_t task_100_flag = 0;
static volatile uint8_t task_1s_flag = 0;
static volatile uint8_t task_10s_flag = 0;

static void lowLevelHardwareInit()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}

int main( void )
{
    lowLevelHardwareInit();

    GPIO_InitTypeDef GPIO_InitStructure;
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    
    GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIOC->ODR = 0x0000; /* SUPER EXTREME DANGER DEATH AND STUFF THIS WILL MAKE EXPLOSIONS */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
  
    /* Configure GPIOs for UART */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
  
    initialize_usart(USART2);
    NVIC_EnableIRQ(USART2_IRQn);
	
	SPI_Initialize();
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);

    SysTick_Config(SystemCoreClock/1000);
    initializeTasks();
	
	usart_puts(USART2, "Initialized. Dumping flash...\r\n");
	FLASH_Dump();
	
	uint8_t txBuffer[2] = {0xD0, 0x00};
	uint8_t rxBuffer[2];
	
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
	SPI1_Transfer(txBuffer, rxBuffer, 2);
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	
	while (rxBuffer[1] != 0x55) 
	
    for(;;)
    {
		if (task_100_flag)
		{
			Task_100ms();
			task_100_flag = 0;
		}
        if (task_1s_flag)
        {
            Task_1s();
            task_1s_flag = 0;
        }
		if (task_10s_flag)
		{
            Task_10s();
			task_10s_flag = 0;
		}
    }
}

void SysTick_Handler()
{
	FlightTime++;
	if (FlightTime % 100 == 0)
		task_100_flag = 1;
	if (FlightTime % 1000 == 0)
		task_1s_flag = 1;
    if (FlightTime % 10000 == 0)
        task_10s_flag = 1;
}

void WWDG_IRQHandler()
{
	NVIC_SystemReset();
}
