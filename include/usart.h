#include <stm32f4xx.h>
#include "system.h"
#include "string.h"

status_t initialize_usart(USART_TypeDef *);
status_t usart_putc(USART_TypeDef *, uint8_t);
status_t usart_getc(USART_TypeDef *, uint8_t *);
int16_t usart_gets(USART_TypeDef * usart, uint8_t *buffer, uint16_t maxlen);
int16_t usart_print(USART_TypeDef * usart, uint8_t *str, uint16_t len);

#define usart_puts(usart, str) usart_print(usart, str, strlen(str));
