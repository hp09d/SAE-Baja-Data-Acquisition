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
#define ILI9340_TFTWIDTH 240
#define ILI9340_TFTHEIGHT 320
#define ILI9340_NOP 0x00
#define ILI9340_SWRESET 0x01
#define ILI9340_RDDID 0x04
#define ILI9340_RDDST 0x09
#define ILI9340_SLPIN 0x10
#define ILI9340_SLPOUT 0x11
#define ILI9340_PTLON 0x12
#define ILI9340_NORON 0x13
#define ILI9340_RDMODE 0x0A
#define ILI9340_RDMADCTL 0x0B
#define ILI9340_RDPIXFMT 0x0C
#define ILI9340_RDIMGFMT 0x0A
#define ILI9340_RDSELFDIAG 0x0F
#define ILI9340_INVOFF 0x20
#define ILI9340_INVON 0x21
#define ILI9340_GAMMASET 0x26
#define ILI9340_DISPOFF 0x28
#define ILI9340_DISPON 0x29
#define ILI9340_CASET 0x2A
#define ILI9340_PASET 0x2B
#define ILI9340_RAMWR 0x2C
#define ILI9340_RAMRD 0x2E
#define ILI9340_PTLAR 0x30
#define ILI9340_MADCTL 0x36
#define ILI9340_MADCTL_MY 0x80
#define ILI9340_MADCTL_MX 0x40
#define ILI9340_MADCTL_MV 0x20
#define ILI9340_MADCTL_ML 0x10
#define ILI9340_MADCTL_RGB 0x00
#define ILI9340_MADCTL_BGR 0x08
#define ILI9340_MADCTL_MH 0x04
#define ILI9340_PIXFMT 0x3A
#define ILI9340_FRMCTR1 0xB1
#define ILI9340_FRMCTR2 0xB2
#define ILI9340_FRMCTR3 0xB3
#define ILI9340_INVCTR 0xB4
#define ILI9340_DFUNCTR 0xB6
#define ILI9340_PWCTR1 0xC0
#define ILI9340_PWCTR2 0xC1
#define ILI9340_PWCTR3 0xC2
#define ILI9340_PWCTR4 0xC3
#define ILI9340_PWCTR5 0xC4
#define ILI9340_VMCTR1 0xC5
#define ILI9340_VMCTR2 0xC7
#define ILI9340_RDID1 0xDA
#define ILI9340_RDID2 0xDB
#define ILI9340_RDID3 0xDC
#define ILI9340_RDID4 0xDD
#define ILI9340_GMCTRP1 0xE0
#define ILI9340_GMCTRN1 0xE1
/* USER CODE END */

/* Include Files */

#include "sys_common.h"

/* USER CODE BEGIN (1) */
#include "mibspi.h"
#define BUFFER_LENGTH 24
#define GIMP_IMG frenchy
//#include "spi.h"
#include "gio.h"
//#include "logo.h"
#include "frenchy.h"
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */
void writecommand(uint16_t c);
void writedata(uint16_t d);
void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void drawPixel(int16_t x, int16_t y, uint16_t color);
uint16_t newColor(uint8_t r, uint8_t g, uint8_t b);
uint16_t buf[] = {0x00};
uint16_t rcv[1] = {0x00};
//spiDAT1_t dataconfig1_t;
/* USER CODE END */

void main(void)
{
/* USER CODE BEGIN (3) */



	mibspiInit();
	//mibspiEnableLoopback(mibspiREG1, Analog_Lbk);
	mibspiEnableGroupNotification(mibspiREG1, 0, 0);

	/*spiInit();
	dataconfig1_t.CS_HOLD = TRUE;
	dataconfig1_t.WDEL    = FALSE;
	dataconfig1_t.DFSEL   = SPI_FMT_0;
	dataconfig1_t.CSNR    = SPI_CS_1;*/

	gioInit();

	gioSetDirection(gioPORTA, 1<<6);

	writecommand(0xEF);
	writedata(0x03);
	writedata(0x80);
	writedata(0x02);

	writecommand(0xCF);
	writedata(0x00);
	writedata(0XC1);
	writedata(0X30);

	writecommand(0xED);
	writedata(0x64);
	writedata(0x03);
	writedata(0X12);
	writedata(0X81);

	writecommand(0xE8);
	writedata(0x85);
	writedata(0x00);
	writedata(0x78);

	writecommand(0xCB);
	writedata(0x39);
	writedata(0x2C);
	writedata(0x00);
	writedata(0x34);
	writedata(0x02);

	writecommand(0xF7);
	writedata(0x20);

	writecommand(0xEA);
	writedata(0x00);
	writedata(0x00);

	writecommand(ILI9340_PWCTR1);    //Power control
	writedata(0x23);   //VRH[5:0]

	writecommand(ILI9340_PWCTR2);    //Power control
	writedata(0x10);   //SAP[2:0];BT[3:0]

	writecommand(ILI9340_VMCTR1);    //VCM control
	writedata(0x3e); //�Աȶȵ���
	writedata(0x28);

	writecommand(ILI9340_VMCTR2);    //VCM control2
	writedata(0x86);  //--

	writecommand(ILI9340_MADCTL);    // Memory Access Control
	writedata(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);

	writecommand(ILI9340_PIXFMT);
	writedata(0x55);

	writecommand(ILI9340_FRMCTR1);
	writedata(0x00);
	writedata(0x18);

	writecommand(ILI9340_DFUNCTR);    // Display Function Control
	writedata(0x08);
	writedata(0x82);
	writedata(0x27);

	writecommand(0xF2);    // 3Gamma Function Disable
	writedata(0x00);

	writecommand(ILI9340_GAMMASET);    //Gamma curve selected
	writedata(0x01);

	writecommand(ILI9340_GMCTRP1);    //Set Gamma
	writedata(0x0F);
	writedata(0x31);
	writedata(0x2B);
	writedata(0x0C);
	writedata(0x0E);
	writedata(0x08);
	writedata(0x4E);
	writedata(0xF1);
	writedata(0x37);
	writedata(0x07);
	writedata(0x10);
	writedata(0x03);
	writedata(0x0E);
	writedata(0x09);
	writedata(0x00);

	writecommand(ILI9340_GMCTRN1);    //Set Gamma
	writedata(0x00);
	writedata(0x0E);
	writedata(0x14);
	writedata(0x03);
	writedata(0x11);
	writedata(0x07);
	writedata(0x31);
	writedata(0xC1);
	writedata(0x48);
	writedata(0x08);
	writedata(0x0F);
	writedata(0x0C);
	writedata(0x31);
	writedata(0x36);
	writedata(0x0F);

	writecommand(ILI9340_SLPOUT);    //Exit Sleep
	int i=0;

	while(i < 500)
	{
	  ++i;
	}

	writecommand(ILI9340_MADCTL);		//Rotate the Screen
	writedata(ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);
	writecommand(ILI9340_DISPON);    //Display on

	int numPixels = GIMP_IMG.width * GIMP_IMG.height;
	int index = 0;
	uint16_t pixBuf[BUFFER_LENGTH];
	const uint8_t * data = GIMP_IMG.pixel_data;

	for(i=0;i<BUFFER_LENGTH;++i)
	{
		pixBuf[i] = data[index++];
		pixBuf[i] |= data[index++]<<8;
	}
	numPixels -= BUFFER_LENGTH;

	//setAddrWindow(6, 83, 314, 200);	//TI Logo
	setAddrWindow(0, 0, 320, 240);		//Frenchy
	gioSetBit(gioPORTA, 6, 1); //Get ready to send pixels

	mibspiSetData(mibspiREG1,1,pixBuf);
	mibspiTransfer(mibspiREG1,1);

	while(numPixels > 0)
	{
		if(numPixels >= BUFFER_LENGTH)
		{
			for(i=0;i<BUFFER_LENGTH;++i)
			{
				pixBuf[i] = data[index++];
				pixBuf[i] |= data[index++]<<8;
				if(pixBuf[i] != 0xFFFF)
				{
					pixBuf[i] = pixBuf[i];
				}
			}
			numPixels -= BUFFER_LENGTH;
		}else{
			for(i=0;i<numPixels;++i)
			{
				pixBuf[i] = data[index++];
				pixBuf[i] |= data[index++]<<8;
			}
			for(i = numPixels; i<BUFFER_LENGTH; ++i)
			{
				pixBuf[i] = 0xFFFF;
			}
			numPixels = 0;
		}
		i = 0;
		while(!(mibspiIsTransferComplete(mibspiREG1,1)))
		{
			++i;
		}
		mibspiSetData(mibspiREG1,1,pixBuf);
		mibspiTransfer(mibspiREG1,1);
	}

	while(1)
	{
	}
/* USER CODE END */
}

/* USER CODE BEGIN (4) */
void writecommand(uint16_t c)
{
	gioSetBit(gioPORTA, 6, 0);

	buf[0] = c;

	mibspiSetData(mibspiREG1,0,buf);
	mibspiTransfer(mibspiREG1, 0);
	//spiTransmitData(spiREG1,&dataconfig1_t,1,buf);
	while(!(mibspiIsTransferComplete(mibspiREG1,0)))
	{

	}
}

void writedata(uint16_t d)
{
	gioSetBit(gioPORTA, 6, 1);

	buf[0] = d;

	mibspiSetData(mibspiREG1,0,buf);
	mibspiTransfer(mibspiREG1, 0);
	//spiTransmitData(spiREG1,&dataconfig1_t,1,buf);
	while(!(mibspiIsTransferComplete(mibspiREG1,0)))
	{

	}
}

void setAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
	writecommand(ILI9340_CASET); // Column addr set
	writedata(x0 >> 8);
	writedata(x0 & 0xFF); // XSTART
	writedata(x1 >> 8);
	writedata(x1 & 0xFF); // XEND
	writecommand(ILI9340_PASET); // Row addr set
	writedata(y0>>8);
	writedata(y0); // YSTART
	writedata(y1>>8);
	writedata(y1); // YEND
	writecommand(ILI9340_RAMWR); // write to RAM
}

void drawPixel(int16_t x, int16_t y, uint16_t color) {
	setAddrWindow(x,y,x+1,y+1);
	writedata(color);
}

uint16_t newColor(uint8_t r, uint8_t g, uint8_t b) {
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/* USER CODE END */
