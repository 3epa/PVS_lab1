#ifndef INC_GARLAND_H_
#define INC_GARLAND_H_

#include <stdint.h>

#define GARLAND_MODE_COUNT  4

/**
 * @brief Инициализация гирлянды (запуск нулевого режима)
 */
void Garland_Init(void);

/**
 * @brief Неблокирующее обновление анимации гирлянды
 *        Вызывается непрерывно в главном цикле while(1)
 */
void Garland_Update(void);

/**
 * @brief Переключение на следующий режим (0 -> 1 -> 2 -> 3 -> 0)
 *        Вызывается по факту нажатия кнопки
 */
void Garland_NextMode(void);

/**
 * @brief Получить номер текущего активного режима (0..3)
 */
uint8_t Garland_GetCurrentMode(void);

#endif /* INC_GARLAND_H_ */
