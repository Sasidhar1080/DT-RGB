#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <WiFi.h>
#include <HTTPClient.h>

int red, green, blue;
int id = 0; // Variable to increment

const char *ssid = "SCRC Lab 3";
const char *password = "Scrciiith@123";

#define CSE_IP    "192.168.0.109"
#define CSE_PORT  8200
#define HTTPS  false
#define OM2M_ORGIN  "admin:admin"
#define OM2M_MN  "/~/in-cse/in-name/"
#define OM2M_AE   "AE-DT"
#define OM2M_DATA_CONT "AE-SIM-DT-2/Data"

WiFiClient client;
HTTPClient http;

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void post_onem2m() {
  String data = "[" + String(red) + " , " + String(green) + " , " + String(blue) + " , " + String(id) +  "]";
  String server = "http://" + String(CSE_IP) + ":" + String(CSE_PORT) + String(OM2M_MN);

  if (WiFi.status() == WL_CONNECTED) {
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
    if (code > 0) {
      Serial.println("HTTP Response Code: " + String(code));
    } else {
      Serial.println("HTTP POST failed");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected!");
  }
}

void turnOffLED() {
  digitalWrite(13, LOW); // Assuming pin 13 controls the LED
}

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);
  pinMode(13, OUTPUT);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  if (!tcs.begin()) {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

  tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_101MS);
  tcs.setGain(TCS34725_GAIN_1X);
}

void colour_sensor() {
  tcs.setInterrupt(false);
  red = tcs.read16(TCS34725_RDATAL);
  green = tcs.read16(TCS34725_GDATAL);
  blue = tcs.read16(TCS34725_BDATAL);
  tcs.setInterrupt(true);

  Serial.print("Sensor R:\t"); Serial.print(red);
  Serial.print("\tG:\t"); Serial.print(green);
  Serial.print("\tB:\t"); Serial.print(blue);
  Serial.println();
}

void loop() {
  turnOffLED();
  colour_sensor();
  id++;
  post_onem2m();
  delay(23000);
}
