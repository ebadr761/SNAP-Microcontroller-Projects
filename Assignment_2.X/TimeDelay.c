/*
 * File:   TimeDelay.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on September 28, 2025, 08:43 PM
 */


#include "xc.h"

#define LED2_LAT LATAbits.LATA6

// Global variables for timing(volatile for ISR access)
volatile uint16_t ms_count = 0; // ms counter
volatile uint16_t ms_cap = 0; // ms cap (1ms, 250ms, 1000ms, 6000ms )
volatile uint8_t delay_complete = 0; // Flag to determine if delay is complete

// Initialize timer2 for 1ms interrupts 
void timer2_init(void){
    //T2CON config
    T2CONbits.TCKPS = 1; // Prescaler 1:8
    T2CONbits.TCS = 0;  // Use internal clock
    
    // Timer 2 interrupt config
    IPC1bits.T2IP = 2; // Set priority to two
    IFS0bits.T2IF = 0; // Clear interrupt flag
    IEC0bits.T2IE = 1; // Enable interrupts
    PR2 = 31;          // Approximately 1ms period
    TMR2 = 0;          // Start timer at 0 
    T2CONbits.TON = 1; // Start timer
}
// Delay for specified milliseconds using idle mode
void delay_ms(uint16_t time_ms){
    ms_count = 0;
    ms_cap = time_ms;
    delay_complete = 0; 
    
    while(delay_complete == 0){
        Idle(); // CPU sleep
    }
}

// Timer2 ISR - counts the milliseconds and toggles LED2(0.5s intervals)
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void){
    IFS0bits.T2IF = 0; // Flag reset
   
    ms_count++;
    if(ms_count >= ms_cap){
        delay_complete = 1;
    }
    
    // LED (0.5s interval logic)
    static uint16_t led2_count = 0;
    led2_count++;
    if(led2_count >= 500){
        LED2_LAT = !LED2_LAT;
        led2_count = 0;
    }
}
