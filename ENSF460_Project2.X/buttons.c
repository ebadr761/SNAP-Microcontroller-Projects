/*
 * File:   buttons.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on November 20, 2025
 */

#include "buttons.h"

// Global variables
volatile ButtonEvent pending_button_event = BTN_NONE;

// Button states
static volatile uint8_t pb1_pressed = 0;
static volatile uint8_t pb2_pressed = 0;
static volatile uint8_t pb3_pressed = 0;

// Timing for long press detection (>3 seconds = 60 ticks at 50ms)
static volatile uint16_t button_hold_counter = 0;
static volatile uint8_t long_press_detected = 0;

void initButtons(void) {
    // PB1 on RB7
    TRISBbits.TRISB7 = 1;
    CNPU2bits.CN23PUE = 1;  // Pull-up
    CNEN2bits.CN23IE = 1;   // Enable CN interrupt
    
    // PB2 on RB4 
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1;   // Pull-up
    CNEN1bits.CN1IE = 1;    // Enable CN interrupt
    
    // PB3 on RA4
    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;   // Pull-up
    CNEN1bits.CN0IE = 1;    // Enable CN interrupt
    
    // Configure CN interrupt priority
    IPC4bits.CNIP = 7;      // Highest priority
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
    
    // Timer2 for debouncing and long press detection
    // Fosc = 8MHz, Fcy = 4MHz
    // Prescaler 1:64: Timer clock = 4,000,000 / 64 = 62,500 Hz
    // For 50ms: Ticks needed = 62,500 Hz × 0.05s = 3,125 ticks
    // PR2 = 3,125 - 1 = 3,124
    T2CONbits.TCKPS = 2;   // 1:64 prescaler
    T2CONbits.TCS = 0;     // Internal clock
    T2CONbits.TSIDL = 0;
    IPC1bits.T2IP = 5;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    PR2 = 3124;  // 50ms at 8MHz with 1:64 prescaler
    TMR2 = 0;
    T2CONbits.TON = 0;
}

ButtonEvent getButtonEvent(void) {
    ButtonEvent event = pending_button_event;
    if (event != BTN_NONE) {
        pending_button_event = BTN_NONE;
    }
    return event;
}

// CN Interrupt - Button state changed
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void) {
    IFS1bits.CNIF = 0;
    
    // Only start debounce timer if not already running
    if (!T2CONbits.TON) {
        TMR2 = 0;
        T2CONbits.TON = 1;
    }
}

// Timer2 Interrupt - Debounce timer + long press detection
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0;
    
    // Read button states (active low)
    uint8_t pb1_now = (PORTBbits.RB7 == 0);
    uint8_t pb2_now = (PORTBbits.RB4 == 0);  
    uint8_t pb3_now = (PORTAbits.RA4 == 0); 
    
    if (pb1_now || pb2_now || pb3_now) {
        // At least one button pressed
        pb1_pressed = pb1_now;
        pb2_pressed = pb2_now;
        pb3_pressed = pb3_now;
        
        button_hold_counter++;
        
        // Long press detection for PB1 (>3 seconds = 60 ticks)
        if (pb1_pressed && button_hold_counter > 60) {
            if (!long_press_detected) {
                long_press_detected = 1;
                pending_button_event = BTN_PB1_LONG;
            }
        }
        
        // Keep timer running while button held
        TMR2 = 0;
    }
    else {
        // All buttons released
        
        // Determine what kind of press it was
        if (pb1_pressed && !pb2_pressed && !pb3_pressed && !long_press_detected) {
            pending_button_event = BTN_PB1_CLICK;
        }
        else if (pb2_pressed && !pb1_pressed && !pb3_pressed) {
            pending_button_event = BTN_PB2_CLICK;
        }
        else if (pb3_pressed && !pb1_pressed && !pb2_pressed) {
            pending_button_event = BTN_PB3_CLICK;
        }
        
        // Reset state
        pb1_pressed = 0;
        pb2_pressed = 0;
        pb3_pressed = 0;
        button_hold_counter = 0;
        long_press_detected = 0;
        
        T2CONbits.TON = 0;  // Stop timer
    }
}