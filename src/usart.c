#include "usart.h"
#include "stdio.h"
#include "stdlib.h"

#define RINGBUFFER_SIZE 1024

static uint8_t txBuffer[RINGBUFFER_SIZE];
static uint8_t rxBuffer[RINGBUFFER_SIZE];

static uint8_t *txHead = txBuffer;
static uint8_t *txTail = txBuffer;

static uint8_t *rxHead = txBuffer;
static uint8_t *rxTail = txBuffer;

static status_t usart_send_byte(USART_TypeDef * usart);
static status_t usart_get_byte(USART_TypeDef * usart);

status_t initialize_usart(USART_TypeDef *usart)
{
    
    /* Set baud rate to 9600 baud. Assumes 16MHz Fck */
    /* Mantissa = 0d104, Frac = 3/16 */
    
    usart->BRR = 0;
    usart->BRR |= (104 << 4);
    usart->BRR |= 3;
    
    bitSet(usart->CR1, USART_CR1_RE); /* RX Enable */
    bitSet(usart->CR1, USART_CR1_TE); /* TX Enable */
    bitSet(usart->CR1, USART_CR1_UE); /* USART Enable */
    bitSet(usart->CR1, USART_CR1_TCIE); /* TC Interrupt Enabled */
    bitSet(usart->CR1, USART_CR1_RXNEIE); /* RXNE Interrupt Enabled */
    
    return STATUS_SUCCESS;
}

status_t usart_putc(USART_TypeDef * usart, uint8_t byte)
{
    *txHead = byte;
    txHead++;
    if (txHead >= txBuffer + RINGBUFFER_SIZE)
        txHead = txBuffer;
    
    NVIC_SetPendingIRQ(USART2_IRQn);
    
    return STATUS_SUCCESS;
}

status_t usart_getc(USART_TypeDef * usart, uint8_t * byte)
{
    if (rxHead == rxTail)
        return STATUS_RINGBUFFER_EMPTY;
    
    *byte = *rxTail;
    rxTail++;
    if (rxTail >= rxBuffer + RINGBUFFER_SIZE)
        rxTail = rxBuffer;
    return STATUS_SUCCESS;
}

int16_t usart_print(USART_TypeDef * usart, uint8_t *str, uint16_t len)
{
    uint16_t i = 0;
    for(i = 0; i < len; i++)
    {
        usart_putc( usart, str[i]);
    }
    return len;
}

int16_t usart_gets(USART_TypeDef * usart, uint8_t *buffer, uint16_t maxLen)
{
    uint16_t len = 0; 
    uint8_t *cursor;
    
    cursor = buffer;
    while ((len < maxLen) && (usart_getc(usart, cursor) == STATUS_SUCCESS))
    {
        cursor++;
        len++;
    }
    
    return len;
}

static status_t usart_get_byte(USART_TypeDef * usart)
{
    if (1) /* TODO: DETERMINE RINGBUFFER OVERRUN*/
    {
        if (usart->SR & USART_SR_RXNE)
        {
            *rxHead = usart->DR;
            rxHead++;
            if(rxHead >= rxBuffer + RINGBUFFER_SIZE)
                rxHead = rxBuffer;
            return STATUS_SUCCESS;
        }
        else
            return STATUS_RXBUFFER_EMPTY;
    }
    else
        return STATUS_RINGBUFFER_FULL;
}

static status_t usart_send_byte(USART_TypeDef * usart)
{
    if (txHead != txTail)
    {
        if (usart->SR & USART_SR_TXE)
        {
            usart->DR = *txTail & 0xff;
            txTail++;
            if (txTail >= txBuffer + RINGBUFFER_SIZE)
                txTail = txBuffer;
            return STATUS_SUCCESS;
        }
        else
            return STATUS_TXBUFFER_FULL;
    }
    else
    {
        bitClear(usart->SR, USART_SR_TC);
        return STATUS_RINGBUFFER_EMPTY;
    }
}

void USART2_IRQHandler()
{
    usart_send_byte(USART2);
    usart_get_byte(USART2);
    NVIC_ClearPendingIRQ(USART2_IRQn);
}