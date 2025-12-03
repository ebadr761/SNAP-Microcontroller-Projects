/*
 * File:   pwm.h
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on November 20, 2025
 */

#ifndef PWM_H
#define PWM_H

#include <xc.h>
#include <stdint.h>

// Initialize software PWM using Timer1
void initPWM(void);

// Set PWM duty cycle (0-100%)
void setPWMDutyCycle(uint8_t duty_cycle);

// Start PWM generation
void startPWM(void);

// Stop PWM generation
void stopPWM(void);

// Global PWM state
extern volatile uint8_t pwm_duty_cycle;
extern volatile uint8_t pwm_running;

#endif /* PWM_H */