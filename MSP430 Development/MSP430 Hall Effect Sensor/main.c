/*
 * ======== Standard MSP430 includes ========
 */
#include <msp430g2553.h>
#include <stdio.h>
#include <math.h>

#define T_ACLK 12000
#define CIRCUMFERENCE 72.2566
#define IN_2_MILE 63360
#define SECONDS_PER_HOUR 3600

void SPI_Init( void )
{
	P1SEL |= BIT0 | BIT4 | BIT5 | BIT6 | BIT7;
	P1SEL2 |= BIT4 | BIT5 | BIT6 | BIT7;

	UCB0CTL1 |= UCSWRST;
	UCB0CTL0 = UCCKPL | UCMSB | UCSYNC | UCMODE_2;
	UCB0CTL1 &= ~UCSWRST;
	IE2 |= UCB0RXIE;
	__bis_SR_register(GIE);
}

/* Global data declaration/initialization */
unsigned int ADCconverted;
unsigned int timer = 0;
unsigned int count;
int num;
float RPM;
float time_in_seconds = 0;
float rev_per_sec = 0;
float rev_per_hour = 0;
float speed = 0;
unsigned char fixed_point_speed;

/* ====================================== */
//int cyclespersec = 12000; //@ 12kHz it takes 12000 cycles for one second
double end_time_value = 0;		//temp variable

/*
  Deleted to remove overhead from function calls
 */
/*double calculate_time() //Calculate the RPM value using timertimeytime
{

	//First convert to seconds
	end_time_value = (double)timertimeytime/(double)cyclespersec;	//seconds/rotation

	end_time_value = 60/end_time_value;				//Rotations/Min

	end_time_value = round(end_time_value);
	return end_time_value;
}*/


int main( void )
{
	WDTCTL = WDTPW | WDTHOLD;
	SPI_Init();
	P1SEL |= BIT0;							//Enable A/D Channel A0 (P1.0)
	ADC10CTL0 &= ~ENC;
	ADC10CTL0 = SREF_1 + REF2_5V + REFON + ADC10ON;
    ADC10CTL1 = ADC10DIV_3 + INCH_0 + SHS_0 + CONSEQ_2 + ADC10SSEL_2;


    //Initialize Timer0_A
    TA0CCR0 = 65535;					//TO DO: Change the timer to be able to count beyond 5 seconds
    									//maybe do tops 30 seconds and anything beyond that gets an automatic 1 RPM?
    TA0CTL |= TASSEL_1 + MC_1 + TAIE; 	//ACLK @ 12kHZ and sets up count mode


	while ( 1 ) {
		ADC10CTL0 |= ENC + ADC10SC;
		ADCconverted = ADC10MEM;

		//Rotation count. Increments one per rotation regardless of clock cycle speed
		if(ADCconverted >= 800){			//If magnet has passed increment rotation count
			++num;

			if( num > 0 ) {			//If its first pass reset timer
				TA0R = 55671;
			} else {					//If its second pass record value on the timer register
				count = TA0R - 55671;
				TA0R = 0;
				time_in_seconds = (float)count / T_ACLK;
				rev_per_sec = (float)1 / time_in_seconds;
				rev_per_hour = rev_per_sec * SECONDS_PER_HOUR;
				speed = ( rev_per_hour * CIRCUMFERENCE ) / IN_2_MILE;
				fixed_point_speed = speed;
				fixed_point_speed <<= 2;
				speed -= floor(speed);

				if ( speed >= 0.5 ) {
					fixed_point_speed |= BIT1;
				    speed -= 0.5;
				}
				if ( speed >= 0.25 ) {
					fixed_point_speed |= BIT0;
				}
				while ( !(IFG2 & UCB0TXIFG) );
				UCB0TXBUF = fixed_point_speed;
				num = 0;
				TA0R = 55671;
			} //Reset magnet pass count

			while(!(ADCconverted <= 700 ))			// Wait for the falling edge
			{
				ADC10CTL0 &= ~(ADC10SC);
				ADC10CTL0 |= ENC + ADC10SC;
				__delay_cycles( 100 );
				ADCconverted = ADC10MEM;
			}
			if ( num == 0 ) TA0R = 55671;

		}

			ADC10CTL0 &= ~(ADC10SC);
	}

    return ( 0 );
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR_HOOK (void)
{
	//betweenBytes ^= 1;			//flip the bit
	while (!(IFG2 & UCB0RXIFG)) {};
	//data = UCB0RXBUF;
	while (!(IFG2 & UCB0TXIFG)) {};
	//UCB0TXBUF = Const;
	__delay_cycles(50);
}

#pragma VECTOR=TIMERA0_VECTOR
__interrupt void Timer_A( void )
{
	UCB0TXBUF = 0;
	num = 0;
}

