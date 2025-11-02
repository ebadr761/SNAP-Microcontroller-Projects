#ifndef TIMER_H
#define TIMER_H

#include <xc.h>
#include <stdint.h>

// Global flag
extern volatile uint8_t timer_tick_flag;

// Function prototypes
void initTimer1(void);
void startTimer1(void);
void stopTimer1(void);

void initTimer3(void);
void startTimer3(void);
void stopTimer3(void);

#endif //TIMER_H