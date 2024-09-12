/*  Color Speak
    by Bithead

    Dependancies:
    - Arduino Pro Mini (5V, ATMega328, 16MHz)
    - TCS34725 Color Sensor (5V)
    - Adafruit SFX Sound Board (5V)

    Description:
    Button press triggers the logic to detect the color of an object in range and speak
    the color.  Useful for colour blind or visually impared people, and an inexpensive
    alternative to some similar items on the market.


    Pin   Function
    2     SFX RX
    3     SFX TX
    4     SFX RESET
    7     Trigger Button
    9     TCS34725 LED trigger
    A4    SDA  (I2C interface with TCS34725)
    A5    SCL  (I2C interface with TCS34725)

    Color Codes:
    Primary
      Red         1
      Yellow      2
      Blue        3

    Secondary
      Orange      4
      Green       5
      Purple      6

    Tertiary
      Yellow-orange (Y-O)   7
      Red-orange (R-O)      8
      Red-violet (R-V)      9
      Blue-violet (B-V)     10
      Blue-green (Aqua)     11
      Yellow-green (Y-G)    12

    Other
      Black       13
      White       14
      Grey        15
      Brown       16


 ************************************************************************************/
#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <SoftwareSerial.h>
#include <Adafruit_Soundboard.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 9
#define BUTTON_PIN 7
#define SFX_RX 2
#define SFX_TX 3
#define SFX_RESET 4
#define CELL_SIZE 32
#define RING_PIN 6

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_16X);
SoftwareSerial ss = SoftwareSerial(SFX_TX, SFX_RX);
Adafruit_Soundboard sfx = Adafruit_Soundboard(&ss, NULL, SFX_RESET);  //Software Serial Pointer, NULL (Debug, unused), SFX Reset Pin
Adafruit_NeoPixel strip = Adafruit_NeoPixel(16, RING_PIN, NEO_GRBW + NEO_KHZ800);

//                  B  G  R
uint8_t colorMatrix[8][8][8] =
{
  { // Z = 0
    //0,0
    {13, 13, 13, 1, 1, 1, 1, 1},
    {13, 13, 13, 1, 1, 1, 1, 1},
    {13, 5, 5, 16, 16, 1, 1, 1},
    {5, 5, 5, 5, 16, 4, 8, 1},
    {5, 5, 5, 5, 5, 4, 4, 1},
    {5, 5, 5, 5, 5, 5, 4, 2},
    {5, 5, 5, 5, 12, 12, 12, 2},
    {5, 5, 5, 5, 5, 2, 2, 2}
  },
  { // Z = 1
    //0,0
    {13, 13, 13, 1, 1, 1, 1, 1},
    {13, 13, 13, 1, 1, 1, 1, 1},
    {5, 5, 5, 16, 16, 8, 1, 1},
    {5, 5, 5, 5, 16, 4, 8, 4},
    {5, 5, 5, 5, 5, 16, 4, 4},
    {5, 5, 5, 5, 5, 5, 4, 7},
    {5, 5, 5, 5, 5, 12, 12, 2},
    {5, 5, 5, 5, 12, 12, 12, 2}
  },
  { // Z = 2
    //0,0
    {13, 3, 6, 6, 6, 9, 1, 1},
    {3, 3, 6, 6, 6, 9, 1, 1},
    {3, 5, 15, 6, 16, 8, 8, 1},
    {5, 5, 5, 5, 16, 8, 4, 1},
    {5, 5, 5, 5, 5, 4, 4, 4},
    {5, 5, 5, 5, 5, 5, 2, 7},
    {5, 5, 5, 5, 5, 12, 2, 7},
    {5, 5, 5, 5, 5, 5, 12, 2}
  },
  { // Z = 3
    //0,0
    {3, 3, 3, 6, 6, 9, 9, 1},
    {3, 3, 3, 6, 6, 9, 9, 1},
    {3, 3, 3, 6, 6, 9, 9, 1},
    {11, 11, 11, 15, 16, 16, 8, 1},
    {11, 5, 5, 5, 5, 16, 4, 16},
    {5, 5, 5, 5, 5, 13, 16, 4},
    {5, 5, 5, 5, 5, 5, 12, 7},
    {5, 5, 5, 5, 5, 5, 12, 2}
  },
  { // Z = 4
    //0,0
    {3, 3, 10, 6, 9, 9, 9, 9},
    {3, 3, 10, 6, 6, 9, 9, 9},
    {3, 3, 10, 6, 6, 9, 9, 9},
    {3, 3, 11, 10, 10, 9, 9, 9},
    {11, 11, 11, 11, 15, 9, 9, 9},
    {5, 5, 5, 5, 5, 7, 4, 4},
    {5, 5, 5, 5, 5, 5, 12, 7},
    {5, 5, 5, 5, 5, 12, 12, 2}
  },
  { // Z = 5
    //0,0
    {3, 3, 10, 10, 6, 6, 9, 9},
    {3, 3, 3, 10, 6, 6, 9, 9},
    {3, 3, 3, 10, 6, 6, 9, 9},
    {3, 3, 3, 10, 6, 6, 9, 9},
    {3, 11, 3, 11, 10, 10, 9, 9},
    {11, 11, 11, 11, 3, 15, 9, 9},
    {11, 11, 11, 11, 5, 5, 5, 7},
    {11, 11, 11, 5, 5, 5, 12, 2}
  },
  { // Z = 6
    //0,0
    {3, 3, 3, 10, 6, 6, 9, 9},
    {3, 3, 3, 10, 6, 6, 9, 9},
    {3, 3, 3, 10, 6, 6, 9, 9},
    {3, 3, 3, 10, 6, 6, 9, 9},
    {3, 3, 3, 10, 6, 6, 9, 9},
    {11, 11,  3, 3, 3, 10, 6, 9},
    {11, 11, 11, 11, 11, 11, 15, 9},
    {11, 11, 11, 11, 11, 11, 11, 2}
  },
  { // Z = 7
    //0,0
    {3, 3, 3, 3, 3, 6, 6, 6},
    {3, 3, 3, 3, 6, 6, 6, 6},
    {3, 3, 3, 10, 10, 6, 6, 6},
    {3, 3, 3, 3, 10, 6, 6, 9},
    {3, 3, 3, 3, 10, 6, 6, 9},
    {11, 11, 11, 3, 11, 10, 6, 9},
    {11, 11, 11, 11, 11, 11, 6, 9},
    {11, 11, 11, 11, 11, 11, 11, 14}
  }
};

uint8_t sound;

void setup() {
  Serial.begin(115200);
  Serial.println(F("** Color Speak Test **"));

  if (tcs.begin()) {
    //Serial.println(F("Found sensor"));
  } else {
    Serial.println(F("No TCS34725 found ... check your connections"));
    while (1); // halt!
  }

  pinMode(LED_PIN, OUTPUT);  //TCS34725 LED
  digitalWrite(LED_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);  //Trigger Button
  pinMode(RING_PIN, OUTPUT);  //NeoPixel Ring

  strip.begin();
  strip.setBrightness(50);
  strip.show(); // Initialize all pixels to 'off'

  ss.begin(9600);
  sound = 0;
  if (! sfx.reset()) {  //Ready
    Serial.println(F("No SFX board found ... check your connections"));
    while (1);  //halt!
  }
  else {
    sfx.volUp();
    sfx.playTrack(sound);
  }

  Serial.println(F("Ready!"));
}

void loop() {
  float red, green, blue;
  uint8_t r, g, b;

  if (digitalRead(BUTTON_PIN) == LOW)  //Low = Button pressed
  {
    lightOn();
    delay(500);
    tcs.getRGB(&red, &green, &blue);

    red = red *2;
    //green = green *1.8;
    green = green *2;
    blue = blue *2;
    if (red > 255) red = 255;
    if (green > 255) green = 255;
    if (blue > 255) blue = 255;

    Serial.print(F("R:\t")); Serial.print(int(red));
    Serial.print(F("\tG:\t")); Serial.print(int(green));
    Serial.print(F("\tB:\t")); Serial.println(int(blue));
    r = int(red / CELL_SIZE);
    g = int(green / CELL_SIZE);
    b = int(blue / CELL_SIZE);

    Serial.print(F("R:\t")); Serial.print(r);
    Serial.print(F("\tG:\t")); Serial.print(g);
    Serial.print(F("\tB:\t")); Serial.print(b);
    Serial.print(F("\tCode\t")); Serial.println(colorMatrix[b][g][r]);

    lightOff();
    sfx.playTrack(colorMatrix[b][g][r]);
    delay(1000);
  }
  delay(10);
}

void lightOn() {
  digitalWrite(LED_PIN, HIGH);
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(255, 255, 255));
  }
  strip.show();
}

void lightOff() {
  digitalWrite(LED_PIN, LOW);
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}
