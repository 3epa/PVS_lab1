#include "garland.h"
#include "leds.h"
#include "main.h"
#include <string.h>

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

// Режим 1: бегущий огонь
static const step_t M0[] = { { 1, 0, 0, 250 }, { 0, 1, 0, 250 },
		{ 0, 0, 1, 250 }, { 0, 1, 0, 250 } };

// Режим 2: стробоскоп
static const step_t M1[] = { { 1, 0, 1, 70 }, { 0, 0, 0, 70 }, { 1, 0, 1, 70 },
		{ 0, 0, 0, 180 }, { 0, 1, 0, 70 }, { 0, 0, 0, 70 }, { 0, 1, 0, 70 }, {
				0, 0, 0, 250 } };

// Режим 3: по парам
static const step_t M2[] = { { 1, 1, 0, 500 }, { 0, 0, 0, 200 },
		{ 1, 0, 1, 500 }, { 0, 0, 0, 200 } };

// Режим 4: накопление
static const step_t M3[] = { { 1, 0, 0, 350 }, { 1, 1, 0, 350 },
		{ 0, 0, 0, 250 }, { 1, 0, 0, 350 }, { 1, 0, 1, 350 }, { 0, 0, 0, 250 } };

// Память под 4 пользовательских режима (номера 5..8), до 8 шагов в каждом
static step_t s_user_steps[4][8];

static mode_t s_modes[GARLAND_MAX_MODES] = { { M0, sizeof(M0) / sizeof(step_t),
		0 }, { M1, sizeof(M1) / sizeof(step_t), 0 }, { M2, sizeof(M2)
		/ sizeof(step_t), 0 }, { M3, sizeof(M3) / sizeof(step_t), 0 }, {
		s_user_steps[0], 0, 0 }, { s_user_steps[1], 0, 0 }, { s_user_steps[2],
		0, 0 }, { s_user_steps[3], 0, 0 } };

static uint8_t s_cur_mode = 0;
static uint8_t s_modes_count = GARLAND_BASE_MODES;
static uint8_t s_next_user_slot = 0; // 0..3 (соответствует режимам 5..8)
static uint32_t s_last_tick = 0;

static void ApplyStep(void) {
	mode_t *m = &s_modes[s_cur_mode];
	const step_t *s = &m->steps[m->cur_step];

	LED_SetAll(s->g ? LED_ON : LED_OFF, s->y ? LED_ON : LED_OFF,
			s->r ? LED_ON : LED_OFF);
}

void Garland_Init(void) {
	s_cur_mode = 0;
	s_modes_count = GARLAND_BASE_MODES;
	s_next_user_slot = 0;
	s_last_tick = HAL_GetTick();
	ApplyStep();
}

void Garland_Update(void) {
	mode_t *m = &s_modes[s_cur_mode];
	const step_t *s = &m->steps[m->cur_step];
	uint32_t now = HAL_GetTick();

	if (now - s_last_tick >= s->delay) {
		m->cur_step = (m->cur_step + 1) % m->count;
		s_last_tick = now;
		ApplyStep();
	}
}

void Garland_NextMode(void) {
	s_cur_mode = (s_cur_mode + 1) % s_modes_count;
	s_last_tick = HAL_GetTick();
	ApplyStep();
}

bool Garland_SetMode(uint8_t mode_num) {
	if (mode_num < 1 || mode_num > s_modes_count) {
		return false;
	}

	s_cur_mode = mode_num - 1;
	s_last_tick = HAL_GetTick();
	ApplyStep();
	return true;
}

uint8_t Garland_GetMode(void) {
	return s_cur_mode + 1;
}

bool Garland_CheckPattern(const char *seq) {
	size_t len = strlen(seq);
	if (len < 2 || len > 8) {
		return false;
	}

	for (size_t i = 0; i < len; i++) {
		if (seq[i] != 'g' && seq[i] != 'r' && seq[i] != 'y' && seq[i] != 'n') {
			return false;
		}
	}
	return true;
}

uint8_t Garland_AddMode(const char *seq, uint16_t delay_ms) {
	uint8_t slot = s_next_user_slot;
	uint8_t idx = GARLAND_BASE_MODES + slot;
	size_t len = strlen(seq);

	for (size_t i = 0; i < len; i++) {
		s_user_steps[slot][i].g = (seq[i] == 'g') ? 1 : 0;
		s_user_steps[slot][i].y = (seq[i] == 'y') ? 1 : 0;
		s_user_steps[slot][i].r = (seq[i] == 'r') ? 1 : 0;
		s_user_steps[slot][i].delay = delay_ms;
	}

	s_modes[idx].count = (uint8_t) len;
	s_modes[idx].cur_step = 0;

	if (s_modes_count < GARLAND_MAX_MODES) {
		s_modes_count++;
	}

	s_next_user_slot = (s_next_user_slot + 1) % 4;

	if (s_cur_mode == idx) {
		s_last_tick = HAL_GetTick();
		ApplyStep();
	}

	return idx + 1; // возвращаем номер 5..8
}
