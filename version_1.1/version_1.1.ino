#include <Wire.h>
#include "Adafruit_TCS34725.h"

#define redpin 19
#define greenpin 18
#define bluepin 5
#define commonAnode false

byte gammatable[256];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
  Serial.begin(9600);

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

#if defined(ARDUINO_ARCH_ESP32)
  ledcAttachPin(redpin, 1);
  ledcSetup(1, 12000, 8);
  ledcAttachPin(greenpin, 2);
  ledcSetup(2, 12000, 8);
  ledcAttachPin(bluepin, 3);
  ledcSetup(3, 12000, 8);
#else
  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
#endif

  for (int i = 0; i < 256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;

    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;
    }
  }
}

void loop() {
  // Generate random RGB values
  int randomRed = random(256);
  int randomGreen = random(256);
  int randomBlue = random(256);

  Serial.print("Random R:\t"); Serial.print(randomRed);
  Serial.print("\tG:\t"); Serial.print(randomGreen);
  Serial.print("\tB:\t"); Serial.print(randomBlue);
  Serial.println();

  // Set LED to random values
#if defined(ARDUINO_ARCH_ESP32)
  ledcWrite(1, gammatable[randomRed]);
  ledcWrite(2, gammatable[randomGreen]);
  ledcWrite(3, gammatable[randomBlue]);
#else
  analogWrite(redpin, gammatable[randomRed]);
  analogWrite(greenpin, gammatable[randomGreen]);
  analogWrite(bluepin, gammatable[randomBlue]);
#endif

  delay(1000);  // Wait for 1 second (adjust as needed)

  // Read color sensor values
  int red, green, blue, clear;
  int colorTemp, lux;

  tcs.setInterrupt(false);  // turn on LED

  delay(60);  // takes 50ms to read

  red = tcs.read8(TCS34725_RDATAL);  // Read 16-bit red value
  green = tcs.read8(TCS34725_GDATAL);  // Read 16-bit green value
  blue = tcs.read8(TCS34725_BDATAL);  // Read 16-bit blue value
  clear = tcs.read8(TCS34725_CDATAL);  // Read 16-bit clear value

  colorTemp = tcs.calculateColorTemperature(red, green, blue);
  lux = tcs.calculateLux(red, green, blue);
  tcs.setInterrupt(true);  // turn off LED

  Serial.print("Sensor R:\t"); Serial.print(red);
  Serial.print("\tG:\t"); Serial.print(green);
  Serial.print("\tB:\t"); Serial.print(blue);
  Serial.print("\tClear:\t"); Serial.print(clear);
  Serial.print("\tColor Temp:\t"); Serial.print(colorTemp);
  Serial.print("\tLux:\t"); Serial.print(lux);

  Serial.println();

  delay(5000);
}
