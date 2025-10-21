#ifndef IO_H
#define IO_H

#include <xc.h>
#include <stdint.h>

// Function declarations
void IOinit(void);
void IOcheck(void);
void setTimerRate(uint16_t rate);
void stopTimer(void);

// Button state definitions
#define NO_BUTTON 0
#define PB1_ONLY 1
#define PB2_ONLY 2
#define PB3_ONLY 3
#define PB1_PB2 4
#define PB1_PB3 5
#define PB2_PB3 6
#define ALL_PBS 7

// Global flags
extern volatile uint16_t button_state_changed;
extern volatile uint16_t current_button_state;

#endif /* IO_H */