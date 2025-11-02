#ifndef BUTTONS_H
#define BUTTONS_H

#include <xc.h>
#include <stdint.h>

// Button event types
typedef enum {
    BTN_NONE,
    BTN_PB1_ONLY,
    BTN_PB2_ONLY,
    BTN_PB3_ONLY,
    BTN_PB1_PB2_SHORT,
    BTN_PB1_PB2_LONG,
    BTN_PB3_PB1,
    BTN_PB3_PB2,
    BTN_PB3_SHORT,
    BTN_PB3_LONG,
    BTN_ALL_THREE,
    BTN_RELEASED
} ButtonEvent;

// Global flags
extern volatile uint8_t button_event_flag;
extern volatile ButtonEvent current_button_event;

// Function prototypes
void initButtons(void);

#endif // BUTTONS_H
