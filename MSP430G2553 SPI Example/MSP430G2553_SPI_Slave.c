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
unsigned int pot;
unsigned char data;
void SPI_Init( void )
{
	WDTCTL = WDTPW | WDTHOLD;
	P1SEL |= BIT0 | BIT4 | BIT5 | BIT6 | BIT7;
	P1SEL2 |= BIT4 | BIT5 | BIT6 | BIT7;

	UCB0CTL1 |= UCSWRST;
	UCB0CTL0 = UCCKPL | UCMSB | UCSYNC | UCMODE_2;
	UCB0CTL1 &= ~UCSWRST;
	IE2 |= UCB0RXIE;
	__bis_SR_register(GIE);
}

int main( void )
{
	SPI_Init();
	P1SEL |= BIT0;							//Enable A/D Channel A0 (P1.0)

	/*ADC10CTL0 &= ~ENC;
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REF2_5V + REFON + ADC10IE + ADC10ON;
    ADC10CTL1 = ADC10DIV_3 + INCH_0 + SHS_0 + CONSEQ_2 + ADC10SSEL_2;
	ADC10CTL0 |= ENC + ADC10SC;

	//while (!(ADC10IFG & ADC10CTL0)) {}
    //pot = ADC10MEM;

	/*while ( 1 ) {
		ADC10CTL0 &= ~(ADC10SC);
		ADC10CTL0 |= ENC + ADC10SC;

		while( !(ADC10CTL0 & ADC10IFG) ) {};
		pot = ADC10MEM;
		//ADC10CTL0 |= ENC + ADC10SC;
	}*/
    while ( 1 );
    return (0);
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR_HOOK (void)
{
	while (!(IFG2 & UCB0RXIFG)) {};
	data = UCB0RXBUF;
	while (!(IFG2 & UCB0TXIFG)) {};
	UCB0TXBUF = SLV_data++;
	__delay_cycles(50);		//change this to something way lower

 //   IE2 &= ~UCB0RXIFG;
}
