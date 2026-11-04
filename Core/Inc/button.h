#ifndef INC_BUTTON_H_
#define INC_BUTTON_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	BUTTON_EVENT_NONE = 0, BUTTON_EVENT_CLICK
} button_event_t;

void Button_Init(void);
button_event_t Button_Update(void);

#endif /* INC_BUTTON_H_ */
