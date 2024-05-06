#include <Wire.h>
#include "Adafruit_TCS34725.h"

#define redpin 19
#define greenpin 18
#define bluepin 5

#define commonAnode true


byte gammatable[256];


// Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_60X);

void setup() {
  Serial.begin(9600);
  
  if (tcs.begin()) {
    
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); 
  }


  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);


  for (int i=0; i<256; i++) {
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
  float red, green, blue;
  
  tcs.setInterrupt(false); 

  delay(60);

  tcs.getRGB(&red, &green, &blue);
  
  tcs.setInterrupt(true);

  Serial.print("R:\t"); Serial.print(int(red)); 
  Serial.print("\tG:\t"); Serial.print(int(green)); 
  Serial.print("\tB:\t"); Serial.println(int(blue));
  
  // analogWrite(redpin, gammatable[(int)red]);
  // analogWrite(greenpin, gammatable[(int)green]);
  // analogWrite(bluepin, gammatable[(int)blue]);

  analogWrite(redpin, map(red, 0, 255, 0, 255));  // Map sensor value directly to LED value
  analogWrite(greenpin, map(green, 0, 255, 0, 255));
  analogWrite(bluepin, map(blue, 0, 255, 0, 255));

  // delay(2000);
}
