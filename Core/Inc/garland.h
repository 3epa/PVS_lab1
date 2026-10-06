#ifndef INC_GARLAND_H_
#define INC_GARLAND_H_

#include <stdint.h>

#define GARLAND_MODES_COUNT  4

void Garland_Init(void);
void Garland_Update(void);
void Garland_NextMode(void);
uint8_t Garland_GetMode(void);

#endif /* INC_GARLAND_H_ */
