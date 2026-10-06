#include "button.h"
#include "main.h"

/* Конфигурация пина кнопки на стенде SDK-1.1M */
#define BUTTON_GPIO_PORT         GPIOC
#define BUTTON_PIN               GPIO_PIN_15

/* Время подавления дребезга (мс) */
#define BUTTON_DEBOUNCE_MS       50

/**
 * @brief Состояния конечного автомата кнопки
 */
typedef enum {
    BTN_STATE_IDLE = 0,         // Кнопка отпущена, покой
    BTN_STATE_DEBOUNCE_PRESS,   // Зафиксирован спад напряжения, ждем стабилизации
    BTN_STATE_PRESSED,          // Кнопка уверенно нажата (удерживается)
    BTN_STATE_DEBOUNCE_RELEASE  // Зафиксирован подъем напряжения, ждем отпускания
} btn_fsm_state_t;

static btn_fsm_state_t s_fsm_state = BTN_STATE_IDLE;
static uint32_t s_state_start_time = 0;

void Button_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* Включаем тактирование порта GPIOC */
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* Настройка PC15 как вход с внутренней подтяжкой к питанию */
    GPIO_InitStruct.Pin = BUTTON_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(BUTTON_GPIO_PORT, &GPIO_InitStruct);

    s_fsm_state = BTN_STATE_IDLE;
}

button_event_t Button_Update(void)
{
    button_event_t event = BUTTON_EVENT_NONE;
    uint32_t current_time = HAL_GetTick();

    /* Читаем физический уровень: нажата = лог. 0 (GPIO_PIN_RESET) */
    bool is_physically_pressed = (HAL_GPIO_ReadPin(BUTTON_GPIO_PORT, BUTTON_PIN) == GPIO_PIN_RESET);

    switch (s_fsm_state)
    {
        case BTN_STATE_IDLE:
            if (is_physically_pressed) {
                // Зафиксирован переход из 1 в 0, засекаем время для антидребезга
                s_state_start_time = current_time;
                s_fsm_state = BTN_STATE_DEBOUNCE_PRESS;
            }
            break;

        case BTN_STATE_DEBOUNCE_PRESS:
            // Прошло ли необходимое время фильтрации дребезга?
            if (current_time - s_state_start_time >= BUTTON_DEBOUNCE_MS) {
                if (is_physically_pressed) {
                    // Контакт стабильно замкнут: фиксируем клик!
                    s_fsm_state = BTN_STATE_PRESSED;
                    event = BUTTON_EVENT_CLICK;
                } else {
                    // Ложное срабатывание (помеха)
                    s_fsm_state = BTN_STATE_IDLE;
                }
            }
            break;

        case BTN_STATE_PRESSED:
            // Кнопка удерживается. Ждем начала отпускания (уровень вернулся в 1)
            if (!is_physically_pressed) {
                s_state_start_time = current_time;
                s_fsm_state = BTN_STATE_DEBOUNCE_RELEASE;
            }
            break;

        case BTN_STATE_DEBOUNCE_RELEASE:
            // Фильтруем дребезг при размыкании контактов
            if (current_time - s_state_start_time >= BUTTON_DEBOUNCE_MS) {
                if (!is_physically_pressed) {
                    // Кнопка окончательно отпущена, готовы к следующему нажатию
                    s_fsm_state = BTN_STATE_IDLE;
                } else {
                    // Еще не отпустили до конца
                    s_fsm_state = BTN_STATE_PRESSED;
                }
            }
            break;
    }

    return event;
}
