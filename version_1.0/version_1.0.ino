#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define LED_RED_PIN    19 // GPIO23
#define LED_GREEN_PIN  18 // GPIO22
#define LED_BLUE_PIN   5  // GPIO21

int currentLedRed, currentLedGreen, currentLedBlue; // Current LED color components
int savedSensorRed, savedSensorGreen, savedSensorBlue, savedClear; // Saved sensor readings
int savedColorTemp, savedLux; // Saved color temperature and lux readings

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_154MS, TCS34725_GAIN_60X);

const char *ssid = "SCRC_LAB_IOT";
const char *password = "Scrciiith@123";

#define CSE_IP      "dev-onem2m.iiit.ac.in"//192.168.0.109
#define CSE_PORT    443//8200
#define HTTPS       false
#define OM2M_ORGIN  "Tue_20_12_22:Tue_20_12_22"//admin:admin
#define OM2M_MN     "/~/in-cse/in-name/"
#define OM2M_AE     "AE-WM/WM-WD"//"AE-DT"
#define OM2M_DATA_CONT  "WM-WD-PL04-00/Data"//"DT-1/Data"

WiFiClient client;
HTTPClient http;

static int i = 0;

void setLEDColor(int red, int green, int blue) {
  analogWrite(LED_RED_PIN, red);
  analogWrite(LED_GREEN_PIN, green);
  analogWrite(LED_BLUE_PIN, blue);
}

void setup(void) {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_BLUE_PIN, OUTPUT);

  if (tcs.begin()) {
    Serial.println("Color sensor found");
  } else {
    Serial.println("Color sensor not detected. Please check connections");
    while (1);
  }

  // Initialize random seed
  randomSeed(analogRead(0));

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
}

void onem2m() {
  String data = "[" + String(currentLedRed) + " , " + String(currentLedGreen) + " , " + String(currentLedBlue) + " , " +
                String(savedSensorRed) + " , " + String(savedSensorGreen) + " , " + String(savedSensorBlue) + " , " +
                String(savedClear) + " , " + String(savedColorTemp) + " , " + String(savedLux) + "]";
  String server = "http";
  if (HTTPS) {
    server += "s";
  }
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

void loop(void) {
  unsigned long startTime = millis();

  while (millis() - startTime < 10000) { // Total loop time of 10 seconds
    // Generate random values for LED R, G, and B components
    currentLedRed = random(256); // Random value between 0 and 255 for Red
    currentLedGreen = random(256); // Random value between 0 and 255 for Green
    currentLedBlue = random(256); // Random value between 0 and 255 for Blue

    // Set the LED color to the random R, G, B values
    setLEDColor(currentLedRed, currentLedGreen, currentLedBlue);

    delay(1000); // Keep the color for 3 seconds

    uint16_t sensorRed, sensorGreen, sensorBlue, clear, colorTemp, lux;

    tcs.getRawData(&sensorRed, &sensorGreen, &sensorBlue, &clear);
    colorTemp = tcs.calculateColorTemperature(sensorRed, sensorGreen, sensorBlue);
    lux = tcs.calculateLux(sensorRed, sensorGreen, sensorBlue);

    // Store the sensor data as the saved values
    savedSensorRed = sensorRed;
    savedSensorGreen = sensorGreen;
    savedSensorBlue = sensorBlue;
    savedClear = clear;
    savedColorTemp = colorTemp;
    savedLux = lux;

    onem2m();
    delay(9000); // Wait for 7 seconds for sensor data

    Serial.print("Saved LED Colors - R: ");
    Serial.print(currentLedRed);
    Serial.print(" G: ");
    Serial.print(currentLedGreen);
    Serial.print(" B: ");
    Serial.println(currentLedBlue);

    Serial.print("Saved Sensor RGB - R: ");
    Serial.print(savedSensorRed);
    Serial.print(" G: ");
    Serial.print(savedSensorGreen);
    Serial.print(" B: ");
    Serial.println(savedSensorBlue);

    Serial.print("Saved Clear Value: ");
    Serial.println(savedClear);

    Serial.print("Saved Color Temp: ");
    Serial.print(savedColorTemp);
    Serial.println(" K");

    Serial.print("Saved Lux: ");
    Serial.println(savedLux);
  }
}
