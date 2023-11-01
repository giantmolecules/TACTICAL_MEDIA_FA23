//----------------------------------------------------------------//
//
// SAIC Tactical Media Fall 2023
// Brett Ian Balogh
// https://github.com/giantmolecules/TACTICAL_MEDIA_FA23.git
//
// painless_mesh_bb_fa23.ino
//
// Example using painless mesh library for simple chat.
//
//----------------------------------------------------------------//

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <painlessMesh.h>

// some gpio pin that is connected to an LED...
// on my rig, this is 5, change to the right number of your LED.
#define LED 13

#define   BLINK_PERIOD    1000 // milliseconds until cycle repeat
#define   BLINK_DURATION  50  // milliseconds LED is on for

#define   MESH_SSID       "BRETTMESH"
#define   MESH_PASSWORD   "8characters"
#define   MESH_PORT       5555


// Prototypes
void sendMessage();
void receivedCallback(uint32_t from, String & msg);
void newConnectionCallback(uint32_t nodeId);
void changedConnectionCallback();
void nodeTimeAdjustedCallback(int32_t offset);
void delayReceivedCallback(uint32_t from, int32_t delay);

Scheduler     userScheduler; // to control your personal task
painlessMesh  mesh;

// Create a TFT object
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

bool calc_delay = false;
SimpleList<uint32_t> nodes;

//Task taskSendMessage( TASK_IMMEDIATE, TASK_ONCE, &sendMessage ); // start with a one second interval

// Task to blink the number of nodes
Task blinkNoNodes;

bool onFlag = false;

String activeSendMessage = "";
String activeReceiveMessage = "";
String nodeID;
String shortID;
int fontSize = 2;
int fontWidth = 6 * fontSize;
int fontHeight = 8 * fontSize;
int screenWidth = 240;
int screenHeight = 135;
int textY = 0;
int message1Length = 0;
int message2Length = 0;
int sendMessageLength = 0;
int receiveMessageLength = 0;
int scrollPosition1 = screenWidth;
int scrollPosition2 = screenWidth;
boolean scroll1State = 0;
boolean scroll2State = 0;
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete

//----------------------------------------------------------------//
// Setup
//----------------------------------------------------------------//


void setup() {
  Serial.begin(115200);

  pinMode(13, OUTPUT);

  inputString.reserve(200);

  //----------------------------------------------------------------//
  // Screen Stuff
  //----------------------------------------------------------------//


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
  tft.setTextSize(fontSize);

  // set text foreground and background colors
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);
  tft.setCursor(0,0);
  tft.println(F("TFT Initialized"));

  //----------------------------------------------------------------//
  // Mesh Stuff
  //----------------------------------------------------------------//


  mesh.setDebugMsgTypes(ERROR | DEBUG);  // set before init() so that you can see error messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);
  mesh.onNodeDelayReceived(&delayReceivedCallback);

 // userScheduler.addTask( taskSendMessage );
 // taskSendMessage.enable();

  blinkNoNodes.set(BLINK_PERIOD, (mesh.getNodeList().size() + 1) * 2, []() {
    // If on, switch off, else switch on
    if (onFlag)
      onFlag = false;
    else
      onFlag = true;
    blinkNoNodes.delay(BLINK_DURATION);
    tft.println("blinkNoNodes");
    if (blinkNoNodes.isLastIteration()) {
      // Finished blinking. Reset task for next run
      // blink number of nodes (including this node) times
      blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
      // Calculate delay based on current mesh time and BLINK_PERIOD
      // This results in blinks between nodes being synced
      blinkNoNodes.enableDelayed(BLINK_PERIOD -
                                 (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);
    }
  });
  
  userScheduler.addTask(blinkNoNodes);
  blinkNoNodes.enable();

  randomSeed(analogRead(A0));
  nodeID = String(mesh.getNodeId());
  shortID = nodeID.substring(7);
}

//----------------------------------------------------------------//
// Loop
//----------------------------------------------------------------//


void loop() {
  mesh.update();
    while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
      sendMessage(inputString);
      tft.setCursor(0,0);
      tft.println(inputString);
      sendMessageLength = setMessageLength(inputString);
      inputString="";
    }
  }
  scrollMessage1();
  scrollMessage2();
}

//----------------------------------------------------------------//

void sendMessage(String str) {
  String msg;
  msg += " From ";
  msg += shortID;
  msg += ": ";
  msg += str;
  mesh.sendBroadcast(msg);

  if (calc_delay) {
    SimpleList<uint32_t>::iterator node = nodes.begin();
    while (node != nodes.end()) {
      mesh.startDelayMeas(*node);
      node++;
    }
    calc_delay = false;
  }

  Serial.printf("Sending message: %s\n", msg.c_str());

  //taskSendMessage.setInterval(TASK_IMMEDIATE);  // 1 seconds
  scroll1State = 1;
}

//----------------------------------------------------------------//

void receivedCallback(uint32_t from, String & msg) {
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());
  activeReceiveMessage = msg;
  receiveMessageLength = setMessageLength(activeReceiveMessage);
  scroll2State = 1;
}

//----------------------------------------------------------------//

void newConnectionCallback(uint32_t nodeId) {
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);

  Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
  Serial.printf("--> startHere: New Connection, %s\n", mesh.subConnectionJson(true).c_str());
}

//----------------------------------------------------------------//

void changedConnectionCallback() {
  Serial.printf("Changed connections\n");
  // Reset blink task
  onFlag = false;
  blinkNoNodes.setIterations((mesh.getNodeList().size() + 1) * 2);
  blinkNoNodes.enableDelayed(BLINK_PERIOD - (mesh.getNodeTime() % (BLINK_PERIOD * 1000)) / 1000);

  nodes = mesh.getNodeList();

  Serial.printf("Num nodes: %d\n", nodes.size());
  Serial.printf("Connection list:");

  SimpleList<uint32_t>::iterator node = nodes.begin();
  while (node != nodes.end()) {
    Serial.printf(" %u", *node);
    node++;
  }
  Serial.println();
  calc_delay = true;
}

//----------------------------------------------------------------//

void nodeTimeAdjustedCallback(int32_t offset) {
  Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(), offset);
}

//----------------------------------------------------------------//

void delayReceivedCallback(uint32_t from, int32_t delay) {
  Serial.printf("Delay to node %u is %d us\n", from, delay);
}

//----------------------------------------------------------------//

void scrollMessage1() {

  if (sendMessageLength > receiveMessageLength) {
    message1Length = sendMessageLength;
  }
  else {
    message1Length = receiveMessageLength;
  }
  //message1Length = sendMessageLength;
  if (scrollPosition1 < -message1Length) {
    scrollPosition1 = screenWidth;
    activeReceiveMessage = "";
    scroll1State = 0;
  }

  //for (int i = screenWidth; i > -messageLength; i--) {}
  tft.setCursor(scrollPosition1, 10);
  tft.print(activeSendMessage);

  scrollPosition1 = scrollPosition1 - 1;
}

//----------------------------------------------------------------//

void scrollMessage2() {

  if (sendMessageLength > receiveMessageLength) {
    message2Length = sendMessageLength;
  }
  else {
    message2Length = receiveMessageLength;
  }
  //message2Length = receiveMessageLength;
  if (scrollPosition2 < -message2Length) {
    scrollPosition2 = screenWidth;
    activeReceiveMessage = "";
    scroll2State = 0;
  }

  //for (int i = screenWidth; i > -messageLength; i--) {}
  tft.setCursor(scrollPosition2, 20);
  tft.print(activeReceiveMessage);

  scrollPosition2 = scrollPosition2 - 1;
}

//----------------------------------------------------------------//

int setMessageLength(String m) {
  int length = m.length() * fontWidth;
  return length;
}

//----------------------------------------------------------------//
//----------------------------------------------------------------//
