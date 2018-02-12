#include <ArduinoJson.h>
#include <Ethernet.h>
#include <SPI.h>


/*
Duration to close the switch on the door opener. This should be long
enough for the mechanism to start; typically it doesn't to remain 
activated for the door to complete its motion. It is the same as the
time you'd hold down the button to start the door moving. 
*/
#define DOOR_ACTIVATION_PERIOD 600 // [ms]


// I read pins 11-13 are used by the
// Ethernet shield and shouldn't be
// used for the LEDs. I tried by accident
// and got crazy performance so I changed
// it
int LEFT_OPEN_PIN_INDICATOR = 5;
int LEFT_CLOSED_PIN_INDICATOR = 6;
int RIGHT_OPEN_PIN_INDICATOR = 7;
int RIGHT_CLOSED_PIN_INDICATOR = 8;

// Digital output connected to a relay that activates the door
const uint8_t LeftDoorControlPin = 9;
// Digital output connected to a relay that activates the door
const uint8_t RightDoorControlPin = 10;

size_t capacity = JSON_OBJECT_SIZE(6) + 85;

int leftOpenLED = 0; // 0 = inactive, 1 = blink
int leftClosedLED = 1; // 0 = inactive, 1 = blink
int rightOpenLED = 0; // 0 = inactive, 1 = blink
int rightClosedLED = 1; // 0 = inactive, 1 = blink
int madeInitialContact = 0; // 0 = false, 1 = true

EthernetServer arduinoServer(80);

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xDE};
byte arduinoIP[] = {192, 168, 1, 74};
byte gateway[] = {192, 168, 1, 35}; // internet access via router
byte subnet[] = {255, 255, 255, 0}; //subnet mask

/* THIS WORKS...USED FOR EARLIER VERSION OF CODE
void SetInitialStates() 
{
  // This sends the initial state of the doors
  // to the remote device when it first connects

  EthernetClient client = arduinoServer.available();
  Serial.print("Initial client = ");
  Serial.println(client);  
  if (client) 
  {
    Serial.println("new client");
    //an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json;charset=utf-8");
          client.println("Server: Arduino");
          client.println("Connection: close");
          client.println();
          
          // Write JSON document
          // StaticJsonBuffer allocates memory on the stack, it can be
          // replaced by DynamicJsonBuffer which allocates in the heap.
          DynamicJsonBuffer jsonBuffer(capacity);
          // root is a reference to the JsonObject, the actual bytes are inside the
          // JsonBuffer with all the other nodes of the object tree.
          // Memory is freed when jsonBuffer goes out of scope.
          JsonObject& root = jsonBuffer.createObject();
          // add json data
          root["leftOpenLED"] = leftOpenLED;
          root["leftClosedLED"] = leftClosedLED;
          root["rightOpenLED"] = rightOpenLED;
          root["rightClosedLED"] = rightClosedLED;
          // send to client
          root.prettyPrintTo(client);

          // don't call again for this host
          madeInitialContact = 1;
          
          break;
        }

        if (c == '\n') 
        {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') 
        {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
  }
}
*/

/* THIS WORKS...USED FOR EARLIER VERSION OF CODE
void ParseJSONFromDevices() 
{
  
  // This parses the JSON from the remote
  // device (e.g. iPhone) and then updates
  // the state of the doors
  // listen for incoming clients
  EthernetClient client = arduinoServer.available();
  if (client) 
  {
    Serial.println("new client");
    //an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        // TO BE USED FOR DEBUGGING
        // WILL SHOW HEADER AND DATA
        //char c = client.read();
        //Serial.write(c);

        // Skip HTTP headers
        char endOfHeaders[] = "\r\n\r\n";
        if (!client.find(endOfHeaders)) 
        {
          Serial.println(F("Invalid response"));
          delay(2000);
          return;
        }

        // Allocate JsonBuffer
        // Use arduinojson.org/assistant to compute the capacity.
        DynamicJsonBuffer jsonBuffer(capacity);
        JsonObject& root = jsonBuffer.parseObject(client);
        if (!root.success()) 
        {
          Serial.println("Parsing failed!");
          delay(2000);
          return;
        }

        // Extract values.
        int leftOpen = root["leftOpenLED"];
        int leftClosed = root["leftClosedLED"];
        int rightOpen = root["rightOpenLED"];
        int rightClosed = root["rightClosedLED"];

        // Update global variables
        leftOpenLED = leftOpen;
        leftClosedLED = leftClosed;
        rightOpenLED = rightOpen;
        rightClosedLED = rightClosed;

        Serial.print("Rx from iPhone, leftOpenLED = ");
        Serial.println(leftOpenLED);
        Serial.print("Rx from iPhone, leftClosedLED = ");
        Serial.println(leftClosedLED);
        Serial.print("Rx from iPhone, rightOpenLED = ");
        Serial.println(rightOpenLED);
        Serial.print("Rx from iPhone, rightClosedLED = ");
        Serial.println(rightClosedLED);

        // Disconnect
        client.stop();
      }
    }
  }
}
*/

// set LEDs based on garage door status
void setLEDs() 
{
  if (leftOpenLED == 1 && leftClosedLED == 0 && rightOpenLED == 1 && rightClosedLED == 0) 
  {
    blinkLEDs(LEFT_OPEN_PIN_INDICATOR, RIGHT_OPEN_PIN_INDICATOR);
    DeactivateLED(LEFT_CLOSED_PIN_INDICATOR, RIGHT_CLOSED_PIN_INDICATOR);
  } else if (leftOpenLED == 0 && leftClosedLED == 1 && rightOpenLED == 1 && rightClosedLED == 0) 
  {
    blinkLEDs(LEFT_CLOSED_PIN_INDICATOR, RIGHT_OPEN_PIN_INDICATOR);
    DeactivateLED(LEFT_OPEN_PIN_INDICATOR, RIGHT_CLOSED_PIN_INDICATOR);
  } else if  (leftOpenLED == 1 && leftClosedLED == 0 && rightOpenLED == 0 && rightClosedLED == 1) 
  {
    blinkLEDs(LEFT_OPEN_PIN_INDICATOR, RIGHT_CLOSED_PIN_INDICATOR);
    DeactivateLED(LEFT_CLOSED_PIN_INDICATOR, RIGHT_OPEN_PIN_INDICATOR);
  } else if (leftOpenLED == 0 && leftClosedLED == 1 && rightOpenLED == 0 && rightClosedLED == 1) 
  {
    blinkLEDs(LEFT_CLOSED_PIN_INDICATOR, RIGHT_CLOSED_PIN_INDICATOR);
    DeactivateLED(LEFT_OPEN_PIN_INDICATOR, RIGHT_OPEN_PIN_INDICATOR);
  }
}


// mechanics for actually blinking LEDs
// based on pin it's connected to
void blinkLEDs(int pinNum1, int pinNum2) 
{
  int nn = 0;  
  while (nn < 5) 
  {
    digitalWrite(pinNum1, HIGH);
    digitalWrite(pinNum2, HIGH);
    delay(100);
    digitalWrite(pinNum1, LOW);
    digitalWrite(pinNum2, LOW);
    delay(100);

    nn += 1;
  }
}


void DeactivateLED(int pinNum1, int pinNum2) 
{
  digitalWrite(pinNum1, LOW);
  digitalWrite(pinNum2, LOW);
}


void SendAndReceiveJSONData() 
{
  // This parses the JSON from the remote
  // device (e.g. iPhone) and then updates
  // the state of the doors
  // listen for incoming clients
  EthernetClient client = arduinoServer.available();
  if (client) 
  {
    Serial.println("new client");
    //an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) 
    {
      if (client.available()) 
      {
        // TO BE USED FOR DEBUGGING
        // WILL SHOW HEADER AND DATA
        //char c = client.read();
        //Serial.write(c);

        // Skip HTTP headers
        char endOfHeaders[] = "\r\n\r\n";
        if (!client.find(endOfHeaders)) 
        {
          Serial.println(F("Invalid response"));
          delay(2000);
          return;
        }

        // Allocate JsonBuffer
        // Use arduinojson.org/assistant to compute the capacity.
        DynamicJsonBuffer jsonBuffer(capacity);
        JsonObject& root = jsonBuffer.parseObject(client);
        if (root.success()) 
        {
          // This was a POST request from the 
          // external device with data we have to
          // extract and updates state of the
          // doors and LEDs with
          
          // Extract values.
          int leftOpen = root["leftOpenLED"];
          int leftClosed = root["leftClosedLED"];
          int rightOpen = root["rightOpenLED"];
          int rightClosed = root["rightClosedLED"];

          // Update global variables
          leftOpenLED = leftOpen;
          leftClosedLED = leftClosed;
          rightOpenLED = rightOpen;
          rightClosedLED = rightClosed;

          // Print values for DEBUGGING
          /*
          Serial.print("Rx from iPhone, leftOpenLED = ");
          Serial.println(leftOpenLED);
          Serial.print("Rx from iPhone, leftClosedLED = ");
          Serial.println(leftClosedLED);
          Serial.print("Rx from iPhone, rightOpenLED = ");
          Serial.println(rightOpenLED);
          Serial.print("Rx from iPhone, rightClosedLED = ");
          Serial.println(rightClosedLED);
          */

          // open or close the doors as needed
          ActivateGarageDoor();

          // Disconnect
          client.stop();
          
        } else 
        {
          // This was an initial GET contact request.
          // In this case we need to update the
          // device with the current state
          // of the doors/LEDs

          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json;charset=utf-8");
          client.println("Server: Arduino");
          client.println("Connection: close");
          client.println();
         
          // root is a reference to the JsonObject, the actual bytes are inside the
          // JsonBuffer with all the other nodes of the object tree.
          // Memory is freed when jsonBuffer goes out of scope.
          JsonObject& tx_root = jsonBuffer.createObject();
          // add json data
          tx_root["leftOpenLED"] = leftOpenLED;
          tx_root["leftClosedLED"] = leftClosedLED;
          tx_root["rightOpenLED"] = rightOpenLED;
          tx_root["rightClosedLED"] = rightClosedLED;
          // send to client
          tx_root.prettyPrintTo(client); 

          // Print values for DEBUGGING
          /*
          Serial.print("Tx from Arduino, leftOpenLED = ");
          Serial.println(leftOpenLED);
          Serial.print("Tx from Arduino, leftClosedLED = ");
          Serial.println(leftClosedLED);
          Serial.print("Tx from Arduino, rightOpenLED = ");
          Serial.println(rightOpenLED);
          Serial.print("Tx from Arduino, rightClosedLED = ");
          Serial.println(rightClosedLED);
          */

          // Disconnect
          client.stop();

          break;
        }
      }
    }
  }
}


void InitializeHardware()
{
  pinMode(LeftDoorControlPin, OUTPUT);
  digitalWrite(LeftDoorControlPin, LOW);

  pinMode(RightDoorControlPin, OUTPUT);
  digitalWrite(RightDoorControlPin, LOW);
}

void ActivateGarageDoor(int doorControlPin) 
{
  digitalWrite(doorControlPin, HIGH);
  delay(DOOR_ACTIVATION_PERIOD);
  digitalWrite(doorControlPin, LOW);
}

void ActivateGarageDoor() 
{
  
  ActivateGarageDoor(LeftDoorControlPin);
  ActivateGarageDoor(RightDoorControlPin);
  
}

/**************/
// Begin Arduino setup and loop
void setup() 
{
  Serial.begin(9600);

  pinMode(LEFT_OPEN_PIN_INDICATOR, OUTPUT);
  pinMode(LEFT_CLOSED_PIN_INDICATOR, OUTPUT);
  pinMode(RIGHT_OPEN_PIN_INDICATOR, OUTPUT);
  pinMode(RIGHT_CLOSED_PIN_INDICATOR, OUTPUT);

  InitializeHardware();

  Ethernet.begin(mac, arduinoIP);//, gateway, subnet);
  Serial.println("Ethernet ready");
  
  delay(1000);
}


void loop() 
{

  /*
  // Make initial contact with
  // iPhone and tell it what 
  // current state of doors/LEDs
  // are
  if (madeInitialContact == 0) {
    SetInitialStates();
  }
  
  // Arduino listens for requests from clients
  // which means devices are clients and arduino 
  // is server parsing through JSON to update 
  // door states
  ParseJSONFromDevices();
  */

  
  SendAndReceiveJSONData();

  // set LEDs based on door states
  setLEDs();
}
