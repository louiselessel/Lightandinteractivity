/* Based on
   HueBlink example for ArduinoHttpClient library

   Uses ArduinoHttpClient library to control Philips Hue
   For more on Hue developer API see http://developer.meethue.com
   For more on the ArduinoHttpClient, install the library from the
   Library Manager.

  To control a light, the Hue expects a HTTP PUT request to:

  http://hue.hub.address/api/hueUserName/lights/lightNumber/state

  The body of the PUT request looks like this:
  {"on": true} or {"on":false}

  This example  shows how to concatenate Strings to assemble the
  PUT request and the body of the request.

  note: WiFi SSID and password are stored in arduino_secrets.h file.
  If it is not present, add a new tab, call it "arduino_secrets.h"
  and add the following defines, and change to your own values:

  #define SECRET_SSID "ssid"
  #define SECRET_PASS "password"
  char hueHubIP[] = "000.000.000.000";  // IP address of your hue bridge
  String hueUserName = "averylongstringofnumbersandletters"; // the user name you got from your hue bridge

   modified 6 Jan 2018
   by Tom Igoe (tigoe)
   https://github.com/tigoe/hue-control/tree/master/ArduinoExamples

   modified 6 April 2020
   by Louise Lessél
   Note: This code allows you to package all the hue bulb commands into one json string
   before sending it.
*/

#include <SPI.h>
//#include <WiFi101.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>
#include "arduino_secrets.h"

// ------ WIFI

int status = WL_IDLE_STATUS;      // the Wifi radio's status

// make a wifi instance and a HttpClient instance:
WiFiClient wifi;
HttpClient httpClient = HttpClient(wifi, hueHubIP);
// change the values of these two in the arduino_serets.h file:
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

// ------ GLOBAL VARIABLES
// variables for including multiple commands to hue in the json http request
int multipleItems = 0;
String phueCmd = "";


void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial); // wait for serial port to connect.

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data:
  Serial.print("You're connected to the network IP = ");
  IPAddress ip = WiFi.localIP();
  Serial.println(ip);
}


void loop() {

  // package to 2 part json
  int hueColor = random(0, 65535); // give random color
  packageRequest("hue", String(hueColor));
  packageRequest("on", "true");   // turn light on

  // send request to hue
  sendRequest(10, phueCmd);
  delay(4000);

  // package only one command and send request to hue
  packageRequest("on", "false");   // turn light off
  sendRequest(10, phueCmd);
  delay(4000);
}


void packageRequest(String cmd, String value) {
  // if this is the first call
  if (multipleItems < 1) {
    // make a string for the JSON command:
    phueCmd = "\"" + cmd;
    phueCmd += "\":";
    phueCmd += value;
  } else {
    // if this is the second call or more
    phueCmd += ",";
    phueCmd += "\"" + cmd;
    phueCmd += "\":";
    phueCmd += value;
  }

  // see what you assembled to send:
  Serial.print("packaged: ");
  Serial.println(phueCmd);

  // increment so we know packageRequest has been called before (this controls using the comma in json string)
  multipleItems++;
}


void sendRequest(int light, String packagedRequest) {
  // make a String for the HTTP request path:
  String request = "/api/" + hueUserName;
  request += "/lights/";
  request += light;
  request += "/state/";

  String contentType = "application/json";

  // make a string for the JSON command:
  String hueCmd = "{" + packagedRequest;
  hueCmd += "}";

  // see what you assembled to send:
  //Serial.print("PUT request to server: ");
  //Serial.println(request);
  Serial.print("JSON command to server: ");


  // make the PUT request to the hub:
  httpClient.put(request, contentType, hueCmd);

  // read the status code and body of the response
  int statusCode = httpClient.responseStatusCode();
  String response = httpClient.responseBody();

  Serial.println(hueCmd);
  Serial.print("Status code from server: ");
  Serial.println(statusCode);
  Serial.print("Server response: ");
  Serial.println(response);
  Serial.println();

  // reset variables
  multipleItems = 0;
}
