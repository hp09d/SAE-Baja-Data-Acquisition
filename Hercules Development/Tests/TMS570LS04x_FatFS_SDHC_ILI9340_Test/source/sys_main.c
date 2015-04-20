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
#include "ili9340.h"
#include "ff.h"
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */
FATFS disk;
FIL file;
/* USER CODE END */

void main(void)
{
/* USER CODE BEGIN (3) */
	char buf[100];
	FRESULT RES;

	ili9340Init();
	ili9340FillRect(0,0,320,240,0xFFFF);

	RES = f_mount(&disk,"",1);

	if(RES != FR_OK){
		while(1);
	}

	f_open(&file,"ti.raw", FA_READ | FA_OPEN_EXISTING );

	ili9340DrawFile(&file,282,70);

	/*uint8_t img[19740];
	uint32_t br;

	f_read(&file, &img[0], 19740, &br);

	ili9340DrawImage(282,70,&img[0]);*/

	f_close(&file);

	RES = f_open(&file,"test.txt", FA_READ | FA_OPEN_EXISTING);

	if(RES != FR_OK){
		while(1);
	}

	uint16_t i = 0;

	while( !f_eof(&file) && i < 11 ){
		f_gets(buf,100,&file);
		ili9340Write(7,20*i+10,1,&buf[0],0x001F);
		++i;
	}

	f_close(&file);

	while(1);
/* USER CODE END */
}

/* USER CODE BEGIN (4) */
/* USER CODE END */
