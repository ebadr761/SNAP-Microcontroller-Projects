/*
 * File:   adc.h
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on November 20, 2025
 */

#ifndef ADC_H
#define ADC_H

#include <xc.h>
#include <stdint.h>

// Initialize ADC for potentiometer reading
void initADC(void);

// Read ADC value (0-1023)
uint16_t readADC(void);

// Convert ADC reading to intensity percentage (0-100)
uint8_t adcToIntensity(uint16_t adc_value);

#endif /* ADC_H */