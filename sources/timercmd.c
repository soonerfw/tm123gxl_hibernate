
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/ssi.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/uart.h"
#include "inc/hw_types.h"

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3
		

//*****************************************************************************
//
// The interrupt handler for the first timer interrupt.
//
//*****************************************************************************
void Timer0IntHandler(void)
{
//  uint32_t ui32ADC0Value;
//	uint8_t channel[10]="Channel  :"; 
	static uint32_t tickCount=0;
	static bool greenLEDStatus = false;	
//	uint32_t time1, time2, time_elapse; 
//	float adc_time;
    //
    // Clear the timer interrupt.
    //
  MAP_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
		

	tickCount++;

	if(tickCount%5 ==0)
	{
			  greenLEDStatus ^=1; 
			  if(greenLEDStatus)
				{				
					// Turn off the LED.
					//
					GPIOPinWrite(GPIO_PORTF_BASE, GREEN_LED, 0x0);
				}
		    else					
				{			
					// Turn on the LED.
					//
					GPIOPinWrite(GPIO_PORTF_BASE, GREEN_LED, GREEN_LED);					
					
				}
	}	
	
	if(tickCount%20 ==0) // every two seconds, the sensor data is transmitted
	{
		MAP_UARTCharPutNonBlocking(UART0_BASE, '1');
		
	}
	
}


