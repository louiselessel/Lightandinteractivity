/*
    An example for Grove - Human Presence Sensor

    Copyright (c) 2018 seeed technology co., ltd.
    Author      : Jack Shao
    Create Time: June 2018
    Change Log :

    The MIT License (MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

    Adapted to control DMX ligthing using https://github.com/tigoe/sACNSource by Tom Igoe
    by Louise Lessél
    April 14 2020

   To run this code you'll need to set up your own DMX on ethernet
   and add a tab to this sketch called arduino_secrets.h
   for the SSID and password of the network to which you plan to connect, as follows:
   #define SECRET_SSID "ssid"  // fill in your value
   #define SECRET_PASS "password" // fill in your value
   #define SECRET_SACN_RECV "192.168.0.14"  // your sACM receiver's IP address
   // Unique ID of your SACN source. You can generate one from https://uuidgenerator.net
   // or on the command line by typing uuidgen
   #define SECRET_SACN_UUID "CBC0C271-8022-4032-BC6A-69F614C62816"

   created 17 Jan 2018
   updated 2 Apr 2020
   by Tom Igoe

*/
/* AK9753 movement sensor */
#include <Wire.h>
#include "Grove_Human_Presence_Sensor.h"

/* DMX over network */
#include <SPI.h>
//#include <WiFi101.h>      // use this for MKR1000
#include <WiFiNINA.h>       // use this for MKR1010, Nano33 IoT
//#include <ESP8266WiFi.h>    // This should work with the ESP8266 as well.
#include <WiFiUdp.h>
#include <sACNSource.h>
#include "arduino_secrets.h"



/* AK9753 movement sensor */
AK9753 movementSensor;
// need to adjust these sensitivities lower if you want to detect more far
// but will introduce error detection
float sensitivity_presence = 1.0;
float sensitivity_movement = 5.0;
int detect_interval = 30; //milliseconds
PresenceDetector detector(movementSensor, sensitivity_presence, sensitivity_movement, detect_interval);
uint32_t last_time;
int intervalSensor = 100;                     // in milliseconds

/* DMX over network */
WiFiUDP Udp;                                  // instance of UDP library
sACNSource myController(Udp);                 // Your Ethernet-to-DMX device
int myUniverse = 1;                                 // DMX universe
char myDevice[] = "myDeviceNamsee";                   // sender name
int intervalDMX = 500;                              // in milliseconds
uint32_t last_time_DMX;
int r = 0;
int g = 0;
int b = 0;

void setup() {
  Serial.begin(9600);
  /* AK9753 initalization */
  Serial.println("Grove - Human Presence Sensor initializing");
  Wire.begin();

  //Turn on sensor
  if (movementSensor.initialize() == false) {
    Serial.println("Device not found. Check wiring.");
    while (1);
  }

  /* DMX setup */
  //  while you're not connected to a WiFi AP
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(SECRET_SSID);           // print the network name (SSID)
    WiFi.begin(SECRET_SSID, SECRET_PASS);     // try to connect
    delay(2000);
  }
  // initialize sACN source:
  myController.begin(myDevice, SECRET_SACN_UUID, myUniverse);

  // When you're connected, print out the device's network status:
  IPAddress ip = WiFi.localIP();
  //Serial.print("IP Address: ");
  //Serial.println(ip);

  // set DMX channel values to 0:
  for (int dmxChannel = 0; dmxChannel < 513; dmxChannel++) {
    myController.setChannel(dmxChannel, 0);
  }
  myController.sendPacket(SECRET_SACN_RECV);



  last_time = millis();
}

void loop() {
  detector.loop();

  uint32_t now = millis();
  if (now - last_time > intervalSensor) {
    uint8_t m = detector.getMovement();  //read movement state will clear it

    // movement 1-3 (up down) in blue
    Serial.print(detector.getDerivativeOfDiff13());
    Serial.print(" ");

    //plot a pulse 1-3 - red spikes
    if (m & MOVEMENT_FROM_1_TO_3) {             // to down
      Serial.print("20 ");
      r += 10;
    } else if (m & MOVEMENT_FROM_3_TO_1) {      // to up
      Serial.print("-20 ");
      r -= 10;
    } else {
      Serial.print("0 ");
    }

    // movement 2-4 (left - right) in green
    Serial.print(detector.getDerivativeOfDiff24());
    Serial.print(" ");

    //plot a pulse for 2-4 - orange spikes
    if (m & MOVEMENT_FROM_2_TO_4) {             // to right
      Serial.println("20 ");
      b += 10;
    } else if (m & MOVEMENT_FROM_4_TO_2) {      // to left
      Serial.println("-20 ");
      b -= 10;
    } else {
      Serial.println("0 ");
    }

    last_time = now;
  }

  delay(1); // minimal delay for sensor

  /* DMX commands */
  if (now - last_time_DMX > intervalDMX) {
    r = constrain(r, 0, 255);
    g = constrain(g, 0, 255);
    b = constrain(b, 0, 255);
    Serial.println(r);
    
    myController.setChannel(81, r);              // set channel on ADJ Dotz Par at ITP floor
    //myController.setChannel(82, g);              // set channel 
    myController.setChannel(83, b);              // set channel 
    myController.sendPacket(SECRET_SACN_RECV);       // send the data
    
    delay(100);                                    // wait .1 second

    last_time_DMX = now;
  }

  /* DMX commands */
  /*
    // fade up:
    for (int level = 0; level < 256; level++) {
    myController.setChannel(4, level);              // set channel 1 (intensity)
    Serial.println(level);                          // print level
    myController.sendPacket(SECRET_SACN_RECV);       // send the data
    delay(100);                                    // wait .1 second
    }

    delay(1000);
    // fade down:
    for (int level = 255; level >= 0; level--) {
    myController.setChannel(4, level);              // set channel 1 (intensity)
    Serial.println(level);                          // print level
    myController.sendPacket(SECRET_SACN_RECV);      // send the data
    delay(100);                                    // wait .1 second
    }
    delay(1000);
  */

}
