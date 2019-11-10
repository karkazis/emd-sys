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
#include "bsp/lcd.h"

static void TimerEventHandler( void );


int main(void) {
    uint16_t pot, temp;
    
    /*Enable as Leds 10 and 3*/
    LED_Enable ( LED_D10 );
    LED_Enable ( LED_D3 );
    
    /*Enable Button 3*/
    BUTTON_Enable ( BUTTON_S3 );
    
    /* Get a timer event once every 100ms for the blink alive. */
    TIMER_SetConfiguration ( TIMER_CONFIGURATION_1MS );
    TIMER_RequestTick( &TimerEventHandler, 1000 );
    
    /*Initiate AD*/
    ADC_SetConfiguration ( ADC_CONFIGURATION_DEFAULT );
    ADC_ChannelEnable ( ADC_CHANNEL_POTENTIOMETER );
    ADC_ChannelEnable ( ADC_CHANNEL_TEMPERATURE_SENSOR );
    
    /*Initiate LCD*/
    LCD_Initialize ( ) ;
    
    /* Clear the screen */
    printf( "\f" );  
    
    while(1){
        pot = ADC_Read10bit( ADC_CHANNEL_POTENTIOMETER );
        temp = ADC_Read10bit( ADC_CHANNEL_TEMPERATURE_SENSOR );

        printf("Embedded SYS Lab\r\nP=%4d T=%4d\r\n", pot, temp);

        if(pot > 512)
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

