This board design is based around the MSP430G2553IN20. It includes a stabilizing RC pair on the reset pin, as well as an optional reset button and status LED.

This board is used as a platform for building sensor modules for the Baja car. It breaks out power, ground, and both serial interfaces (UCA and UCB), as well as analog pins 0-2.
The pinout of the UCB header matches with pins 2-7 of the UCA/ANALOG header for easily switching interfaces.

UCB Header:

|Pin #|MSP430 Pin|Primary Function|Secondary Function|
|-------|--------|----------------|------------------|
|1|VCC|3V3||
|2|P1.6|UCB0SOMI|UCB0SCL|
|3|P1.7|UCB0SIMO|UCB0SDA|
|4|P1.5|UCB0CLK|UCA0STE|
|5|P1.4|UCB0STE|UCA0CLK|
|6|VSS|GND||

UCA/ANALOG Header:

|Pin #|MSP430 Pin|Primary Function|Secondary Function|Tertiary Function|
|-----|----------|----------------|------------------|-----------------|
|1|P1.0|A0|||
|2|DVCC|3V3|||
|3|P1.1|A1|UCA0SOMI|UCA0RXD|
|4|P1.2|A2|UCA0SIMO|UCA0TXD|
|5|P1.4|UCA0CLK|UCB0STE|A4|
|6|P1.5|UCA0STE|UCB0CLK|A5|
|7|DVSS|GND|||

Note that UCA or analog must be selected internally. 
