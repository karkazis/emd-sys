/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*******************************************************************************/

#include <xc.h>
#include <stdint.h>

#include "lcd.h"

#ifndef SYSTEM_PERIPHERAL_CLOCK
#define SYSTEM_PERIPHERAL_CLOCK 16000000
#pragma message "This module requires a definition for the peripheral clock frequency.  Assuming 16MHz Fcy (32MHz Fosc).  Define value if this is not correct."
#endif

/* This defines the number of cycles per loop through the delay routine.  Spans
 * between 12-18 depending on optimization mode.*/
#define CYCLES_PER_DELAY_LOOP 12

/* Private Definitions ***********************************************/

/* Define a delay for the various time intervals required between various signal
 * transitions (E pulse, R/W setup, etc.).  This will be rounded up to 1us for 
 * the sake of easy calculation though all of these are less than that
 * typically 60-300ns. */
#define LCD_SIGNAL_TIMING         (((SYSTEM_PERIPHERAL_CLOCK/1000)*1)/1000)/CYCLES_PER_DELAY_LOOP

// Define a fast instruction execution time in terms of loop time
// typically > 40us
#define LCD_F_INSTR         (((SYSTEM_PERIPHERAL_CLOCK/1000)*40)/1000)/CYCLES_PER_DELAY_LOOP

// Define a slow instruction execution time in terms of loop time
// typically > 1.64ms
#define LCD_S_INSTR         (((SYSTEM_PERIPHERAL_CLOCK/1000)*1640)/1000)/CYCLES_PER_DELAY_LOOP

// Define the startup time for the LCD in terms of loop time
// typically > 60ms (double than 8 bit mode)
#define LCD_STARTUP         (((SYSTEM_PERIPHERAL_CLOCK/1000)*60000)/1000)/CYCLES_PER_DELAY_LOOP

#define LCD_MAX_COLUMN      16

#define CS1_BASE_ADDRESS    0x00020000ul
#define CS2_BASE_ADDRESS    0x000A0000ul

#define LCD_SendData(data) { ADDR1 = data; LCD_Wait(LCD_F_INSTR); }
#define LCD_SendCommand(command, delay) { ADDR0 = command; LCD_Wait(delay); }
#define LCD_COMMAND_CLEAR_SCREEN        0x01
#define LCD_COMMAND_RETURN_HOME         0x02
#define LCD_COMMAND_ENTER_DATA_MODE     0x06
#define LCD_COMMAND_CURSOR_OFF          0x0C
#define LCD_COMMAND_CURSOR_ON           0x0F
#define LCD_COMMAND_MOVE_CURSOR_LEFT    0x10
#define LCD_COMMAND_MOVE_CURSOR_RIGHT   0x14
#define LCD_COMMAND_SET_MODE_8_BIT      0x38
#define LCD_COMMAND_ROW_0_HOME          0x80
#define LCD_COMMAND_ROW_1_HOME          0xC0
#define LCD_START_UP_COMMAND_1          0x33    
#define LCD_START_UP_COMMAND_2          0x32    

/* Private Functions *************************************************/
static void LCD_CarriageReturn ( void ) ;
static void LCD_ShiftCursorLeft ( void ) ;
static void LCD_ShiftCursorRight ( void ) ;
static void LCD_ShiftCursorUp ( void ) ;
static void LCD_ShiftCursorDown ( void ) ;
static void LCD_Wait ( uint32_t ) ;

/* Private variables ************************************************/
static uint8_t row ;
static uint8_t column ;
static __eds__ unsigned int __attribute__ ( ( noload , section ( "epmp_cs1" ) , address ( CS1_BASE_ADDRESS ) ) ) ADDR0 __attribute__ ( ( space ( eds ) ) ) ;
static __eds__ unsigned int __attribute__ ( ( noload , section ( "epmp_cs1" ) , address ( CS1_BASE_ADDRESS ) ) ) ADDR1 __attribute__ ( ( space ( eds ) ) ) ;
/*********************************************************************
 * Function: bool LCD_Initialize(void);
 *
 * Overview: Initializes the LCD screen.  Can take several hundred
 *           milliseconds.
 *
 * PreCondition: none
 *
 * Input: None
 *
 * Output: true if initialized, false otherwise
 *
 ********************************************************************/
bool LCD_Initialize ( void )
{
    PMCON1bits.ADRMUX = 0 ;      // address is not multiplexed
    PMCON1bits.MODE = 3 ;        // master mode
    PMCON1bits.CSF = 0 ;         // PMCS1 pin used for chip select 1,
    // PMCS2 pin used for chip select 2
    PMCON1bits.ALP = 0 ;         //
    PMCON1bits.ALMODE = 0 ;      // "smart" address strobes are not used
    PMCON1bits.BUSKEEP = 0 ;     // bus keeper is not used

    // CS1 start address
    PMCS1BS = ( CS1_BASE_ADDRESS >> 8 ) ;

    PMCS1CFbits.CSDIS = 0 ;      // enable CS
    PMCS1CFbits.CSP = 1 ;        // CS1 polarity
    PMCS1CFbits.BEP = 1 ;        // byte enable polarity
    PMCS1CFbits.WRSP = 1 ;       // write strobe polarity
    PMCS1CFbits.RDSP = 1 ;        // read strobe polarity
    PMCS1CFbits.CSPTEN = 1 ;     // enable CS port
    PMCS1CFbits.SM = 0 ;         // read and write strobes on separate lines
    PMCS1CFbits.PTSZ = 2 ;       // data bus width is 16-bit
    PMCS1MDbits.ACKM = 0 ;       // PMACK is not used

    // The device timing parameters. Set the proper timing
    // according to the device used (the timing macros are defined in the hardware profile)
    PMCS1MDbits.DWAITB = 0b11 ;     // access time 3 1/4 Tcy
    PMCS1MDbits.DWAITM = 0b1111 ;
    PMCS1MDbits.DWAITE = 0b11 ;

    PMCON2bits.RADDR = 0 ;       // don't care since CS2 is not be used
    PMCON4 = 0x0001 ;            // PMA0 - PMA15 address lines are enabled

    PMCON3bits.PTWREN = 1 ;      // enable write strobe port
    PMCON3bits.PTRDEN = 1 ;      // enable read strobe port
    PMCON3bits.PTBE0EN = 1 ;     // enable byte enable port
    PMCON3bits.PTBE1EN = 1 ;     // enable byte enable port
    PMCON3bits.AWAITM = 0b11 ;      // set address latch pulses width to 3 1/2 Tcy
    PMCON3bits.AWAITE = 1 ;      // set address hold time to 1 1/4 Tcy

    PMCON1bits.PMPEN = 1 ;       // enable the module

    LCD_Wait ( LCD_STARTUP ) ;
    LCD_Wait ( LCD_STARTUP ) ;
    
    LCD_SendCommand ( LCD_START_UP_COMMAND_1 , LCD_F_INSTR) ;
    LCD_SendCommand ( LCD_START_UP_COMMAND_2 , LCD_F_INSTR) ;
    
    LCD_SendCommand ( LCD_COMMAND_SET_MODE_8_BIT ,     LCD_F_INSTR ) ;
    LCD_SendCommand ( LCD_COMMAND_CURSOR_OFF , LCD_F_INSTR  ) ;
    LCD_SendCommand ( LCD_COMMAND_ENTER_DATA_MODE , LCD_F_INSTR ) ;

    LCD_ClearScreen ( ) ;

    return true ;
}
/*********************************************************************
 * Function: void LCD_PutString(char* inputString, uint16_t length);
 *
 * Overview: Puts a string on the LCD screen.  Unsupported characters will be
 *           discarded.  May block or throw away characters is LCD is not ready
 *           or buffer space is not available.  Will terminate when either a
 *           null terminator character (0x00) is reached or the length number
 *           of characters is printed, which ever comes first.
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: char* - string to print
 *        uint16_t - length of string to print
 *
 * Output: None
 *
 ********************************************************************/
void LCD_PutString ( char* inputString , uint16_t length )
{
    while (length--)
    {
        switch (*inputString)
        {
            case 0x00:
                return ;

            default:
                LCD_PutChar ( *inputString++ ) ;
                break ;
        }
    }
}
/*********************************************************************
 * Function: void LCD_PutChar(char);
 *
 * Overview: Puts a character on the LCD screen.  Unsupported characters will be
 *           discarded.  May block or throw away characters is LCD is not ready
 *           or buffer space is not available.
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: char - character to print
 *
 * Output: None
 *
 ********************************************************************/
void LCD_PutChar ( char inputCharacter )
{
    switch (inputCharacter)
    {
        case '\r':
            LCD_CarriageReturn ( ) ;
            break ;

        case '\n':
            if (row == 0)
            {
                LCD_ShiftCursorDown ( ) ;
            }
            else
            {
                LCD_ShiftCursorUp ( ) ;
            }
            break ;

        case '\b':
            LCD_ShiftCursorLeft ( ) ;
            LCD_PutChar ( ' ' ) ;
            LCD_ShiftCursorLeft ( ) ;
            break ;
            
        case '\f':
            LCD_ClearScreen();
            break;

        default:
            if (column == LCD_MAX_COLUMN)
            {
                column = 0 ;
                if (row == 0)
                {
                    LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME, LCD_F_INSTR ) ;
                    row = 1 ;
                }
                else
                {
                    LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME, LCD_F_INSTR ) ;
                    row = 0 ;
                }
            }
            
            LCD_SendData ( inputCharacter ) ;
            column++ ;
            break ;
    }
}
/*********************************************************************
 * Function: void LCD_ClearScreen(void);
 *
 * Overview: Clears the screen, if possible.
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
void LCD_ClearScreen ( void )
{
    LCD_SendCommand ( LCD_COMMAND_CLEAR_SCREEN , LCD_S_INSTR ) ;
    LCD_SendCommand ( LCD_COMMAND_RETURN_HOME , LCD_S_INSTR) ;

    row = 0 ;
    column = 0 ;
}


/*******************************************************************/
/*******************************************************************/
/* Private Functions ***********************************************/
/*******************************************************************/
/*******************************************************************/
/*********************************************************************
 * Function: static void LCD_CarriageReturn(void)
 *
 * Overview: Handles a carriage return
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_CarriageReturn ( void )
{
    if (row == 0)
    {
        LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME , LCD_F_INSTR) ;
    }
    else
    {
        LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME , LCD_F_INSTR ) ;
    }
    column = 0 ;
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorLeft(void)
 *
 * Overview: Shifts cursor left one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorLeft ( void )
{
    uint8_t i ;

    if (column == 0)
    {
        if (row == 0)
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME , LCD_F_INSTR ) ;
            row = 1 ;
        }
        else
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME , LCD_F_INSTR ) ;
            row = 0 ;
        }

        //Now shift to the end of the row
        for (i = 0 ; i < ( LCD_MAX_COLUMN - 1 ) ; i++)
        {
            LCD_ShiftCursorRight ( ) ;
        }
    }
    else
    {
        column-- ;
        LCD_SendCommand ( LCD_COMMAND_MOVE_CURSOR_LEFT , LCD_F_INSTR ) ;
    }
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorRight(void)
 *
 * Overview: Shifts cursor right one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorRight ( void )
{
    LCD_SendCommand ( LCD_COMMAND_MOVE_CURSOR_RIGHT , LCD_F_INSTR ) ;
    column++ ;

    if (column == LCD_MAX_COLUMN)
    {
        column = 0 ;
        if (row == 0)
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_1_HOME , LCD_F_INSTR ) ;
            row = 1 ;
        }
        else
        {
            LCD_SendCommand ( LCD_COMMAND_ROW_0_HOME , LCD_F_INSTR ) ;
            row = 0 ;
        }
    }
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorUp(void)
 *
 * Overview: Shifts cursor up one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorUp ( void )
{
    uint8_t i ;

    for (i = 0 ; i < LCD_MAX_COLUMN ; i++)
    {
        LCD_ShiftCursorLeft ( ) ;
    }
}
/*********************************************************************
 * Function: static void LCD_ShiftCursorDown(void)
 *
 * Overview: Shifts cursor down one spot (wrapping if required)
 *
 * PreCondition: already initialized via LCD_Initialize()
 *
 * Input: None
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_ShiftCursorDown ( void )
{
    uint8_t i ;

    for (i = 0 ; i < LCD_MAX_COLUMN ; i++)
    {
        LCD_ShiftCursorRight ( ) ;
    }
}
/*********************************************************************
 * Function: static void LCD_Wait(unsigned int B)
 *
 * Overview: A crude wait function that just cycle burns
 *
 * PreCondition: None
 *
 * Input: unsigned int - arbitrary delay time based on loop counts.
 *
 * Output: None
 *
 ********************************************************************/
static void LCD_Wait ( uint32_t delay )
{
    while (delay)
    {
        Nop();
        delay-- ;
    }
}
/*********************************************************************
 * Function: void LCD_CursorEnable(bool enable)
 *
 * Overview: Enables/disables the cursor
 *
 * PreCondition: None
 *
 * Input: bool - specifies if the cursor should be on or off
 *
 * Output: None
 *
 ********************************************************************/
void LCD_CursorEnable ( bool enable )
{
    if (enable == true)
    {
        LCD_SendCommand ( LCD_COMMAND_CURSOR_ON , LCD_F_INSTR ) ;
    }
    else
    {
        LCD_SendCommand ( LCD_COMMAND_CURSOR_OFF , LCD_F_INSTR ) ;
    }
}

