
// Runs on LM4F120/TM4C123

// hardware connections
// **********ST7735 TFT and SDC*******************
// Sharp 128x128 Memory LCD BOOSTXL-SHARP 128 LCD interface

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

// **********wide.hk ST7735R with ADXL345 accelerometer *******************
// Silkscreen Label (SDC side up; LCD side down) - Connection
// VCC  - +3.3 V
// GND  - Ground
// !SCL - PA2 Sclk SPI clock from microcontroller to TFT or SDC
// !SDA - PA5 MOSI SPI data from microcontroller to TFT or SDC
// DC   - PA6 TFT data/command
// RES  - PA7 TFT reset
// CS   - PA3 TFT_CS, active low to enable TFT
// *CS  - (NC) SDC_CS, active low to enable SDC
// MISO - (NC) MISO SPI data from SDC to microcontroller
// SDA  – (NC) I2C data for ADXL345 accelerometer
// SCL  – (NC) I2C clock for ADXL345 accelerometer
// SDO  – (NC) I2C alternate address for ADXL345 accelerometer
// Backlight + - Light, backlight connected to +3.3 V

// **********wide.hk ST7735R with ADXL335 accelerometer *******************
// Silkscreen Label (SDC side up; LCD side down) - Connection
// VCC  - +3.3 V
// GND  - Ground
// !SCL - PA2 Sclk SPI clock from microcontroller to TFT or SDC
// !SDA - PA5 MOSI SPI data from microcontroller to TFT or SDC
// DC   - PA6 TFT data/command
// RES  - PA7 TFT reset
// CS   - PA3 TFT_CS, active low to enable TFT
// *CS  - (NC) SDC_CS, active low to enable SDC
// MISO - (NC) MISO SPI data from SDC to microcontroller
// X– (NC) analog input X-axis from ADXL335 accelerometer
// Y– (NC) analog input Y-axis from ADXL335 accelerometer
// Z– (NC) analog input Z-axis from ADXL335 accelerometer
// Backlight + - Light, backlight connected to +3.3 V


// This program receives the information from UART1 and UART 7.
// UART1 from LPV transmitter - for auto/manual mode, mission task #, color pick # and other commands
// UART7 deploy sensor data 


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "time.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_hibernate.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/hibernate.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

#include "funcs.h"

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

void DelayWait10ms(uint32_t n);

uint32_t g_ui32SysClock; 


//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    MAP_GPIOPinConfigure(GPIO_PA0_U0RX);
    MAP_GPIOPinConfigure(GPIO_PA1_U0TX);
    MAP_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Use the internal 16MHz oscillator as the UART clock source.
    //
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, 16000000);
}

//*****************************************************************************
//
// This function is the interrupt handler for the Hibernation Module.  When
// entered, it checks which interrupt occurred, clears all pending Hibernation
// interrupts, and then toggles an LED based on whether the device was woken
// by a button press or the RTC timer reaching the count match.
//
//*****************************************************************************
void
HibernateISRHandler(void)
{
    uint32_t ui32Status;
    float   sensor_temp = 98.123; 
	  char str[80]; // for output to store sensor temperatures
    //
    // Get the interrupt status and clear any pending interrupts.
    //
    ui32Status = MAP_HibernateIntStatus(true);
    MAP_HibernateIntClear(ui32Status);

    //
    // Process the RTC match 0 interrupt.
    //
    if(ui32Status & HIBERNATE_INT_RTC_MATCH_0)
    {
        uint32_t ui32RTCMatch;

        //
        // Reads from the Hibernation RTC Counter (HIBRTCC) and Hibernation RTC
        // Sub Seconds (HIBRTCSS) registers may not be correct per Errata Item
        // HIB#02.
        //
        // The following code block is the workaround to get the correct
        // HIBRTCC value.  To get the value of both HIBRTCC and HIBRTCSS,
        // refer to the errata document for a variant of the workaround.
        //

        //
        // Disable Interrupts.
        //
        MAP_IntMasterDisable();

        //
        // To get the individual value for the HIBRTCC register, use the
        // following method.
        //
        do
        {
            ui32RTCMatch = HWREG(HIB_RTCC);
        } while (ui32RTCMatch != HWREG(HIB_RTCC));

        //
        // Re-enable interrupts.
        //
        MAP_IntMasterEnable();
	
        UARTprintf("Waking from Hiberation, current RTC count %d seconds.\n",
                   ui32RTCMatch);
				// assuming the sensor temperature is 98.123 F
				sprintf(str,"Sensor temperature = %6.2f\n",sensor_temp); 
	      UARTprintf("Sensor Temperature %3s\n",str);

				MAP_UARTCharPut(UART0_BASE,0x30);
				MAP_UARTCharPut(UART0_BASE,0x31);
				MAP_UARTCharPut(UART0_BASE,0x0d);
				MAP_UARTCharPut(UART0_BASE,0x0a);
        //
        // Set next interrupt for 5 seconds in future.
        //
        MAP_HibernateRTCMatchSet(0, ui32RTCMatch + 5);

        //
        // Toggle the current state of the Blue LED.
        //
        if(MAP_GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2))
        {
            MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, 0x00);
        }
        else
        {
            MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);
        }
        //
        // Request Hibernation.
        //
        MAP_HibernateRequest();
    }
}


uint32_t g_ui32SysClock; 
int main(void){  

    uint32_t ui32Status;

    //
    // 
    //
    MAP_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);

		  g_ui32SysClock = MAP_SysCtlClockGet();
	
    //
    // Enable the hibernate module.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_HIBERNATE);

    //
    // Read and clear any status bits that might have been set since
    // last clearing them.
    //
    if(MAP_HibernateIsActive())
    {
        //
        // Read the status bits to see what caused the wake.
        //
        ui32Status = MAP_HibernateIntStatus(0);
        MAP_HibernateIntClear(ui32Status);
    }

    //
    // Set up the serial console to use for displaying messages.
    //
    ConfigureUART();
    UARTprintf("Hibernation with RTC Wake Example.\n");

    //
    // Enable the GPIO peripheral for the on-board LED.
    //
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    //
    // Configure the GPIOs for the RGB LED.
    //
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 |
                                               GPIO_PIN_1);

    //
    // Enable the Hibernation module for operation.
    //
    MAP_HibernateEnableExpClk(0);

    //
    // Wait for 32.768kHz clock to stabilize.
    //
    while(!(HWREG(HIB_RIS) & HIB_RIS_WC));

    //
    // Configure the drive strength for the crystal.
    //
    MAP_HibernateClockConfig(HIBERNATE_OSC_LOWDRIVE);

    //
    // Enable the Hibernate module RTC mode.
    //
    MAP_HibernateRTCEnable();

    //
    // Load initial RTC value.
    //
    MAP_HibernateRTCSet(0);

    //
    // Set initial match value to trigger RTC after 5 seconds.
    //
    MAP_HibernateRTCMatchSet(0, 5);

    //
    // Enable RTC match interrupt.
    //
    MAP_HibernateIntEnable(HIBERNATE_INT_RTC_MATCH_0);

    //
    // Re-intializes the Trim which gets modified due to a known issue of
    // Hibernate register initialization from HIB#01 errata.
    //
    MAP_HibernateRTCTrimSet(0x7FFF);

    //
    // Configure MCU interrupts.
    //
    MAP_IntEnable(INT_HIBERNATE_TM4C123);

    //
    // Enable MCU interrupts.
    //
    MAP_IntMasterEnable();

    //
    // Turn on the Blue LED.
    //
    MAP_GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);

    //
    // Enter Hibernation Mode.
    //
    UARTprintf("Entering Hiberation, waking in 5 seconds.\n");
    MAP_HibernateRequest();

    while(1)
    {
        //
        // Do nothing, Hibernate interrupt routine will handle the rest.
        //
    }
	}

