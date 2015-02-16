#include <ringbuffer.h>

void Ringbuffer_Create(Ringbuffer_t * ringbuffer, uint8_t * buffer, uint32_t size)
{
	ringbuffer->buffer = buffer;
	ringbuffer->head = buffer;
	ringbuffer->tail = buffer;
	ringbuffer->maximum_size = size;
}

uint8_t Ringbuffer_Put(Ringbuffer_t * ringbuffer, uint8_t c)
{
	/* Todo: Find some way to tell if the buffer is full*/
	*(ringbuffer->head) = c;
	(ringbuffer->head)++;
	if (ringbuffer->head >= (ringbuffer->buffer + ringbuffer->maximum_size))
	{
		ringbuffer->head = ringbuffer->buffer;
	}
	return E_NO_ERROR;
}

uint8_t Ringbuffer_Get(Ringbuffer_t * ringbuffer, uint8_t * c)
{
	if (ringbuffer->head != ringbuffer->tail)
	{
		*c = *(ringbuffer->tail);
		(ringbuffer->tail)++;
		if (ringbuffer->tail >= (ringbuffer->buffer + ringbuffer->maximum_size))
		{
			ringbuffer->tail = ringbuffer->buffer;
		}
		return E_NO_ERROR;
	}
	else
	{
		return E_RINGBUFFER_EMPTY;
	}
}