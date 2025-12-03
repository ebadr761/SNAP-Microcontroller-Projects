/*
 * File:   pwm.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on November 20, 2025
 */
#include "pwm.h"

// LED pins
#define LED1_LAT LATBbits.LATB9
#define LED2_LAT LATAbits.LATA6

volatile uint8_t pwm_duty_cycle = 0; // Target brightness
volatile uint8_t pwm_counter = 0; // Step counter
volatile uint8_t pwm_running = 0;

// External reference to active_led (helps determine which LED we are using)
extern volatile uint8_t active_led;

void initPWM(void) {
    // Timer1 for PWM generation
    // Fosc = 8MHz
    // Instruction cycle Fcy = Fosc / 2 = 4MHz
    //
    // Target: 100Hz PWM with 100 steps
    // Need: 100 steps * 100Hz = 10,000 interrupts/sec
    //
    // With 1:8 prescaler: 4MHz / 8 = 500kHz timer clock
    // For 10,000 int/sec: (PR1+1) = 500000 / 10000 = 50
    // So PR1 = 49
    //
    // Result: 10,000 interrupts/sec
    // PWM frequency: 10000 / 100 = 100Hz
    
    T1CONbits.TON = 0;      // Stop timer
    T1CONbits.TCKPS = 1;    // 1:8 prescaler
    T1CONbits.TCS = 0;      // Internal clock
    T1CONbits.TSIDL = 0;    // Continue in idle
    
    IPC0bits.T1IP = 6;      // High priority for PWM
    IFS0bits.T1IF = 0;      // Clear flag
    IEC0bits.T1IE = 1;      // Enable interrupt
    
    PR1 = 49;  // 10,000 interrupts/sec, 100Hz PWM
    
    TMR1 = 0;
    
    pwm_counter = 0;
    pwm_duty_cycle = 0;
    pwm_running = 0;
}

void setPWMDutyCycle(uint8_t duty_cycle) {
    if (duty_cycle > 100) {
        duty_cycle = 100;
    }
    pwm_duty_cycle = duty_cycle;
}

void startPWM(void) {
    pwm_running = 1;
    pwm_counter = 0;
    TMR1 = 0;
    T1CONbits.TON = 1;
}

void stopPWM(void) {
    pwm_running = 0;
    T1CONbits.TON = 0;
    LED1_LAT = 0;
    LED2_LAT = 0;
}

// Timer1 ISR for PWM generation(Intensity/Brightness)
void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;
    
    if (!pwm_running) {
        return;
    }
    
    // Increment counter (0-99 for 100 steps)
    pwm_counter++;
    if (pwm_counter >= 100) {
        pwm_counter = 0;
    }
    
    // Set LED state based on duty cycle
    if (pwm_counter < pwm_duty_cycle) {
        // LED should be ON
        if (active_led == 1) {
            LED1_LAT = 1;
            LED2_LAT = 0;
        } else {
            LED1_LAT = 0;
            LED2_LAT = 1;
        }
    } else {
        // LED should be OFF
        LED1_LAT = 0;
        LED2_LAT = 0;
    }
}