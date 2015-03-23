#include "lsm303.h"
#include "I2C_SoftwareLibrary.h"
#include <msp430.h>

#define ACCEL_ADDRESS_READ 	( 0x33 >> 1 )
#define ACCEL_ADDRESS_WRITE	( 0x32 >> 1 )

#define MAG_ADDRESS_READ	( 0x3D >> 1 )
#define MAG_ADDRESS_WRITE	( 0x3C >> 1 )

void EnableDefault ( void )
{
	WriteAccelReg( LSM303DLH_CTRL_REG1_A, 0x57 );
}

void WriteAccelReg( char reg, char value )
{
	beginTransmission( ACCEL_ADDRESS_WRITE );
	writebyte( reg );
	writebyte( value );
	endTransmission( );
}

char ReadAccelReg ( char reg )
{
	char value;
	beginTransmission( ACCEL_ADDRESS_READ );
	writebyte( reg );
	endTransmission( );
	requestFrom( ACCEL_ADDRESS_READ, 1);
	value = read( );
	endTransmission( );
	return value;
}

char ReadMagReg ( char reg )
{
	char value;

	beginTransmission( MAG_ADDRESS_READ );
	writebyte( reg );
	endTransmission( );
	requestFrom( MAG_ADDRESS_READ, 1 );
	value = read( );
	endTransmission( );

	return value;
}

void ReadAccel ( int* x, int* y, int* z )
{
	beginTransmission( ACCEL_ADDRESS_READ );
	writebyte( LSM303DLH_OUT_X_L_A  | (1 << 7 ) );
	endTransmission( );
	requestFrom( ACCEL_ADDRESS_READ, 6 );

	while( available() < 6 );

	char _x[2] = {0, 0} , _y[2] = {0, 0}, _z[2] = {0, 0};

	_x[0] = read();
	_x[1] = read();
	_y[0] = read();
	_y[1] = read();
	_z[0] = read();
	_z[1] = read();

	*x = _x[0] | ( _x[1] << 8 );
	*y = _y[0] | ( _y[1] << 8 );
	*z = _z[0] | ( _z[1] << 8 );
}

void ReadMag ( int* xm, int* ym, int* zm )
{
	beginTransmission( MAG_ADDRESS_READ );
	writebyte( LSM303DLH_OUT_X_H_M );
	endTransmission();
	requestFrom( MAG_ADDRESS_READ, 6 );

	while ( available() < 6 );

	char _xm[2] = {0, 0}, _ym[2] = {0, 0}, _zm[2] = {0, 0};

	_xm[0] = read();
	_xm[1] = read();
	_ym[0] = read();
	_ym[1] = read();
	_zm[0] = read();
	_zm[0] = read();

	*xm = _xm[0] | (_xm[1] << 8);
	*ym = _ym[0] | (_ym[1] << 8);
	*zm = _zm[0] | (_zm[1] << 8);
}

void Read ( int* x, int* y, int* z, int* xm, int* ym, int* zm )
{
	ReadAccel( x, y, z );
	ReadMag( xm, ym, zm );
}
