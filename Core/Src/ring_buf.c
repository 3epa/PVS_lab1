#include "ring_buf.h"
#include "stm32f4xx.h"

void RingBuf_Init(ring_buf_t *rb) {
	uint32_t pmask = __get_PRIMASK();
	__disable_irq();

	rb->head = 0;
	rb->count = 0;

	__set_PRIMASK(pmask);
}

bool RingBuf_Push(ring_buf_t *rb, uint8_t val) {
	bool ok = false;
	uint32_t pmask = __get_PRIMASK();
	__disable_irq();

	if (rb->count < RING_BUF_SIZE) {
		rb->data[rb->head] = val;
		rb->head = (rb->head + 1) % RING_BUF_SIZE;
		rb->count++;
		ok = true;
	}

	__set_PRIMASK(pmask);
	return ok;
}

bool RingBuf_Pop(ring_buf_t *rb, uint8_t *val) {
	bool ok = false;
	uint32_t pmask = __get_PRIMASK();
	__disable_irq();

	if (rb->count > 0) {
		// Вычисляем индекс чтения из указателя записи и счетчика
		uint16_t tail = (rb->head + RING_BUF_SIZE - rb->count) % RING_BUF_SIZE;
		*val = rb->data[tail];
		rb->count--;
		ok = true;
	}

	__set_PRIMASK(pmask);
	return ok;
}

bool RingBuf_IsEmpty(const ring_buf_t *rb) {
	return (rb->count == 0);
}
