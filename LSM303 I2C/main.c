#include <msp430.h> 
#include <stdlib.h>
#include <stdio.h>
#include "i2c.h"

#define ACCEL_ADDR_READ 	(0x33 >> 1)
#define ACCEL_ADDR_WRITE	(0x32 >> 1)

/*
 * main.c
 */

char accel_x_h, accel_x_l, accel_y_h, accel_y_l, accel_z_h, accel_z_l;

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    char lsm303_setup[2] = {0x20, 0xE7};
    char lsm303_flush[2] = { 0x00, 0x00 };
    char lsm303_read[1] = {0x20};
    char lsm303_mode = 0;

    i2cSetupPins();

    i2cSetupTx( ACCEL_ADDR_WRITE );
    i2cTransmit( lsm303_setup, 2 );
    __delay_cycles( 10 );

    /* BEGIN GETTING DATA */

    i2cGetRegister();
    while( 1 );
	
	return 0;
}
