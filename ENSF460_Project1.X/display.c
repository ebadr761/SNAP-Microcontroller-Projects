/*
 * File:   display.c
 * Author: Mazin Taher, Ebad Rehman, Taha Zuberi
 *
 * Created on October 13, 2025, 2:32 PM
 */


#include "display.h"
#include "UART2.h"
#include <stdio.h>
#include <string.h>

// Helper function to convert number to 2-digit string
static void numToStr(uint8_t num, char *str) {
    str[0] = '0' + (num / 10);
    str[1] = '0' + (num % 10);
    str[2] = '\0';
}

void displaySetMode(uint8_t min, uint8_t sec) {
    char min_str[3];
    char sec_str[3];
    
    numToStr(min, min_str);
    numToStr(sec, sec_str);
    
    // Clear line and display
    Disp2String("\033[2K\r");  // Clear line
    Disp2String("SET ");
    Disp2String(min_str);
    Disp2String("m : ");
    Disp2String(sec_str);
    Disp2String("s");
}

void displayCountdown(uint8_t min, uint8_t sec) {
    char min_str[3];
    char sec_str[3];
    
    numToStr(min, min_str);
    numToStr(sec, sec_str);
    
    // Clear line and display
    Disp2String("\033[2K\r");  // Clear line
    Disp2String("CNT ");
    Disp2String(min_str);
    Disp2String("m : ");
    Disp2String(sec_str);
    Disp2String("s");
}

void displayFinished(void) {
    Disp2String("\033[2K\r");  // Clear line
    Disp2String("FIN 00m : 00s - ALARM");
}

void displayClear(void) {
    Disp2String("\033[2K\r");  // Clear line
    Disp2String("CLR 00m : 00s");
}

void displayGroupInfo(void) {
    Disp2String("\033[2K\r");  // Clear line
    Disp2String("2025 ENSF 460 L02 - Group 16");
}
