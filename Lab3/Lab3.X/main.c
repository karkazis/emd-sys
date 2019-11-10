/*
 * File:   main.c
 * Author: panos
 *
 * Created on November 1, 2019, 6:54 PM
 */


#include "xc.h"
#define FP 40000000
#define BAUDRATE 9600
#define BRGVAL ((FP/BAUDRATE)/16)-1
#define DELAY_105uS asm volatile ("REPEAT, #4201"); Nop(); // 105uS delay

int main(void) {
    U2MODEbits.STSEL = 0; // 1-Stop bit
    U2MODEbits.PDSEL = 0; // No Parity, 8-Data bits
    U2MODEbits.ABAUD = 0; // Auto-Baud disabled
    U2MODEbits.BRGH = 0; // Standard-Speed mode
    U2BRG = BRGVAL; // Baud Rate setting for 9600
    U2STAbits.UTXISEL0 = 0; // Interrupt after one TX character is transmitted
    U2STAbits.UTXISEL1 = 0;
    IEC1bits.U2TXIE = 1; // Enable UART TX interrupt
    U2MODEbits.UARTEN = 1; // Enable UART
    U2STAbits.UTXEN = 1; // Enable UART TX
    
    U2TXREG = 'a'; // Transmit one character
    
    while(1)
    {
    }

}

void __attribute__((__interrupt__, auto_psv)) _U2TXInterrupt(void)
{
    IFS1bits.U2TXIF = 0; // Clear TX Interrupt flag
    U2TXREG = 'a'; // Transmit one character
}