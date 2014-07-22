/*
   Compass Module 3-Axis HMC5883L
   
   Read the Compass Module 3-Axis HMC5883L and prints them over the serial connection to the computer.
   
   The circuit:
    * SDA (Data) output of compass to analog pin 4
    * SCL (Clock) output of compass to analog pin 5
    * +V of accelerometer to +5V
    * GND of accelerometer to ground

   created 29 Nov 2012
   by Tony Guntharp
   Modified 21 July 2014
   by Noah Moroze
 */
 
#include <Wire.h>

#define Addr 0x1E               // 7-bit address of HMC5883 compass

void setup() {
  Serial.begin(9600);
  delay(100);                   // Power up delay
  Wire.begin();
  
  // Set operating mode to continuous
  Wire.beginTransmission(Addr); 
  Wire.write(byte(0x02));
  Wire.write(byte(0x00));
  Wire.endTransmission();
}

void loop() {
  int x, y, z;

  // Initiate communications with compass
  Wire.beginTransmission(Addr);
  Wire.write(byte(0x03));       // Send request to X MSB register
  Wire.endTransmission();

  Wire.requestFrom(Addr, 6);    // Request 6 bytes; 2 bytes per axis
  while(Wire.available() < 6);    // If 6 bytes available 
  x = Wire.read() << 8 | Wire.read();
  z = Wire.read() << 8 | Wire.read();
  y = Wire.read() << 8 | Wire.read();
  
  Serial.print("X:");
  Serial.print(x);
  Serial.print(" Y:");
  Serial.print(y);
  Serial.print(" Z:");
  Serial.print(z);

  delay(500);
}

