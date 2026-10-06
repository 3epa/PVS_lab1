#include "button.h"
#include "main.h"

#define BTN_PORT        GPIOC
#define BTN_PIN         GPIO_PIN_15
#define DEBOUNCE_MS     50

typedef enum {
    STATE_IDLE = 0,
    STATE_DEBOUNCE_PRESS,
    STATE_PRESSED,
    STATE_DEBOUNCE_RELEASE
} btn_state_t;

static btn_state_t s_state = STATE_IDLE;
static uint32_t s_timer = 0;

void Button_Init(void)
{
    GPIO_InitTypeDef gpio = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();

    gpio.Pin = BTN_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BTN_PORT, &gpio);

    s_state = STATE_IDLE;
}

button_event_t Button_Update(void)
{
    button_event_t event = BUTTON_EVENT_NONE;
    uint32_t now = HAL_GetTick();

    // Кнопка замыкает на землю (активный 0)
    bool pressed = (HAL_GPIO_ReadPin(BTN_PORT, BTN_PIN) == GPIO_PIN_RESET);

    switch (s_state)
    {
        case STATE_IDLE:
            if (pressed) {
                s_timer = now;
                s_state = STATE_DEBOUNCE_PRESS;
            }
            break;

        case STATE_DEBOUNCE_PRESS:
            if (now - s_timer >= DEBOUNCE_MS) {
                if (pressed) {
                    s_state = STATE_PRESSED;
                    event = BUTTON_EVENT_CLICK;
                } else {
                    s_state = STATE_IDLE;
                }
            }
            break;

        case STATE_PRESSED:
            if (!pressed) {
                s_timer = now;
                s_state = STATE_DEBOUNCE_RELEASE;
            }
            break;

        case STATE_DEBOUNCE_RELEASE:
            if (now - s_timer >= DEBOUNCE_MS) {
                if (!pressed) {
                    s_state = STATE_IDLE;
                } else {
                    s_state = STATE_PRESSED;
                }
            }
            break;
    }

    return event;
}
