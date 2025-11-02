#include "xc.h"
#include "ADC.h"

uint16_t do_ADC(void) {
    uint16_t ADCvalue; // 16-bit register to hold ADC output
    
    /********** STEP 1: CONFIGURE ADC PINS **********/
    // For pin 8 (prep): Configure AN5 as analog input
    // For pin 15 (assignment): You'll change this to the appropriate ANx
    
    AD1PCFG = 0xFFFF;        // Start with all pins as digital
    AD1PCFGbits.PCFG12 = 0;   // Set AN5 (pin 8) as ANALOG (0 = analog)
    // Note: For the assignment, you'll change this to the correct pin
    
    /********** STEP 2: CONFIGURE AD1CON1 (Main Control) **********/
    AD1CON1bits.ADON = 0;    // Turn OFF ADC during configuration
    AD1CON1bits.FORM = 0b00; // Output format: Integer (0000 00dd dddd dddd)
    AD1CON1bits.SSRC = 0b111; // Auto-convert (internal counter ends sampling)
    AD1CON1bits.ASAM = 0;    // Manual sampling start (we'll set SAMP bit)
    
    /********** STEP 3: CONFIGURE AD1CON2 (Voltage Reference) **********/
    AD1CON2bits.VCFG = 0b000; // Vref+ = AVDD, Vref- = AVSS (supply voltage)
    
    /********** STEP 4: CONFIGURE AD1CON3 (Timing) **********/
    AD1CON3bits.ADRC = 0;     // Use system clock = 0 (not internal RC = 1)
    AD1CON3bits.SAMC = 0b11111; // Auto-sample time = 31 TAD cycles (longest sampling time = most accurate)
    AD1CON3bits.ADCS = 0b00000010; // ADC Conversion Clock = 3 * TCY
    
    /********** STEP 5: SELECT INPUT CHANNEL **********/
    // Using MUX A to select channel
    AD1CHSbits.CH0NA = 0;     // Channel 0 negative input is VR- (ground)
    AD1CHSbits.CH0SA = 0b1100; // Channel 0 positive input is AN5
    // For prep: AN5 = 0b0101
    // For assignment: Change to appropriate ANx channel
    
    /********** STEP 6: START ADC MODULE **********/
    AD1CON1bits.ADON = 1;     // Turn ON ADC module
    
    /********** STEP 7: START SAMPLING **********/
    AD1CON1bits.SAMP = 1;     // Start sampling
    
    /********** STEP 8: WAIT FOR CONVERSION TO COMPLETE **********/
    while (AD1CON1bits.DONE == 0) {
        // Wait for conversion to complete
        // DONE bit is automatically set by hardware when conversion finishes
        // so it samples for 31 TAD cycles, converts for around 10 TAD cycles 
        // and finally sets DONE = 1 when finished. so this loop waits around 500 microseconds
    }
    
    /********** STEP 9: READ RESULT **********/
    ADCvalue = ADC1BUF0;      // Read the converted value from buffer
    
    /********** STEP 10: CLEANUP **********/
    AD1CON1bits.SAMP = 0;     // Stop sampling
    AD1CON1bits.ADON = 0;     // Turn off ADC to save power
    
    return ADCvalue;          // Return 10-bit value (0-1023)
}