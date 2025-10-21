/*
 * File:   IOs.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on October 3rd, 2025, 3:43 PM
 */

#include "IO.h"
#include "UART2.h"

volatile uint16_t button_state_changed = 0;
volatile uint16_t current_button_state = NO_BUTTON;

void IOinit(void) {
    AD1PCFG = 0xFFFF;
    
    TRISBbits.TRISB9 = 0;
    LATBbits.LATB9 = 0;
    
    TRISBbits.TRISB7 = 1;
    CNPU2bits.CN23PUE = 1;
    CNEN2bits.CN23IE = 1;
    
    TRISBbits.TRISB4 = 1;
    CNPU1bits.CN1PUE = 1;
    CNEN1bits.CN1IE = 1;
    
    TRISAbits.TRISA4 = 1;
    CNPU1bits.CN0PUE = 1;
    CNEN1bits.CN0IE = 1;
    
    IPC4bits.CNIP = 6;
    IFS1bits.CNIF = 0;
    IEC1bits.CNIE = 1;
    
    // Timer 2 for debounce
    T2CONbits.TCKPS = 1;
    T2CONbits.TCS = 0;
    T2CONbits.TSIDL = 0;
    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 1;
    PR2 = 1562;  // 50ms: (250kHz/8) * 0.05s = 1562.5
    TMR2 = 0;
    T2CONbits.TON = 0;
    
    // Timer 3 for LED blinking
    T2CONbits.T32 = 0;
    T3CONbits.TCKPS = 2;   // *** 1:64 prescaler *** Timer frequency = 250 kHz ÷ 64 = 3,906.25 Hz
    // We cannot use the 1:8 prescaler because our 3rd push button toggle period is too long for the 1:8 prescaler (Timer frequency = 250 kHz ÷ 8 = 31,250 Hz)
    //    For toggle periods:
    //PB1 (0.25s): 31,250 × 0.25 = 7,812
    //PB2 (1s): 31,250 × 1 = 31,250
    //PB3 (3s): 31,250 × 3 = 93,750 ---- exceeds 65,535!
    T3CONbits.TCS = 0;
    T3CONbits.TSIDL = 0;
    IPC2bits.T3IP = 2;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    TMR3 = 0;
    T3CONbits.TON = 0;
}

void IOcheck(void) {
    static uint16_t last_state = 0xFF;
    
    if (button_state_changed) {
        button_state_changed = 0;
        
        if (current_button_state != last_state) {
            Disp2String("\033[2K\r");
            
            switch(current_button_state) {
                case PB1_ONLY:
                    Disp2String("PB1 is pressed");
                    // 250kHz/64 = 3,906.25 Hz; 0.25s toggle: 3,906.25 * 0.25s = 976
                    setTimerRate(976);
                    break;
                    
                case PB2_ONLY:
                    Disp2String("PB2 is pressed");
                    // 1s toggle: 3,906.25 Hz * 1s = 3,906.25
                    setTimerRate(3906);
                    break;
                    
                case PB3_ONLY:
                    Disp2String("PB3 is pressed");
                    // 3s toggle: 3,906.25 Hz * 3s = 11719
                    setTimerRate(11719);
                    break;
                    
                case PB1_PB2:
                    Disp2String("PB1 and PB2 are pressed");
                    stopTimer();
                    LATBbits.LATB9 = 1;
                    break;
                    
                case PB1_PB3:
                    Disp2String("PB1 and PB3 are pressed");
                    stopTimer();
                    LATBbits.LATB9 = 1;
                    break;
                    
                case PB2_PB3:
                    Disp2String("PB2 and PB3 are pressed");
                    stopTimer();
                    LATBbits.LATB9 = 1;
                    break;
                    
                case ALL_PBS:
                    Disp2String("All PBs pressed");
                    stopTimer();
                    LATBbits.LATB9 = 1;
                    break;
                    
                case NO_BUTTON:
                default:
                    Disp2String("Nothing pressed");
                    stopTimer();
                    LATBbits.LATB9 = 0;
                    break;
            }
            
            last_state = current_button_state;
        }
    }
}

void setTimerRate(uint16_t rate) {
    T3CONbits.TON = 0;
    TMR3 = 0;
    PR3 = rate;
    T3CONbits.TON = 1;
}

void stopTimer(void) {
    T3CONbits.TON = 0;
    TMR3 = 0;
}