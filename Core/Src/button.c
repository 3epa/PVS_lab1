#include "button.h"
#include "my_gpio.h"
#include "main.h" // Нужен только для счетчика времени HAL_GetTick()

#define BTN_PORT        GPIOC
#define BTN_PIN         (1U << 15)
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
    // Включение тактирования GPIOC напрямую в регистрах RCC
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;

    my_gpio_init_t cfg = {
        .pin_mask   = BTN_PIN,
        .mode       = MY_GPIO_MODE_INPUT,
        .otype      = MY_GPIO_OTYPE_PP,
        .speed      = MY_GPIO_SPEED_LOW,
        .pull       = MY_GPIO_PULL_UP, // Подтяжка к питанию (нажата = 0)
        .init_state = MY_GPIO_RESET
    };

    my_gpio_init(BTN_PORT, &cfg);
    s_state = STATE_IDLE;
}

button_event_t Button_Update(void)
{
    button_event_t event = BUTTON_EVENT_NONE;
    uint32_t now = HAL_GetTick();

    // Читаем входной регистр IDR через наш драйвер
    bool pressed = (my_gpio_read_pin(BTN_PORT, BTN_PIN) == MY_GPIO_RESET);

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
