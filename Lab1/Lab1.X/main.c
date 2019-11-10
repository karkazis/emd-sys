/*
 * File:   main.c
 * Author: panos
 *
 * Created on October 12, 2019, 11:49 AM
 */


#define BUTTON_DEBOUCE_TIME_MS 20

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
static void ButtonDebounce( void );
unsigned char cnt = 0;

int main(void) {
    LED_Enable_all();
    BUTTON_Enable ( BUTTON_S3 );
    
    /* Get a timer event once every 100ms for the blink alive. */
    TIMER_SetConfiguration ( TIMER_CONFIGURATION_1MS );
    TIMER_RequestTick( &TimerEventHandler, 1000 );
    TIMER_RequestTick( &ButtonDebounce, 1 );
    
    while(1){

        if ( cnt == 128 ){
            cnt = 0;
        }
        LEDS_Set(cnt);   
    }  
}

static void TimerEventHandler(void)
{    
    LED_Toggle( LED_D10 );
}

static void ButtonDebounce(void)
{    
    static uint16_t debounceCounterS3 = 0;

    
    if(BUTTON_IsPressed( BUTTON_S3 ) == true){
        if(debounceCounterS3 == 0)
        {
            LED_Toggle( LED_D9 );
            cnt++;
        }
        debounceCounterS3 = BUTTON_DEBOUCE_TIME_MS;
    } else{
        if(debounceCounterS3 != 0)
        {
            debounceCounterS3--;
        }
    }
    
}