#include <msp430.h> 
#include "I2C_SoftwareLibrary.h"
#include "lsm303.h"
#include "stdarg.h"
//#include
/*
 * main.c
 */

#define SDA	10
#define SCL	9

void SPI_Init( void )
{
	P1SEL |= BIT4 + BIT5 + BIT6 + BIT7;
	P1SEL2 |= BIT4 + BIT5 + BIT6 + BIT7;

	UCB0CTL1 |= UCSWRST;
	UCB0CTL0 = UCCKPL | UCMSB | UCSYNC | UCMODE_2;
	UCB0CTL1 &= ~UCSWRST;
	IE2 |= UCB0RXIE;
	UCB0TXBUF = 0xAE;
	__bis_SR_register( GIE );
}

unsigned char sample_buf[5] = { 0xBB, 0x00, 0x00, 0x00, 0x00 };
unsigned char transferRequested = 0, accelReadComplete = 0;
int x,y,z;

int main(void) {

    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	char b = 0;

	/* Initialize bit banging */
	SPI_Init();
	SoftwareWireInit( SDA, SCL );
	begin( );

	/* LSM303 code */
	EnableDefault( );
	UCB0TXBUF = 0xAE;
	while( 1 ) {
		if ( transferRequested ) {
			ReadAccel( &x, &y, &z );
		//if( !inTransfer ) {
			//__bic_SR_register( GIE );
		//	UCB0TXBUF = 0xAE;
			sample_buf[1] = ( x >> 8 ) & 0xFF;
			sample_buf[2] = ( x & 0xFF );
			sample_buf[3] = ( y >> 8 ) & 0xFF;
			sample_buf[4] = ( y & 0xFF );
			accelReadComplete = 1;
			transferRequested = 0;
			//__bis_SR_register( GIE );
		}
	}

	return 0;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR_HOOK( void )
{
	static char byteCtr = 0;
	if ( byteCtr == 0 && transferRequested == 0) {
		//inTransfer = 1;
		transferRequested = 1;
	} else if ( byteCtr == 0 && accelReadComplete == 1 ) {
		accelReadComplete = 0;
		while ( !( IFG2 & UCB0RXIFG ) );
		UCB0TXBUF = sample_buf[ byteCtr++ ];
	} else if ( byteCtr < 5 && byteCtr > 0 ) {
		while ( !( IFG2 & UCB0RXIFG ) );
		UCB0TXBUF = sample_buf[byteCtr++];
	} else if ( byteCtr == 5 ) {
		while( !(IFG2 & UCB0RXIFG) );
		UCB0TXBUF = 0xAE;
		byteCtr = 0;
	}

	/*if ( byteCtr == 5 ) {
		//while ( !( IFG2 & UCB0RXIFG ) );
		//UCB0TXBUF = 0xAE;
		inTransfer = byteCtr = transferRequested = 0;			//elegance
	}*/
	IFG2 &= ~UCB0RXIFG;
}
