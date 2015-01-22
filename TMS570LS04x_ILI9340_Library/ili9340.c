#include "ili9340.h"
#include "font-9x8.h"

uint16_t ili9340CmdBuf[1];
uint16_t ili9340PixBuf[ILI9340_MULTI_SIZE];

void ili9340Init(){
	mibspiInit();
	gioInit();

	int i=0;

	//Configure RST and DC as outputs
	gioPORTA->DIR |= (1 << ILI9340_RST);
	gioPORTA->DIR |= (1 << ILI9340_DC);

	//Set pullups
	gioPORTA->PSL |= (1 << ILI9340_RST);
	gioPORTA->PSL |= (1 << ILI9340_DC);

	//Reset the display from GPIO
	gioSetBit(gioPORTA,ILI9340_RST,1);

	for(i=0; i< 400000; ++i)
	{

	}

	gioSetBit(gioPORTA,ILI9340_RST,0);

	for(i=0; i < 1600000; ++i)
	{

	}

	gioSetBit(gioPORTA,ILI9340_RST,1);

	for(i=0; i < 12000000; ++i)
	{

	}

	i = 0;
	while(i < 5000)
	{
	  ++i;
	}

	//Init commands from Adafruit library
	//Could probably be optimized with a multi-buffer
	//***********************************
	ili9340WriteCommand(0xEF);
	ili9340WriteData(0x03);
	ili9340WriteData(0x80);
	ili9340WriteData(0x02);

	ili9340WriteCommand(0xCF);
	ili9340WriteData(0x00);
	ili9340WriteData(0XC1);
	ili9340WriteData(0X30);

	ili9340WriteCommand(0xED);
	ili9340WriteData(0x64);
	ili9340WriteData(0x03);
	ili9340WriteData(0X12);
	ili9340WriteData(0X81);

	ili9340WriteCommand(0xE8);
	ili9340WriteData(0x85);
	ili9340WriteData(0x00);
	ili9340WriteData(0x78);

	ili9340WriteCommand(0xCB);
	ili9340WriteData(0x39);
	ili9340WriteData(0x2C);
	ili9340WriteData(0x00);
	ili9340WriteData(0x34);
	ili9340WriteData(0x02);

	ili9340WriteCommand(0xF7);
	ili9340WriteData(0x20);

	ili9340WriteCommand(0xEA);
	ili9340WriteData(0x00);
	ili9340WriteData(0x00);

	ili9340WriteCommand(ILI9340_PWCTR1);    //Power control
	ili9340WriteData(0x23);   //VRH[5:0]

	ili9340WriteCommand(ILI9340_PWCTR2);    //Power control
	ili9340WriteData(0x10);   //SAP[2:0];BT[3:0]

	ili9340WriteCommand(ILI9340_VMCTR1);    //VCM control
	ili9340WriteData(0x3e);
	ili9340WriteData(0x28);

	ili9340WriteCommand(ILI9340_VMCTR2);    //VCM control2
	ili9340WriteData(0x86);  //--

	ili9340WriteCommand(ILI9340_MADCTL);    // Memory Access Control
	ili9340WriteData(ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);

	ili9340WriteCommand(ILI9340_PIXFMT);
	ili9340WriteData(0x55);

	ili9340WriteCommand(ILI9340_FRMCTR1);
	ili9340WriteData(0x00);
	ili9340WriteData(0x18);

	ili9340WriteCommand(ILI9340_DFUNCTR);    // Display Function Control
	ili9340WriteData(0x08);
	ili9340WriteData(0x82);
	ili9340WriteData(0x27);

	ili9340WriteCommand(0xF2);    // 3Gamma Function Disable
	ili9340WriteData(0x00);

	ili9340WriteCommand(ILI9340_GAMMASET);    //Gamma curve selected
	ili9340WriteData(0x01);

	ili9340WriteCommand(ILI9340_GMCTRP1);    //Set Gamma
	ili9340WriteData(0x0F);
	ili9340WriteData(0x31);
	ili9340WriteData(0x2B);
	ili9340WriteData(0x0C);
	ili9340WriteData(0x0E);
	ili9340WriteData(0x08);
	ili9340WriteData(0x4E);
	ili9340WriteData(0xF1);
	ili9340WriteData(0x37);
	ili9340WriteData(0x07);
	ili9340WriteData(0x10);
	ili9340WriteData(0x03);
	ili9340WriteData(0x0E);
	ili9340WriteData(0x09);
	ili9340WriteData(0x00);

	ili9340WriteCommand(ILI9340_GMCTRN1);    //Set Gamma
	ili9340WriteData(0x00);
	ili9340WriteData(0x0E);
	ili9340WriteData(0x14);
	ili9340WriteData(0x03);
	ili9340WriteData(0x11);
	ili9340WriteData(0x07);
	ili9340WriteData(0x31);
	ili9340WriteData(0xC1);
	ili9340WriteData(0x48);
	ili9340WriteData(0x08);
	ili9340WriteData(0x0F);
	ili9340WriteData(0x0C);
	ili9340WriteData(0x31);
	ili9340WriteData(0x36);
	ili9340WriteData(0x0F);
	//**************************

	ili9340WriteCommand(ILI9340_SLPOUT);    //Exit Sleep

	i = 0;
	while(i < 500)
	{
	  ++i;
	}

	//Rotate the screen
	ili9340WriteCommand(ILI9340_MADCTL);
	ili9340WriteData(ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);

	ili9340WriteCommand(ILI9340_DISPON);    //Display on

/*	ili9340SetAddrWindow(0,0,320,240);

	for(i=0; i < 2*320*240; ++i)
	{
		ili9340WriteData(0xAA);
	}*/
}

void ili9340WriteCommand(uint16_t c){
	gioSetBit(gioPORTA, ILI9340_DC, 0);

	ili9340CmdBuf[0] = c;

	mibspiSetData(mibspiREG1,ILI9340_8BIT_TG,ili9340CmdBuf);
	mibspiTransfer(mibspiREG1, ILI9340_8BIT_TG);
	while(!(mibspiIsTransferComplete(mibspiREG1,ILI9340_8BIT_TG)))
	{
		//Maybe not needed?
	}
}

void ili9340WriteData(uint16_t d){
	gioSetBit(gioPORTA, ILI9340_DC, 1);

	ili9340CmdBuf[0] = d;

	mibspiSetData(mibspiREG1,ILI9340_8BIT_TG,ili9340CmdBuf);
	mibspiTransfer(mibspiREG1, ILI9340_8BIT_TG);
	while(!(mibspiIsTransferComplete(mibspiREG1,ILI9340_8BIT_TG)))
	{
		//Maybe not needed?
	}
}

void ili9340FillRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color)
{
	int i;
	int numPixels = (width)*(height);
	if(numPixels == 0){
		return;
	}

	ili9340SetAddrWindow(x,y,x+width-1,y+height-1);
	gioSetBit(gioPORTA, ILI9340_DC, 1);

	if(numPixels < ILI9340_MULTI_SIZE)
	{
		for(i=0; i<numPixels; ++i){
			ili9340WriteData(color>>8);
			ili9340WriteData(color);
		}
		return;
	}else{
		for(i=0; i < ILI9340_MULTI_SIZE; ++i)
		{
			ili9340PixBuf[i] = color;
		}
		mibspiSetData(mibspiREG1,ILI9340_16BIT_MULTI,ili9340PixBuf);
		mibspiTransfer(mibspiREG1,ILI9340_16BIT_MULTI);
		numPixels -= ILI9340_MULTI_SIZE;
		for(i=0;i<1000;++i){
			//Not sure why this delay is needed, but it fixes a bug...
		}
	}

	while(numPixels > 0)
	{
		if(numPixels >= ILI9340_MULTI_SIZE)
		{
			while(!(mibspiIsTransferComplete(mibspiREG1,ILI9340_16BIT_MULTI)))
			{

			}
			mibspiTransfer(mibspiREG1,ILI9340_16BIT_MULTI);
			numPixels -= ILI9340_MULTI_SIZE;
		}else{
			for(i=0;i<numPixels;++i)
			{
				ili9340WriteData(color>>8);		//Send last pixels one byte at a time
				ili9340WriteData(color);
			}
			return;
		}
	}

	/*while(!(mibspiIsTransferComplete(mibspiREG1,ILI9340_8BIT_TG)))
	{
	}*/
}

void ili9340DrawImage(uint16_t width, uint16_t height, const uint8_t * pixels){
	if(width > ILI9340_TFTWIDTH || height > ILI9340_TFTHEIGHT)
	{
		return;
	}

	int numPixels = width * height;
	int index = 0;
	int i;
	uint16_t x0,y0;
	x0 = (ILI9340_TFTWIDTH - width)/2;
	y0 = (ILI9340_TFTHEIGHT - height)/2;

	ili9340SetAddrWindow(x0, y0, x0+width, y0+height);

	for(i=0;i<ILI9340_MULTI_SIZE;++i)
	{
		ili9340PixBuf[i] = pixels[index++];
		ili9340PixBuf[i] |= pixels[index++]<<8;
	}
	numPixels -= ILI9340_MULTI_SIZE;

	gioSetBit(gioPORTA, ILI9340_DC, 1); //Get ready to send pixels

	mibspiSetData(mibspiREG1,ILI9340_16BIT_MULTI,ili9340PixBuf);
	mibspiTransfer(mibspiREG1,ILI9340_16BIT_MULTI);

	while(numPixels > 0)
	{
		if(numPixels >= ILI9340_MULTI_SIZE)
		{
			for(i=0;i<ILI9340_MULTI_SIZE;++i)
			{
				ili9340PixBuf[i] = pixels[index++];
				ili9340PixBuf[i] |= pixels[index++]<<8;
			}
			numPixels -= ILI9340_MULTI_SIZE;
			while(!(mibspiIsTransferComplete(mibspiREG1,ILI9340_16BIT_MULTI)))
			{

			}
			mibspiSetData(mibspiREG1,ILI9340_16BIT_MULTI,ili9340PixBuf);
			mibspiTransfer(mibspiREG1,ILI9340_16BIT_MULTI);
		}else{
			for(i=0;i<numPixels;++i)
			{
				ili9340WriteData(pixels[index]>>8);		//Send last pixels one byte at a time
				ili9340WriteData(pixels[index++]);
			}
			numPixels = 0;
		}
	}

	while(!(mibspiIsTransferComplete(mibspiREG1,ILI9340_16BIT_MULTI)))
	{

	}
}

void ili9340SetAddrWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1){
	ili9340WriteCommand(ILI9340_CASET); // Column addr set
	ili9340WriteData(x0 >> 8);
	ili9340WriteData(x0 & 0xFF); // XSTART
	ili9340WriteData(x1 >> 8);
	ili9340WriteData(x1 & 0xFF); // XEND
	ili9340WriteCommand(ILI9340_PASET); // Row addr set
	ili9340WriteData(y0>>8);
	ili9340WriteData(y0); // YSTART
	ili9340WriteData(y1>>8);
	ili9340WriteData(y1); // YEND
	ili9340WriteCommand(ILI9340_RAMWR); // write to RAM
}

void ili9340DrawChar(uint16_t x, uint16_t y, uint8_t size, char c, uint16_t color){
	uint8_t row,col;
	uint16_t pix, mask, x0, y0, blob;
	for(row = 0; row < 8; ++row)
	{
		pix = console_font_9x8[16*c + 2*row];
		pix = pix<<8;
		pix |= console_font_9x8[16*c + 2*row + 1];
		mask = 0x8000;
		for(col = 0; col < 9;){
			if(pix & mask){
				blob = 0;
				x0 = x+col*size;
				y0 = y+row*size;
				while(pix & mask && col < 9){		//Find "blobs" in the row to optimize drawing
					++blob;
					mask = mask >> 1;
					++col;
				}
				ili9340FillRect(x0, y0, blob*size, size, color);
			}else{
				++col;
				mask = mask >> 1;
			}
		}
	}
}

void ili9340Write(uint16_t x, uint16_t y, uint8_t size, char * s, uint16_t color){
	while(*s != '\0'){
		ili9340DrawChar(x,y,size,*s,color);
		x += 9*size;
		++s;
	}
}
