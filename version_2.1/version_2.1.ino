#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <WiFi.h>
#include <HTTPClient.h>

#define redpin 19
#define greenpin 18
#define bluepin 5
#define commonAnode false

#define sensor_red 12
#define sensor_green 14
#define sensor_blue 27

int randomRed, randomGreen, randomBlue;
int red, green, blue, clear;
String red1, green1, blue1;
int colorTemp, lux;
String conValue = "0";
long prev_time = 0;
long new_time = 0;

const char *ssid = "SCRC Lab 3";
// const char *ssid = "SCRC_Lab_IOT";
const char *password = "Scrciiith@123";

#define CSE_IP    "10.3.1.117"
#define CSE_PORT  8200
#define HTTPS  false
#define OM2M_ORGIN  "admin:admin"
#define OM2M_MN  "/~/in-cse/in-name/"
#define OM2M_AE   "AE-DT"
#define OM2M_DATA_CONT "DT-1/Data"
// #define OM2M_DATA_CONT "DT-2/Data"

// const char* ssid = "IIIT-Guest";
// const char* password = "I%GR#*S@!";

// #define CSE_IP      "dev-onem2m.iiit.ac.in"
// #define OM2M_ORGIN    "Tue_20_12_22:Tue_20_12_22"
// #define CSE_PORT    443
// #define HTTPS     false
// #define OM2M_MN     "/~/in-cse/in-name/"
// #define OM2M_AE     "AE-AQ"
// #define OM2M_NODE_ID   "AQ-MG00-00"
// #define OM2M_DATA_CONT  "AQ-MG00-00/Data"
// #define OM2M_DATA_LBL   "[\"AE-AQ\", \"V4.0.0\", \"AQ-MG00-00\", \"AQ-V4.0.0\"]"

String url = "http://10.3.1.117:8200/~/in-cse/in-name/AE-DT/DT-ACT-1/Data/la";

WiFiClient client;
HTTPClient http;

byte gammatable[256];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

String sendGET(String url) {
  HTTPClient http;
  http.setTimeout(10000); // Set a timeout of 10 seconds (adjust as needed)
  http.begin(url);
  http.addHeader("X-M2M-Origin", "admin:admin");
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.GET();
  String payload = "";

  if (httpCode > 0) {
    payload = http.getString();
    Serial.println("---------------------------------check onem2m after 10 sec ----------------------------------------------");

    int conStart = payload.indexOf("\"con\" : ");
    if (conStart >= 0) {
      int conEnd = payload.indexOf("\n", conStart);
      if (conEnd >= 0) {
        conValue = payload.substring(conStart + 8, conEnd);  
        conValue.trim();     
        Serial.print("OneM2M value : ");
        Serial.println(conValue);

        // Remove square brackets
        conValue = conValue.substring(1, conValue.length() - 1);

        int firstCommaIndex = conValue.indexOf(',');
        int secondCommaIndex = conValue.indexOf(',', firstCommaIndex + 1);
        int thirdCommaIndex = conValue.indexOf(',', secondCommaIndex + 1);

        String timestamp = conValue.substring(1, firstCommaIndex);
        red1 = conValue.substring(firstCommaIndex + 1, secondCommaIndex);
        green1 = conValue.substring(secondCommaIndex + 1, thirdCommaIndex);
        blue1 = conValue.substring(thirdCommaIndex + 1, conValue.length() - 1);

        new_time = atol(timestamp.c_str());
        
        Serial.print("new: ");
        Serial.println(new_time);
        Serial.print("prev: ");
        Serial.println(prev_time);
      
        
        Serial.print("Red: ");
        Serial.println(red1);
        Serial.print("Green: ");
        Serial.println(green1);
        Serial.print("Blue: ");
        Serial.println(blue1);
      }
    }
  } else {
    Serial.print("HTTP request failed with error code: ");
    Serial.println(httpCode);
  }

  http.end();
  return conValue;
}

void get_onem2m(){
  conValue = sendGET(url);
}

void post_onem2m() {
  String data;
  if (new_time == prev_time) {
    Serial.println("Matched");
    data = "[" + String(randomRed) + " , " + String(randomGreen) + " , " + String(randomBlue) + " , " +
                String(red) + " , " + String(green) + " , " + String(blue) + " , " +
                String(clear) + " , " + String(colorTemp) + " , " + String(lux) + "," + 0 +"]";
    } else {
      Serial.println("Not Matched");
      Serial.print("Red: ");
      Serial.println(red1);
      Serial.print("Green: ");
      Serial.println(green1);
      Serial.print("Blue: ");
      Serial.println(blue1);
      data = "[" + String(red1) + " , " + String(green1) + " , " + String(blue1) + " , " +
                  String(red) + " , " + String(green) + " , " + String(blue) + " , " +
                String(clear) + " , " + String(colorTemp) + " , " + String(lux) + "," + 1 + "]";
      prev_time = new_time;
    }
  
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

  pinMode(redpin, OUTPUT);
  pinMode(greenpin, OUTPUT);
  pinMode(bluepin, OUTPUT);

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

void random_light() {

  if (new_time == prev_time) {
    Serial.println("Matched");
    
  randomRed = random(256);
  randomGreen = random(256);
  randomBlue = random(256);

  Serial.print("Random R:\t"); Serial.print(randomRed);
  Serial.print("\tG:\t"); Serial.print(randomGreen);
  Serial.print("\tB:\t"); Serial.print(randomBlue);
  Serial.println();

  analogWrite(redpin, gammatable[randomRed]);
  analogWrite(greenpin, gammatable[randomGreen]);
  analogWrite(bluepin, gammatable[randomBlue]);

      } else {
        Serial.println("Not Matched");

      int red1Int = red1.toInt();
      int green1Int = green1.toInt();
      int blue1Int = blue1.toInt();

      // Ensure the values are within the valid range (0 to 255)
      red1Int = constrain(red1Int, 0, 255);
      green1Int = constrain(green1Int, 0, 255);
      blue1Int = constrain(blue1Int, 0, 255);

      // Use the integer values to access the gammatable array
      analogWrite(redpin, gammatable[red1Int]);
      analogWrite(greenpin, gammatable[green1Int]);
      analogWrite(bluepin, gammatable[blue1Int]);
    }

  delay(5000);
}

void colour_sensor() {
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

  analogWrite(sensor_red, red);
  analogWrite(sensor_green, green);
  analogWrite(sensor_blue, blue);
}

void loop() {
  turnOffLED();
  get_onem2m();
  random_light();
  colour_sensor();
  post_onem2m();
  delay(15000);
}
