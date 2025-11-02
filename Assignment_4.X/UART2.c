/*
 * File:   UART2.c
 * Author: Rushi V
 *
 * Created on October 18, 2020, 9:32 PM
 */


#include "xc.h"
#include "math.h"
#include "string.h"

#include "UART2.h"




unsigned int clkval;

///// Initialization of UART 2 module.

void InitUART2(void) 
{
	// configures UART2 module on pins RB0 (Tx) and RB1 (Rx) on PIC24F16KA101 
	// Enables UART2 
	//Set to Baud 4800 with 500kHz clk on PIC24F
	
	TRISBbits.TRISB0=0;
	TRISBbits.TRISB1=1;
	LATBbits.LATB0=1;

	// configure U2MODE
    U2MODE = 0b0000000000001000;
	if (OSCCONbits.COSC == 0b110)
	{
		U2BRG = 12;	// gives a baud rate of 4807.7 Baud with 500kHz clock; Set Baud to 4800 on realterm
	}
	else if (OSCCONbits.COSC == 0b101)
	{
		U2BRG = 12;	// gives a baud rate of 300 Baud with 32kHz clock; set Baud to 300 on realterm
	}
	else if (OSCCONbits.COSC == 0b000)
	{
		U2BRG=103;	// gives a baud rate of 9600 with 8MHz clock; set Baud to 9600 on real term
	}
	// Load all values in for U1STA SFR
	U2STA = 0b1010000000000000;
	IFS1bits.U2TXIF = 0;	// Clear the Transmit Interrupt Flag
    IPC7bits.U2TXIP = 3; // UART2 TX interrupt has interrupt priority 3-4th highest priority
    
	IEC1bits.U2TXIE = 1;	// Enable Transmit Interrupts
	IFS1bits.U2RXIF = 0;	// Clear the Recieve Interrupt Flag
	IPC7bits.U2RXIP = 4; //UART2 Rx interrupt has 2nd highest priority
    IEC1bits.U2RXIE = 0;	// Disable Recieve Interrupts

	U2MODEbits.UARTEN = 1;	// And turn the peripheral on

	U2STAbits.UTXEN = 1;
	return;
}



///// XmitUART2: 
///// Displays 'DispData' on realterm 'repeatNo' of times using UART to PC. 
///// Adjust Baud on real term as per clock: 32kHz clock - Baud=300 // 500kHz clock - Baud=4800 

void XmitUART2(char CharNum, unsigned int repeatNo)
{	
	while(repeatNo!=0) 
	{
		while(U2STAbits.UTXBF==1)	//Just loop here till the FIFO buffers have room for one more entry
		{
			// Idle();  //commented to try out serialplot app
		}	
		U2TXREG=CharNum;	//Move Data to be displayed in UART FIFO buffer
		repeatNo--;
	}
	while(U2STAbits.TRMT==0)	//Turn off UART2 upon transmission of last character; also can be Verified in interrupt subroutine U2TXInterrupt()
	{
		//Idle();
	}
	return;
}


void __attribute__ ((interrupt, no_auto_psv)) _U2RXInterrupt(void) {
//	LATA = U2RXREG;
	IFS1bits.U2RXIF = 0;
}
void __attribute__ ((interrupt, no_auto_psv)) _U2TXInterrupt(void) {
	IFS1bits.U2TXIF = 0;

}




// Displays 16 bit number in Hex form using UART2
void Disp2Hex(unsigned int DispData)   
{
    char i;
    char nib = 0x00;
    XmitUART2(' ',1);  // Disp Gap
    XmitUART2('0',1);  // Disp Hex notation 0x
    XmitUART2('x',1);
    
    for (i=3; i>=0; i--)
    {
        nib = ((DispData >> (4*i)) & 0x000F);
        if (nib >= 0x0A)
        {
            nib = nib +0x37;  //For Hex values A-F
        }
        else 
        {
            nib = nib+0x30;  //For hex values 0-9
        }
        XmitUART2(nib,1);
    }
    
    XmitUART2(' ',1);
    DispData = 0x0000;  // Clear DispData
    return;
}


void Disp2Hex32(unsigned long int DispData32)   // Displays 32 bit number in Hex form using UART2
{
    char i;
    char nib = 0x00;
    XmitUART2(' ',1);  // Disp Gap
    XmitUART2('0',1);  // Disp Hex notation 0x
    XmitUART2('x',1);
    
    for (i=7; i>=0; i--)
    {
        nib = ((DispData32 >> (4*i)) & 0x000F);
        if (nib >= 0x0A)
        {
            nib = nib +0x37;  //For Hex values A-F
        }
        else 
        {
            nib = nib+0x30;  //For hex values 0-9
        }
        XmitUART2(nib,1);
    }
    
    XmitUART2(' ',1);
    DispData32 = 0x00000000;  // Clear DispData
    return;
}

// Displays 16 bit unsigned in in decimal form
void Disp2Dec(uint16_t Dec_num)
{
    uint8_t rem;  //remainder in div by 10
    uint16_t quot; 
    uint8_t ctr = 0;  //counter
    XmitUART2(' ',1);  // Disp Gap
    while(ctr<5)
    {
        quot = Dec_num/(pow(10,(4-ctr)));
        rem = quot%10;
        XmitUART2(rem + 0x30 , 1);
        ctr = ctr + 1;
    }
    XmitUART2(' ',1);  // Disp Gap
   
    return;
}


void Disp2String(char *str) //Displays String of characters
{
    unsigned int i;
    for (i=0; i<= strlen(str); i++)
    {
          
        XmitUART2(str[i],1);
    }
    return;
}
