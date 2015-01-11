/* Common.h */

#include "stdint.h"

#define pdTRUE 1
#define pdFALSE 0


typedef uint8_t status_t;
#define STATUS_SUCCESS 0
#define STATUS_RINGBUFFER_EMPTY 80
#define STATUS_TXBUFFER_FULL 81
#define STATUS_RINGBUFFER_FULL 82
#define STATUS_RXBUFFER_EMPTY 83

#define configKERNEL_INTERRUPT_PRIORITY 255
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY 15
