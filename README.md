## TM4C123GXL Hibernate Project
The CPU will be in hibernate mode and be waken up every 5 seconds. Inside the hibernate 
This has been tested on TM4C 123GXL launchpad,inside the hibernate ISR, it can activate ADC,, read the value and send it through UART terminal, and then back to hibernate

every 5 seconds, the MCU will wake up, and sample the sensor data and send it through serial port.

The MCU is programmed in hibernate mode (deep sleep).
