/*
 * File:   buttons.h
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on November 20, 2025
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include <xc.h>
#include <stdint.h>

// Button event types
typedef enum {
    BTN_NONE = 0,
    BTN_PB1_CLICK,
    BTN_PB1_LONG,
    BTN_PB2_CLICK,
    BTN_PB3_CLICK
} ButtonEvent;

// Initialize button inputs and interrupts
void initButtons(void);

// Get button event (cleared after reading)
ButtonEvent getButtonEvent(void);

#endif /* BUTTONS_H */