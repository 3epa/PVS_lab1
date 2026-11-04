#include "leds.h"
#include "main.h"

#define LED_PORT        GPIOD
#define PIN_GREEN       GPIO_PIN_13
#define PIN_YELLOW      GPIO_PIN_14
#define PIN_RED         GPIO_PIN_15

// Текущее состояние светодиодов
static led_state_t s_states[LED_COUNT] = { LED_OFF, LED_OFF, LED_OFF };

static void LED_Update(void) {
	// Зеленый на PD13
	if (s_states[LED_GREEN] == LED_ON) {
		HAL_GPIO_WritePin(LED_PORT, PIN_GREEN, GPIO_PIN_SET);
	} else {
		HAL_GPIO_WritePin(LED_PORT, PIN_GREEN, GPIO_PIN_RESET);
	}

	// Двухцветный светодиод на PD14 и PD15 (встречно-параллельный)
	if (s_states[LED_YELLOW] == LED_ON && s_states[LED_RED] == LED_OFF) {
		HAL_GPIO_WritePin(LED_PORT, PIN_YELLOW, GPIO_PIN_SET);
		HAL_GPIO_WritePin(LED_PORT, PIN_RED, GPIO_PIN_RESET);
	} else if (s_states[LED_RED] == LED_ON && s_states[LED_YELLOW] == LED_OFF) {
		HAL_GPIO_WritePin(LED_PORT, PIN_YELLOW, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(LED_PORT, PIN_RED, GPIO_PIN_SET);
	} else {
		// Оба выключены
		HAL_GPIO_WritePin(LED_PORT, PIN_YELLOW | PIN_RED, GPIO_PIN_RESET);
	}
}

void LED_Init(void) {
	GPIO_InitTypeDef gpio = { 0 };

	__HAL_RCC_GPIOD_CLK_ENABLE();

	HAL_GPIO_WritePin(LED_PORT, PIN_GREEN | PIN_YELLOW | PIN_RED,
			GPIO_PIN_RESET);

	gpio.Pin = PIN_GREEN | PIN_YELLOW | PIN_RED;
	gpio.Mode = GPIO_MODE_OUTPUT_PP;
	gpio.Pull = GPIO_NOPULL;
	gpio.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(LED_PORT, &gpio);

	LED_AllOff();
}

void LED_Set(led_color_t led, led_state_t state) {
	if (led < LED_COUNT) {
		s_states[led] = state;
		LED_Update();
	}
}

void LED_Toggle(led_color_t led) {
	if (led < LED_COUNT) {
		s_states[led] = (s_states[led] == LED_ON) ? LED_OFF : LED_ON;
		LED_Update();
	}
}

void LED_SetAll(led_state_t green, led_state_t yellow, led_state_t red) {
	s_states[LED_GREEN] = green;
	s_states[LED_YELLOW] = yellow;
	s_states[LED_RED] = red;
	LED_Update();
}

void LED_AllOff(void) {
	LED_SetAll(LED_OFF, LED_OFF, LED_OFF);
}
