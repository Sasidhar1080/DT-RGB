#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define redpin 19
#define greenpin 18
#define bluepin 5
#define commonAnode false

int randomRed, randomGreen, randomBlue;
int red , green , blue , clear ;
int colorTemp, lux;

const char* ssid = "IIIT-Guest";
const char* password = "I%GR#*S@!";

#define CSE_IP      "dev-onem2m.iiit.ac.in"
#define OM2M_ORGIN    "Tue_20_12_22:Tue_20_12_22"
#define CSE_PORT    443
#define HTTPS     false
#define OM2M_MN     "/~/in-cse/in-name/"
#define OM2M_AE     "AE-AQ"
#define OM2M_NODE_ID   "AQ-MG00-00"
#define OM2M_DATA_CONT  "AQ-MG00-00/Data"
#define OM2M_DATA_LBL   "[\"AE-AQ\", \"V4.0.0\", \"AQ-MG00-00\", \"AQ-V4.0.0\"]"


// const char *ssid = "SCRC Lab 3";
// const char *password = "Scrciiith@123";

// #define CSE_IP          "192.168.0.109"
// #define CSE_PORT        8200
// #define HTTPS           false
// #define OM2M_ORGIN      "admin:admin"
// #define OM2M_MN         "/~/in-cse/in-name/"
// #define OM2M_AE         "AE-DT"
// #define OM2M_DATA_CONT  "DT-1/Data"

WiFiClient client;
HTTPClient http;

byte gammatable[256];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void onem2m() {
  // String data = "[" + String(randomRed) + "," + String(randomGreen) + "," + String(randomBlue) + "]";
  String data = "[" + String(randomRed) + " , " + String(randomGreen) + " , " + String(randomBlue) + " , " +
                String(red) + " , " + String(green) + " , " + String(blue) + " , " +
                String(clear) + " , " + String(colorTemp) + " , " + String(lux) + "]";
  String server = "http://" + String(CSE_IP) + ":" + String(CSE_PORT) + String(OM2M_MN);

  http.begin(client, server + OM2M_AE + "/" + OM2M_DATA_CONT);
  http.addHeader("X-M2M-Origin", OM2M_ORGIN);
  http.addHeader("Content-Type", "application/json;ty=4");
  
  String req_data = String() + "{\"m2m:cin\": {"
                    + "\"con\": \"" + data + "\","
                    + "\"rn\": \"" + "cin_" + String(millis()) + "\","
                    + "\"cnf\": \"text\""
                    + "}}";

  Serial.println("Server URL: " + server);
  Serial.println("Request Data: " + req_data);

  int code = http.POST(req_data);
  http.end();
  Serial.println("HTTP Response Code: " + String(code));
  delay(1000);
}

void turnOffLED() {
  digitalWrite(13, LOW); // Assuming pin 13 controls the LED
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  pinMode(13,OUTPUT);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

  tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_101MS);
  tcs.setGain(TCS34725_GAIN_1X);

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
  turnOffLED();
  randomRed = random(256);
  randomGreen = random(256);
  randomBlue = random(256);

  Serial.print("Random R:\t"); Serial.print(randomRed);
  Serial.print("\tG:\t"); Serial.print(randomGreen);
  Serial.print("\tB:\t"); Serial.print(randomBlue);
  Serial.println();

#if defined(ARDUINO_ARCH_ESP32)
  ledcWrite(1, gammatable[randomRed]);
  ledcWrite(2, gammatable[randomGreen]);
  ledcWrite(3, gammatable[randomBlue]);
#else
  analogWrite(redpin, gammatable[randomRed]);
  analogWrite(greenpin, gammatable[randomGreen]);
  analogWrite(bluepin, gammatable[randomBlue]);
#endif

  delay(30000);

  tcs.setInterrupt(false);
  red = tcs.read8(TCS34725_RDATAL);
  green = tcs.read8(TCS34725_GDATAL);
  blue = tcs.read8(TCS34725_BDATAL);
  clear = tcs.read8(TCS34725_CDATAL);

  colorTemp = tcs.calculateColorTemperature(red, green, blue);
  lux = tcs.calculateLux(red, green, blue);
  tcs.setInterrupt(true);

  Serial.print("Sensor R:\t"); Serial.print(red);
  Serial.print("\tG:\t"); Serial.print(green);
  Serial.print("\tB:\t"); Serial.print(blue);
  Serial.print("\tClear:\t"); Serial.print(clear);
  Serial.print("\tColor Temp:\t"); Serial.print(colorTemp);
  Serial.print("\tLux:\t"); Serial.print(lux);
  Serial.println();

  onem2m();

  delay(30000);
}
