/*
* ======== Standard MSP430 includes ========
*/
#include <msp430g2553.h>
#include <stdio.h>
#include <math.h>

/* #defines used in calculation of speed in MPH */
#define T_ACLK 12000			//ACLK default frequency = 12kHz
#define CIRCUMFERENCE 72.2566		// pi * 23 inches
#define IN_2_MILE 63360			//63360 inches in a mile
#define SECONDS_PER_HOUR 3600		// 3600 seconds per hour

void SPI_Init( void )
{
P1SEL |= BIT0 | BIT4 | BIT5 | BIT6 | BIT7;
P1SEL2 |= BIT4 | BIT5 | BIT6 | BIT7;

UCB0CTL1 |= UCSWRST;
UCB0CTL0 = UCCKPL | UCMSB | UCSYNC | UCMODE_2;
UCB0CTL1 &= ~UCSWRST;
//IE2 |= UCB0RXIE;
__bis_SR_register(GIE);
}

/* Global data declaration/initialization */
unsigned int ADCconverted;
unsigned int count = 0;
int num;

/* May need to change to double precision */
float RPM;
float time_in_seconds = 0;
//float rev_per_sec = 0;
float rev_per_hour = 0;
float speed = 0;

/* UQ6.2 Fixed Point */
/* 6 whole bits, 2 fractional bits */
unsigned char fixed_point_speed;

int main( void )
{
	WDTCTL = WDTPW | WDTHOLD;
	//BCSCTL1 = CALBC1_16MHZ;
	//DCOCTL = CALDCO_16MHZ;
	SPI_Init();
	P1SEL |= BIT0;							//Enable A/D Channel A0 (P1.0)
	ADC10CTL0 &= ~ENC;
	ADC10CTL0 = SREF_1 + REF2_5V + REFON + ADC10ON;
	ADC10CTL1 = ADC10DIV_3 + INCH_0 + SHS_0 + CONSEQ_2 + ADC10SSEL_2;

	//Initialize Timer0_A
	TA0CCR0 = 65535;					
	TA0CTL = TASSEL_1 | ID_0 | MC_1 | TAIE; 	//ACLK @ 12kHZ and sets up count mode

	while ( 1 ) {
		ADC10CTL0 |= ENC + ADC10SC;
		ADCconverted = ADC10MEM;

		//Rotation count. Increments one per rotation regardless of clock cycle speed
		if(ADCconverted >= 800){			//If magnet has passed increment rotation count
			++num;

			if( num == 1 ) {			//If its first pass reset timer
				TA0R = 55671;
			} else {					//If its second pass record value on the timer register
				count = TA0R;
				TA0R = 0;
				count -= 55671;
				time_in_seconds = (float)count / T_ACLK;		// count / counts/s = s/rev
				//rev_per_sec = (float)1 / time_in_seconds;
				rev_per_hour = SECONDS_PER_HOUR / time_in_seconds;		// change rev/s to rev/hr
				speed = ( rev_per_hour * CIRCUMFERENCE ) / IN_2_MILE;	// RPH to MPH
				fixed_point_speed = speed;				// truncate
				fixed_point_speed <<= 2;				// shift left 2 to clear fractional bits
				speed -= floor(speed);					// subtract off whole part

				if ( speed >= 0.5 ) {
					fixed_point_speed |= BIT1;			//set 1st fractional bit
					speed -= 0.5;
				}
				if ( speed >= 0.25 ) {	
					fixed_point_speed |= BIT0;			//set 2nd fractional bit
				}
				//while ( !(IFG2 & UCB0TXIFG) );
				// fill transmit buffer - done in main code to remove interrupts
				UCB0TXBUF = fixed_point_speed;
				num = 0;
				TA0R = 55671;
			}
			// Wait for falling edge
			while(!(ADCconverted <= 700 ))
			{
			ADC10CTL0 &= ~(ADC10SC);
			ADC10CTL0 |= ENC + ADC10SC;
			//__delay_cycles( 100 );		//may not be necessary?
			ADCconverted = ADC10MEM;
			}

			if ( num == 0 ) TA0R = 55671;
			
		}

		ADC10CTL0 &= ~(ADC10SC);
	}

	return ( 0 );
}

/* TO DO: Remove */
/*#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR_HOOK (void)
{
	//betweenBytes ^= 1;			//flip the bit
	while (!(IFG2 & UCB0RXIFG)) {};
	//data = UCB0RXBUF;
	while (!(IFG2 & UCB0TXIFG)) {};
	//UCB0TXBUF = Const;
	__delay_cycles(50);
}
*/
/* Handles time-out. Sets transmit buffer to 0 and resets pulse count
to wait for pulse in the pair */
#pragma vector=TIMER0_A1_VECTOR
__interrupt void TIMER0_A1_ISR_HOOK(void)
{
	UCB0TXBUF = 0;
	num = 0;
	TA0CTL &= ~TAIFG;
}


