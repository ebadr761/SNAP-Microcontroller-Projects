#include "timer.h"
#include <xc.h>

void timer1_init(void) {
    // Timer1 Configuration for 1-second interval
    // Fcy = 250 kHz = 250,000 Hz
    
    T1CONbits.TON = 0;      // Turn off Timer1 while configuring
    T1CONbits.TCS = 0;      // Use internal clock (Fcy)
    T1CONbits.TCKPS = 0b11; // Prescaler 1:256
    
    // Calculate PR1 value for 1 second
    // Formula: PR1 = (Desired_Time * Fcy) / Prescaler - 1
    // For Fcy = 250 kHz, Prescaler = 256:
    // PR1 = (1 sec * 250,000 Hz) / 256 - 1 = 976 - 1 = 975
    PR1 = 975;
    
    TMR1 = 0;               // Clear timer register
    IFS0bits.T1IF = 0;      // Clear Timer1 interrupt flag
    
    T1CONbits.TON = 1;      // Turn on Timer1
}

uint8_t timer1_flag_check(void) {
    // Check if Timer1 has overflowed (1 second passed)
    if (IFS0bits.T1IF == 1) {
        return 1;  // 1 second has elapsed
    }
    return 0;
}

void timer1_flag_clear(void) {
    IFS0bits.T1IF = 0;  // Clear the Timer1 interrupt flag
    TMR1 = 0;           // Optional: reset timer to 0 for more accuracy
}