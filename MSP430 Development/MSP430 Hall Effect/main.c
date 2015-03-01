/*
 * ======== Standard MSP430 includes ========
 */
#include <msp430g2553.h>
#include <stdio.h>
#include <math.h>
/*
 * ======== Grace related declaration ========
 */

/*
 *  ======== main ========
 */

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
unsigned char SLV_data = 0x00;
unsigned int ADCconverted;
unsigned char data;
int byteNum = 0;
//char sampleArray[4];
char betweenBytes = 0;
char matchingLSB;
int num = 0;
unsigned int timertimeytime = 0;
double RPM;
unsigned char Const = 0x03;
/* ====================================== */
int cyclespersec = 12000; //@ 12kHz it takes 12000 cycles for one second
	double end_time_value = 0;		//temp variable
double calculate_time() //Calculate the RPM value using timertimeytime
{

	//First convert to seconds
	end_time_value = (double)timertimeytime/(double)cyclespersec;	//seconds/rotation

	end_time_value = 60/end_time_value;				//Rotations/Min

	end_time_value = round(end_time_value);
	return end_time_value;
}


int main( void )
{
	WDTCTL = WDTPW | WDTHOLD;
	SPI_Init();
	P1SEL |= BIT0;							//Enable A/D Channel A0 (P1.0)

	ADC10CTL0 &= ~ENC;
	ADC10CTL0 = SREF_1 + ADC10SHT_3 + REF2_5V + REFON + ADC10ON;
    	ADC10CTL1 = ADC10DIV_3 + INCH_0 + SHS_0 + CONSEQ_2 + ADC10SSEL_2;


    //Initialize Timer0_A
    TA0CCR0 = 65535;					//TO DO: Change the timer to be able to count beyond 5 seconds
    									//maybe do tops 30 seconds and anything beyond that gets an automatic 1 RPM?
    TA0CTL = TASSEL_1 + MC_1; 	//ACLK @ 12kHZ and sets up count mode


	while ( 1 ) {


		//ADC10CTL0 |= ADC10IFG;
		//ADC10CTL0 |= ADC10IFG;
		//while( !(ADC10CTL0 & ADC10IFG) ) {};
		ADC10CTL0 |= ENC + ADC10SC;
		ADCconverted = ADC10MEM;

		//Rotation count. Increments one per rotation regardless of clock cycle speed
		if(ADCconverted >= 450){			//If magnet has passed increment rotation count
		num++;

		if(num == 1){			//If its first pass reset timer
			TA0R = 0;}
		else{					//If its second pass record value on the timer register
		timertimeytime = TA0R;
		RPM = (int)calculate_time();		//Calculate time in seconds
		num = 0;}								//Reset magnet pass count
		while(!(ADCconverted <= 417 ))				//Hold code until magnet pass complete
								{
									ADC10CTL0 &= ~(ADC10SC);
									ADC10CTL0 |= ENC + ADC10SC;
									ADCconverted = ADC10MEM;
								}

		}
		if ( betweenBytes == 0 ) {
		        /*  !!!!!  BEGIN DANGER ZONE  !!!!!   */
			/*sampleArray[ byteNum ] = ( pot >> 8 );
			UCB0TXBUF = sampleArray[ byteNum ];
			matchingLSB = byteNum + 1;
			byteNum = (++byteNum) % 4;
			sampleArray[ byteNum ] = ( pot & 0x00FF );
			byteNum = (++byteNum) % 4;*/
			/*  !!!!!  END DANGER ZONE   !!!!!   */
		}
		ADC10CTL0 &= ~(ADC10SC);
	//delay(10000);
	}
    while ( 1 );
    return ( 0 );
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR_HOOK (void)
{
	betweenBytes ^= 1;			//flip the bit
	while (!(IFG2 & UCB0RXIFG)) {};
	data = UCB0RXBUF;
	while (!(IFG2 & UCB0TXIFG)) {};
	UCB0TXBUF = Const;
	__delay_cycles(50);
}
//#pragma vector = ADC10_VECTOR
//__interrupt void ADC10_ISR(void)
//{
 //TAoR = 0;
//}

