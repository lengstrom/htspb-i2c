/*
RobotC I2C implementation for the HiTechnic Protoboard
Copyright 2014 Noah Moroze, FTC 4029 Captain

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/lgpl.txt>.

API is based on the Wire library for Arduino
http://arduino.cc/en/Reference/Wire
*/

#include "drivers/hitechnic-superpro.h"

tSensors _i2c_HTSPB;
ubyte _i2c_sda;
ubyte _i2c_scl;
bool _i2c_sda_state;
bool _i2c_scl_state;

ubyte _i2c_rxBuffer[32]; //max buffer size is 32
ubyte _i2c_rxBufferLen = 0;

/**
 * Initialize I2C library
 * @param HTSPB - the HiTechnic Sensor Protoboard being used
 * @param sda - which digital pin (from 0-7) is being used as the SDA pin
 * @param scl - which digital pin (from 0-7) is being used as the SCL pin
 */
void i2c_begin(tSensors HTSPB, int sda, int scl); // init I2C and join bus as master

/**
 * Send start sequence and address of slave to begin sending data
 * @param address - the 7-bit address of the slave
 * @param write - if 0, begins transmission in write mode, otherwise begins transmission in read mode (default 0)
 */
void i2c_beginTransmission(ubyte address, ubyte write);
void i2c_beginTransmission(ubyte address);

/**
 * Send stop sequence
 */
void i2c_endTransmission(); // send stop sequence

/**
	* Write byte to slave
	* @param value - byte to write to slave
	* @return - true if ACK bit received, false if not
	*/
bool i2c_write(ubyte value);

/**
  * Write bytes to slave
  * @param values - bytes to write to slave
  * @param length - how many bytes to write from the array, starting at index 0
  * @return - how many bytes received an ACK bit in response
  */
int i2c_write(ubyte *values, int length);

/**
  * Write string to slave
  * @param str - string to write to slave
  * @return - how many bytes received an ACK bit in response
  */
int i2c_write(char *str);

/**
 	* Read bytes from slave into RX buffer
 	* @param address - address of slave
 	* @param quantity - number of bytes to request
 	* @return - number of bytes received
 	*/
ubyte i2c_requestFrom(ubyte address, int quantity);

/**
  * @return - number of bytes in RX buffer
  */
int i2c_available(); // returns number of unread bytes in the read buffer

/**
	* @return - the oldest byte in RX buffer
	*/
ubyte i2c_read();	// return first byte in read buffer

// internal functions to handle I/O
void _i2c_sda_write(bool val);
bool _i2c_sda_read();
void _i2c_scl_write(bool val);
bool _i2c_scl_read();

void i2c_begin(tSensors HTSPB, int sda, int scl) {
	_i2c_HTSPB = HTSPB;
	_i2c_sda = 1 << sda;
	_i2c_scl = 1 << scl;
}

void i2c_beginTransmission(ubyte address) {
	i2c_beginTransmission(address, 0);
}

void i2c_beginTransmission(ubyte address, ubyte write) {
	// write start sequence
	_i2c_sda_write(1);
	_i2c_scl_write(1);
	_i2c_sda_write(0);
	_i2c_scl_write(0);

	// write address of slave
	i2c_write((address << 1) | write); // send write bit as LSB
}

void i2c_endTransmission() {
	// write stop sequence
	_i2c_sda_write(0);
	_i2c_scl_write(1);
	_i2c_sda_write(1);
}

bool i2c_write(ubyte value) {
	ubyte mask;

	// iterates over bits from MSB to LSB and writes each one
	for(mask = 1 << 7; mask != 0; mask >>= 1) {
		bool b = (value & mask) != 0;
		_i2c_sda_write(b);
		_i2c_scl_write(1);
		_i2c_scl_write(0);
	}

	// check ACK bit
	_i2c_sda_write(1);
	_i2c_scl_write(1);
	bool ack = !_i2c_sda_read();
	_i2c_scl_write(0);
	_i2c_sda_write(0);
	return ack;
}

int i2c_write(ubyte *values, int length) {
	int ack = 0;
	int i;
	for(i=0; i<length; i++) {
		if(i2c_write(values[i]))
			ack++;
	}

	return ack;
}

int i2c_write(char* str) {
	int length = strlen(str);
	int ack = 0;
	int i;
	for(i=0; i<length; i++) {
		if(i2c_write(str[i]))
			ack++;
	}

	return ack;
}

ubyte i2c_requestFrom(ubyte address, int quantity) {
	i2c_beginTransmission(address, 1);

	int i;
	for(i=0; i<quantity; i++) {
		_i2c_sda_write(1); // let go of SDA line
		ubyte response = 0;
		int j;
		for(j=0; j<8; j++) {
			response <<= 1;
			_i2c_scl_write(1);
			long beganReadTime = time1[T1];
			// wait for clock stretching
			while(!_i2c_scl_read()) {
				if(time1[T1] > beganReadTime + 1000) // timeout after 1000ms
					return i;
			}
			wait1Msec(2); // wait a bit before reading
			response |= (_i2c_sda_read() ? 1 : 0);
			_i2c_scl_write(0);
		}

		_i2c_sda_write(0); // send ACK bit
		_i2c_scl_write(1);
		_i2c_scl_write(0);
		_i2c_rxBuffer[_i2c_rxBufferLen] = response;
		_i2c_rxBufferLen++;
	}

	return quantity;
}

int i2c_available() {
	return _i2c_rxBufferLen;
}

ubyte i2c_read() {
	ubyte val = _i2c_rxBuffer[0]; // return last byte read
	// shift remaining bytes over one
	int i;
	for(i=0; i<_i2c_rxBufferLen-1; i++) {
		_i2c_rxBuffer[i] = _i2c_rxBuffer[i+1];
	}
	_i2c_rxBufferLen--;

	return val;
}

void _i2c_sda_write(bool val) {
	_i2c_sda_state = val;
	ubyte mask =
		(_i2c_sda_state ? 0:1) * _i2c_sda |
		(_i2c_scl_state ? 0:1) * _i2c_scl;
	HTSPBsetupIO(_i2c_HTSPB, mask);
}

void _i2c_scl_write(bool val) {
	_i2c_scl_state = val;
	ubyte mask =
		(_i2c_sda_state ? 0:1) * _i2c_sda |
		(_i2c_scl_state ? 0:1) * _i2c_scl;
	HTSPBsetupIO(_i2c_HTSPB, mask);
}

bool _i2c_sda_read() {
	return HTSPBreadIO(_i2c_HTSPB, _i2c_sda) != 0;
}

bool _i2c_scl_read() {
	return HTSPBreadIO(_i2c_HTSPB, _i2c_scl) != 0;
}
