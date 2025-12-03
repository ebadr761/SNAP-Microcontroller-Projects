/*
 * File:   main.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 * Project: ENSF 460 - Project 2: LED Intensity Controller
 *
 * Created on November 22, 2025
 */

// Configuration Bits
#pragma config BWRP = OFF
#pragma config BSS = OFF
#pragma config GWRP = OFF
#pragma config GCP = OFF
#pragma config FNOSC = FRC
#pragma config IESO = OFF
#pragma config POSCMOD = NONE
#pragma config OSCIOFNC = ON
#pragma config POSCFREQ = HS
#pragma config SOSCSEL = SOSCHP
#pragma config FCKSM = CSECMD
#pragma config WDTPS = PS32768
#pragma config FWPSA = PR128
#pragma config WINDIS = OFF
#pragma config FWDTEN = OFF
#pragma config BOREN = BOR3
#pragma config PWRTEN = ON
#pragma config I2C1SEL = PRI
#pragma config BORV = V18
#pragma config MCLRE = ON

#include <xc.h>
#include <stdint.h>
#include <string.h>
#include "adc.h"
#include "buttons.h"
#include "pwm.h"
#include "UART2.h"
#include "clkChange.h"

// System States
typedef enum {
    STATE_OFF = 0,
    STATE_ON,
    STATE_BLINK
} SystemState;

// Global Variables
volatile SystemState current_state = STATE_OFF;
volatile SystemState pre_blink_state = STATE_OFF;  // Track state before blinking
volatile uint8_t active_led = 1;        // 1 = LED1 (RB9), 2 = LED2 (RA6)
volatile uint8_t is_blinking = 0;
volatile uint8_t uart_transmitting = 0;
volatile uint32_t blink_counter = 0;    // counts Timer3 ticks while blinking
volatile uint8_t blink_state = 0;       // 0 = LED on phase, 1 = LED off phase

// Timer3 tick flag (drives UART rate and helps blink timing)
volatile uint8_t timer3_flag = 0;

// LED pin definitions
#define LED1_TRIS TRISBbits.TRISB9
#define LED2_TRIS TRISAbits.TRISA6
#define LED1_LAT  LATBbits.LATB9
#define LED2_LAT  LATAbits.LATA6

// Function Prototypes
void initHardware(void);
void initTimer3(void);
void handleButtonEvent(ButtonEvent event);
void updateLEDState(void);
void transmitUARTData(void);

// ---------------------------------------------------------------------------

int main(void) {
    // Initialize system
    newClk(8); // Set clock to 8MHz
    initHardware();
    
    while(1) {
        // Check for button events
        ButtonEvent event = getButtonEvent();
        if (event != BTN_NONE) {
            handleButtonEvent(event);
        }
        
        // Update LED state based on current mode
        updateLEDState();
        
        // Auto-stop UART if we're not in ON or BLINK mode
        if (uart_transmitting && (current_state == STATE_OFF)) {
            uart_transmitting = 0;
            if (!is_blinking) {
                T3CONbits.TON = 0;
                TMR3 = 0;
            }
        }
        
        // Handle UART transmission if active (Timer3 ticks at ~100 ms)
        if (uart_transmitting && timer3_flag) {
            timer3_flag = 0;
            transmitUARTData();
        }
        
        // Enter low power mode when system is fully OFF and idle
        if (current_state == STATE_OFF &&
            !uart_transmitting &&
            !is_blinking &&
            !timer3_flag) {
            Idle();     // sleep until next interrupt
        }
    }
    
    return 0;
}

// ---------------------------------------------------------------------------

void initHardware(void) {
    // AD1PCFG is configured in initADC()
    
    // Configure LED pins as outputs
    LED1_TRIS = 0;   // LED1 (RB9)
    LED2_TRIS = 0;   // LED2 (RA6)
    
    // Turn LEDs off initially
    LED1_LAT = 0;
    LED2_LAT = 0;
    
    // Initialize peripherals
    initADC();
    initButtons();
    initPWM();
    initTimer3();
    
    // Start in OFF mode
    current_state = STATE_OFF;
    pre_blink_state = STATE_OFF;
    active_led = 1;
    is_blinking = 0;
    uart_transmitting = 0;
    blink_counter = 0;
    blink_state = 0;
}

// ---------------------------------------------------------------------------

void initTimer3(void) {
    // Timer3 for 100 ms intervals (used for blink timing & UART sampling)
    // At 8 MHz oscillator, Fcy = 4 MHz
    // Timer clock with prescaler 1:256: 4,000,000 / 256 = 15,625 Hz
    // For 100 ms: counts = 15,625 * 0.1 ? 1,562.5 ? use PR3 = 1561 (0..1561 = 1562 ticks)
    
    T3CONbits.TON = 0;      // Stop timer
    T3CONbits.TCKPS = 3;    // 1:256 prescaler
    T3CONbits.TCS = 0;      // Internal clock
    T3CONbits.TSIDL = 0;    // Continue in idle
    
    TMR3 = 0;
    PR3 = 1561;             // ~100 ms at 8 MHz with 1:256 prescaler
    
    IPC2bits.T3IP = 4;      // Medium priority
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    
    T3CONbits.TON = 0;      // Don't start yet
}

// ---------------------------------------------------------------------------

void handleButtonEvent(ButtonEvent event) {
    switch(event) {
        case BTN_PB1_CLICK:
            // Toggle between ON and OFF modes
            if (current_state == STATE_OFF) {
                // Enter ON MODE
                current_state = STATE_ON;
                is_blinking = 0;
                
                // Read potentiometer and set PWM
                {
                    uint16_t adc_val = readADC();
                    uint8_t intensity = adcToIntensity(adc_val);
                    setPWMDutyCycle(intensity);
                    startPWM();
                }
            } 
            else if (current_state == STATE_ON || current_state == STATE_BLINK) {
                // Enter OFF MODE
                current_state = STATE_OFF;
                is_blinking = 0;
                stopPWM();
                LED1_LAT = 0;
                LED2_LAT = 0;
                
                // Stop UART transmission if active
                if (uart_transmitting) {
                    uart_transmitting = 0;
                }
                
                // Stop blink timer
                T3CONbits.TON = 0;
                TMR3 = 0;
            }
            break;
            
        case BTN_PB1_LONG:
            // Long press: swap active LED (only in ON or BLINK modes)
            if (current_state == STATE_ON || current_state == STATE_BLINK) {
                active_led = (active_led == 1) ? 2 : 1;
            }
            break;
            
        case BTN_PB2_CLICK:
            if (current_state == STATE_OFF) {
                // Blink at 100% intensity in OFF mode
                pre_blink_state = STATE_OFF;  // Remember we came from OFF
                current_state = STATE_BLINK;
                is_blinking = 1;
                blink_state = 0;              // Start with LED on
                blink_counter = 0;            // Reset blink timing

                // Set 100% duty cycle
                setPWMDutyCycle(100);
                startPWM();
                
                // Start blink timer
                TMR3 = 0;
                T3CONbits.TON = 1;
            }
            else if (current_state == STATE_ON) {
                // Start blinking at current intensity
                pre_blink_state = STATE_ON;   // Remember we came from ON
                current_state = STATE_BLINK;
                is_blinking = 1;
                blink_state = 0;              // Start with LED on
                blink_counter = 0;            // Reset blink timing
                
                // PWM already running, just start timer
                TMR3 = 0;
                T3CONbits.TON = 1;
            }
            else if (current_state == STATE_BLINK) {
                // Stop blinking - return to state we came from
                is_blinking = 0;
                current_state = pre_blink_state;  // Return to OFF or ON
                
                // Stop blink timer ONLY if UART is not transmitting
                if (!uart_transmitting) {
                    T3CONbits.TON = 0;
                    TMR3 = 0;
                }
                
                if (pre_blink_state == STATE_OFF) {
                    // Return to OFF - turn everything off
                    stopPWM();
                    LED1_LAT = 0;
                    LED2_LAT = 0;
                } else {
                    // Return to ON - ensure LED is on at correct intensity
                    uint16_t adc_val = readADC();
                    uint8_t intensity = adcToIntensity(adc_val);
                    setPWMDutyCycle(intensity);
                    startPWM();
                }
            }
            break;
            
        case BTN_PB3_CLICK:
            // Toggle UART transmission (only in ON or ON-derived BLINK modes)
            if (!uart_transmitting &&
                (current_state == STATE_ON ||
                 (current_state == STATE_BLINK && pre_blink_state == STATE_ON))) {
                // Start UART transmission
                uart_transmitting = 1;
                // Start timer for UART transmission rate (reuse Timer3)
                if (!is_blinking) {
                    TMR3 = 0;
                    T3CONbits.TON = 1;
                }
            }
            else if (uart_transmitting) {
                // Stop UART transmission (can stop from any state)
                uart_transmitting = 0;
                // Stop timer if not blinking
                if (!is_blinking) {
                    T3CONbits.TON = 0;
                    TMR3 = 0;
                }
            }
            break;
            
        default:
            break;
    }
}

// ---------------------------------------------------------------------------

void updateLEDState(void) {
    static uint16_t adc_update_counter = 0;
    static uint16_t last_adc_val = 512;
    static uint8_t last_intensity = 50;
    static uint8_t last_blink_state = 0xFF;   // invalid initial to force update
    
    if (current_state == STATE_OFF) {
        // LEDs off in OFF mode
        if (!is_blinking) {
            LED1_LAT = 0;
            LED2_LAT = 0;
        }
    }
    else if (current_state == STATE_ON) {
        // Read potentiometer every N iterations (not every single loop)
        adc_update_counter++;
        if (adc_update_counter > 100) {
            adc_update_counter = 0;
            last_adc_val = readADC();
            last_intensity = adcToIntensity(last_adc_val);
            setPWMDutyCycle(last_intensity);
        }
    }
    else if (current_state == STATE_BLINK) {
        // Check if blink state changed (set by Timer3 ISR)
        if (blink_state != last_blink_state) {
            last_blink_state = blink_state;
            
            if (blink_state == 0) {
                // LED on phase
                if (pre_blink_state == STATE_OFF) {
                    // Blinking from OFF mode - always 100%
                    setPWMDutyCycle(100);
                } else {
                    // Blinking from ON mode - use pot value
                    last_adc_val = readADC();
                    last_intensity = adcToIntensity(last_adc_val);
                    setPWMDutyCycle(last_intensity);
                }
            } else {
                // LED off phase
                setPWMDutyCycle(0);
            }
        }
    }
}

// ---------------------------------------------------------------------------

void transmitUARTData(void) {
    if (!uart_transmitting) {
        return;
    }
    
    // Read current intensity
    uint16_t adc_val = readADC();
    uint8_t intensity;
    
    if (is_blinking && blink_state == 1) {
        // LED is off during blink
        intensity = 0;
    } else {
        intensity = adcToIntensity(adc_val);
    }
    
    // Transmit intensity
    Disp2Dec(intensity);
    Disp2String("%,");
    
    // Transmit ADC value
    Disp2Dec(adc_val);
    Disp2String("\n");
}

// ---------------------------------------------------------------------------

// Timer3 ISR - Handles blink timing and UART transmission rate
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void) {
    IFS0bits.T3IF = 0;
    timer3_flag = 1;    // signal main loop every ~100 ms
    
    if (is_blinking) {
        // Accumulate 100 ms ticks; toggle blink_state every 500 ms (5 ticks)
        blink_counter++;
        if (blink_counter >= 5) {
            blink_counter = 0;
            blink_state = !blink_state;
        }
    }
}
