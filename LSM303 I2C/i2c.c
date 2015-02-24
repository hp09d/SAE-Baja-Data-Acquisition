/*
* Copyright (C)
* Originally by: D. Dang, Texas Instruments Inc. December 2010
* Modified by: Alan Barr 2011
*/

#include "i2c.h"

static char i2cPolling = 0;                                             //Polling i2c
static int i2cRXByteCtr = 0;
static const char * pI2cTxData;                                         // Pointer to TX data
static volatile char * pI2cRxData;                                      // Pointer to i2cRX data
static char i2cTXByteCtr = 0;
static char i2cRX = 0;
static char noStop = 0;
extern char reg_val = 0;

#define ACCEL_ADDR_READ 	(0x33 >> 1)
#define ACCEL_ADDR_WRITE	(0x32 >> 1)

void i2cSetupPins(void)
{
    P1SEL |= BIT6 + BIT7;                                               // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;
   // P1REN |= BIT6 + BIT7;
    //P1OUT |= BIT6 + BIT7;
    // Assign I2C pins to USCI_B0
}

void i2cSetupTx(const char address )
{
    //dint();
    i2cRX = 0;
    IE2 &= ~UCB0RXIE;
    while (UCB0CTL1 & UCTXSTP);                                         // Ensure stop condition got sent// Disable i2cRX interrupt
    UCB0CTL1 |= UCSWRST;                                                // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;                               // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;                                      // Use SMCLK, keep SW reset
    UCB0BR0 = 12;                                                       // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = address;
    UCB0CTL1 &= ~UCSWRST;                                               // Clear SW reset, resume operation
    UCB0I2CIE |= UCNACKIE;
    IE2 |= UCB0TXIE + UCB0RXIE;
    __bis_SR_register(GIE);// Enable TX interrupt
}

void i2cSetupRx(const char address)
{
    //dint();
    i2cRX = 1;
    IE2 &= ~UCB0TXIE;
    UCB0CTL1 |= UCSWRST;                                                // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;                               // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;                                      // Use SMCLK, keep SW reset
    UCB0BR0 = 12;                                                       // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = address;
    UCB0CTL1 &= ~UCSWRST;                                               // Clear SW reset, resume operation
    UCB0I2CIE |= UCNACKIE;
    IE2 |= UCB0RXIE + UCB0TXIE;
    __bis_SR_register( GIE );// Enable i2cRX interrupt
}

void i2cTransmit(const char * data, const unsigned char numberOfBytes)
{
    pI2cTxData = data;                                                  // TX array start address
    i2cTXByteCtr = numberOfBytes;                                       // Load TX byte counter
    while (UCB0CTL1 & UCTXSTP);
    UCB0CTL1 |= UCTR + UCTXSTT;
    __bis_SR_register(GIE);                                    // Enter LPM0 w/ interrupts
}

void i2cGetRegister( void )
{
	noStop = 1;
	IE2 &= ~UCB0TXIE;
	IE2 &= ~UCB0RXIE;

	UCB0CTL1 |= UCSWRST;
	UCB0I2CSA = ACCEL_ADDR_WRITE;
	UCB0CTL1 &= ~UCSWRST;
	IE2 |= UCB0TXIE + UCB0RXIE;
	__bis_SR_register( GIE );

	const char lsm303_read_addr[1] = {0x20};
	i2cTransmit( lsm303_read_addr, 1 );
	UCB0TXBUF = ACCEL_ADDR_READ;
	UCB0CTL1 |= UCTXSTT;

	while( UCB0CTL1 & UCTXSTT ); 			// Wait for start condition to complete?
							// May help with synchronization, or it may get
							// stuck here. Comment out if so.
	UCB0CTL1 |= UCTXNACK;
	UCB0CTL1 |= UCTXSTP;
}

void i2cReceive(volatile char * data, const int numberOfBytes)
{
    pI2cRxData = data;                                                  // Start of i2cRX buffer
    i2cRXByteCtr = numberOfBytes;
    // Load i2cRX byte counter
    while (UCB0CTL1 & UCTXSTP);
    UCB0CTL1 |= UCTXSTT;                                                // I2C start condition
    __bis_SR_register(GIE);                                    // Enter LPM0 w/ interrupts
}

void i2cPoll(char address)
{
    int ctr;
    for(ctr=0;ctr<2000;ctr++);

    do                          //i2cPolling for write complete
    {
        i2cPolling = 1;
        Setup_TX(address);
        i2cTransmit(0,1);       //DEBUG
    }
    while(i2cPolling == 2);
    i2cPolling = 0;
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
    if ((UCB0STAT & UCNACKIFG) == UCNACKIFG && i2cPolling == 1)
    {
        UCB0STAT &= ~UCNACKIFG;
        UCB0CTL1 |= UCTXSTP;
        while(UCB0CTL1 & UCTXSTP);
        i2cPolling = 2;
        //__bic_SR_register_on_exit(CPUOFF);
    }
    else
    {
        UCB0CTL1 |= UCTXSTP;
        while(UCB0CTL1 & UCTXSTP);
        //while(1);
    }
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
	if(i2cRX == 1)                                                          // Master Recieve?
    {
        i2cRXByteCtr--;                                                     // Decrement i2cRX byte counter
        if (i2cRXByteCtr > 0)
        {
        	while ( UCB0CTL1 & UCB0TXIFG );
        	--i2cRXByteCtr;
        	reg_val = UCB0RXBUF;
            *pI2cRxData++ = UCB0RXBUF;                                      // Move i2cRX data to address pI2cRxData
        }
        else if(i2cRXByteCtr == 0)
        {
			IE2 &= ~ UCB0TXIE;      //DEBUG disable interrupt
			IFG2 &= ~UCB0TXIFG;
			UCB0CTL1 |= UCTXSTP;
			*pI2cRxData++ = UCB0RXBUF;                                  // I2C stop condition
			while (UCB0STAT & UCBBUSY);
			UCB0CTL1 |= UCSWRST;                                        // Move final i2cRX data to pI2cRxData
			 //__bic_SR_register_on_exit(CPUOFF);
        }
    }
    else
    {                                                                       // Master Transmit
        if (i2cTXByteCtr)                                                   // Check TX byte counter
        {
		  UCB0TXBUF = *pI2cTxData++;                                        // Load TX buffer
		  i2cTXByteCtr--;
        }
        else
        {
        	if ( !noStop ){
        		UCB0CTL1 |= UCTXSTP;
        	}// I2C stop condition

            IFG2 &= ~UCB0TXIFG;
            //__bic_SR_register_on_exit(CPUOFF);                              // Exit LPM0
        }
    }

}

