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
#define SLAVE_DELAY 300

#define FUEL_LIFE 3600

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
uint8_t sciTX[7] = {0xBA,0xDA,0x55,0,0,0,0};
char potStr[3];
uint8_t sciBuffer;
uint8_t gotPreamble = 0;
uint8_t dispErr = 0;
uint8_t clrErr = 0;
uint8_t writeLog = 0;

uint32_t TIME = 0;
uint16_t FUEL = FUEL_LIFE;

typedef uint8_t herc_status_t;

herc_status_t STATUS = 0;

uint8_t getPreamble(uint8_t val);

uint8_t requestAccel();
uint8_t getAccel( uint16_t * x, uint16_t * y );

void filterAccel( uint16_t X, uint16_t Y );
void filterSpeed( uint16_t speed );

uint16_t getSpeed();
uint16_t samples = 0;

uint32_t speed_total = 0;
uint16_t speed_average = 0;
uint16_t speed_min = 0;
uint16_t speed_max = 0;

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
	gioInit();
	gioPORTA->DIR |= 3;
	gioPORTA->DOUT |= 3;
	rtiInit();
	spiInit();
	mibspiInit();
	ili9340Init();
	ili9340FillRect(0,0,320,240,0xFFFF);

	rtiEnableNotification(rtiNOTIFICATION_COMPARE0);
	sciEnableNotification(scilinREG, SCI_TX_INT | SCI_RX_INT );

	sciReceive(scilinREG,1,&sciBuffer);

	uint32_t i = 0;

	FRESULT RES;

	ili9340FillRect(0,0,320,240,0xFFFF);

	RES = f_mount(&fs,"",1);
	if(RES == FR_OK){
		RES = f_open( &file, "COE.raw", FA_READ | FA_OPEN_EXISTING );

		if(RES == FR_OK){
			ili9340DrawFile(&file,240,240);

			for( i = 0; i < 16000000; ++i );

			RES = f_open( &file, "SAE.raw", FA_READ | FA_OPEN_EXISTING );
			if(RES == FR_OK){
				ili9340FillRect(0,0,320,21,0xFFFF);
				ili9340DrawFile(&file,320,198);
				ili9340FillRect(0,219,320,21,0xFFFF);
			}else{
				ili9340Write(10,40,2,(char *) "File Open Error",0xF800);
				STATUS |= SDStatus;
			}

			for( i = 0; i < 16000000; ++i );
		}else{
			ili9340Write(10,40,2,(char *) "File Open Error",0xF800);
			STATUS |= SDStatus;
		}
	}else{
		ili9340Write(10,10,2,(char *) "FS Mount Error",0xF800);
		STATUS |= SDStatus;
	}

	_enable_IRQ();

	rtiStartCounter(rtiCOUNTER_BLOCK1);

	if(RES == FR_OK){
		RES = f_open( &file, "display.raw", FA_READ | FA_OPEN_EXISTING );

		if(RES == FR_OK){
			ili9340DrawFile(&file,320,240);
		}else{
			ili9340Write(10,40,2,(char *) "File Open Error",0xF800);
			STATUS |= SDStatus;
		}
	}

	//gioPORTA->DIR |= (1 << 3);
	gioPORTA->DIR &= ~(1 << 4);
	//gioPORTA->DOUT |= (1 << 3);

    uint16_t hold = 0;
    uint8_t sensErr = 0;
	uint16_t accelX, accelY, speed = 0;
	char sdBuf[100];
	uint32_t bw;

	sensErr = requestAccel();
	for( bw = 0; bw < 10000; ++bw );

	while(1){
		sensErr |= getAccel( &accelX, &accelY );
		for( bw = 0; bw < 1000; ++bw );
		sensErr |= requestAccel();
		if( sensErr > 0 ){
			STATUS |= SlvStatus;
			sensErr = 0;
		}else{
			STATUS &= ~(SlvStatus);
			accelX += 0x8000;
			accelY += 0x8000;
			speed = getSpeed();

			if( ( samples > 10 && (STATUS & LogStatus) == 0 ) || samples == 0 ){
				//Reset stats
				samples = 0;
				accel_min_X = accel_max_X = accelX;
				accel_min_Y = accel_max_Y = accelY;
				accel_total_X = 0;
				accel_total_Y = 0;
				speed_min = speed_max = speed;
				speed_total = 0;
			}
			samples++;

			filterAccel( accelX, accelY );
			//drawProgress( accelX );
			filterSpeed( speed );
			speed >>= 2;

			rtiDisableNotification(rtiNOTIFICATION_COMPARE0);
			sciTX[4] = (uint16_t)( (uint32_t)(FUEL*255)/FUEL_LIFE );
			sciTX[5] = speed;
			sciTX[6] = accelX >> 8;
			rtiEnableNotification(rtiNOTIFICATION_COMPARE0);
			sprintf(&potStr[0],"%2d",speed);
			ili9340WriteFill(75,45,13,&potStr[0],0xFFFF,0x528A);

			if( writeLog ){
				sprintf( &sdBuf[0], "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
						TIME>>1, samples, accel_min_X, accel_max_X, accel_average_X,
						accel_min_Y, accel_max_Y, accel_average_Y,
						speed_min, speed_max, speed_average,
						FUEL );
				RES = f_write(&file, &sdBuf[0], strlen( &sdBuf[0] ), &bw);
				if( RES != FR_OK ){
					STATUS &= ~LogStatus;	//Stop Logging
					STATUS |= SDStatus;		//Set SD Error Flag
				}

				//Reset sample filtering
				samples = 0;
				writeLog = 0;
			}
		}
		if( dispErr ){
			ili9340FillRect(83,184,81,38,0xF800);
			ili9340Write(110,189,4,(char *) "P",0xFFFF);
			dispErr = 0;
		}else if( clrErr ){
			ili9340FillRect(83,184,81,38,0x07E0);
			clrErr = 0;
		}
		drawProgress(FUEL);
		if( ((gioPORTA->DIN) & 0x10) == 0x00 ){
			hold++;
			if(hold == 50){
				FUEL = FUEL_LIFE;
				/*STATUS ^= LogStatus;
				if( (STATUS & LogStatus) == LogStatus ){
					//Logging
					ili9340FillRect(164,184,82,38,0xFB40);
					ili9340Write(191,189,4,(char *) "L",0xFFFF);
				}else{
					ili9340FillRect(164,184,82,38,0x07E0);
				}*/
			}
		}else{
			if( hold > 0 && hold < 50 ){
				//FUEL = FUEL_LIFE;
				STATUS ^= DriverNot;;
				if( (STATUS & DriverNot) == DriverNot ){
					//Set Notification
					ili9340FillRect(164,184,82,38,0xFB40);
					ili9340Write(191,189,4,(char *) "D",0xFFFF);
				}else{
					ili9340FillRect(164,184,82,38,0x07E0);
				}
			}
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
								RES = f_write( &file, (char *)"\nTIME,N,X Min,X Max,X Avg,Y Min,Y Max,Y Avg, Speed Min, Speed Max, Speed Avg, Fuel Time\n", 88, &bw);
								if( RES != FR_OK ){
									STATUS |= SDStatus;
								}
							}
							STATUS |= LogStatus;
						}
					}
				}else if( buf[0] == 'r' && buf[1] == 'f' ){
					FUEL = FUEL_LIFE;
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
	sciSend(scilinREG,7,&sciTX[0]);

	if( TIME % 2 == 0 ){
		if( FUEL > 1 ){
			FUEL -= 1;
		}else{
			FUEL = 0;
		}
		if( (STATUS & LogStatus) == LogStatus ){
			writeLog = 1;
		}
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
		y0 = 188-((new*177)/FUEL_LIFE);
		height = (178*(new-old))/FUEL_LIFE;
		if( y0+height > 188){
			height = 188-y0;
		}
		ili9340FillRect(12,y0,31,height+1,0xFF70);
	}else{
		//Draw Black
		y0 = 185-((old*177)/FUEL_LIFE);
		if( y0 < 11 ){
			y0 = 11;
		}
		height = 1+(178*(old-new))/FUEL_LIFE;
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

    uint16_t src = 0xFF;
    uint16_t dst = 0x00;
    int i=0;
    int j=0;

	while( dst != 0xAE ){		//Get past busy data
		for( i = 0; i < SLAVE_DELAY; ++i );
		gioPORTA->DOUT &= ~(1 << 0);
		//for( i = 0; i < SLAVE_DELAY; ++i );
		spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
		//for( i = 0; i < SLAVE_DELAY; ++i );
		gioPORTA->DOUT |= (1 << 0);
		++j;
		if( dst == 0x5D ){
			//Fix phase
			dataConfig1.DFSEL = SPI_FMT_2;
			for(i = 0; i < 7; ++i){
				for( i = 0; i < SLAVE_DELAY; ++i );
				gioPORTA->DOUT &= ~(1 << 0);
				spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
				gioPORTA->DOUT |= (1 << 0);
			}
			if( dst == 0xAE ){
				return 0;
			}
		}
		if( j > 100 ){
			//gioPORTA->DIR |= (1 << 0);
			return 1;
		}
	}

	//gioPORTA->DIR |= (1 << 0);
	return 0;
}

uint8_t getAccel( uint16_t * x, uint16_t * y ){
	spiDAT1_t dataConfig1;
    dataConfig1.CSNR = 0xFF;
    dataConfig1.CS_HOLD = 0;
    dataConfig1.DFSEL = SPI_FMT_1;
    dataConfig1.WDEL = 1;

    uint16_t src = 0xFF;
    uint16_t dst = 0xAE;
    uint16_t X, Y;
    int i=0;
    int j=0;

    gioPORTA->DOUT &= ~(1 << 0);

	while( dst == 0xAE ){		//Get past busy data
		for( i = 0; i < SLAVE_DELAY; ++i );
		gioPORTA->DOUT &= ~(1 << 0);
		//for( i = 0; i < SLAVE_DELAY; ++i );
		spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
		//for( i = 0; i < SLAVE_DELAY; ++i );
		gioPORTA->DOUT |= (1 << 0);
		++j;
		if( j > 100 ){
			gioPORTA->DOUT |= (1 << 0);
			return 1;
		}
	}
	if( dst == 0xBB ){			//Start code
		j = 0;
		while( dst == 0xBB ){
			for( i = 0; i < SLAVE_DELAY; ++i );
			gioPORTA->DOUT &= ~(1 << 0);
			//for( i = 0; i < SLAVE_DELAY; ++i );
			spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
			//for( i = 0; i < SLAVE_DELAY; ++i );
			gioPORTA->DOUT |= (1 << 0);
			++j;
			if( j > 100 ){
				gioPORTA->DOUT |= (1 << 0);
				return 1;
			}
		}
		X = dst << 8;

		for( i = 0; i < SLAVE_DELAY; ++i );
		gioPORTA->DOUT &= ~(1 << 0);
		//for( i = 0; i < SLAVE_DELAY; ++i );
		spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
		//for( i = 0; i < SLAVE_DELAY; ++i );
		gioPORTA->DOUT |= (1 << 0);
		X = X | dst;

		for( i = 0; i < SLAVE_DELAY; ++i );
		gioPORTA->DOUT &= ~(1 << 0);
		//for( i = 0; i < SLAVE_DELAY; ++i );
		spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
		//for( i = 0; i < SLAVE_DELAY; ++i );
		gioPORTA->DOUT |= (1 << 0);
		Y = dst << 8;

		for( i = 0; i < SLAVE_DELAY; ++i );
		gioPORTA->DOUT &= ~(1 << 0);
		//for( i = 0; i < SLAVE_DELAY; ++i );
		spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
		//for( i = 0; i < SLAVE_DELAY; ++i );
		gioPORTA->DOUT |= (1 << 0);
		Y = Y | dst;
	}

	*x = X;
	*y = Y;
	//gioPORTA->DOUT |= (1 << 0);
	return 0;
}

void filterAccel( uint16_t X, uint16_t Y ){
	if( samples == 0 ){

	}

	accel_total_X += X;
	accel_average_X = accel_total_X/samples;

	if( X < accel_min_X ){
		accel_min_X = X;
	}

	if( X > accel_max_X ){
		accel_max_X = X;
	}

	accel_total_Y += Y;
	accel_average_Y = accel_total_Y/samples;

	if( Y < accel_min_Y ){
		accel_min_Y = Y;
	}

	if( Y > accel_max_Y ){
		accel_max_Y = Y;
	}
}

void filterSpeed( uint16_t speed ){
	if( samples == 0 ){

	}

	speed_total += speed;
	speed_average = speed_total/samples;

	if( speed < speed_min ){
		speed_min = speed;
	}

	if( speed > speed_max ){
		speed_max = speed;
	}
}

uint16_t getSpeed(){
	spiDAT1_t dataConfig1;
    dataConfig1.CSNR = 0xFF;
    dataConfig1.CS_HOLD = 0;
    dataConfig1.DFSEL = SPI_FMT_1;
    dataConfig1.WDEL = 1;

    uint16_t src = 0xFF;
    uint16_t dst = 0;
    gioPORTA->DOUT &= ~(1 << 1);
    spiTransmitAndReceiveData(spiREG2, &dataConfig1,1,&src,&dst);
    gioPORTA->DOUT |= (1 << 1);
    return dst;
}


/* USER CODE END */
