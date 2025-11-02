#ifndef TIMER_H
#define TIMER_H

#include <xc.h>
#include <stdint.h>

// Initialize Timer1 for 1-second intervals
void timer1_init(void);

// Check if 1 second has elapsed (returns 1 if yes, 0 if no)
uint8_t timer1_flag_check(void);

// Clear the timer flag
void timer1_flag_clear(void);

#endif