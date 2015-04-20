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
#include "spi.h"
#include "gio.h"
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */
// SD card commands
/** GO_IDLE_STATE - init card in spi mode if CS low */
uint8_t const CMD0 = 0X00;
/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
uint8_t const CMD8 = 0X08;
/** SEND_CSD - read the Card Specific Data (CSD register) */
uint8_t const CMD9 = 0X09;
/** SEND_CID - read the card identification information (CID register) */
uint8_t const CMD10 = 0X0A;
/** SEND_STATUS - read the card status register */
uint8_t const CMD13 = 0X0D;
/** READ_BLOCK - read a single data block from the card */
uint8_t const CMD17 = 0X11;
/** WRITE_BLOCK - write a single data block to the card */
uint8_t const CMD24 = 0X18;
/** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
uint8_t const CMD25 = 0X19;
/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
uint8_t const CMD32 = 0X20;
/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
uint8_t const CMD33 = 0X21;
/** ERASE - erase all previously selected blocks */
uint8_t const CMD38 = 0X26;
/** APP_CMD - escape for application specific command */
uint8_t const CMD55 = 0X37;
/** READ_OCR - read the OCR register of a card */
uint8_t const CMD58 = 0X3A;
/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
uint8_t const ACMD23 = 0X17;
/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
uint8_t const ACMD41 = 0X29;
//------------------------------------------------------------------------------
/** status for card in the ready state */
uint8_t const R1_READY_STATE = 0X00;
/** status for card in the idle state */
uint8_t const R1_IDLE_STATE = 0X01;
/** status bit for illegal command */
uint8_t const R1_ILLEGAL_COMMAND = 0X04;
/** start data token for read or write single block*/
uint8_t const DATA_START_BLOCK = 0XFE;
/** stop token for write multiple blocks*/
uint8_t const STOP_TRAN_TOKEN = 0XFD;
/** start data token for write multiple blocks*/
uint8_t const WRITE_MULTIPLE_TOKEN = 0XFC;
/** mask for data response tokens after a write block operation */
uint8_t const DATA_RES_MASK = 0X1F;
/** write data accepted token */
uint8_t const DATA_RES_ACCEPTED = 0X05;
//------------------------------------------------------------------------------

uint16_t sendCmd(uint8_t cmd, uint32_t arg);
uint16_t sendAcmd(uint8_t acmd, uint32_t arg);
uint16_t getByte();

uint16_t dataRead(uint32_t block, uint16_t offset, uint16_t count, uint16_t * dest);

spiDAT1_t dataConfig1; 	//SPI configuration

uint16_t dest[512];
/* USER CODE END */

void main(void)
{
/* USER CODE BEGIN (3) */
	spiInit(); //Initialize SPI
	gioInit();

	gioPORTA->DIR &= 0xFFFE;		//GIOA[0] is chip detect
	gioPORTA->DIR |= 0x0002;		//GIOA[1] is chip select
	gioPORTA->DOUT |= 0x02;			//CS high


	//Set up the configuration
	dataConfig1.CSNR = SPI_CS_1;		//Using chip select #1 (mibSPI1CS[1])
	dataConfig1.CS_HOLD = 1; 			//Hold CS between transfers
	dataConfig1.DFSEL = SPI_FMT_0;		//Select data format 0
	dataConfig1.WDEL = 0;				//No extra delays

	uint32_t i = 0;

	while( !( gioPORTA->DIN & 0x01 ) )
	{
		//Wait for a card to be inserted
	}
	for(i=0;i<0x00FFFFFF;++i){}

	/*
	 * BEGIN SD CARD INITIALIZATION
	 */

	//Send 80 cycles with SIMO and CS high
	uint16_t cmd[5] = {0xFF,0,0,0,0};
	dataConfig1.CSNR = 0xFF;
	for( i = 0; i < 11; ++i){
		spiTransmitData(spiREG1, &dataConfig1, 1, &cmd[0]);
	}

	dataConfig1.CSNR = SPI_CS_1;
	gioPORTA->DOUT &= 0xFFFD; //CS Low
	spiTransmitData(spiREG1, &dataConfig1, 1, &cmd[0]); //Send an FF just cause
	uint16_t status;

	status = sendCmd(CMD0,0);
	while(status != R1_IDLE_STATE){
		//Wait for card to go idle
		status = sendCmd(CMD0,0);
	}

	status = sendCmd(CMD8,0x1AA);
	if( status & R1_ILLEGAL_COMMAND ){
		while(1){} //Card is not SD2
	}else{
		//Response is R7, get last byte
		for( i = 0; i < 4; ++i){
			status = getByte();
		}
		if(status != 0xAA){
			while(1){} //Error
		}
		//Card is SD2
	}

	status = sendAcmd(ACMD41,0X40000000);
	while(status != R1_READY_STATE){
		status = sendAcmd(ACMD41,0X40000000);
	}

	if(sendCmd(CMD58,0)){
		while(1){} //Error
	}

	if(getByte() & 0xC0 != 0xC0){
		while(1){} //Card is not SDHC
	}else{
		//Card is SDHC
		for( i = 0; i < 3; ++i ){
			getByte();	//Throw away rest of response
		}
	}

	/*
	 * END SD CARD INITIALIZATION
	 */

	/*
	 * BEGIN PARTITION TABLE READ
	 */

	dataRead( 0, 0x1BE, 16, &dest[0] );	//Read the partition table

	uint32_t part1_begin = dest[8] | (dest[9] << 8) | (dest[10] << 16) | (dest[11] << 24);
	uint32_t part1_size = dest[12] | (dest[13] << 8) | (dest[14] << 16) | (dest[15] << 24);

	/*
	 * END PARTITION TABLE READ
	 */

	/*
	 * 	BEGIN MBR READ
	 */

	if( dataRead( part1_begin, 0, 512, &dest[0]) ){
		while(1){} //Read Error
	}

	uint16_t nRES = dest[0x0E] | (dest[0x0F] << 8);
	uint16_t nFAT = dest[0x10];
	uint32_t FAT_size = dest[0x24] | (dest[0x25] << 8) | (dest[0x26] << 16) | (dest[0x27] << 24);

	/*
	 * END MBR READ
	 */

	/*
	 * BEGIN ROOT READ
	 */

	uint32_t FROOT = part1_begin + nRES + (nFAT*FAT_size);

	if( dataRead( FROOT, 0, 512, &dest[0]) ){
		while(1){} //Read Error
	}

	/*
	 * END ROOT READ
	 */
	while(1)
	{

	}
/* USER CODE END */
}

/* USER CODE BEGIN (4) */
uint16_t sendCmd(uint8_t cmd, uint32_t arg){
	uint16_t i = 0;
	uint16_t src[5];

	while(getByte() != 0xFF){
		//Wait for ready
	}

	src[0] = cmd | 0x40;
	for( i = 0; i < 4; ++i)
	{
		src[i+1] = (arg >> 8*(3-i)) & 0xFF;
	}

	uint16_t resp = 0x80; //Init to some invalid response
	uint16_t crc = 0xFF;
	if (cmd == CMD0) crc = 0X95;  // correct crc for CMD0 with arg 0
	if (cmd == CMD8) crc = 0X87;  // correct crc for CMD8 with arg 0X1AA

	spiTransmitData(spiREG1, &dataConfig1, 5, &src[0]); //send cmd and arg
	spiTransmitAndReceiveData(spiREG1, &dataConfig1, 1, &crc, &resp); //send CRC

	while( resp & 0x80 ){ //Send clocks until a response is received
		resp = getByte();
	}

	return resp;
}

uint16_t sendAcmd(uint8_t acmd, uint32_t arg){
	sendCmd(CMD55,0);
	return sendCmd(acmd,arg);
}

uint16_t getByte(){
	uint16_t src = 0xFF;
	uint16_t dst = 0;

	spiTransmitAndReceiveData(spiREG1, &dataConfig1, 1, &src, &dst);
	return dst;
}

uint16_t dataRead(uint32_t block, uint16_t offset, uint16_t count, uint16_t * dest){
	uint16_t status = sendCmd(CMD17,block);
	uint16_t i;

	if( count + offset > 512 ){
		return 1;		//Out of bounds
	}

	if( status != 0 ){
		return 1; 		//Fail
	}else{
		status = 0xFF;
		while(status == 0xFF){
			status = getByte();
		}
		if( status != DATA_START_BLOCK ){
			return 1; 	//Fail
		}
	}

	for( i = 0; i < offset; ++i ){
		getByte();			//Skip to the offset
	}
	for( i = 0; i< count; ++i ){
		dest[i] = getByte();		//Read the data
	}
	for( i = count+offset; i < 512; ++i ){
		getByte(); 			//Throw away  the rest
	}

	return 0;
}
/* USER CODE END */
