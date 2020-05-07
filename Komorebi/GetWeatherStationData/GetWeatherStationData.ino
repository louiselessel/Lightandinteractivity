/*  This sketches accesses the ITP Weather Station's database,
    gets weather data for selected dates,
    and parses the data to save them into variables.

    You'll need to know the weather station's mac addrss and session key
    to access the database.

    April 2020

    This example sketch is put together by Yeseul Song for the ITP Weather Band,
    based on Tom Igoe's Connected Devices example.
    Thanks: Arnab Chakravarty.
*/

#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <WiFiNINA.h> // you might want to use WiFi101.h instead depending on which arduino you're using
#include <Time.h>
#include "arduino_secrets.h"

#include <Adafruit_NeoPixel.h> //including the Adafruit library$12
#include <ColorConverter.h>
#define PIN A7 //defining the PWM pin
#define N_LEDS 24      //number of LED units on the strip
#define N_LEDS_USE 7

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_RGB + NEO_KHZ800);; //declared neopixel object
ColorConverter converter;

// RTC zero - https://github.com/arduino-libraries/RTCZero


char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
String mac_identity = MAC_ID;
String session_identity = SESSION_KEY;

const char serverAddress[] = "tigoe.io";  // server address
int port = 443;  // for https

WiFiSSLClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

int lastCheck;
int interval = 10000;
uint32_t now;
uint32_t last_time;
uint32_t sway_time;

const char* recorded_at;
int wind_dir;
int windddir_avg2m;
float windspeedmph;
float windgustdir_10m;
float windgustmph_10m;

int r = 100;
int g = 100;
int b = 100;

int startP = 9;
int endP = startP + 6;

int h = 120;         // hue
int sat = 15;        // saturation
int i = 90;        // intensity
int change = 1;              // increment to change hue by
RGBColor color;



void setup() {

  Serial.begin(9600);
  while (!Serial);
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);     // print the network name (SSID);

    // connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
  }

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  //t = now(); // store the current time in time variable t
  //t = setTime(23,0,0,4,5,2020); // Another way to set
  //setTime(t);

  strip.begin(); // This initializes the NeoPixel library.
  strip.clear();
  strip.show();
}

void loop() {
  /*
    Serial.println(minute(t));
    Serial.println(second(t));
    Serial.println(timeStatus());
  */

  now = millis();
  if (now > lastCheck + interval) {
    // get data from the database
    getData();

    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    // below is to parse the response using the ArduinoJson library

    // remove the '[', ']' and extra white spaces from the response for parsing
    response = response.substring(3, response.length() - 2);

    // set the capacity of the memory pool in bytes
    DynamicJsonDocument doc(500);

    // deserialize the JSON document
    DeserializationError error = deserializeJson(doc, response);

    // test if parsing suceeds
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    // extract data and assign them to each variable
    recorded_at = doc["recorded_at"];
    wind_dir = doc["wind_dir"]; // 225
    windddir_avg2m = doc["winddir_avg2m"]; // 206
    windspeedmph = doc["windspeedmph"]; // 0.47
    windgustdir_10m = doc["windgustdir_10m"]; // 270
    windgustmph_10m = doc["windgustmph_10m"]; // 4.45

    // Print values
    Serial.print("recorded_at: ");
    Serial.println(recorded_at);
    Serial.print("wind_dir: ");
    Serial.println(wind_dir);
    Serial.print("windddir_avg2m: ");
    Serial.println(windddir_avg2m);
    Serial.print("windspeedmph: ");
    Serial.println(windspeedmph);
    Serial.print("windgustdir_10m: ");
    Serial.println(windgustdir_10m);
    Serial.print("windgustmph_10m: ");
    Serial.println(windgustmph_10m);

    lastCheck = now;
  }

  //int windspeedmph =  0.47; // mph
  //int windgust =  4.5;

  windgustmph_10m = constrain(int(map (windgustmph_10m, 0, 30, 150, 20)), 20, 150); // mph
  //Serial.println(windgust);

  // wind avg
  //int wind_dir = 225;
  // approximate direction
  if (wind_dir < 90) {
    startP = 0;
  } else if (wind_dir < 180) {
    startP = 6 ;
  } else if (wind_dir < 270) {
    startP = 12 ;
  } else if (wind_dir < 360) {
    startP = 18 ;
  }
  //startP = constrain(int(map (wind_dir, 0, 360, 0, 24)), 0, 24);
  endP = startP + 1;
  //Serial.println(startP);

  // increment
  if (now > sway_time + 500) {
    i = i + change;
    if (i < 50 || i > 99) {
      change = -change;
    }
    sway_time = now;
    color = converter.HSItoRGB(h, sat, i);
  }
  //Serial.println(i);

  for (uint16_t i = startP; i < endP; i++) {
    strip.setPixelColor(i, color.red, color.green, color.blue);
    strip.show();
  }


  // make a wind gust
  if (now > last_time + 10000) {
    // control only 6 pixels is on at a time
    startP = random(startP, 22);
    endP = startP + random(2, 6);

    // wind gusts
    windMovementRight(200, 255, 200, windgustmph_10m);
    windMovementLeft(75, 100, 75, windgustmph_10m);
    delay(random(50, 1000));

    startP = startP + 2;
    endP = startP + random(2, 6);

    windMovementRight(200, 255, 200, windgustmph_10m);
    windMovementLeft(75, 100, 75, windgustmph_10m);
    delay(random(50, 100));
    now = millis();
    last_time = now;
  }
}

void windMovementRight(int r, int g, int b, uint8_t wait) {
  for (uint16_t i = startP; i < endP; i++) {
    // on
    strip.setPixelColor(i, r, g, b);
    // off
    if (i > startP) {
      strip.setPixelColor(i - 1, r / 2, g / 2, b / 2);
    }
    if (i > startP + 1) {
      strip.setPixelColor(i - 2, 0, 0, 0);
    }
    //strip.setPixelColor(random(startP, endP), r, g, b);
    //strip.setPixelColor(startP, r, g, b);
    strip.show();
    delay(wait);
  }
  strip.setPixelColor(startP, 0, 0, 0);
  strip.show();
}

void windMovementLeft(int r, int g, int b, uint8_t wait) {
  for (uint16_t i = endP; i > startP; i--) {
    // on
    strip.setPixelColor(i, r, g, b);
    // off
    if (i > startP) {
      strip.setPixelColor(i + 1, r / 2, g / 2, b / 2);
    }
    if (i > startP + 1) {
      strip.setPixelColor(i + 2, 0, 0, 0);
    }
    //strip.setPixelColor(random(startP, endP), r, g, b);
    //strip.setPixelColor(endP, r, g, b);
    strip.show();
    delay(wait);
  }
  strip.setPixelColor(endP, 0, 0, 0);
  strip.show();
}


void getData() {

  // set path and content type
  String path = "/itpower-data/by-time";
  String contentType = "application/json";

  // set the date/time range. Get only 1-2 at a time, otherwise, the memory can't deal with it.
  String dateFrom = "5-06-2020 15:00:00";
  String dateTo = "5-06-2020 15:10:00";

  // assemble the query
  String mac = "\"macAddress\":\"" + mac_identity + "\"";
  String session = "\"sessionKey\":" + session_identity + "";
  String dFrom = "\"dateFrom\":\"" + dateFrom + "\"";
  String dTo = "\"dateTo\":\"" + dateTo + "\"";

  //combining GET request data as a JSON string object
  String getBody = "{" + mac + "," + session + "," + dFrom + "," + dTo + "}";

  Serial.println(path);
  Serial.println(getBody);

  // make the request:
  client.beginRequest();
  client.get(path);
  client.sendHeader("Content-Type", "application/json");
  client.sendHeader("Content-Length", getBody.length());
  client.beginBody();
  client.print(getBody);
  client.endRequest();

}



/*
  float rainin = doc["rainin"];
  float dailyrainin = doc["dailyrainin"];
  float rain_avg1m = doc["rain_avg1m"];
  float rain_avg10m = doc["rain_avg10m"];
  float temperature = doc["temperature"];
  float humidity = doc["humidity"];
  float pressure = doc["pressure"];
  float illuminance = doc["illuminance"];
  float uva = doc["uva"];
  float uvb = doc["uvb"];
  float uvindex = doc["uvindex"];
*/

/*
  Serial.print("rainin: ");
  Serial.println(rainin);
  Serial.print("dailyrainin: ");
  Serial.println(dailyrainin);
  Serial.print("rain_avg1m: ");
  Serial.println(rain_avg1m);
  Serial.print("rain_avg10m: ");
  Serial.println(rain_avg10m);
  Serial.print("temperature: ");
  Serial.println(temperature);
  Serial.print("humidity: ");
  Serial.println(humidity);
  Serial.print("pressure: ");
  Serial.println(pressure);
  Serial.print("illuminance: ");
  Serial.println(illuminance);
  Serial.print("uva: ");
  Serial.println(uva);
  Serial.print("uvb: ");
  Serial.println(uvb);
  Serial.println("uvindex: ");
  Serial.println(uvindex);
*/
