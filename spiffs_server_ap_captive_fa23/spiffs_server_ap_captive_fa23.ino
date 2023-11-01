#include <Adafruit_GrayOLED.h>
//----------------------------------------------------------------//
//
// SAIC Tactical Media Fall 2023
// Brett Ian Balogh
// https://github.com/giantmolecules/TACTICAL_MEDIA_FA23.git
//
// spiffs_server_ap_captive_fa23.ino
//
// Complete project details at https://randomnerdtutorials.com  
// Modified by Brett Balogh Fall 2023
//
//----------------------------------------------------------------//

#include <SPI.h>
#include <Adafruit_ST7789.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <DNSServer.h>
#include <splash.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

// Create a TFT object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

const byte DNS_PORT = 53;
//IPAddress apIP(8,8,4,4); // The default android DNS
IPAddress apIP(8,8,8,8);

DNSServer dnsServer;

// Set LED GPIO
const int ledPin = 13;
// Stores LED state
String ledState;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Replaces placeholder with LED state value
String processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if(digitalRead(ledPin)){
      ledState = "ON";
    }
    else{
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
  return String();
}
 
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
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
  tft.setTextSize(3);

  // set text foreground and background colors
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  Serial.println(F("TFT Initialized"));
  // Initialize SPIFFS
  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-DNSServer");
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

//  // Connect to Wi-Fi
//  WiFi.begin(ssid, password);
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(1000);
//    Serial.println("Connecting to WiFi..");
//  }


//  // Connect to Wi-Fi network with SSID and password
//  Serial.print("Setting AP (Access Point)…");
//  // Remove the password parameter, if you want the AP (Access Point) to be open
//  WiFi.softAP(ssid, password);
//
//  IPAddress IP = WiFi.softAPIP();
//  Serial.print("AP IP address: ");
//  Serial.println(IP);
//
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());



  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to set GPIO to HIGH
  server.on("/on", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, HIGH); 
    tft.setCursor(0,32);
    tft.print("ON ");  
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });
  
  // Route to set GPIO to LOW
  server.on("/off", HTTP_GET, [](AsyncWebServerRequest *request){
    digitalWrite(ledPin, LOW);
    tft.setCursor(0,32);
    tft.print("OFF");     
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  // Start server
  server.begin();

}
 
void loop(){
    dnsServer.processNextRequest();
}
