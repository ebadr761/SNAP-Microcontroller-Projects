/*
 * File:   buttons.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on October 13, 2025, 9:43 AM
 */


#include "buttons.h"
#include <xc.h>

// Global variables
volatile uint8_t button_event_flag = 0;
volatile ButtonEvent current_button_event = BTN_NONE;

// Button states
static volatile uint8_t pb1_pressed = 0;
static volatile uint8_t pb2_pressed = 0;
static volatile uint8_t pb3_pressed = 0;

// Timing for long press detection
static volatile uint16_t button_hold_counter = 0;
static volatile uint8_t long_press_detected = 0;

// Increment acceleration
volatile uint8_t fast_increment_mode = 0;

// Repeat rate control
static volatile uint8_t repeat_counter = 0;
#define INITIAL_REPEAT_DELAY 6     // 300ms before first repeat (6 * 50ms)
#define SLOW_REPEAT_RATE 8         // 400ms between slow increments (8 * 50ms)
#define FAST_REPEAT_RATE 2         // 100ms between fast increments (2 * 50ms)
#define FAST_MODE_THRESHOLD 40     // Enter fast mode after ~2 seconds

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
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
    
    // Timer2 for debouncing and long press detection
    T2CONbits.TCKPS = 2;   // 1:64 prescaler
    T2CONbits.TCS = 0;     // Internal clock
    T2CONbits.TSIDL = 0;
    IPC1bits.T2IP = 4;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    PR2 = 195;  // 50ms
    TMR2 = 0;
    T2CONbits.TON = 0;  // Don't start yet
}

// CN Interrupt - Button state changed
void __attribute__((interrupt, no_auto_psv)) _CNInterrupt(void) {
    IFS1bits.CNIF = 0;
    
    // Start debounce timer
    TMR2 = 0;
    T2CONbits.TON = 1;
}

// Timer2 Interrupt - Debounce timer expired + repeat rate control
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void) {
    IFS0bits.T2IF = 0;
    
    // Read button states (active low)
    uint8_t pb1_now = (PORTBbits.RB7 == 0);
    uint8_t pb2_now = (PORTBbits.RB4 == 0);  
    uint8_t pb3_now = (PORTAbits.RA4 == 0); 
    
    // Detect button press/release
    if (pb1_now || pb2_now || pb3_now) {
        // At least one button pressed
        pb1_pressed = pb1_now;
        pb2_pressed = pb2_now;
        pb3_pressed = pb3_now;
        
        button_hold_counter++;
        repeat_counter++;
        
        // Long press detection (>3 seconds = 60 ticks at 50ms)
        if (button_hold_counter > 60) {
            long_press_detected = 1;
        }
        
        // Fast increment mode (after ~2 seconds)
        if (button_hold_counter > FAST_MODE_THRESHOLD) {
            fast_increment_mode = 1;
        }
        
        // Determine if we should fire an event based on repeat rate
        uint8_t should_fire = 0;
        
        if (button_hold_counter == 1) {
            // First press - always fire immediately
            should_fire = 1;
            repeat_counter = 0;
        }
        else if (repeat_counter >= INITIAL_REPEAT_DELAY) {
            // After initial delay (6), check repeat rate
            if (fast_increment_mode) {
                // Fast mode - fire every FAST_REPEAT_RATE (2) ticks
                if (repeat_counter >= FAST_REPEAT_RATE) {
                    should_fire = 1;
                    repeat_counter = 0;
                }
            }
            else {
                // Slow mode - fire every SLOW_REPEAT_RATE (8) ticks
                if (repeat_counter >= SLOW_REPEAT_RATE) {
                    should_fire = 1;
                    repeat_counter = 0;
                }
            }
        }
        
        // Only set event flag if repeat timing allows
        if (should_fire) {
            // Determine button combination
            if (pb1_now && pb2_now && pb3_now) {
                current_button_event = BTN_ALL_THREE;
                button_event_flag = 1;
            }
            else if (pb1_now && pb2_now) {
                if (long_press_detected) {
                    current_button_event = BTN_PB1_PB2_LONG;
                    button_event_flag = 1;
                    long_press_detected = 0;  // Only trigger once
                }
            }
            else if (pb3_now && pb1_now) {
                current_button_event = BTN_PB3_PB1;
                button_event_flag = 1;
            }
            else if (pb3_now && pb2_now) {
                current_button_event = BTN_PB3_PB2;
                button_event_flag = 1;
            }
            else if (pb1_now) {
                current_button_event = BTN_PB1_ONLY;
                button_event_flag = 1;
            }
            else if (pb2_now) {
                current_button_event = BTN_PB2_ONLY;
                button_event_flag = 1;
            }
            else if (pb3_now) {
                if (long_press_detected) {
                    current_button_event = BTN_PB3_LONG;
                    button_event_flag = 1;
                    long_press_detected = 0;
                }
            }
        }
        
        // Keep timer running while button held
        TMR2 = 0;
    }
    else {
        // All buttons released
        
        // Determine what kind of press it was
        if (pb3_pressed && !pb1_pressed && !pb2_pressed && !long_press_detected) {
            current_button_event = BTN_PB3_SHORT;
            button_event_flag = 1;
        }
        else if (pb1_pressed && pb2_pressed && !long_press_detected) {
            current_button_event = BTN_PB1_PB2_SHORT;
            button_event_flag = 1;
        }
        else {
            // Any other release
            current_button_event = BTN_RELEASED;
            button_event_flag = 1; 
        }
        
        // Reset state
        pb1_pressed = 0;
        pb2_pressed = 0;
        pb3_pressed = 0;
        button_hold_counter = 0;
        long_press_detected = 0;
        fast_increment_mode = 0;
        repeat_counter = 0;
        
        T2CONbits.TON = 0;  // Stop timer
    }
}