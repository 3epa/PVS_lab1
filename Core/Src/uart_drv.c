#include "uart_drv.h"
#include "ring_buf.h"
#include <string.h>

static UART_HandleTypeDef *s_huart = NULL;
static bool s_use_it = true;

static ring_buf_t s_rx_buf;
static ring_buf_t s_tx_buf;

static uint8_t s_rx_byte;
static uint8_t s_tx_byte;
static volatile bool s_tx_busy = false;

static void StartNextTx(void) {
	uint32_t pmask = __get_PRIMASK();
	__disable_irq();

	if (!s_tx_busy && RingBuf_Pop(&s_tx_buf, &s_tx_byte)) {
		s_tx_busy = true;
		__set_PRIMASK(pmask);
		HAL_UART_Transmit_IT(s_huart, &s_tx_byte, 1);
		return;
	}

	__set_PRIMASK(pmask);
}

void UART_Init(UART_HandleTypeDef *huart, bool use_interrupts) {
	s_huart = huart;
	RingBuf_Init(&s_rx_buf);
	RingBuf_Init(&s_tx_buf);
	s_tx_busy = false;

	UART_SetInterruptMode(use_interrupts);
}

void UART_SetInterruptMode(bool enable) {
	if (!enable) {
		// Ждем, пока дошлются байты из очереди перед отключением прерываний
		uint32_t start = HAL_GetTick();
		while ((s_tx_busy || !RingBuf_IsEmpty(&s_tx_buf))
				&& (HAL_GetTick() - start < 50)) {
		}

		HAL_UART_Abort(s_huart);
		HAL_NVIC_DisableIRQ(USART6_IRQn);
		s_tx_busy = false;
		s_use_it = false;
	} else {
		HAL_UART_Abort(s_huart);
		s_tx_busy = false;
		s_use_it = true;

		HAL_NVIC_EnableIRQ(USART6_IRQn);
		HAL_UART_Receive_IT(s_huart, &s_rx_byte, 1);
		StartNextTx();
	}
}

bool UART_IsInterruptMode(void) {
	return s_use_it;
}

bool UART_ReadByte(uint8_t *ch) {
	if (s_use_it) {
		return RingBuf_Pop(&s_rx_buf, ch);
	}

	// Если что-то осталось в буфере после переключения режима
	if (RingBuf_Pop(&s_rx_buf, ch)) {
		return true;
	}

	// Неблокирующее чтение по опросу (timeout = 0)
	if (HAL_UART_Receive(s_huart, ch, 1, 0) == HAL_OK) {
		return true;
	}

	return false;
}

void UART_SendByte(uint8_t ch) {
	if (s_use_it) {
		RingBuf_Push(&s_tx_buf, ch);
		StartNextTx();
	} else {
		HAL_UART_Transmit(s_huart, &ch, 1, 10);
	}
}

void UART_SendString(const char *str) {
	uint16_t len = strlen(str);
	if (s_use_it) {
		for (uint16_t i = 0; i < len; i++) {
			RingBuf_Push(&s_tx_buf, (uint8_t) str[i]);
		}
		StartNextTx();
	} else {
		HAL_UART_Transmit(s_huart, (uint8_t*) str, len, 50);
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == s_huart && s_use_it) {
		RingBuf_Push(&s_rx_buf, s_rx_byte);
		HAL_UART_Receive_IT(s_huart, &s_rx_byte, 1);
	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == s_huart && s_use_it) {
		if (RingBuf_Pop(&s_tx_buf, &s_tx_byte)) {
			HAL_UART_Transmit_IT(s_huart, &s_tx_byte, 1);
		} else {
			s_tx_busy = false;
		}
	}
}
