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

#include <stdbool.h>

#include "buttons.h"

/*** Button Definitions *********************************************/
//      S1  is MCLR button
#define S3_PORT  PORTDbits.RD6
#define S6_PORT  PORTDbits.RD7
#define S5_PORT  PORTAbits.RA7       //Overlaps with D10
#define S4_PORT  PORTDbits.RD13

#define S3_TRIS  TRISDbits.TRISD6
#define S6_TRIS  TRISDbits.TRISD7
#define S5_TRIS  TRISAbits.TRISA7
#define S4_TRIS  TRISDbits.TRISD13

#define S3_ANSEL ANSDbits.ANSD6
#define S6_ANSEL ANSDbits.ANSD7
#define S5_ANSEL ANSAbits.ANSA7
//      S4_ANSEL D13 doesn't have an ANSEL

#define BUTTON_PRESSED      0
#define BUTTON_NOT_PRESSED  1

#define PIN_INPUT           1
#define PIN_OUTPUT          0

#define ANALOG      1
#define DIGITAL     0
/*********************************************************************
 * Function: bool BUTTON_IsPressed(BUTTON button);
 *
 * Overview: Returns the current state of the requested button
 *
 * PreCondition: button configured via BUTTON_SetConfiguration()
 *
 * Input: BUTTON button - enumeration of the buttons available in
 *        this demo.  They should be meaningful names and not the names
 *        of the buttons on the silk-screen on the board (as the demo
 *        code may be ported to other boards).
 *         i.e. - ButtonIsPressed(BUTTON_SEND_MESSAGE);
 *
 * Output: true if pressed; false if not pressed.
 *
 ********************************************************************/
bool BUTTON_IsPressed ( BUTTON button )
{
    switch(button)
    {
        case BUTTON_S3:
            return ( (S3_PORT == BUTTON_PRESSED) ? true : false);

        case BUTTON_S6:
            return ( (S6_PORT == BUTTON_PRESSED) ? true : false);

        case BUTTON_S5:
            return ( ( S5_PORT == BUTTON_PRESSED ) ? true : false ) ;

        case BUTTON_S4:
            return ( ( S4_PORT == BUTTON_PRESSED ) ? true : false ) ;

        default:
            return false;
    }
    
    return false;
}

/*********************************************************************
* Function: void BUTTON_Enable(BUTTON button);
*
* Overview: Returns the current state of the requested button
*
* PreCondition: button configured via BUTTON_SetConfiguration()
*
* Input: BUTTON button - enumeration of the buttons available in
*        this demo.  They should be meaningful names and not the names
*        of the buttons on the silk-screen on the board (as the demo
*        code may be ported to other boards).
*         i.e. - ButtonIsPressed(BUTTON_SEND_MESSAGE);
*
* Output: None
*
********************************************************************/
void BUTTON_Enable(BUTTON button)
{
    switch(button)
    {
        case BUTTON_S3:
            S3_TRIS = PIN_INPUT ;
            S3_ANSEL = DIGITAL ;
            break ;

        case BUTTON_S6:
            S6_TRIS = PIN_INPUT;
            S6_ANSEL = DIGITAL ;
            break;

        case BUTTON_S5:
            S5_TRIS = PIN_INPUT ;
            S5_ANSEL = DIGITAL ;
            break ;

        case BUTTON_S4:
            S4_TRIS = PIN_INPUT ;
            //No ANSEL required for D13
            break ;

        default:
            break ;
    }
}
