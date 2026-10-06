#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief События кнопки
 */
typedef enum {
    BUTTON_EVENT_NONE = 0, // Нет событий
    BUTTON_EVENT_CLICK     // Зафиксировано одиночное нажатие
} button_event_t;

/**
 * @brief Инициализация GPIO контакта кнопки (PC15)
 */
void Button_Init(void);

/**
 * @brief Неблокирующая функция обновления состояния кнопки
 *        Вызывается периодически в главном цикле while(1)
 * @return button_event_t: возвращает BUTTON_EVENT_CLICK ровно один раз за нажатие
 */
button_event_t Button_Update(void);

#endif /* INC_BUTTON_H_ */
