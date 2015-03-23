//
// SoftwareWire.cpp
// Library C++ code
// ----------------------------------
// Developed with embedXcode
// http://embedXcode.weebly.com
//
// Project embed1
//
// Created by Rei VILO, mars 29, 2013 09:59
// http://embedXcode.weebly.com
//
//
// Copyright © Rei VILO, 2013-2014
// License CC = BY NC SA
//
// See I2C_SoftwareLibrary.h and ReadMe.txt for references
//
#include "I2C_SoftwareLibrary.h"
#include <msp430g2553.h>
#include "Energia.h"
///
/// @brief Delay values for software I2C
///
#define F_CPU 16000000L

const unsigned char DELAY_LONG = 32;
const unsigned char DELAY_FULL = 8;
const unsigned char DELAY_HALF = 4;
const unsigned char DELAY_PART = 2;
#define I2C_READ 1
#define I2C_WRITE 0
#define TICKS_PER_MS (F_CPU / 1000)
#define TICKS_PER_US (TICKS_PER_MS / 1000)

static char _pinSDA, _pinSCL;

void delayI2Cms(unsigned int delay)
{
    while (delay--) __delay_cycles(TICKS_PER_MS);
}
void delayI2Cus(unsigned int delay)
{
    while (delay--) __delay_cycles(TICKS_PER_US);
}
unsigned char rxBuffer[BUFFER_LENGTH];
unsigned char rxBufferIndex = 0;
unsigned char rxBufferLength = 0;
unsigned char txAddress = 0;
unsigned char txBuffer[BUFFER_LENGTH];
unsigned char txBufferIndex = 0;
unsigned char txBufferLength = 0;
unsigned char transmitting = 0;

void SoftwareWireInit( unsigned char pinSDA, unsigned char pinSCL )
{
	_pinSDA = pinSDA;
	_pinSCL = pinSCL;
}

void begin()
{
    pinMode(_pinSDA, OUTPUT);
    digitalWrite(_pinSDA, HIGH);
    pinMode(_pinSCL, OUTPUT);
    digitalWrite(_pinSCL, HIGH);
    rxBufferIndex = 0;
    rxBufferLength = 0;
    txBufferIndex = 0;
    txBufferLength = 0;
}

void beginTransmission(unsigned char address)
{
    transmitting = 1;
    txAddress = address;
    txBufferIndex = 0;
    txBufferLength = 0;
}

unsigned char endTransmission(void)
{
    unsigned char result = 0;
    startI2C(txAddress, I2C_WRITE);
    result += writeI2C(txBuffer, txBufferLength);
    stopI2C();
    txBufferIndex = 0;
    txBufferLength = 0;
    transmitting = 0;
    return result;
}

unsigned char requestFrom(unsigned char address, unsigned char length)
{
    if (length > BUFFER_LENGTH) length = BUFFER_LENGTH;
    startI2C(address, I2C_READ);
    if (length > 1) {
    	unsigned char i;
        for ( i=0; i<length-1; i++) rxBuffer[i] = readI2Cbyte(false);
    }
    rxBuffer[length-1] = readI2Cbyte(true);
    stopI2C();
    rxBufferIndex = 0;
    rxBufferLength = length;
    return length;
}

unsigned int writebyte(unsigned char data)
{
    if (transmitting){
        if (txBufferLength >= BUFFER_LENGTH) {
            return 0;
        }
        txBuffer[txBufferIndex] = data;
        ++txBufferIndex;
        txBufferLength = txBufferIndex;
    } else {
        writeI2Cbyte(data);
    }
    return 1;
}

unsigned int write(const unsigned char *data, unsigned int length)
{
	unsigned int i;
    if (transmitting) {
        for (i = 0; i < length; ++i) writebyte(data[i]);
    } else {
        for (i = 0; i < length; ++i) writeI2Cbyte(data[i]);
    }
    return length;
}

int available(void)
{
    return rxBufferLength - rxBufferIndex;
}

int read(void)
{
    int value = -1;
    if(rxBufferIndex < rxBufferLength){
        value = rxBuffer[rxBufferIndex];
        ++rxBufferIndex;
    }
    return value;
}

int peek(void)
{
    int value = -1;
    if (rxBufferIndex < rxBufferLength) value = rxBuffer[rxBufferIndex];
    return value;
}

void flush(void)
{
    ;
}

unsigned char readI2Cbyte(unsigned char last)
{
    unsigned char data = 0;
    digitalWrite(_pinSDA, LOW);
    delayI2Cus(DELAY_LONG);
    pinMode(_pinSDA, INPUT);
    delayI2Cus(DELAY_HALF);
    unsigned char i;
    for (i = 0; i < 8; i++) {
        data <<= 1;
        digitalWrite(_pinSCL, HIGH);
        delayI2Cus(DELAY_PART);
        if ( digitalRead(_pinSDA) ) data |= 1;
        delayI2Cus(DELAY_PART);
        digitalWrite(_pinSCL, LOW);
        delayI2Cus(DELAY_HALF);
    }
    pinMode(_pinSDA, OUTPUT);
    digitalWrite(_pinSDA, last);
    digitalWrite(_pinSCL, HIGH);
    delayI2Cus(DELAY_HALF);
    digitalWrite(_pinSCL, LOW);
    digitalWrite(_pinSDA, LOW);
    delayI2Cus(DELAY_LONG);
    return data;
}

unsigned char readI2C(unsigned char* data, unsigned char length)
{
    if (length > 1) {
        while (--length) *data++= readI2Cbyte(false);
    }
    *data = readI2Cbyte(true);
}

unsigned char restartI2C(unsigned char address, unsigned char RW)
{
    digitalWrite(_pinSDA, HIGH);
    digitalWrite(_pinSCL, HIGH);
    delayI2Cus(DELAY_FULL);
    return startI2C(address, RW);
}

unsigned char startI2C(unsigned char address, unsigned char RW)
{
    pinMode(_pinSDA, OUTPUT);
    digitalWrite(_pinSDA, LOW);
    digitalWrite(_pinSCL, LOW);
    delayI2Cus(DELAY_FULL);
    return writeI2Cbyte((address << 1) + RW);
}

void stopI2C(void) {
    pinMode(_pinSDA, OUTPUT);
    digitalWrite(_pinSDA, LOW);
    delayI2Cus(DELAY_FULL);
    digitalWrite(_pinSCL, HIGH);
    delayI2Cus(DELAY_FULL);
    digitalWrite(_pinSDA, HIGH);
    delayI2Cus(DELAY_FULL);
}

unsigned char writeI2Cbyte(unsigned char data)
{
    pinMode(_pinSDA, OUTPUT);
    unsigned char i;
    for (i=0; i < 8; ++i ){
        if (data & 0x80) digitalWrite(_pinSDA, HIGH);
        else digitalWrite(_pinSDA, LOW);
        digitalWrite(_pinSCL, HIGH);
        data <<= 1;  
        delayI2Cus(DELAY_HALF);
        digitalWrite(_pinSCL, LOW);
        delayI2Cus(DELAY_HALF);
    }
    pinMode(_pinSDA, INPUT);
    digitalWrite(_pinSCL, HIGH);
    unsigned char result = ! ( digitalRead(_pinSDA) );
    if (result) digitalWrite(_pinSDA, LOW);
    delayI2Cus(DELAY_HALF);
    digitalWrite(_pinSCL, LOW);
    delayI2Cus(DELAY_FULL);
    return result;
}

unsigned char writeI2C(unsigned char* data, unsigned int length)
{
    unsigned char result;
    unsigned char i;
    for (i=0; i < length; i++) result = writeI2Cbyte(data[i]);
    return result;
}
