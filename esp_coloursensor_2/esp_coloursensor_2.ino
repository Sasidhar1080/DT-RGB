#include <Wire.h>
#include "Adafruit_TCS34725.h"

#define redpin 9
#define greenpin 10
#define bluepin 11
#define commonAnode false

#define led_sensor1 13
#define led_sensor2 16

Adafruit_TCS34725 tcs1 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs2 = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
  Serial.begin(9600);
  
  tcs1.begin();
  tcs1.setIntegrationTime(TCS34725_INTEGRATIONTIME_101MS);
  tcs1.setGain(TCS34725_GAIN_1X);

  tcs2.begin();
  tcs2.setIntegrationTime(TCS34725_INTEGRATIONTIME_101MS);
  tcs2.setGain(TCS34725_GAIN_1X);

  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);
  pinMode(led_sensor1, OUTPUT);
  pinMode(led_sensor2, OUTPUT);
}

void turnOffLED() {
  digitalWrite(led_sensor1, LOW);
  digitalWrite(led_sensor2, LOW);
}

void loop() {
  turnOffLED();
  uint16_t red1, green1, blue1, clear1;
  uint16_t red2, green2, blue2, clear2;

  tcs1.setInterrupt(false);
  tcs1.getRawData(&red1, &green1, &blue1, &clear1);
  tcs1.setInterrupt(true);

  analogWrite(redpin, map(red1, 0, 65535, 0, 255));
  analogWrite(greenpin, map(green1, 0, 65535, 0, 255));
  analogWrite(bluepin, map(blue1, 0, 65535, 0, 255));

  Serial.print("Sensor 1 - R: "); Serial.print(red1);
  Serial.print(", G: "); Serial.print(green1);
  Serial.print(", B: "); Serial.print(blue1);
  Serial.print(", C: "); Serial.println(clear1);

  delay(10000);

  turnOffLED();

  tcs2.setInterrupt(false);
  tcs2.getRawData(&red2, &green2, &blue2, &clear2);
  tcs2.setInterrupt(true);

  analogWrite(redpin, map(red2, 0, 65535, 0, 255));
  analogWrite(greenpin, map(green2, 0, 65535, 0, 255));
  analogWrite(bluepin, map(blue2, 0, 65535, 0, 255));

  Serial.print("Sensor 2 - R: "); Serial.print(red2);
  Serial.print(", G: "); Serial.print(green2);
  Serial.print(", B: "); Serial.print(blue2);
  Serial.print(", C: "); Serial.println(clear2);

  delay(10000);
}
