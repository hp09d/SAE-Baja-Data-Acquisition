/*
 * ======== Standard MSP430 includes ========
 */
#include <msp430g2553.h>
/*
 * ======== Grace related declaration ========
 */

/*
 *  ======== main ========
 */
unsigned char SLV_data = 0x00;
unsigned int acc;
unsigned char data;
unsigned char transmissionNumber = 0;

void SPI_Init( void )
{
	WDTCTL = WDTPW | WDTHOLD;

	P1SEL |= BIT0 | BIT4 | BIT5 | BIT6 | BIT7;
	P1SEL2 |= BIT4 | BIT5 | BIT6 | BIT7;

	UCB0CTL1 |= UCSWRST;
	UCB0CTL0 = UCCKPL | UCMSB | UCSYNC | UCMODE_2;
	UCB0CTL1 &= ~UCSWRST;
	IE2 |= UCB0RXIE;
	UCB0TXBUF = SLV_data;
	__bis_SR_register(GIE);
}

int main( void )
{
	SPI_Init();
	P1SEL |= BIT1;							//Enable A/D Channel A0 (P1.0)

	ADC10CTL0 &= ~ENC;
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + REF2_5V + ADC10ON;
    ADC10CTL1 = ADC10DIV_3 + INCH_0;
    //ADC10CTL1 = INCH_1 + ADC10DIV_3;
	__delay_cycles(30);
	ADC10CTL0 |= ENC + ADC10SC;

	while ( 1 ) {
		ADC10CTL0 |= ENC + ADC10SC;
		while( (ADC10CTL1 & ADC10BUSY) ) {};
		acc = ADC10MEM;
		UCB0TXBUF = ( (acc >> 2 ) & 0x00FF );
		ADC10CTL0 &= ~ENC;
	}
    while ( 1 );
    return (0);
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR_HOOK (void)
{
	while (!(IFG2 & UCB0RXIFG)) {};
	data = UCB0RXBUF;
	while (!(IFG2 & UCB0TXIFG)) {};

	/*switch ( transmissionNumber ) {
		case 0:
			UCB0TXBUF = acc >> 8;
			break;
		case 1:
			UCB0TXBUF = acc & 0xFF;
			break;
		default:break;
	}
	transmissionNumber = (++transmissionNumber) % 2;*/
}
