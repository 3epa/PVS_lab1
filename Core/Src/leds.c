#include "leds.h"
#include "main.h"

/* Конфигурация пинов стенда */
#define LED_GPIO_PORT       GPIOD
#define LED_GREEN_PIN       GPIO_PIN_13
#define LED_YELLOW_PIN      GPIO_PIN_14
#define LED_RED_PIN         GPIO_PIN_15

/* Хранилище текущего состояния каждого светодиода */
static led_state_t s_led_states[LED_COUNT] = {LED_OFF, LED_OFF, LED_OFF};

/**
 * @brief Внутренняя функция обновления физических уровней на пинах
 */
static void LED_UpdateHardware(void)
{
    /* 1. Управление зеленым светодиодом */
    if (s_led_states[LED_GREEN] == LED_ON) {
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GREEN_PIN, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
    }

    /* 2. Управление встречно-параллельным красно-желтым светодиодом */
    if (s_led_states[LED_YELLOW] == LED_ON && s_led_states[LED_RED] == LED_OFF) {
        // Ток течет от PD14 к PD15 -> горит желтый
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_YELLOW_PIN, GPIO_PIN_SET);
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_RED_PIN, GPIO_PIN_RESET);
    }
    else if (s_led_states[LED_RED] == LED_ON && s_led_states[LED_YELLOW] == LED_OFF) {
        // Ток течет от PD15 к PD14 -> горит красный
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_YELLOW_PIN, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_RED_PIN, GPIO_PIN_SET);
    }
    else {
        // Если оба выключены (или одновременно включены в один момент времени)
        HAL_GPIO_WritePin(LED_GPIO_PORT, LED_YELLOW_PIN | LED_RED_PIN, GPIO_PIN_RESET);
    }
}

void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Тактирование порта D */
    __HAL_RCC_GPIOD_CLK_ENABLE();

    /* Сброс всех пинов в 0 */
    	HAL_GPIO_WritePin(LED_GPIO_PORT, LED_GREEN_PIN | LED_YELLOW_PIN | LED_RED_PIN, GPIO_PIN_RESET);

    /* Конфигурация пинов на двухтактный выход */
    GPIO_InitStruct.Pin = LED_GREEN_PIN | LED_YELLOW_PIN | LED_RED_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(LED_GPIO_PORT, &GPIO_InitStruct);

    LED_AllOff();
}

void LED_Set(led_color_t led, led_state_t state)
{
    if (led < LED_COUNT) {
        s_led_states[led] = state;
        LED_UpdateHardware();
    }
}

void LED_Toggle(led_color_t led)
{
    if (led < LED_COUNT) {
        s_led_states[led] = (s_led_states[led] == LED_ON) ? LED_OFF : LED_ON;
        LED_UpdateHardware();
    }
}

void LED_SetAll(led_state_t green, led_state_t yellow, led_state_t red)
{
    s_led_states[LED_GREEN]  = green;
    s_led_states[LED_YELLOW] = yellow;
    s_led_states[LED_RED]    = red;
    LED_UpdateHardware();
}

void LED_AllOff(void)
{
    LED_SetAll(LED_OFF, LED_OFF, LED_OFF);
}
