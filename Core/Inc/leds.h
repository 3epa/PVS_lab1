#ifndef INC_LEDS_H_
#define INC_LEDS_H_

#include <stdint.h>

/**
 * @brief Доступные цвета светодиодов на боковой панели SDK-1.1M
 */
typedef enum {
    LED_GREEN = 0,
    LED_YELLOW,
    LED_RED,
    LED_COUNT
} led_color_t;

/**
 * @brief Состояние светодиода
 */
typedef enum {
    LED_OFF = 0,
    LED_ON  = 1
} led_state_t;

/**
 * @brief Инициализация портов GPIO для светодиодов
 */
void LED_Init(void);

/**
 * @brief Установка состояния отдельного светодиода
 * @param led: цвет светодиода (LED_GREEN, LED_YELLOW, LED_RED)
 * @param state: состояние (LED_OFF, LED_ON)
 */
void LED_Set(led_color_t led, led_state_t state);

/**
 * @brief Инвертировать состояние отдельного светодиода
 * @param led: цвет светодиода
 */
void LED_Toggle(led_color_t led);

/**
 * @brief Управление всеми светодиодами одновременно
 */
void LED_SetAll(led_state_t green, led_state_t yellow, led_state_t red);

/**
 * @brief Выключить все светодиоды
 */
void LED_AllOff(void);

#endif /* INC_LEDS_H_ */
