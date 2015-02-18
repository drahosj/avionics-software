#include <stm32f4xx.h>
#include "system.h"
#include "string.h"
#ifndef USART_H
#define USART_H

typedef uint8_t status_t;
#define STATUS_SUCCESS 0
#define STATUS_RINGBUFFER_EMPTY 80
#define STATUS_TXBUFFER_FULL 81
#define STATUS_RINGBUFFER_FULL 82
#define STATUS_RXBUFFER_EMPTY 83

status_t initialize_usart();
status_t usart_putc(USART_TypeDef *, uint8_t);
status_t usart_getc(USART_TypeDef *, uint8_t *);
int16_t usart_gets(USART_TypeDef * usart, uint8_t *buffer, uint16_t maxlen);
int16_t usart_print(USART_TypeDef * usart, uint8_t *str, uint16_t len);

int16_t uprintf(const char *fmt, ...);



#define usart_puts(usart, str) usart_print(usart, str, strlen(str));
#endif
