/*
 * File:   main.c
 * Author: panos
 *
 * Created on November 1, 2019, 6:54 PM
 */


#include "xc.h"
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>

#include <libpic30.h>
#include "bsp/leds.h"
#include "bsp/timer_1ms.h"
#include "bsp/uart2.h"

static void TimerEventHandler( void );

int main(void) {
    LED_Enable ( LED_D10 );
    /* Get a timer event once every 100ms for the blink alive. */
    TIMER_SetConfiguration ( TIMER_CONFIGURATION_1MS );
    TIMER_RequestTick( &TimerEventHandler, 1000 );
    initU2();
    while(1)
    {

    }

}

static void TimerEventHandler(void)
{    
    LED_Toggle( LED_D10 );
    char* buf = "12345"; 
    write(2, buf, 5);

}