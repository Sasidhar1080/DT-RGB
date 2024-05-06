#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define redpin 19
#define greenpin 18
#define bluepin 5
#define commonAnode false

int randomRed ,randomGreen,randomBlue;
// Read color sensor values
int avgRed, avgGreen, avgBlue, avgClear;
int colorTemp, lux;

// const char *ssid = "SCRC_LAB_IOT";
// const char *password = "Scrciiith@123";

// #define CSE_IP      "dev-onem2m.iiit.ac.in"//192.168.0.109
// #define CSE_PORT    443//8200
// #define HTTPS       false
// #define OM2M_ORGIN  "Tue_20_12_22:Tue_20_12_22"//admin:admin
// #define OM2M_MN     "/~/in-cse/in-name/"
// #define OM2M_AE     "AE-WM/WM-WD"//"AE-DT"
// #define OM2M_DATA_CONT  "WM-WD-BK04-00/Data"//"DT-1/Data"

const char *ssid = "SCRC Lab 3";
const char *password = "Scrciiith@123";

#define CSE_IP      "192.168.0.109"
#define CSE_PORT     8200
#define HTTPS       false
#define OM2M_ORGIN  "admin:admin"
#define OM2M_MN     "/~/in-cse/in-name/"
#define OM2M_AE     "AE-DT"
#define OM2M_DATA_CONT  "DT-1/Data"

WiFiClient client;
HTTPClient http;

static int i = 0;

byte gammatable[256];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);


void onem2m() {
  String data = "[" + String(randomRed) + " , " + String(randomGreen) + " , " + String(randomBlue) + " , " +
                String(avgRed) + " , " + String(avgGreen) + " , " + String(avgBlue) + " , " +
                String(avgClear) + " , " + String(colorTemp) + " , " + String(lux) + "]";
  String server = "http";

  server += "://" + String(CSE_IP) + ":" + String(CSE_PORT) + String(OM2M_MN);

  http.begin(client, server + OM2M_AE + "/" + OM2M_DATA_CONT);

  http.addHeader("X-M2M-Origin", OM2M_ORGIN);
  http.addHeader("Content-Type", "application/json;ty=4");
  http.addHeader("Content-Length", "100");

  String req_data = String() + "{\"m2m:cin\": {"
                    + "\"con\": \"" + data + "\","
                    + "\"rn\": \"" + "cin_" + String(i++) + "\","
                    + "\"cnf\": \"text\""
                    + "}}";


  Serial.println("Server URL: " + server);
  Serial.println("Request Data: " + req_data);

  int code = http.POST(req_data);
  http.end();
  Serial.println(code);
  delay(1000);
  http.addHeader("Connection", "close"); // Add debugging header
}


void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

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

void takeReadings(int& avgRed, int& avgGreen, int& avgBlue, int& avgClear) {
  const int numReadings = 5; // Adjust as needed
  avgRed = avgGreen = avgBlue = avgClear = 0;

  for (int i = 0; i < numReadings; ++i) {
    int red, green, blue, clear;
    red = tcs.read8(TCS34725_RDATAL);
    green = tcs.read8(TCS34725_GDATAL);
    blue = tcs.read8(TCS34725_BDATAL);
    clear = tcs.read8(TCS34725_CDATAL);
    avgRed += red;
    avgGreen += green;
    avgBlue += blue;
    avgClear += clear;
    delay(50); // Adjust delay between readings as needed
  }

  avgRed /= numReadings;
  avgGreen /= numReadings;
  avgBlue /= numReadings;
  avgClear /= numReadings;
}

void loop() {
  // Generate random RGB values
  randomRed = random(256);
  randomGreen = random(256);
  randomBlue = random(256);

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

  delay(9000);  // Wait for 1 second (adjust as needed)

  tcs.setInterrupt(false);  // turn on LED

  takeReadings(avgRed, avgGreen, avgBlue, avgClear);

  colorTemp = tcs.calculateColorTemperature(avgRed, avgGreen, avgBlue);
  lux = tcs.calculateLux(avgRed, avgGreen, avgBlue);
  tcs.setInterrupt(true);  // turn off LED

  Serial.print("Sensor R:\t"); Serial.print(avgRed);
  Serial.print("\tG:\t"); Serial.print(avgGreen);
  Serial.print("\tB:\t"); Serial.print(avgBlue);
  Serial.print("\tClear:\t"); Serial.print(avgClear);
  Serial.print("\tColor Temp:\t"); Serial.print(colorTemp);
  Serial.print("\tLux:\t"); Serial.print(lux);
  Serial.println();
  onem2m();

  delay(1000);
}