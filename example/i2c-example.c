#pragma config(Sensor, S1, HTSPB, sensorI2CCustom9V);

/*
Example Usage of RobotC I2C implementation for the HiTechnic Protoboard
By Noah Moroze, FTC 4029 Captain

This example program uses my RobotC I2C library to retrieve and display data
from all 3 axes of the Parallax HMC5883L compass module breakout. It should
be compatible with different brands of breakouts for the same compass chip.

Bit-banging I2C on the protoboard is fairly slow - even though this loop is
running as tightly as possible, it still takes a little over 1 second to
read all 6 bytes per iteration.

Wiring:
* Protoboard pin 0 -> compass SDA
* Protoboard pin 1 -> compass SCL
* Protoboard 3v -> compass VIN
* Protoboard GND -> compass GND
*/

#include "i2c.h"

#define Addr 0x1E

task main()
{
	i2c_begin(HTSPB, 0, 1); //initialize I2C library

	//write 2 bytes to set continuous mode
	i2c_beginTransmission(Addr);
	i2c_write(0x02);
	i2c_write(0x00);
	i2c_endTransmission();

	while(true) {
		int x, y, z;

		//following 3 commands write to X MSB read register
		i2c_beginTransmission(Addr);
		i2c_write(0x03);
		i2c_endTransmission();

		i2c_requestFrom(Addr, 6); //request 6 bytes from compass
		while(i2c_available() < 6); //wait until all 6 bytes are received
		//(note: not strictly necessary, as i2c_requestFrom is blocking)

		//read all 6 bytes and combine MSB and LSB for each axis
		x = i2c_read() << 8 | i2c_read();
		z = i2c_read() << 8 | i2c_read();
		y = i2c_read() << 8 | i2c_read();

		//display value for each axis
		nxtDisplayCenteredTextLine(2, "X: %d", x);
		nxtDisplayCenteredTextLine(3, "Y: %d", y);
		nxtDisplayCenteredTextLine(4, "Z: %d", z);
	}
}
