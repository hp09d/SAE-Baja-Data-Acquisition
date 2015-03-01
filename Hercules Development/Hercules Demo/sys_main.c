/** @file sys_main.c 
*   @brief Application main file
*   @date 17.Nov.2014
*   @version 04.02.00
*
*   This file contains an empty main function,
*   which can be used for the application.
*/

/* 
* Copyright (C) 2009-2014 Texas Instruments Incorporated - http://www.ti.com/ 
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
/* USER CODE END */

/* Include Files */

#include "sys_common.h"

/* USER CODE BEGIN (1) */
#include "sci.h"
#include "rti.h"
#include "ili9340.h"
#include "spi.h"
#include "ff.h"
#include "stdio.h"
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
uint8_t sciTX[5] = {0xBA,0xDA,0x55,0,0};
char potStr[3];
uint8_t sciBuffer;
uint8_t gotPreamble = 0;

uint8_t getPreamble(uint8_t val);

void drawProgress(uint8_t new);
FATFS fs;
FIL file;
/* USER CODE END */

void main(void)
{
/* USER CODE BEGIN (3) */
	sciInit();
	spiInit();
	rtiInit();
	ili9340Init();
	ili9340FillRect(0,0,320,240,0xFFFF);

	rtiEnableNotification(rtiNOTIFICATION_COMPARE0);
	sciEnableNotification(scilinREG, SCI_TX_INT | SCI_RX_INT );

	sciReceive(scilinREG,1,&sciBuffer);

	_enable_IRQ();

	rtiStartCounter(rtiCOUNTER_BLOCK1);

	FRESULT RES;
	RES = f_mount(&fs,"",1);

	spiDAT1_t dataConfig1;
    dataConfig1.CSNR = SPI_CS_0;
    dataConfig1.CS_HOLD = 0;
    dataConfig1.DFSEL = SPI_FMT_0;
    dataConfig1.WDEL = 0;

    uint16_t src = 0xFF;
	uint16_t accel = 0;
	uint16_t pot = 0;
	uint16_t i = 0;

	if(RES != FR_OK){
		ili9340Write(10,10,2,(uint8_t *) "FS Mount Error",0xF800);
		while(1);
	}

	RES = f_open( &file, "display.raw", FA_READ | FA_OPEN_EXISTING );

	if(RES != FR_OK){
		ili9340Write(10,40,2,(uint8_t *) "File Open Error",0xF800);
		while(1);
	}else{
		ili9340DrawFile(&file,320,240);
	}

	while(1){
		//sciReceive(scilinREG,1,&sciBuffer);
		spiTransmitAndReceiveData(spiREG2, &dataConfig1, 1, &src, &accel);
		drawProgress(accel);
		_disable_IRQ();
		sciTX[3] = accel;
		dataConfig1.CSNR = SPI_CS_2;
		for( i = 0; i < 65535; ++i ){

		}
		spiTransmitAndReceiveData(spiREG2, &dataConfig1, 1, &src, &pot);
		pot = (pot*99)/255;
		sciTX[4] = pot;
		_enable_IRQ();
		sprintf(&potStr[0],"%d",pot);
		ili9340WriteFill(75,45,13,&potStr[0],0xFFFF,0x528A);
		dataConfig1.CSNR = SPI_CS_0;
		for( i = 0; i < 65535; ++i ){

		}
	}
/* USER CODE END */
}

/* USER CODE BEGIN (4) */
void sciNotification(sciBASE_t *sci, uint32 flags){
	static uint8_t buf[2] = {0,0};
	static uint8_t bytectr = 0;

	if(flags & SCI_RX_INT){
		if(gotPreamble == 0){
			gotPreamble = getPreamble(sciBuffer);
		}else{
			buf[bytectr++] = sciBuffer;

			if( bytectr >= 2 ){
				if( buf[0] == 'e'  && buf[1] == 'r' ){
					if( !ili9340IsBusy() ){
						ili9340FillRect(83,184,163,38,0xF800);
						ili9340Write(100,192,3,(char *) "Error",0xFFFF);
					}
				}else if( buf[0] == 'c' && buf[1] == 'l' ){
					if( !ili9340IsBusy() ){
						ili9340FillRect(83,184,163,38,0x07E0);
					}
				}else{
					//Not recognized
				}

				gotPreamble = 0;
				bytectr = 0;
			}
		}
		return;
	}else if(flags & SCI_TX_INT){
		return;
	}
}

void rtiNotification(uint32 notification){
	//static char d[6] = {'0','0',0};
	//status uint8_t
	/*if( !ili9340IsBusy() ){
		ili9340WriteFill(75,45,13,&d[0],0xFFFF,0x528A);
		//drawProgress(i[4]);
	}*/
	sciSend(scilinREG,5,&sciTX[0]);
	//--i[4];
	/*if( ++d[1] > '9' ){
		d[1] = '0';
		if( ++d[0] > '9' ){
			d[0] = '0';
		}
	}*/
	return;
}

/*void rtiOverflow1Interrupt(){
	return;
}*/

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


void drawProgress(uint8_t new){
	static uint8_t old = 0;
	static uint8_t y0_old = 0;
	uint16_t height;
	uint16_t y0;

	if(new > old){
		//Draw Yellow
		y0 = 188-(new*177)/255;
		height = (177*(new-old))/255;
		ili9340FillRect(12,y0-1,31,height+1,0xFF70);
	}else{
		//Draw Black
		y0 = 188-(old*177)/255;
		height = (177*(old-new))/255;
		ili9340FillRect(12,y0-1,31,height+1,0x0000);
	}

	if( height != 0 ){
		old = new;
	}
}


/* USER CODE END */
