/*
 * File:   adc.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on November 20, 2025
 */
#include "adc.h"

void initADC(void) {
    // STEP 1: PIN CONFIGURATION
    AD1PCFG = 0xFFFF;               // Set ALL pins to digital
    AD1PCFGbits.PCFG12 = 0;         // Set AN12 (pin 15) to ANALOG
    TRISBbits.TRISB12 = 1;          // Set RB12 as INPUT
    
    // STEP 2: ADC CONTROL
    AD1CON1bits.ADON = 0;           // Turn OFF ADC during configuration
    AD1CON1bits.FORM = 0b00;        // Output format: Integer (0-1023)
    AD1CON1bits.SSRC = 0b111;       // Auto-convert after sampling
    AD1CON1bits.ASAM = 0;           // Manual sampling start
    
    // STEP 3: VOLTAGE REFERENCE
    AD1CON2bits.VCFG = 0b000;       // Vref+ = AVDD (3.3V), Vref- = AVSS (0V)
   
    // STEP 4: TIMING CONFIGURATION
    AD1CON3bits.ADRC = 0;           // Use system clock (not RC oscillator)
    AD1CON3bits.SAMC = 0b11111;     // Auto-sample time = 31 TAD cycles
    AD1CON3bits.ADCS = 0b00111111;  // TAD = 64 × TCY (conversion clock)

    // STEP 5: CHANNEL SELECTION
    AD1CHSbits.CH0NA = 0;           // Negative input = VR- (ground)
    AD1CHSbits.CH0SA = 12;          // Positive input = AN12 (potentiometer)
    
    // STEP 6: TURN ON ADC
    AD1CON1bits.ADON = 1;           // Turn ON ADC module
}

uint16_t readADC(void) {
    AD1CON1bits.SAMP = 1;   // Start sampling
    
    // Add timeout to prevent infinite hang
    uint16_t timeout = 10000;
    while (!AD1CON1bits.DONE && timeout > 0) {
        timeout--;
    }
    
    if (timeout == 0) {
        // ADC timeout - return middle value (50%)
        return 512;
    }
    
    return ADC1BUF0;         // Read result (0-1023)
}

uint8_t adcToIntensity(uint16_t adc_value) {
    // Convert 0-1023 to 0-100
    // Use 32-bit arithmetic to avoid overflow
    uint32_t temp = ((uint32_t)adc_value * 100) / 1023;
    return (uint8_t)temp;
}