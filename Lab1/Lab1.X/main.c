/*
 * File:   main.c
 * Author: panos
 *
 * Created on October 12, 2019, 11:49 AM
 */


#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include "bsp/adc.h"
#include "bsp/timer_1ms.h"
#include "bsp/buttons.h"
#include "bsp/leds.h"

static void TimerEventHandler( void );


int main(void) {
    LED_Enable ( LED_D10 );
    LED_Enable ( LED_D3 );
    
    BUTTON_Enable ( BUTTON_S3 );
    
    /* Get a timer event once every 100ms for the blink alive. */
    TIMER_SetConfiguration ( TIMER_CONFIGURATION_1MS );
    TIMER_RequestTick( &TimerEventHandler, 1000 );
    
    while(1){
     
        if(BUTTON_IsPressed( BUTTON_S3 ) == true)
        {
            LED_On( LED_D3 );
        }
        else
        {
            LED_Off( LED_D3 );
        }
    }
        
}

static void TimerEventHandler(void)
{    
    LED_Toggle( LED_D10 );
}