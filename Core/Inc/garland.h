#ifndef INC_GARLAND_H_
#define INC_GARLAND_H_

#include <stdint.h>
#include <stdbool.h>

#define GARLAND_BASE_MODES  4
#define GARLAND_MAX_MODES   8

void Garland_Init(void);
void Garland_Update(void);
void Garland_NextMode(void);

bool Garland_SetMode(uint8_t mode_num);
uint8_t Garland_GetMode(void);

bool Garland_CheckPattern(const char *seq);
uint8_t Garland_AddMode(const char *seq, uint16_t delay_ms);

#endif /* INC_GARLAND_H_ */
