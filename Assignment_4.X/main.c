/*
 * File:   main.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on October 31, 2025, 8:48 PM
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

#include "xc.h"
#include "ADC.h"
#include "UART2.h"
#include "timer.h"
#include <p24F16KA101.h>
#include "clkChange.h"

// Function to display Mode 0 bar graph
void display_mode0_bargraph(uint16_t adc_value) {
    uint8_t num_stars;
    uint8_t i;
    
    // Scale ADC value (0-1023) to number of stars (1-33)
    num_stars = (adc_value * 33) / 1023;
    if (num_stars == 0) num_stars = 1;  // Minimum 1 star
    
    // Display "Mode 0: "
    Disp2String("\rMode 0: ");
    
    // Display asterisks
    for (i = 0; i < num_stars; i++) {
        XmitUART2('*', 1);
    }
    
    // Pad with spaces to clear old bars
    for (i = num_stars; i < 33; i++) {
        XmitUART2(' ', 1);
    }
    
    // Display hex value
    Disp2String(" ");
    Disp2Hex(adc_value);
}

int main(void) {
    newClk(500);  // Set to 500 kHz
    
    uint16_t adc_value = 0;
    uint8_t current_mode = 0;  // 0 = Mode 0, 1 = Mode 1
    uint8_t button_prev = 1;   // Button state tracking
    
    // Initialize peripherals
    timer1_init();
    InitUART2();
    
    // Configure PB1 (RB7) as input with pull-up
    TRISBbits.TRISB7 = 1;      // Input
    CNPU2bits.CN23PUE = 1;     // Enable pull-up resistor
    
    // Startup message
    Disp2String("\r\n=== ADC System Starting ===\r\n");
    Disp2String("Press PB1 to switch modes\r\n");
    Disp2String(">>> Mode 0 - Bar Graph <<<\r\n\r\n");
    
    while(1) {
        // Check for button press (PB1 on RB7)
        if (PORTBbits.RB7 == 0 && button_prev == 1) {
            // Button just pressed! Toggle mode
            current_mode = !current_mode;
            
            // Display mode change message
            if (current_mode == 0) {
                Disp2String("\r\n\r\n>>> Mode 0 - Bar Graph <<<\r\n\r\n");
            } else {
                Disp2String("\r\n\r\n>>> Mode 1 - Data Stream <<<\r\n");
                Disp2String("Close terminal, run Python script\r\n\r\n");
            }
            
            button_prev = 0;  // Remember button is pressed
        }
        
        // Track button release
        if (PORTBbits.RB7 == 1) {
            button_prev = 1;
        }
        
        // Timer-triggered ADC reading (every 1 second)
        if (timer1_flag_check()) {
            
            // Read ADC
            adc_value = do_ADC();
            
            // Execute based on current mode
            if (current_mode == 0) {
                // ===== MODE 0: Bar Graph =====
                display_mode0_bargraph(adc_value);
                
            } else {
                // ===== MODE 1: Data Streaming (for Python) =====
                // Send ADC value in simple format
                Disp2Hex(adc_value);
                XmitUART2('\r', 1);
                XmitUART2('\n', 1);
            }
            
            // Clear timer flag
            timer1_flag_clear();
        }
        Idle();
    }
    
    return 0;
}