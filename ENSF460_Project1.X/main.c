/*
 * File:   main.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on October 12, 2025, 12:43 PM
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

#include <xc.h>
#include <stdint.h>
#include "clkChange.h"
#include "UART2.h"
#include "timer.h"
#include "buttons.h"
#include "display.h"

// State machine states
typedef enum {
    STATE_SET_MODE,      // Setting the timer
    STATE_RUNNING,       // Timer counting down
    STATE_PAUSED,        // Timer paused
    STATE_ALARM,         // Timer finished, alarm active
    STATE_DISPLAY_INFO   // Showing group info
} TimerState;

// Global state variables
volatile TimerState current_state = STATE_SET_MODE;
volatile uint8_t minutes = 0;
volatile uint8_t seconds = 0;
volatile uint8_t display_update_needed = 1;

// Button flags (set by ISRs)
extern volatile uint8_t button_event_flag;
extern volatile ButtonEvent current_button_event;
extern volatile uint8_t fast_increment_mode;  //indicates fast mode

// Timer flags
extern volatile uint8_t timer_tick_flag;

// LED control
#define LED1_LAT LATBbits.LATB9
#define LED2_LAT LATAbits.LATA6

// Function prototypes
void initHardware(void);
void handleSetMode(void);
void handleRunning(void);
void handlePaused(void);
void handleAlarm(void);
void handleDisplayInfo(void);
void updateLEDs(void);

int main(void) {
    newClk(500); 
    initHardware();
    InitUART2();
    initTimer1();
    initTimer3();  // For LED2 rapid blink
    initButtons();
    
    // Initial display
    displaySetMode(minutes, seconds);
    
    while(1) {
        Idle();
        
        // Handle button events
        if (button_event_flag) {
            button_event_flag = 0;
            
            switch(current_state) {
                case STATE_SET_MODE:
                    handleSetMode();
                    break;
                    
                case STATE_RUNNING:
                    handleRunning();
                    break;
                    
                case STATE_PAUSED:
                    handlePaused();
                    break;
                    
                case STATE_ALARM:
                    handleAlarm();
                    break;
                    
                case STATE_DISPLAY_INFO:
                    handleDisplayInfo();
                    break;
            }
        }
        
        // Handle timer ticks (1 second)
        if (timer_tick_flag) {
            timer_tick_flag = 0;
            
            if (current_state == STATE_RUNNING) {
                // Decrement timer
                if (seconds > 0) {
                    seconds--;
                    display_update_needed = 1;
                } else if (minutes > 0) {
                    minutes--;
                    seconds = 59;
                    display_update_needed = 1;
                } else {
                    // Timer reached 0:00
                    current_state = STATE_ALARM;
                    stopTimer1();       // Stop countdown timer
                    LED1_LAT = 1;       // LED1 solid ON
                    LED2_LAT = 0;       // LED2 starts OFF
                    startTimer3();      // Start LED2 rapid blink timer
                    displayFinished();
                }
            }
        }
        
        // Update LEDs based on state
        updateLEDs();
        
        // Update display if needed
        if (display_update_needed) {
            display_update_needed = 0;
            
            switch(current_state) {
                case STATE_SET_MODE:
                    displaySetMode(minutes, seconds);
                    break;
                case STATE_RUNNING:
                    displayCountdown(minutes, seconds);
                    break;
                case STATE_PAUSED:
                    displayCountdown(minutes, seconds);
                    break;
                case STATE_ALARM:
                    displayFinished();
                    break;
            }
        }
    }
    
    return 0;
}

void initHardware(void) {
    AD1PCFG = 0xFFFF;  /* keep this line as it sets I/O pins that can also be analog to be digital */
    
    // LED1 on RB9
    TRISBbits.TRISB9 = 0;
    LED1_LAT = 0;
    
    // LED2 on RA6
    TRISAbits.TRISA6 = 0;
    LED2_LAT = 0;
}

void handleSetMode(void) {
    switch(current_button_event) {
        case BTN_PB1_ONLY:
            // Increment seconds (by 1 or 5 depending on fast mode)
            if (fast_increment_mode) {
                seconds += 5;
                if (seconds > 59) {
                    seconds = seconds % 60;  // Wrap around properly
                }
            } else {
                seconds++;
                if (seconds > 59) {
                    seconds = 0;
                }
            }
            display_update_needed = 1;
            break;
            
        case BTN_PB2_ONLY:
            // Increment minutes (by 1 or 5 depending on fast mode)
            if (fast_increment_mode) {
                minutes += 5;
                if (minutes > 59) {
                    minutes = minutes % 60;  // Wrap around properly
                }
            } else {
                minutes++;
                if (minutes > 59) {
                    minutes = 0;
                }
            }
            display_update_needed = 1;
            break;
            
        case BTN_PB3_PB1:
            // Decrement seconds (always by 1, not affected by fast mode)
            if (seconds > 0) {
                seconds--;
            } else {
                seconds = 59;
            }
            display_update_needed = 1;
            break;
            
        case BTN_PB3_PB2:
            // Decrement minutes (always by 1, not affected by fast mode)
            if (minutes > 0) {
                minutes--;
            } else {
                minutes = 59;
            }
            display_update_needed = 1;
            break;
            
        case BTN_PB1_PB2_SHORT:
            // Start timer
            if (minutes > 0 || seconds > 0) {
                current_state = STATE_RUNNING;
                startTimer1();
                display_update_needed = 1;
            }
            break;
            
        case BTN_PB1_PB2_LONG:
            // Reset to 0:00
            minutes = 0;
            seconds = 0;
            display_update_needed = 1;
            break;
            
        case BTN_ALL_THREE:
            // Display group info
            current_state = STATE_DISPLAY_INFO;
            displayGroupInfo();
            break;
            
        default:
            break;
    }
}

void handleRunning(void) {
    switch(current_button_event) {
        case BTN_PB3_SHORT:
            // Pause timer
            current_state = STATE_PAUSED;
            stopTimer1();
            break;
            
        case BTN_PB3_LONG:
            // Stop and reset
            current_state = STATE_SET_MODE;
            stopTimer1();
            minutes = 0;
            seconds = 0;
            displayClear();
            LED1_LAT = 0;
            break;
            
        default:
            break;
    }
}

void handlePaused(void) {
    switch(current_button_event) {
        case BTN_PB3_SHORT:
            // Resume timer
            current_state = STATE_RUNNING;
            startTimer1();
            break;
            
        case BTN_PB3_LONG:
            // Stop and reset
            current_state = STATE_SET_MODE;
            stopTimer1();
            minutes = 0;
            seconds = 0;
            displayClear();
            LED1_LAT = 0;
            break;
            
        default:
            break;
    }
}

void handleAlarm(void) {
    // Any button press clears alarm and returns to set mode
    if (current_button_event != BTN_NONE) {
        current_state = STATE_SET_MODE;
        stopTimer1();
        stopTimer3();       // Stop LED2 blink timer
        minutes = 0;
        seconds = 0;
        LED1_LAT = 0;
        LED2_LAT = 0;
        display_update_needed = 1;
    }
}

void handleDisplayInfo(void) {
    // Return to set mode when buttons released
    if (current_button_event == BTN_RELEASED) {
        current_state = STATE_SET_MODE;
        display_update_needed = 1;
    }
}

void updateLEDs(void) {
    static uint8_t last_second = 0;
    
    if (current_state == STATE_RUNNING) {
        // LED1 blinks: toggle every timer tick
        if (seconds != last_second) {
            LED1_LAT ^= 1;  // Toggle every second
            last_second = seconds;
        }
    }
    else if (current_state == STATE_ALARM) {
        // LED1 solid on, LED2 blinking (handled by Timer3 ISR)
    }
    else {
        // All other states: LEDs off
        LED1_LAT = 0;
        LED2_LAT = 0;
    }
}