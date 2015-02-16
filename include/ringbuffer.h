#ifndef RINGBUFFER_H
#define RINGBUFFER_H
#include <stdint.h>

typedef struct ringbuffer_struct
{
	uint8_t * buffer;
	uint8_t * head;
	uint8_t * tail;
	
	uint32_t maximum_size;
} Ringbuffer_t;

void Ringbuffer_Create(Ringbuffer_t *, uint8_t *, uint32_t);
uint8_t Ringbuffer_Put(Ringbuffer_t *, uint8_t);
uint8_t Ringbuffer_Get(Ringbuffer_t *, uint8_t *);

#define E_NO_ERROR 0
#define E_RINGBUFFER_EMPTY 1
#define E_RINGBUFFER_FULL 2
#endif