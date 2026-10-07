#include "leds.h"
#include "my_gpio.h"

#define LED_PORT        GPIOD
#define PIN_GREEN       (1U << 13)
#define PIN_YELLOW      (1U << 14)
#define PIN_RED         (1U << 15)

static led_state_t s_states[LED_COUNT] = {LED_OFF, LED_OFF, LED_OFF};

static void LED_Update(void)
{
    // 1. Зеленый светодиод на PD13
    my_gpio_write_pin(LED_PORT, PIN_GREEN,
                      (s_states[LED_GREEN] == LED_ON) ? MY_GPIO_SET : MY_GPIO_RESET);

    // 2. Двухцветный встречно-параллельный светодиод (PD14 / PD15)
    if (s_states[LED_YELLOW] == LED_ON && s_states[LED_RED] == LED_OFF) {
        // Зажигаем желтый: PD14 = 1, PD15 = 0 в одну атомарную операцию
        my_gpio_write_mask(LED_PORT, PIN_YELLOW, PIN_RED);
    }
    else if (s_states[LED_RED] == LED_ON && s_states[LED_YELLOW] == LED_OFF) {
        // Зажигаем красный: PD15 = 1, PD14 = 0 в одну атомарную операцию
        my_gpio_write_mask(LED_PORT, PIN_RED, PIN_YELLOW);
    }
    else {
        // Оба выключены
        my_gpio_write_pin(LED_PORT, PIN_YELLOW | PIN_RED, MY_GPIO_RESET);
    }
}

void LED_Init(void)
{
    // Прямое включение тактирования GPIOD в шине AHB1
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;

    my_gpio_init_t cfg = {
        .pin_mask   = PIN_GREEN | PIN_YELLOW | PIN_RED,
        .mode       = MY_GPIO_MODE_OUTPUT,
        .otype      = MY_GPIO_OTYPE_PP,
        .speed      = MY_GPIO_SPEED_LOW,
        .pull       = MY_GPIO_PULL_NONE,
        .init_state = MY_GPIO_RESET // Защита от выброса: защелка сразу в 0
    };

    my_gpio_init(LED_PORT, &cfg);
    LED_AllOff();
}

void LED_Set(led_color_t led, led_state_t state)
{
    if (led < LED_COUNT) {
        s_states[led] = state;
        LED_Update();
    }
}

void LED_Toggle(led_color_t led)
{
    if (led < LED_COUNT) {
        s_states[led] = (s_states[led] == LED_ON) ? LED_OFF : LED_ON;
        LED_Update();
    }
}

void LED_SetAll(led_state_t green, led_state_t yellow, led_state_t red)
{
    s_states[LED_GREEN]  = green;
    s_states[LED_YELLOW] = yellow;
    s_states[LED_RED]    = red;
    LED_Update();
}

void LED_AllOff(void)
{
    LED_SetAll(LED_OFF, LED_OFF, LED_OFF);
}
