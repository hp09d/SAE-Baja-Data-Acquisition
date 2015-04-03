#include <msp430.h> 
#include "I2C_SoftwareLibrary.h"
#include "lsm303.h"
#include "stdarg.h"
//#include
/*
 * main.c
 */

//                      +-\/-+
//               VCC   1|    |20  GND
//         (A0)  P1.0  2|    |19  XIN
//         (A1)  P1.1  3|    |18  XOUT
//         (A2)  P1.2  4|    |17  TEST
//         (A3)  P1.3  5|    |16  RST#
//         (A4)  P1.4  6|    |15  P1.7  (A7) (SCL) (MISO) depends on chip
//         (A5)  P1.5  7|    |14  P1.6  (A6) (SDA) (MOSI)
//               P2.0  8|    |13  P2.5
//               P2.1  9|    |12  P2.4
//               P2.2 10|    |11  P2.3
//                      +----+
//

#define SDA	3
#define SCL	2

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
    P1DIR |= BIT2;
    P1OUT &= ~BIT2;

	/* Initialize bit banging */
	SPI_Init();
	SoftwareWireInit( SDA, SCL );
	begin( );

	/* LSM303 code */
	EnableDefault( );
	UCB0TXBUF = 0xAE;
	while( 1 ) {
		if ( transferRequested ) {
			__bic_SR_register( GIE );
			ReadAccel( &x, &y, &z );
			__bis_SR_register( GIE );
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
	/*if ( byteCtr == 0 && transferRequested == 0) {
		transferRequested = 1;
		P1OUT |= BIT2;
	} else*/
	if ( byteCtr == 0 && accelReadComplete == 1 ) {
		accelReadComplete = 0;
		while ( !( IFG2 & UCB0RXIFG ) );
		UCB0TXBUF = sample_buf[ byteCtr++ ];
	} else if ( byteCtr == 0 && transferRequested == 0) {
		transferRequested = 1;
		P1OUT |= BIT2;
	} else if ( byteCtr < 5 && byteCtr > 0 ) {
		while ( !( IFG2 & UCB0RXIFG ) );
		UCB0TXBUF = sample_buf[byteCtr++];
	} else if ( byteCtr == 5 ) {
		while( !(IFG2 & UCB0RXIFG) );
		UCB0TXBUF = 0xAE;
		P1OUT &= ~BIT2;
		byteCtr = 0;
	}

	IFG2 &= ~UCB0RXIFG;
}
