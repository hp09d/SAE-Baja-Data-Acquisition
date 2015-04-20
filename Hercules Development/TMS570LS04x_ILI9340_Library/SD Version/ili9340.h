/*
 * ILI9340.h
 *
 *  Created on: Jan 14, 2015
 *      Author: Dewey Williams
 */

#ifndef ILI9340_H_
#define ILI9340_H_

#include "mibspi.h"
#include "gio.h"
#include "ff.h"

#define ILI9340_DC 6		//GPIO pins for display functions
#define ILI9340_RST 7

#define ILI9340_8BIT_TG 0				//mibSPI transfer group for transmitting 8 bit data
#define ILI9340_16BIT_SINGLE 1
#define ILI9340_16BIT_MULTI 2			//mibSPI transfer group for transmitting 16 bit data in bulk
#define ILI9340_MULTI_SIZE 24		//Size of the 16bit transfer group's buffer (in 16bit words)-

#define ILI9340_XMIT_DELAY 200

//Defines from the Adafruit library (might not need all of them)
#define ILI9340_TFTWIDTH 320	//Display is rotated so height and width are flipped
#define ILI9340_TFTHEIGHT 240
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

void ili9340Init();						//Initialize
uint16_t ili9340IsBusy();
void ili9340WriteCommand(uint16_t c);	//Write an 8bit command (DC low)
void ili9340WriteData(uint16_t d);		//Write 8bit data (DC high)
void ili9340DrawImage(uint16_t width, uint16_t height, const uint8_t * pixels);	//Draw a GIMP-exported image
void ili9340DrawFile(FIL * file, uint16_t width, uint16_t height);				//Draw a raw 565 file from FatFS
void ili9340FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);
void ili9340SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);	//Set the draw area
void ili9340DrawChar(uint16_t x, uint16_t y, uint8_t size, char c, uint16_t color);
void ili9340DrawCharFill(uint16_t x, uint16_t y, uint8_t size, char c, uint16_t color, uint16_t fill);
void ili9340Write(uint16_t x, uint16_t y, uint8_t size, char * s, uint16_t color);
void ili9340WriteFill(uint16_t x, uint16_t y, uint8_t size, char * s, uint16_t color, uint16_t fill);

#endif /* ILI9340_H_ */
