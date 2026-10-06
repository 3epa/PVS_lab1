#include "garland.h"
#include "leds.h"
#include "main.h"

typedef struct {
    uint8_t g;
    uint8_t y;
    uint8_t r;
    uint16_t delay;
} step_t;

typedef struct {
    const step_t *steps;
    uint8_t count;
    uint8_t cur_step; // сохраняем шаг при переключении
} mode_t;

// Режим 0: бегущий огонь
static const step_t M0[] = {
    {1, 0, 0, 250},
    {0, 1, 0, 250},
    {0, 0, 1, 250},
    {0, 1, 0, 250}
};

// Режим 1: стробоскоп
static const step_t M1[] = {
    {1, 0, 1, 70},
    {0, 0, 0, 70},
    {1, 0, 1, 70},
    {0, 0, 0, 180},
    {0, 1, 0, 70},
    {0, 0, 0, 70},
    {0, 1, 0, 70},
    {0, 0, 0, 250}
};

// Режим 2: по парам
static const step_t M2[] = {
    {1, 1, 0, 500},
    {0, 0, 0, 200},
    {1, 0, 1, 500},
    {0, 0, 0, 200}
};

// Режим 3: накопление
static const step_t M3[] = {
    {1, 0, 0, 350},
    {1, 1, 0, 350},
    {0, 0, 0, 250},
    {1, 0, 0, 350},
    {1, 0, 1, 350},
    {0, 0, 0, 250}
};

static mode_t s_modes[GARLAND_MODES_COUNT] = {
    {M0, sizeof(M0) / sizeof(step_t), 0},
    {M1, sizeof(M1) / sizeof(step_t), 0},
    {M2, sizeof(M2) / sizeof(step_t), 0},
    {M3, sizeof(M3) / sizeof(step_t), 0}
};

static uint8_t  s_cur_mode = 0;
static uint32_t s_last_tick = 0;

static void ApplyStep(void)
{
    mode_t *m = &s_modes[s_cur_mode];
    const step_t *s = &m->steps[m->cur_step];

    LED_SetAll(s->g ? LED_ON : LED_OFF,
               s->y ? LED_ON : LED_OFF,
               s->r ? LED_ON : LED_OFF);
}

void Garland_Init(void)
{
    s_cur_mode = 0;
    s_last_tick = HAL_GetTick();
    ApplyStep();
}

void Garland_Update(void)
{
    mode_t *m = &s_modes[s_cur_mode];
    const step_t *s = &m->steps[m->cur_step];
    uint32_t now = HAL_GetTick();

    if (now - s_last_tick >= s->delay) {
        m->cur_step = (m->cur_step + 1) % m->count;
        s_last_tick = now;
        ApplyStep();
    }
}

void Garland_NextMode(void)
{
    // Шаг прошлого режима остается в s_modes[s_cur_mode].cur_step
    s_cur_mode = (s_cur_mode + 1) % GARLAND_MODES_COUNT;
    s_last_tick = HAL_GetTick();
    ApplyStep();
}

uint8_t Garland_GetMode(void)
{
    return s_cur_mode;
}
