#ifndef INC_LEDS_H_
#define INC_LEDS_H_

#include <stdint.h>

typedef enum {
    LED_GREEN = 0,
    LED_YELLOW,
    LED_RED,
    LED_COUNT
} led_color_t;

typedef enum {
    LED_OFF = 0,
    LED_ON  = 1
} led_state_t;

void LED_Init(void);
void LED_Set(led_color_t led, led_state_t state);
void LED_Toggle(led_color_t led);
void LED_SetAll(led_state_t green, led_state_t yellow, led_state_t red);
void LED_AllOff(void);

#endif /* INC_LEDS_H_ */
