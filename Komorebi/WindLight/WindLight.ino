#include <Adafruit_NeoPixel.h> //including the Adafruit library$12
#include <ColorConverter.h>
#define PIN A7 //defining the PWM pin
#define N_LEDS 24      //number of LED units on the strip
#define N_LEDS_USE 7

Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_RGB + NEO_KHZ800);; //declared neopixel object
ColorConverter converter;

int startP = 9;
int endP = startP + 6;

int h = 120;         // hue
int sat = 15;        // saturation
int i = 90;        // intensity
int change = 1;              // increment to change hue by
RGBColor color;


uint32_t last_time;
uint32_t sway_time;

void setup() {
  strip.begin(); // This initializes the NeoPixel library.
  strip.clear();
  strip.show();
}

void loop() {

  uint32_t now = millis();

  int windspeedmph =  2.77; // mph
  int windgust =  33.16;

  windgust = constrain(int(map (windgust, 0, 50, 200, 100)), 100, 200); // mph
  //Serial.println(windgust);

  // wind avg
  int wind_dir = 260;//285;
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
  Serial.println(i);

  for (uint16_t i = startP; i < endP; i++) {
    strip.setPixelColor(i, color.red, color.green, color.blue);
    strip.show();
  }

  boolean test = true;

  // make a wind gust
  if (test) {
    if (now > last_time + 500) {
      // control only 6 pixels is on at a time
      startP = random(0, 22);
      endP = startP + random(4, 6);

      // wind gusts
      windMovementRight(200, 255, 200, windgust);
      windMovementLeft(150, 255, 150, windgust);
      //delay(random(50, 1000));

      startP = startP + 2;
      endP = startP + random(4, 8);

      windMovementRight(200, 255, 200, windgust);
      windMovementLeft(75, 125, 75, windgust);
      //delay(random(50, 100));
      now = millis();
      last_time = now;
    }
  } else {
    // make a wind gust
    if (now > last_time + 500) {
      // control only 6 pixels is on at a time
      startP = random(0, 22);
      endP = startP + random(4, 6);

      // wind gusts
      color = converter.HSItoRGB(120, 10, 255);
      windMovementRight(windgust, color);
      windMovementLeft(windgust, color);
      //delay(random(50, 1000));

      startP = startP + 2;
      endP = startP + random(4, 8);

      windMovementRight(windgust, color);
      windMovementLeft(windgust, color);
      //delay(random(50, 100));
      now = millis();
      last_time = now;
    }
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


void windMovementRight(uint8_t wait, RGBColor color) {
  for (uint16_t i = startP; i < endP; i++) {
    strip.setPixelColor(i, color.red, color.green, color.blue);
    if (i > startP) {
      strip.setPixelColor(i - 1, color.red / 2, color.green / 2, color.blue / 2);
    }
    if (i > startP + 1) {
      strip.setPixelColor(i - 2, 0, 0, 0);
    }
    strip.show();
    delay(wait);
  }
}

void windMovementLeft(uint8_t wait, RGBColor color) {
  for (uint16_t i = endP; i > startP; i--) {
    strip.setPixelColor(i, color.red, color.green, color.blue);
    if (i > startP) {
      strip.setPixelColor(i + 1, color.red / 2, color.green / 2, color.blue / 2);
    }
    if (i > startP + 1) {
      strip.setPixelColor(i + 2, 0, 0, 0);
    }
    strip.show();
    delay(wait);
  }
}
