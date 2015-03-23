///
/// @mainpage I2C_Software
///
/// @details Software I2C for MSP430
/// @n
/// @n @b   Contribute!
/// @n  Help me for my developments: http://embeddedcomputing.weebly.com/contact
///
/// @n @a Developed with [embedXcode](http://embedXcode.weebly.com)
///
/// @author Rei VILO
/// @author http://embedXcode.weebly.com
/// @date Feb 10, 2014
/// @version 104
///
/// @copyright (c) Rei VILO, 2013-2014hi
/// @copyright CC = BY NC SA
///
/// @see
/// * ReadMe.txt for references
/// * TwoWire.h - TWI/I2C library for Arduino & Wiring
/// @n Copyright (c) 2006 Nicholas Zambetti.  All right reserved.
/// * Arduino core files for MSP430
/// @n Copyright (c) 2012 Robert Wessels. All right reserved.
#ifndef I2C_MASTER_H
#define I2C_MASTER_H
#define BUFFER_LENGTH 16


 	/* FUNCTIONS TO BE USED BY THE PROGRAMMER */
    void SoftwareWireInit(unsigned char pinSDA, unsigned char pinSCL);
    void begin();
    void beginTransmission(unsigned char address);
    unsigned char endTransmission(void);
    unsigned int writebyte(unsigned char data);
    unsigned int write(const unsigned char *data , unsigned int length);
    unsigned char requestFrom(unsigned char address, unsigned char length);
    int available(void);
    int read(void);
    int peek(void);
    void flush(void);

    /* DO NOT CALL THESE ALONE */
    unsigned char readI2Cbyte(unsigned char last);
    unsigned char readI2C(unsigned char* data, unsigned char length);
    unsigned char restartI2C(unsigned char address, unsigned char RW);
    unsigned char startI2C(unsigned char address, unsigned char RW);
    void stopI2C();
    unsigned char writeI2Cbyte(unsigned char data);
    unsigned char writeI2C(unsigned char* data, unsigned int length);
#endif
