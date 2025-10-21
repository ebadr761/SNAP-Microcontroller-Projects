/*
 * File:   IOs.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on September 30, 2025, 11:42 PM
 */


#include "xc.h"
#include "TimeDelay.h"

//LED 1 and 2 define
#define LED1_TRIS TRISBbits.TRISB9
#define LED1_LAT  LATBbits.LATB9

#define LED2_TRIS TRISAbits.TRISA6
#define LED2_LAT  LATAbits.LATA6

// Push button defines for the three buttons
#define PB1_TRIS TRISBbits.TRISB7
#define PB1_PORT PORTBbits.RB7

#define PB2_TRIS TRISBbits.TRISB4
#define PB2_PORT PORTBbits.RB4

#define PB3_TRIS TRISAbits.TRISA4
#define PB3_PORT PORTAbits.RA4

// Initiates Input/Output for LEDs and Push buttons
void IOinit(){
    
    AD1PCFG = 0xFFFF; /* keep this line as it sets I/O pins that can also be analog to be digital */
    
    // Sets LED 1 and 2 as output and sets them low(off)
    LED1_TRIS = 0;
    LED2_TRIS = 0;
    
    LED1_LAT = 0;
    LED2_LAT = 0;
    
    // sets Pushbuttons as Input
    PB1_TRIS = 1;
    PB2_TRIS = 1;
    PB3_TRIS = 1;
    
    // Pull up resistors enabled: Read high when not pressed or low when pressed
    CNPU2bits.CN23PUE = 1;
    CNPU1bits.CN1PUE = 1;
    CNPU1bits.CN0PUE = 1;
    
}
// Run the logic for the circuit
void IOcheck(){

        // Read all button states
        int pb1_pressed = (PB1_PORT == 0); 
        int pb2_pressed = (PB2_PORT == 0);  
        int pb3_pressed = (PB3_PORT == 0); 
        
        int buttons_pressed = pb1_pressed + pb2_pressed + pb3_pressed;
        // PB1 AND PB2 was specifically mentioned to blink at 1ms intervals
        // BUT in demo section it says 2 OR MORE showing LED1.. So we
        // implemented the 1ms intervals for all combinations of 2 PB's or 
        // ALL 3 pressed
        if(buttons_pressed >= 2) {
            
            LED1_LAT = 1;
            delay_ms(1);
            LED1_LAT = 0;
            delay_ms(1);
        }
       
        else if(pb1_pressed) {
            // Only PB1 pressed - blink at 0.25 second intervals
            LED1_LAT = 1;
            delay_ms(250);
            LED1_LAT = 0;
            delay_ms(250);
        }
        
        else if(pb2_pressed) {
            // Only PB2 pressed - blink at 1 second intervals
            LED1_LAT = 1;
            delay_ms(1000);
            LED1_LAT = 0;
            delay_ms(1000);
        }
        
        else if(pb3_pressed) {
            // Only PB3 pressed - blink at 6 second intervals
            LED1_LAT = 1;
            delay_ms(6000);
            LED1_LAT = 0;
            delay_ms(6000);
        }
        
        else {
            // No buttons pressed - LED OFF
            LED1_LAT = 0;
        }
    }
