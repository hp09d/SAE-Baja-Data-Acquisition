/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "gio.h"
#include "spi.h"

/** GO_IDLE_STATE - init card in spi mode if CS low */
#define CMD0 0X00
/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
#define CMD8 0X08
/** SEND_CSD - read the Card Specific Data (CSD register) */
#define CMD9 0X09
/** SEND_CID - read the card identification information (CID register) */
#define CMD10 0X0A
/** END_MULTI_BLOCK - End a multiblock read */
#define CMD12 0x0C
/** SEND_STATUS - read the card status register */
#define CMD13 0X0D
/** READ_BLOCK - read a single data block from the card */
#define CMD17 0X11
/** READ_MULTI_BLOCK - read multiple blocks */
#define CMD18 0x12
/** WRITE_BLOCK - write a single data block to the card */
#define CMD24 0X18
/** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
#define CMD25 0X19
/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
#define CMD32 0X20
/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
#define CMD33 0X21
/** ERASE - erase all previously selected blocks */
#define CMD38 0X26
/** APP_CMD - escape for application specific command */
#define CMD55 0X37
/** READ_OCR - read the OCR register of a card */
#define CMD58 0X3A
/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
#define ACMD23 0X17
/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
#define ACMD41 0X29
//------------------------------------------------------------------------------
/** status for card in the ready state */
#define R1_READY_STATE 0X00
/** status for card in the idle state */
#define R1_IDLE_STATE 0X01
/** status bit for illegal command */
#define R1_ILLEGAL_COMMAND 0X04
/** start data token for read or write single block*/
#define DATA_START_BLOCK 0XFE
/** stop token for write multiple blocks*/
#define STOP_TRAN_TOKEN 0XFD
/** start data token for write multiple blocks*/
#define WRITE_MULTIPLE_TOKEN 0XFC
/** mask for data response tokens after a write block operation */
#define DATA_RES_MASK 0X1F
/** write data accepted token */
#define DATA_RES_ACCEPTED 0X05
//------------------------------------------------------------------------------

spiDAT1_t dataConfig1;

uint16_t getByte(){
	uint16_t src = 0xFF;
	uint16_t dst = 0;

	spiTransmitAndReceiveData(spiREG1, &dataConfig1, 1, &src, &dst);
	return dst;
}

uint16_t sendCmd(uint8_t cmd, uint32_t arg){
	uint16_t i = 0;
	uint16_t src[5];
	uint16_t out[5];

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

	spiTransmitAndReceiveData(spiREG1, &dataConfig1, 5, &src[0], &out[0]); //send cmd and arg
	spiTransmitAndReceiveData(spiREG1, &dataConfig1, 1, &crc, &resp); //send CRC

	for(i = 0; i < 0xFFFF; ++i){
		if( !(resp & 0x80) ){
			return resp;
		}
		resp = getByte();
	}

	return 0xFF;
}

uint16_t sendAcmd(uint8_t acmd, uint32_t arg){
	sendCmd(CMD55,0);
//	getByte();
	return sendCmd(acmd,arg);
}

uint16_t rcv_block(uint8_t *buff){
	uint8_t rcv = 0xFF;
	uint16_t timer,i;

	for(timer = 10000; timer > 0; --timer){
		rcv = getByte();
		if(rcv != 0xFF){
			break;
		}
	}

	if( rcv != DATA_START_BLOCK ){
		return 1;
	}

	for( i = 512; i != 0; --i ){
		rcv = getByte() & 0xFF;
		*(buff++) = rcv;
	}

	for( i = 2; i != 0; --i ){
		getByte();		//Disregard the CRC
	}

	return 0;
}

uint16_t xmit_block(const uint8_t * buf){
	uint16_t rcv = 0;
	uint16_t xmit = 0;
	uint16_t i;

	while(rcv != 0xFF){
		rcv = getByte();
	}

	for(i = 0; i < 512; ++i){
		xmit = *(buf++);
		spiTransmitData(spiREG1, &dataConfig1, 1, &xmit);
	}

	for(i = 0; i< 2; ++i){
		getByte();		//Dummy CRC
	}

	rcv = getByte();
	if( (rcv & 0x1F) != 0x05 ){
		return 1;
	}

	return 0;
}

void SD_select(){

}

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	return 0;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	spiInit();
	gioInit();
	dataConfig1.CSNR = SPI_CS_1;		//Using chip select #1 (mibSPI1CS[1])
	dataConfig1.CS_HOLD = 1; 			//Hold CS between transfers
	dataConfig1.DFSEL = SPI_FMT_0;		//Select data format 0
	dataConfig1.WDEL = 0;				//No extra delays
	gioPORTA->DIR &= 0xFFFE;		//GIOA[0] is chip detect
	gioPORTA->DIR |= 0x0002;		//GIOA[1] is chip select

	uint32_t i = 0;

	while( !( gioPORTA->DIN & 0x01 ) )
	{
		//return STA_NOINIT;
	}
	for(i=0;i<0x00FFFFFF;++i){}  //Wait

	/*
	 * BEGIN SD CARD INITIALIZATION
	 */

	//Send 80 cycles with SIMO and CS high
	uint16_t cmd[5] = {0xFF,0,0,0,0};
	dataConfig1.CSNR = 0xFF;
	gioPORTA->DOUT |= 0x0002;
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
		return STA_NOINIT; //Card is not SD2
	}else{
		//Response is R7, get last byte
		for( i = 0; i < 4; ++i){
			status = getByte();
		}
		if(status != 0xAA){
			return STA_NOINIT; //Error
		}
		//Card is SD2
	}

	for(i=0; i<5; ++i){
		getByte();
	}

	status = sendAcmd(ACMD41,0X40000000);
	while(status != R1_READY_STATE){
		for(i=0; i<5; ++i){
			getByte();
		}
		status = sendAcmd(ACMD41,0X40000000);
	}

	if(sendCmd(CMD58,0)){
		return STA_NOINIT; //Error
	}

	if(getByte() & 0xC0 != 0xC0){
		return STA_NOINIT; //Card is not SDHC
	}else{
		//Card is SDHC
		for( i = 0; i < 3; ++i ){
			getByte();	//Throw away rest of response
		}
	}

	for(i=0; i < 10; ++i){
		getByte();
	}

	return 0;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read */
)
{
	uint16_t status;
	uint8_t cmd;

	if(count > 1){
		cmd = CMD18;  //Multi-Block
	}else{
		cmd = CMD17;
	}

	status = sendCmd(cmd, sector);

	if( status != 0 ){
		return RES_ERROR; //Fail
	}

	while(count--){
		if( rcv_block(buff) ){
			break;
		}
		buff += 512;
	}

	if(cmd == CMD18){
		sendCmd(CMD12,0); //End multi-block
	}

	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	uint16_t status;
	uint16_t xmit;

	if(count > 1){
		sendCmd(ACMD23,count);
		status = sendCmd(CMD25,sector);
		if(status != 0){
			return RES_ERROR;
		}
		xmit = WRITE_MULTIPLE_TOKEN;
		spiTransmitData(spiREG1, &dataConfig1, 1, &xmit);
		while(count--){
			if( xmit_block(buff) ){
				break;
			}
			buff += 512;
		}
		xmit = STOP_TRAN_TOKEN;
		spiTransmitData(spiREG1, &dataConfig1, 1, &xmit);
	}else{
		status = sendCmd(CMD24,sector);
		if(status == 0){
			xmit = DATA_START_BLOCK;
			spiTransmitData(spiREG1, &dataConfig1, 1, &xmit);
			if( xmit_block(buff) ){
				count = 0;
			}
		}

	}

	if(count == 0){
		return RES_OK;
	}else{
		return RES_ERROR;
	}
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res;
	int result;

	return RES_PARERR;
}
#endif
