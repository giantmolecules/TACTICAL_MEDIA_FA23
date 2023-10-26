//----------------------------------------------------------------//
//
// SAIC TacticalMedia Fall 2023
// Brett Ian Balogh
//
// spiffs_server_tft.ino
//
// This is a wireless webserver adapted from:
// Rui Santos
// Complete project details at https://randomnerdtutorials.com
//
//----------------------------------------------------------------//

// Import required libraries
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

// Include libraries for TFT
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

// Create a TFT object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Replace with your network credentials
const char* ssid = "SAIC-Guest";
const char* password = "wifi@saic";

// Set LED GPIO
const int ledPin = 13;

// Stores LED state
String ledState;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
String processor(const String& var) {
  if (var == "STATE") {
    if (digitalRead(ledPin)) {
      ledState = "ON";
      tft.fillRect(0,40,240,135,ST77XX_BLUE);
    }
    else {
      ledState = "OFF";
      tft.fillRect(0,40,240,135,ST77XX_RED);
    }
    //tft.print(ledState);
    updateTFT();
    return ledState;
  }
  return String();
}

void setup() {

  // Define mode of LED pin
  pinMode(ledPin, OUTPUT);

  // turn on backlite
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  // turn on the TFT / I2C power supply
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  // initialize TFT
  tft.init(135, 240); // Init ST7789 240x135
  tft.setRotation(3);
  tft.fillScreen(ST77XX_BLACK);

  // default text size
  tft.setTextSize(2);

  // set text foreground and background colors
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  tft.println(F("TFT Initialized"));

  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    tft.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  tft.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    tft.print(".");
  }

  // Print ESP32 Local IP Address
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("IP: ");
  tft.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(ledPin, HIGH);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest * request) {
    digitalWrite(ledPin, LOW);
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Start server
  server.begin();
}

void loop() {
  // Nothing to do here. Server handles everything.
}

void updateTFT() {
  tft.fillRect(0,0,240,40, ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.print("IP: ");
  tft.println(WiFi.localIP());
  tft.println(ledState);
}
