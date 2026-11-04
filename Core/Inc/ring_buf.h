#ifndef INC_RING_BUF_H_
#define INC_RING_BUF_H_

#include <stdint.h>
#include <stdbool.h>

#define RING_BUF_SIZE  128

typedef struct {
	uint8_t data[RING_BUF_SIZE];
	uint16_t head;
	uint16_t count;
} ring_buf_t;

void RingBuf_Init(ring_buf_t *rb);
bool RingBuf_Push(ring_buf_t *rb, uint8_t val);
bool RingBuf_Pop(ring_buf_t *rb, uint8_t *val);
bool RingBuf_IsEmpty(const ring_buf_t *rb);

#endif /* INC_RING_BUF_H_ */
