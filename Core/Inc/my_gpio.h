#ifndef INC_MY_GPIO_H_
#define INC_MY_GPIO_H_

#include <stdint.h>
#include "stm32f4xx.h"

typedef enum {
    MY_GPIO_MODE_INPUT  = 0x00,
    MY_GPIO_MODE_OUTPUT = 0x01,
    MY_GPIO_MODE_AF     = 0x02,
    MY_GPIO_MODE_ANALOG = 0x03
} my_gpio_mode_t;

typedef enum {
    MY_GPIO_OTYPE_PP = 0x00,
    MY_GPIO_OTYPE_OD = 0x01
} my_gpio_otype_t;

typedef enum {
    MY_GPIO_SPEED_LOW    = 0x00,
    MY_GPIO_SPEED_MEDIUM = 0x01,
    MY_GPIO_SPEED_FAST   = 0x02,
    MY_GPIO_SPEED_HIGH   = 0x03
} my_gpio_speed_t;

typedef enum {
    MY_GPIO_PULL_NONE = 0x00,
    MY_GPIO_PULL_UP   = 0x01,
    MY_GPIO_PULL_DOWN = 0x02
} my_gpio_pull_t;

typedef enum {
    MY_GPIO_RESET = 0,
    MY_GPIO_SET   = 1
} my_gpio_state_t;

typedef struct {
    uint16_t pin_mask;
    my_gpio_mode_t mode;
    my_gpio_otype_t otype;
    my_gpio_speed_t speed;
    my_gpio_pull_t pull;
    my_gpio_state_t init_state; // начальный уровень при настройке на выход
} my_gpio_init_t;

typedef struct {
    my_gpio_mode_t mode;
    my_gpio_otype_t otype;
    my_gpio_speed_t speed;
    my_gpio_pull_t pull;
} my_gpio_config_t;

void my_gpio_init(GPIO_TypeDef *port, const my_gpio_init_t *cfg);
void my_gpio_get_config(GPIO_TypeDef *port, uint8_t pin_num, my_gpio_config_t *cfg);

my_gpio_state_t my_gpio_read_pin(GPIO_TypeDef *port, uint16_t pin_mask);
uint16_t my_gpio_read_port(GPIO_TypeDef *port);

void my_gpio_write_pin(GPIO_TypeDef *port, uint16_t pin_mask, my_gpio_state_t state);
void my_gpio_toggle_pin(GPIO_TypeDef *port, uint16_t pin_mask);
void my_gpio_write_mask(GPIO_TypeDef *port, uint16_t set_mask, uint16_t reset_mask);

#endif /* INC_MY_GPIO_H_ */
