#include "my_gpio.h"

void my_gpio_init(GPIO_TypeDef *port, const my_gpio_init_t *cfg)
{
    if (port == 0 || cfg == 0) return;

    for (uint8_t i = 0; i < 16; i++) {
        if (!(cfg->pin_mask & (1U << i))) {
            continue; // пин не выбран в маске
        }

        uint32_t pos2 = i * 2; // смещение для 2-битных регистров (MODER, OSPEEDR, PUPDR)

        // 1. ЗАЩИТА ОТ ВЫБРОСОВ (Glitches):
        // Если настраиваем пин на выход, предварительно выставляем требуемый
        // уровень в выходную защелку (BSRR) ДО того, как активируем режим выхода в MODER.
        if (cfg->mode == MY_GPIO_MODE_OUTPUT) {
            if (cfg->init_state == MY_GPIO_SET) {
                port->BSRR = (1U << i);          // Предварительный SET
            } else {
                port->BSRR = (1U << (i + 16));   // Предварительный RESET
            }
        }

        // 2. Настройка типа выхода (OTYPER: 1 бит на пин)
        port->OTYPER &= ~(1U << i);
        port->OTYPER |= ((uint32_t)cfg->otype << i);

        // 3. Настройка скорости нарастания фронта (OSPEEDR: 2 бита на пин)
        port->OSPEEDR &= ~(0x3U << pos2);
        port->OSPEEDR |= ((uint32_t)cfg->speed << pos2);

        // 4. Настройка внутренней подтяжки (PUPDR: 2 бита на пин)
        port->PUPDR &= ~(0x3U << pos2);
        port->PUPDR |= ((uint32_t)cfg->pull << pos2);

        // 5. Включение режима работы (MODER: 2 бита на пин)
        // Выполняется последним шагом, когда остальные параметры и защелка уже готовы
        port->MODER &= ~(0x3U << pos2);
        port->MODER |= ((uint32_t)cfg->mode << pos2);
    }
}

void my_gpio_get_config(GPIO_TypeDef *port, uint8_t pin_num, my_gpio_config_t *cfg)
{
    if (port == 0 || cfg == 0 || pin_num > 15) return;

    uint32_t pos2 = pin_num * 2;

    cfg->mode  = (my_gpio_mode_t)((port->MODER   >> pos2) & 0x03U);
    cfg->otype = (my_gpio_otype_t)((port->OTYPER  >> pin_num) & 0x01U);
    cfg->speed = (my_gpio_speed_t)((port->OSPEEDR >> pos2) & 0x03U);
    cfg->pull  = (my_gpio_pull_t)((port->PUPDR   >> pos2) & 0x03U);
}

my_gpio_state_t my_gpio_read_pin(GPIO_TypeDef *port, uint16_t pin_mask)
{
    // Чтение регистра физических уровней на входе (IDR)
    if ((port->IDR & pin_mask) != 0) {
        return MY_GPIO_SET;
    }
    return MY_GPIO_RESET;
}

uint16_t my_gpio_read_port(GPIO_TypeDef *port)
{
    return (uint16_t)(port->IDR & 0xFFFFU);
}

void my_gpio_write_pin(GPIO_TypeDef *port, uint16_t pin_mask, my_gpio_state_t state)
{
    // Атомарная запись в BSRR без необходимости читать ODR
    if (state == MY_GPIO_SET) {
        port->BSRR = (uint32_t)pin_mask;          // Младшие 16 бит = SET
    } else {
        port->BSRR = ((uint32_t)pin_mask << 16U); // Старшие 16 бит = RESET
    }
}

void my_gpio_toggle_pin(GPIO_TypeDef *port, uint16_t pin_mask)
{
    // Считываем текущее состояние выходов и инвертируем его через атомарный BSRR
    uint32_t cur = port->ODR;
    port->BSRR = ((cur & pin_mask) << 16U) | (~cur & pin_mask);
}

void my_gpio_write_mask(GPIO_TypeDef *port, uint16_t set_mask, uint16_t reset_mask)
{
    // Одновременная установка одних пинов и сброс других в один такт процессора
    port->BSRR = ((uint32_t)reset_mask << 16U) | (uint32_t)set_mask;
}
