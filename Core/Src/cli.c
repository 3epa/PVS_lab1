#include "cli.h"
#include "uart_drv.h"
#include "garland.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static char s_cmd_buf[32];
static uint8_t s_pos = 0;

static bool s_wait_period = false;
static char s_new_seq[9];

static void ProcessCommand(const char *cmd) {
	if (strcmp(cmd, "set interrupts on") == 0) {
		UART_SetInterruptMode(true);
		UART_SendString("\r\nOK\r\n");
	} else if (strcmp(cmd, "set interrupts off") == 0) {
		UART_SetInterruptMode(false);
		UART_SendString("\r\nOK\r\n");
	} else if (strncmp(cmd, "new ", 4) == 0) {
		const char *seq = cmd + 4;
		if (Garland_CheckPattern(seq)) {
			strcpy(s_new_seq, seq);
			s_wait_period = true;
			UART_SendString(
					"\r\nEnter period (1 - 200ms, 2 - 500ms, 3 - 1000ms):\r\n");
		} else {
			UART_SendString(
					"\r\nError: invalid sequence (use 2..8 chars of g, r, y, n)\r\n");
		}
	} else if (strncmp(cmd, "set ", 4) == 0) {
		int num = atoi(cmd + 4);
		if (Garland_SetMode((uint8_t) num)) {
			UART_SendString("\r\nOK\r\n");
		} else {
			UART_SendString("\r\nError: invalid mode number\r\n");
		}
	} else {
		UART_SendString("\r\nError: unknown command\r\n");
	}
}

static void ProcessPeriod(const char *input) {
	uint16_t delay = 0;

	if (strcmp(input, "1") == 0 || strcmp(input, "200") == 0) {
		delay = 200;
	} else if (strcmp(input, "2") == 0 || strcmp(input, "500") == 0) {
		delay = 500;
	} else if (strcmp(input, "3") == 0 || strcmp(input, "1000") == 0) {
		delay = 1000;
	}

	s_wait_period = false;

	if (delay > 0) {
		uint8_t mode_num = Garland_AddMode(s_new_seq, delay);
		char msg[32];
		snprintf(msg, sizeof(msg), "\r\nSaved as mode %u\r\n", mode_num);
		UART_SendString(msg);
	} else {
		UART_SendString("\r\nError: invalid period\r\n");
	}
}

void CLI_Init(void) {
	s_pos = 0;
	s_wait_period = false;
}

void CLI_Update(void) {
	uint8_t ch;
	while (UART_ReadByte(&ch)) {
		if (ch == '\r' || ch == '\n') {
			if (s_pos > 0) {
				s_cmd_buf[s_pos] = '\0';
				if (s_wait_period) {
					ProcessPeriod(s_cmd_buf);
				} else {
					ProcessCommand(s_cmd_buf);
				}
				s_pos = 0;
			} else {
				UART_SendString("\r\n");
			}
		} else if (ch == '\b' || ch == 0x7F) {
			if (s_pos > 0) {
				s_pos--;
				UART_SendString("\b \b");
			}
		} else if (ch >= ' ' && ch <= '~') {
			if (s_pos < sizeof(s_cmd_buf) - 1) {
				s_cmd_buf[s_pos++] = (char) ch;
				UART_SendByte(ch); // эхо
			}
		}
	}
}
