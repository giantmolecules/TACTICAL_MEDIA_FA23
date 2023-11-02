//----------------------------------------------------------------//
//
// SAIC Tactical Media Fall 2023
// Brett Ian Balogh
// https://github.com/giantmolecules/TACTICAL_MEDIA_FA23.git
//
// spiffy-captive-portal-p5-fa23.ino
//
// Captive portal that serves a P5 sketch.
// Use Default 4MB With SPIFFS partition scheme in tools menu.
//
//----------------------------------------------------------------//

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "FS.h"
#include "SPIFFS.h"

// Create a TFT object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Create a dns server
DNSServer dnsServer;

// Create a web server
AsyncWebServer server(80);

String user_name;
String proficiency;
bool name_received = false;
bool proficiency_received = false;

//----{listDir}-----------------------------------------------------//

void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("− failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println(" − not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

//----{CaptiveRequestHandler}-----------------------------------------------------//

class CaptiveRequestHandler : public AsyncWebHandler {
  public:
    CaptiveRequestHandler() {}
    virtual ~CaptiveRequestHandler() {}

    bool canHandle(AsyncWebServerRequest *request) {
      //request->addInterestingHeader("ANY");
      return true;
    }

    void handleRequest(AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/page2.html", "text/html", false);
    }
};

//----{setupServer}-----------------------------------------------------//

void setupServer() {
  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html", false);
  });

  // Route to load style.css file
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  // Route to load p5.js
  server.on("/p5.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/p5.min.js", "text/javascript");
  });

  // Route to load p5.sound.min.js
  server.on("/p5.sound.min.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/p5.sound.min.js", "text/javascript");
  });

  // Route to load sketch.js
  server.on("/sketch.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/sketch.js", "text/javascript");
  });

  // Route for page 2
  server.on("/page2.html", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/page2.html", "text/html", false);
  });

  // Route to load gif file
  server.on("/GR.svg", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/GR.svg", "image/svg+xml");
  });


}

//----{SETUP}-----------------------------------------------------//

void setup() {

  //your other setup stuff...
  Serial.begin(115200);

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
  tft.setCursor(0, 0);
  tft.print(F("TFT Initialized"));

  // Mount Filesystem
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  //List contents of SPIFFS
  listDir(SPIFFS, "/", 0);

  // Start servers
  Serial.println();
  tft.println("Setting up AP Mode");
  IPAddress apIP(192, 168, 1, 1);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.mode(WIFI_AP);

  // WiFi Info
  const char* ssid = "BRETT 02";
  const char* password = "NONE";
  
  WiFi.softAP(ssid);
  IPAddress IP = WiFi.softAPIP();
  tft.print("AP IP address: "); Serial.println(IP);
  tft.println("Setting up Async WebServer");
  setupServer();
  tft.println("Starting DNS Server");
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
  //more handlers...
  server.begin();
  tft.println("All Done!");
  delay(1000);
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0,0);
  tft.setTextSize(4);
  tft.print(ssid);
}

//----{LOOP}-----------------------------------------------------//

void loop() {
  dnsServer.processNextRequest();
}
