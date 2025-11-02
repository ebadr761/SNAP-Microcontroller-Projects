#ifndef DISPLAY_H
#define DISPLAY_H

#include <xc.h>
#include <stdint.h>

// Function prototypes
void displaySetMode(uint8_t min, uint8_t sec);
void displayCountdown(uint8_t min, uint8_t sec);
void displayFinished(void);
void displayClear(void);
void displayGroupInfo(void);

#endif // DISPLAY_H
