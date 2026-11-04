#ifndef INC_UART_DRV_H_
#define INC_UART_DRV_H_

#include <stdint.h>
#include <stdbool.h>
#include "main.h"

void UART_Init(UART_HandleTypeDef *huart, bool use_interrupts);
void UART_SetInterruptMode(bool enable);
bool UART_IsInterruptMode(void);

bool UART_ReadByte(uint8_t *ch);
void UART_SendByte(uint8_t ch);
void UART_SendString(const char *str);

#endif /* INC_UART_DRV_H_ */
