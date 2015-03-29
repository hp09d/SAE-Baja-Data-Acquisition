/** @file sys_main.c 
*   @brief Application main file
*   @date 16.Feb.2015
*   @version 04.03.00
*
*   This file contains an empty main function,
*   which can be used for the application.
*/

/* 
* Copyright (C) 2009-2015 Texas Instruments Incorporated - www.ti.com 
* 
* 
*  Redistribution and use in source and binary forms, with or without 
*  modification, are permitted provided that the following conditions 
*  are met:
*
*    Redistributions of source code must retain the above copyright 
*    notice, this list of conditions and the following disclaimer.
*
*    Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the 
*    documentation and/or other materials provided with the   
*    distribution.
*
*    Neither the name of Texas Instruments Incorporated nor the names of
*    its contributors may be used to endorse or promote products derived
*    from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
*  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
*  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
*  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
*  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
*  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
*/


/* USER CODE BEGIN (0) */
#define SLAVE_DELAY 500

//Status Bits
#define SDStatus 0x80			//SD Card Error
#define LogStatus 0x40			//Data Logging On/Off
#define SlvStatus 0x20				//Sensor Error
#define DriverNot 0x10		//Driver Notification State
#define PitNot	0x08		//Pit Notification State
/* USER CODE END */

/* Include Files */

#include "sys_common.h"

/* USER CODE BEGIN (1) */
#include "sci.h"
#include "rti.h"
#include "spi.h"
#include "pinmux.h"
#include "gio.h"

#include "ili9340.h"
#include "ff.h"
#include "stdio.h"
#include "string.h"
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */
uint8_t preamble[3] = {0xBA,0xDA,0x55};
uint8_t sciTX[6] = {0xBA,0xDA,0x55,0,0,0};
char potStr[3];
uint8_t sciBuffer;
uint8_t gotPreamble = 0;
uint8_t dispErr = 0;
uint8_t clrErr = 0;
uint8_t writeLog = 0;

uint32_t TIME = 0;

typedef uint8_t herc_status_t;

herc_status_t STATUS = 0;

uint8_t getPreamble(uint8_t val);

uint8_t requestAccel();
uint8_t getAccel( uint16_t * x, uint16_t * y );

void filterAccel( uint16_t X, uint16_t Y );
uint16_t accel_samples = 0;

uint32_t accel_total_X = 0;
uint16_t accel_average_X = 0;
uint16_t accel_min_X = 0xFFFF;
uint16_t accel_max_X = 0;

uint32_t accel_total_Y = 0;
uint16_t accel_average_Y = 0;
uint16_t accel_min_Y = 0xFFFF;
uint16_t accel_max_Y = 0;

void drawProgress(uint16_t new);
FATFS fs;
FIL file;
/* USER CODE END */

void main(void)
{
/* USER CODE BEGIN (3) */
	sciInit();
	spiInit();
	rtiInit();
	gioInit();
	ili9340Init();
	ili9340FillRect(0,0,320,240,0xFFFF);

	rtiEnableNotification(rtiNOTIFICATION_COMPARE0);
	sciEnableNotification(scilinREG, SCI_TX_INT | SCI_RX_INT );

	sciReceive(scilinREG,1,&sciBuffer);

	_enable_IRQ();

	rtiStartCounter(rtiCOUNTER_BLOCK1);

	FRESULT RES;
	RES = f_mount(&fs,"",1);

	if(RES == FR_OK){
		RES = f_open( &file, "display.raw", FA_READ | FA_OPEN_EXISTING );

		if(RES == FR_OK){
			ili9340DrawFile(&file,320,240);
		}else{
			ili9340Write(10,40,2,(char *) "File Open Error",0xF800);
			STATUS |= SDStatus;
		}
	}else{
		ili9340Write(10,10,2,(char *) "FS Mount Error",0xF800);
		STATUS |= SDStatus;
	}

	gioPORTA->DIR |= (1 << 3);
	gioPORTA->DIR &= ~(1 << 4);
	gioPORTA->DOUT |= (1 << 3);

    uint8_t hold = 0;
    uint8_t sensErr = 0;
	uint16_t accelX, accelY = 0;
	char sdBuf[100];
	uint32_t bw;

	sensErr = requestAccel();

	while(1){
		sensErr |= getAccel( &accelX, &accelY );
		sensErr |= requestAccel();
		if( sensErr > 0 ){
			STATUS |= SlvStatus;
			sensErr = 0;
		}else{
			STATUS &= ~(SlvStatus);
			accelX += 0x8000;
			accelY += 0x8000;
			filterAccel( accelX, accelY );
			drawProgress( accelX );
			accelY = (uint32_t)( accelY * 99 ) / 0xFFFF;
			rtiDisableNotification(rtiNOTIFICATION_COMPARE0);
			sciTX[4] = accelX >> 8;
			sciTX[5] = (uint8_t)accelY;
			rtiEnableNotification(rtiNOTIFICATION_COMPARE0);
			sprintf(&potStr[0],"%2d",accelY);
			ili9340WriteFill(75,45,13,&potStr[0],0xFFFF,0x528A);

			if( writeLog ){
				sprintf( &sdBuf[0], "%d,%d,%d,%d,%d,", TIME>>1, accel_samples, accel_min_X, accel_max_X, accel_average_X);
				RES = f_write(&file, &sdBuf[0], strlen( &sdBuf[0] ), &bw);
				if( RES != FR_OK ){
					STATUS &= ~LogStatus;	//Stop Logging
					STATUS |= SDStatus;		//Set SD Error Flag
				}

				sprintf( &sdBuf[0], "%d,%d,%d\n", accel_min_Y, accel_max_Y, accel_average_Y);
				RES = f_write(&file, &sdBuf[0], strlen( &sdBuf[0] ), &bw);
				if( RES != FR_OK ){
					STATUS &= ~LogStatus;	//Stop Logging
					STATUS |= SDStatus;		//Set SD Error Flag
				}

				//Reset sample filtering
				accel_samples = 0;
				writeLog = 0;
			}
		}
		if( dispErr ){
			ili9340FillRect(83,184,163,38,0xF800);
			ili9340Write(100,192,3,(char *) "Error",0xFFFF);
			dispErr = 0;
		}else if( clrErr ){
			ili9340FillRect(83,184,163,38,0x07E0);
			clrErr = 0;
		}
		if( ((gioPORTA->DIN) & 0x10) == 0x00 ){
			if( hold == 0 ){
				STATUS ^= DriverNot;
				hold = 1;
			}
		}else{
			hold = 0;
		}
	}
/* USER CODE END */
}

/* USER CODE BEGIN (4) */
void sciNotification(sciBASE_t *sci, uint32 flags){
	static uint8_t buf[2] = {0,0};
	static uint8_t bytectr = 0;
	static uint8_t notState = 0;
	uint32_t bw;
	FRESULT RES;

	if(flags & SCI_RX_INT){
		if(gotPreamble == 0){
			gotPreamble = getPreamble(sciBuffer);
		}else{
			buf[bytectr++] = sciBuffer;

			if( bytectr >= 2 ){
				if( buf[0] == 'e'  && buf[1] == 'r' ){
					if( notState == 0 ){
						notState = 1;
						dispErr = 1;
					}else{
						notState = 0;
						clrErr = 1;
					}

					STATUS ^= PitNot;
				}else if( buf[0] == 'l' && buf[1] == 'o' ){
					if( (STATUS & LogStatus) == LogStatus ){
						f_close( &file );
						STATUS &= ~LogStatus;
					}else if( (STATUS & SDStatus) == 0 ){
						RES = f_open( &file, "log.csv", FA_WRITE | FA_OPEN_ALWAYS );
						if( RES != FR_OK ){
							STATUS |= SDStatus;
						}else{
							RES = f_lseek(&file, f_size(&file));
							if( RES != FR_OK ){
								STATUS |= SDStatus;
							}else{
								RES = f_write( &file, (char *)"\nTIME,N,X Min,X Max,X Avg,Y Min,Y Max,Y Avg\n", 44, &bw);
								if( RES != FR_OK ){
									STATUS |= SDStatus;
								}
							}
							STATUS |= LogStatus;
						}
					}
				}else{
					//Not recognized
				}

				gotPreamble = 0;
				bytectr = 0;
			}
		}
		sciReceive(scilinREG,1,&sciBuffer);
		return;
	}else if(flags & SCI_TX_INT){
		return;
	}
}

void rtiNotification(uint32 notification){
	sciTX[3] = STATUS;
	sciSend(scilinREG,6,&sciTX[0]);

	if( ( (STATUS & LogStatus) == LogStatus ) && TIME % 2 == 0 ){
		writeLog = 1;
	}

	TIME++;
}

uint8_t getPreamble(uint8_t val){
	static uint8_t bytectr = 0;

	if( val == preamble[bytectr] ){
		bytectr++;
	}else{
		bytectr = 0;
	}

	if(bytectr == 3){
		bytectr = 0;
		return 1;
	}else{
		return 0;
	}
}


void drawProgress(uint16_t new){
	static uint16_t old = 0;
	uint16_t height;
	uint16_t y0;

	if(new > old){
		//Draw Yellow
		y0 = 188-((new*177)/65535);
		height = (178*(new-old))/65535;
		if( y0+height > 188){
			height = 188-y0;
		}
		ili9340FillRect(12,y0,31,height+1,0xFF70);
	}else{
		//Draw Black
		y0 = 186-((old*177)/65535);
		if( y0 < 11 ){
			y0 = 11;
		}
		height = 1+(178*(old-new))/65535;
		if( y0+height > 188){
			height = 188-y0;
		}
		ili9340FillRect(12,y0,31,height,0x0000);
	}

	if( height != 0 ){
		old = new;
	}
}

uint8_t requestAccel(){
	spiDAT1_t dataConfig1;
    dataConfig1.CSNR = 0xFF;
    dataConfig1.CS_HOLD = 0;
    dataConfig1.DFSEL = SPI_FMT_1;
    dataConfig1.WDEL = 1;

    uint16_t src, dst = 0x00;
    int i=0;
    int j=0;

    gioPORTA->DIR |= (1 << 6);

	while( dst != 0xAE ){		//Get past busy data
		gioPORTA->DOUT &= ~(1 << 6);
		for( i = 0; i < SLAVE_DELAY; ++i );
		spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
		gioPORTA->DOUT |= (1 << 6);
		++j;
		if( j > 1000 ){
			return 1;
		}
	}

	return 0;
}

uint8_t getAccel( uint16_t * x, uint16_t * y ){
	spiDAT1_t dataConfig1;
    dataConfig1.CSNR = 0xFF;
    dataConfig1.CS_HOLD = 0;
    dataConfig1.DFSEL = SPI_FMT_1;
    dataConfig1.WDEL = 1;

    uint16_t src, dst = 0xAE;
    uint16_t X, Y;
    int i=0;
    int j=0;

    gioPORTA->DIR |= (1 << 6);
	while( dst == 0xAE ){		//Get past busy data
		gioPORTA->DOUT &= ~(1 << 6);
		for( i = 0; i < SLAVE_DELAY; ++i );
		spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
		gioPORTA->DOUT |= (1 << 6);
		++j;
		if( j > 1000 ){
			return 1;
		}
	}
	if( dst == 0xBB ){			//Start code
		j = 0;
		while( dst == 0xBB ){
			gioPORTA->DOUT &= ~(1 << 6);
			for( i = 0; i < SLAVE_DELAY; ++i );
			spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
			gioPORTA->DOUT |= (1 << 6);
			++j;
			if( j > 1000 ){
				return 1;
			}
		}
		X = dst << 8;

		gioPORTA->DOUT &= ~(1 << 6);
		for( i = 0; i < SLAVE_DELAY; ++i );
		spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
		gioPORTA->DOUT |= (1 << 6);
		X = X | dst;

		gioPORTA->DOUT &= ~(1 << 6);
		for( i = 0; i < SLAVE_DELAY; ++i );
		spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
		gioPORTA->DOUT |= (1 << 6);
		Y = dst << 8;

		gioPORTA->DOUT &= ~(1 << 6);
		for( i = 0; i < SLAVE_DELAY; ++i );
		spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
		gioPORTA->DOUT |= (1 << 6);
		Y = Y | dst;
	}

	*x = X;
	*y = Y;

	return 0;
}

void filterAccel( uint16_t X, uint16_t Y ){
	if( accel_samples == 0 ){
		accel_min_X = accel_max_X = X;
		accel_min_Y = accel_max_Y = Y;
		accel_total_X = 0;
		accel_total_Y = 0;
	}else if( accel_samples > 10 && (STATUS & LogStatus) == 0 ){
		accel_samples = 0;
		accel_min_X = accel_max_X = X;
		accel_min_Y = accel_max_Y = Y;
		accel_total_X = 0;
		accel_total_Y = 0;
	}

	accel_samples++;

	accel_total_X += X;
	accel_average_X = accel_total_X/accel_samples;

	if( X < accel_min_X ){
		accel_min_X = X;
	}

	if( X > accel_max_X ){
		accel_max_X = X;
	}

	accel_total_Y += Y;
	accel_average_Y = accel_total_Y/accel_samples;

	if( Y < accel_min_Y ){
		accel_min_Y = Y;
	}

	if( Y > accel_max_Y ){
		accel_max_Y = Y;
	}
}


/* USER CODE END */
