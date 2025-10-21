/*
 * File:   main.c
 * Author: UPDATE THIS WITH YOUR GROUP MEMBER NAMES OR POTENTIALLY LOSE POINTS
 *
 * Created on: USE THE INFORMATION FROM THE HEADER MPLAB X IDE GENERATES FOR YOU
 */

// FBS
#pragma config BWRP = OFF               // Table Write Protect Boot (Boot segment may be written)
#pragma config BSS = OFF                // Boot segment Protect (No boot program Flash segment)

// FGS
#pragma config GWRP = OFF               // General Segment Code Flash Write Protection bit (General segment may be written)
#pragma config GCP = OFF                // General Segment Code Flash Code Protection bit (No protection)

// FOSCSEL
#pragma config FNOSC = FRC              // Oscillator Select (Fast RC oscillator (FRC))
#pragma config IESO = OFF               // Internal External Switch Over bit (Internal External Switchover mode disabled (Two-Speed Start-up disabled))

// FOSC
#pragma config POSCMOD = NONE           // Primary Oscillator Configuration bits (Primary oscillator disabled)
#pragma config OSCIOFNC = ON            // CLKO Enable Configuration bit (CLKO output disabled; pin functions as port I/O)
#pragma config POSCFREQ = HS            // Primary Oscillator Frequency Range Configuration bits (Primary oscillator/external clock input frequency greater than 8 MHz)
#pragma config SOSCSEL = SOSCHP         // SOSC Power Selection Configuration bits (Secondary oscillator configured for high-power operation)
#pragma config FCKSM = CSECMD           // Clock Switching and Monitor Selection (Clock switching is enabled, Fail-Safe Clock Monitor is disabled)

// FWDT
#pragma config WDTPS = PS32768          // Watchdog Timer Postscale Select bits (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (WDT prescaler ratio of 1:128)
#pragma config WINDIS = OFF             // Windowed Watchdog Timer Disable bit (Standard WDT selected; windowed WDT disabled)
#pragma config FWDTEN = OFF             // Watchdog Timer Enable bit (WDT disabled (control is placed on the SWDTEN bit))

// FPOR
#pragma config BOREN = BOR3             // Brown-out Reset Enable bits (Brown-out Reset enabled in hardware; SBOREN bit disabled)
#pragma config PWRTEN = ON              // Power-up Timer Enable bit (PWRT enabled)
#pragma config I2C1SEL = PRI            // Alternate I2C1 Pin Mapping bit (Default location for SCL1/SDA1 pins)
#pragma config BORV = V18               // Brown-out Reset Voltage bits (Brown-out Reset set to lowest voltage (1.8V))
#pragma config MCLRE = ON               // MCLR Pin Enable bit (MCLR pin enabled; RA5 input pin disabled)

// FICD
#pragma config ICS = PGx2               // ICD Pin Placement Select bits (PGC2/PGD2 are used for programming and debugging the device)

// FDS
#pragma config DSWDTPS = DSWDTPSF       // Deep Sleep Watchdog Timer Postscale Select bits (1:2,147,483,648 (25.7 Days))
#pragma config DSWDTOSC = LPRC          // DSWDT Reference Clock Select bit (DSWDT uses LPRC as reference clock)
#pragma config RTCOSC = SOSC            // RTCC Reference Clock Select bit (RTCC uses SOSC as reference clock)
#pragma config DSBOREN = ON             // Deep Sleep Zero-Power BOR Enable bit (Deep Sleep BOR enabled in Deep Sleep)
#pragma config DSWDTEN = ON             // Deep Sleep Watchdog Timer Enable bit (DSWDT enabled)

// #pragma config statements should precede project file includes.

#include <xc.h>
#include <p24F16KA101.h>

/**
 * You might find it useful to add your own #defines to improve readability here
 */

//LED define
#define LED_TRIS   TRISBbits.TRISB9
#define LED_LAT    LATBbits.LATB9

// Push button defines for the three buttons
#define PB1_TRIS   TRISBbits.TRISB7    // PB1 on RB7
#define PB1_PORT   PORTBbits.RB7       // Read PB1 state

#define PB2_TRIS   TRISBbits.TRISB4    // PB2 on RB4  
#define PB2_PORT   PORTBbits.RB4       // Read PB2 state

#define PB3_TRIS   TRISAbits.TRISA4    // PB3 on RA4
#define PB3_PORT   PORTAbits.RA4       // Read PB3 state


// Busy wait loop
void delay(unsigned int ms) {
    unsigned int i, j;
    for(i = 0; i < ms; i++) {
        for(j = 0; j < 800; j++) {
        }
    }
}

int main(void) {
    /** This is usually where you would add run-once code
     * e.g., peripheral initialization. For the first labs
     * you might be fine just having it here. For more complex
     * projects, you might consider having one or more initialize() functions
     */
   
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
   
   // Configure RB9 as output for LED
    LED_TRIS = 0;
   
    // Configure push button pins as inputs
    PB1_TRIS = 1;        
    PB2_TRIS = 1;        
    PB3_TRIS = 1;        
   
    // Enable pull-up resistors
    // Could also enable on individual input pins like:
    // (CN23 - RB7), (CN1 - RB4), (CN0 - RA4)
    CNPU1 = 0xFFFF;      // Enable pull-ups on Port B pins
    CNPU2 = 0xFFFF;      // Enable pull-ups on Port A pins
   
    // Turn LED off initially
    LED_LAT = 0;
   
    while(1) {
        // Read all button states
        int pb1_pressed = (PB1_PORT == 0);
        int pb2_pressed = (PB2_PORT == 0);  
        int pb3_pressed = (PB3_PORT == 0);
       
        // Count how many buttons are pressed
        int buttons_pressed = pb1_pressed + pb2_pressed + pb3_pressed;
       
        // 2 or more buttons pressed - LED stays ON
        if(buttons_pressed >= 2) {
           
            LED_LAT = 1;
        }
       
        else if(pb1_pressed) {
            // Only PB1 pressed - blink at 0.25 second intervals
            LED_LAT = 1;
            delay(250);
            LED_LAT = 0;
            delay(250);
        }
       
        else if(pb2_pressed) {
            // Only PB2 pressed - blink at 1 second intervals
            LED_LAT = 1;
            delay(1000);
            LED_LAT = 0;
            delay(1000);
        }
       
        else if(pb3_pressed) {
            // Only PB3 pressed - blink at 6 second intervals
            LED_LAT = 1;
            delay(4000);
            LED_LAT = 0;
            delay(4000);
        }
       
        else {
            // No buttons pressed - LED OFF
            LED_LAT = 0;
        }
    }
    return 0;
}