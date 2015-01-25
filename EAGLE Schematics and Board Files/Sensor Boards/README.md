This board design is based around the MSP430G2553IN20. It includes a stabilizing RC pair on the reset pin, as well as an optional reset button and status LED.

This board is used as a platform for building sensor modules for the Baja car. It breaks out power, ground, and both serial interfaces (UCA and UCB), as well as analog pins 0-2.
The pinout of the UCB header matches with pins 2-7 of the UCA/ANALOG header for easily switching interfaces.

UCB Header:

|Pin #|MSP430 Pin|Primary Function|Secondary Function|
|-------|--------|----------------|------------------|
|1|VCC|3V3||
|2|P1.6|UCB0SOMI|UCB0SCL|
|3|P1.7|UCB0SIMO|UCB0SDA|
|4|P1.5|UCB0CLK||
|5|P1.6|UCB0STE||
|6|VSS|GND||

UCA/ANALOG Header:

|Pin #|MSP430 Pin|Primary Function|Secondary Function|
|-------|--------|----------------|------------------|
|1|P1.0|A0
|2|VCC|3V3||
|3|P1.6|UCB0SOMI|UCB0SCL|
|4|P1.7|UCB0SIMO|UCB0SDA|
|5|P1.5|UCB0CLK||
|6|P1.6|UCB0STE||
|7|VSS|GND||

Note that UCA or analog must be selected internally. 
