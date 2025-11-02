/*
 * File:   timer.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on October 12, 2025, 12:50 PM
 */

#include "timer.h"
#include <xc.h>

volatile uint8_t timer_tick_flag = 0;

// Timer1 configuration for 1 second interrupt
void initTimer1(void) {

    T1CONbits.TCKPS = 2;    // 1:64 prescaler
    T1CONbits.TCS = 0;      // Internal clock (Fcy)
    T1CONbits.TSIDL = 0;    // Continue in idle
    
    IPC0bits.T1IP = 5;      // Priority 5
    IFS0bits.T1IF = 0;      // Clear flag
    IEC0bits.T1IE = 1;      // Enable interrupt
    
    PR1 = 3905;              // 1 second period
    TMR1 = 0;
    
    T1CONbits.TON = 0;      // Don't start yet
}

void startTimer1(void) {
    TMR1 = 0;
    T1CONbits.TON = 1;
}

void stopTimer1(void) {
    T1CONbits.TON = 0;
    TMR1 = 0;
}

// Timer3 for LED2 rapid blink for 0.25 seconds
void initTimer3(void) {
   
    T2CONbits.T32 = 0;      // Timer3 as 16-bit timer (separate from Timer2)
    T3CONbits.TCKPS = 2;    // 1:64 prescaler
    T3CONbits.TCS = 0;      // Internal clock
    T3CONbits.TSIDL = 0;    // Continue in idle
    
    IPC2bits.T3IP = 3;      // Priority 3
    IFS0bits.T3IF = 0;      // Clear flag
    IEC0bits.T3IE = 1;      // Enable interrupt
    
    PR3 = 977;              // 0.25 second period
    TMR3 = 0;
    
    T3CONbits.TON = 0;      // Don't start yet
}

void startTimer3(void) {
    TMR3 = 0;
    T3CONbits.TON = 1;
}

void stopTimer3(void) {
    T3CONbits.TON = 0;
    TMR3 = 0;
}

// Timer1 ISR - 1 second tick
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;
    
    // Set flag for main loop to decrement time
    timer_tick_flag = 1;
}

// Timer3 ISR - LED2 rapid blink (0.25 second)
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {
    IFS0bits.T3IF = 0;
    
    // Toggle LED2
    LATAbits.LATA6 ^= 1;
}