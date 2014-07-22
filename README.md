# RobotC I2C Lib for the HiTechnic SuperPro 
by Noah Moroze, Captain of FTC 4029

This library is under an LGPL license.

## About
Many FTC teams make use of 3rd party sensors via the HiTechnic SuperPro Prototype board. However, the Protoboard does not include native I2C support, so I wrote this bit-banged implementation of I2C. 

Wiring up the protoboard for I2C is easy - just pick two digital I/O pins to serve as your SDA and SCL pins and wire them up accordingly. You can configure which pin is which in software. Depending on the device you're connecting to, you may need pull-up resistors and/or a level shifter. 

For this library I've borrowed the API structure from the popular Arduino I2C ["Wire"](http://www.arduino.cc/en/Reference/Wire) library. Wire's API is based around Atmel's TWI library, so some of their TWI-dependent choices didn't make it into this I2C implementation. However, most Wire calls within an Arduino sketch can be directly translated into their HTSPB-I2C equivalent in a RobotC program. This makes it easier for a team that needs example code for a third-party I2C sensor to make use of Arduino example code, as this is easily found on the Internet. 

An important warning to note, however, is this I2C implementation is *slow*. I'm trying to figure out what to do about this, but since the only legal NXT -> Protoboard connection is via I2C, this means that I have to write a full byte to the Protoboard for every time either the SDA or SCL pin changes state (which happens very rapidly for high-speed communication). More info about this can be found in the Disclaimers section.

## Usage
To include HTSPB-I2C in your RobotC project, put the file i2c.h and the drivers folder in your project directory (both of these can be found under the folder "i2c"). Then put 

    #include "i2c.h"

at the top of your program to include the I2C library functions.

As mentioned previously, this library is used in a similar way to the Arduino Wire library. Function annotations are available in the file itself, below are some examples of usage:

    /* 
    No Wire equivalent, just sets up hardware for the library.
    This specific call says you're using pins 0 and 1 for SDA
    and SCL respectively, with the protoboard configured as "HTSPB"
    */
    i2c_begin(HTSPB, 0, 1); 

    //Transmits the byte 42 followed by the byte 57 to the slave at address 4
    i2c_beginTransmission(4);
    i2c_write(42);
    i2c_write(57);
    i2c_endTransmission(); // important to call this to have slave act on sent bytes

    //Arduino equivalent:
    Wire.beginTransmission(4);
    Wire.write(42);
    Wire.write(57);
    Wire.endTransmission();

    //Requests 2 bytes from the slave at address 4 and reads them
    i2c_requestFrom(4, 2);
    int available = i2c_available(); // returns number of bytes available in RX buffer, in this case 2 assuming no requestFrom calls have been made previously
    ubyte result = i2c_read(); // returns oldest byte read from a slave and flushes it from the buffer

    //Arduino equivalent:
    Wire.requestFrom(4, 2);
    int available = Wire.available();
    byte result = Wire.read();

As you can see, the function names and parameters are pretty much identical between the RobotC and Arduino versions, except "Wire." is replaced by the "i2c_" prefix. 

For an example of how a full I2C Arduino example can be translated into RobotC, compare the two files included in the "examples" folder.

## Disclaimers
The most important thing to watch out for is the fact that this library is *slow*. As mentioned above, I'm trying to figure out what to do about this, but since the only legal NXT to Protoboard connection is via I2C, this means that I have to write a full byte to the Protoboard for every time either the SDA or SCL pin changes state (which happens very rapidly for high-speed communication).

Based on my experiments, it can take 200 ms/byte to receive data from a slave (I don't have exact figures on solely writing data, but while it's probably slightly faster per byte, it's still fairly slow). That being said, this library is unsuitable for any application where you're constantly polling an I2C sensor. However, if you have an I2C device that only needs to occasionally share info with the Protoboard, this library will work. 

Keep in mind that this I2C implementation is *blocking*, and during this time *you may not call any other HTSPB functions or the transfer may fail*. 

A couple other small things to note:
* This library doesn't yet implement any of the slave features of the Wire library, I'm not sure if these are useful enough to FTC teams to warrant implementing them.
* Some return values have been changed from the Wire spec because they wouldn't be logical based on my implementation (see the function annotations in i2c.h for more details).
* ACK-bit checking is iffy and not fully tested. For most applications, I don't think this will be of any concern at all.
* Finally, this library is still a work-in-progress! I will try to get more testing done and iron some things out, and if you have improvements/suggestions please submit a pull request or create an issue on this repo.


## Acknowledgements
* Thanks to Xander for his [RobotC 3rd party sensor driver suite](https://github.com/botbench/rdpartyrobotcdr)! The files included under the "drivers" directory both come from his suite.