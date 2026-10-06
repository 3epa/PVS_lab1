#include "garland.h"
#include "leds.h"
#include "main.h"

/**
 * @brief Структура одного кадра (шага) анимации
 */
typedef struct {
    uint8_t green;        // 1 - включен, 0 - выключен
    uint8_t yellow;
    uint8_t red;
    uint16_t duration_ms; // Длительность шага в миллисекундах
} garland_step_t;

/**
 * @brief Описание режима гирлянды
 */
typedef struct {
    const garland_step_t *steps; // Массив шагов
    uint8_t step_count;          // Количество шагов
    uint8_t current_step;        // Текущий шаг (сохраняется при смене режима!)
} garland_mode_t;

/* ================== РЕЖИМ 0: Бегущий огонь (250 мс) ================== */
static const garland_step_t MODE_0_STEPS[] = {
    {1, 0, 0, 250}, // Зеленый
    {0, 1, 0, 250}, // Желтый
    {0, 0, 1, 250}, // Красный
    {0, 1, 0, 250}  // Желтый
};

/* ================== РЕЖИМ 1: Стробоскоп (быстрый, 70 мс) ================== */
static const garland_step_t MODE_1_STEPS[] = {
    {1, 0, 1, 70},  // Зеленый + Красный
    {0, 0, 0, 70},  // Пауза
    {1, 0, 1, 70},  // Зеленый + Красный
    {0, 0, 0, 180}, // Пауза
    {0, 1, 0, 70},  // Желтый
    {0, 0, 0, 70},  // Пауза
    {0, 1, 0, 70},  // Желтый
    {0, 0, 0, 250}  // Длинная пауза
};

/* ================== РЕЖИМ 2: Парные шаги (медленный, 500 мс) ================== */
static const garland_step_t MODE_2_STEPS[] = {
    {1, 1, 0, 500}, // Зеленый + Желтый
    {0, 0, 0, 200}, // Пауза
    {1, 0, 1, 500}, // Зеленый + Красный
    {0, 0, 0, 200}  // Пауза
};

/* ================== РЕЖИМ 3: Накопление (350 мс) ================== */
static const garland_step_t MODE_3_STEPS[] = {
    {1, 0, 0, 350}, // Зеленый
    {1, 1, 0, 350}, // Зеленый + Желтый
    {0, 0, 0, 250}, // Пауза
    {1, 0, 0, 350}, // Зеленый
    {1, 0, 1, 350}, // Зеленый + Красный
    {0, 0, 0, 250}  // Пауза
};

/* Массив всех 4 режимов */
static garland_mode_t s_modes[GARLAND_MODE_COUNT] = {
    {MODE_0_STEPS, sizeof(MODE_0_STEPS) / sizeof(garland_step_t), 0},
    {MODE_1_STEPS, sizeof(MODE_1_STEPS) / sizeof(garland_step_t), 0},
    {MODE_2_STEPS, sizeof(MODE_2_STEPS) / sizeof(garland_step_t), 0},
    {MODE_3_STEPS, sizeof(MODE_3_STEPS) / sizeof(garland_step_t), 0}
};

static uint8_t  s_active_mode_idx = 0;
static uint32_t s_step_start_time = 0;

/**
 * @brief Применить состояние текущего шага к светодиодам
 */
static void Garland_ApplyCurrentStep(void)
{
    garland_mode_t *mode = &s_modes[s_active_mode_idx];
    const garland_step_t *step = &mode->steps[mode->current_step];

    LED_SetAll(step->green  ? LED_ON : LED_OFF,
               step->yellow ? LED_ON : LED_OFF,
               step->red    ? LED_ON : LED_OFF);
}

void Garland_Init(void)
{
    s_active_mode_idx = 0;
    s_step_start_time = HAL_GetTick();
    Garland_ApplyCurrentStep();
}

void Garland_Update(void)
{
    garland_mode_t *mode = &s_modes[s_active_mode_idx];
    const garland_step_t *step = &mode->steps[mode->current_step];
    uint32_t current_time = HAL_GetTick();

    /* Проверяем, истекло ли время текущего кадра */
    if (current_time - s_step_start_time >= step->duration_ms)
    {
        /* Переходим к следующему шагу внутри текущего режима */
        mode->current_step = (mode->current_step + 1) % mode->step_count;
        s_step_start_time = current_time;

        /* Выводим новый кадр на светодиоды */
        Garland_ApplyCurrentStep();
    }
}

void Garland_NextMode(void)
{
    /*
     * ВАЖНО: Мы НЕ сбрасываем s_modes[s_active_mode_idx].current_step!
     * Он остается сохраненным.
     */
    s_active_mode_idx = (s_active_mode_idx + 1) % GARLAND_MODE_COUNT;

    /* Сразу отображаем сохраненный кадр нового режима и сбрасываем таймер */
    s_step_start_time = HAL_GetTick();
    Garland_ApplyCurrentStep();
}

uint8_t Garland_GetCurrentMode(void)
{
    return s_active_mode_idx;
}
