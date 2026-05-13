#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebSocketsServer.h> 

// --- OLED Display Settings ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Serial & Sensor Pins (ESP32) ---
#define RXD2 16 // Pin 16 connects to Arduino UNO TX (Pin 1)
const int trigPin = 5;  
const int echoPin = 18; 

// Variables for display and logic
int distanceCm = 0;
String systemState = "IDLE"; 

// WiFi & WebSocket Settings
const char* ssid = "AeroBalance";
const char* password = ""; 
WebSocketsServer webSocket = WebSocketsServer(80);

String currentIP = "Not Connected";
String currentMode = "STANDBY";
String directionStr = "STOP";
String tiltAngle = "0.00"; // Default start
bool isSystemOn = false;

// Movement Offsets
float pitchOffset = 0.0;
int turnOffsetVal = 0;

unsigned long lastUpdate = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
    case WStype_CONNECTED:
      break;
    case WStype_TEXT:
      String msg = String((char*)payload);
      
      if (msg.startsWith("BAL:")) {
        if (msg.substring(4) == "ON") {
          isSystemOn = true;
        } else {
          isSystemOn = false;
          currentMode = "STANDBY";
          directionStr = "STOP";
        }
      }
      else if (msg.startsWith("MODE:")) {
        currentMode = msg.substring(5);
      }
      else if (msg.startsWith("ARR:")) {
        int commaIdx = msg.indexOf(',');
        if (commaIdx > 0) {
          int x = msg.substring(4, commaIdx).toInt();
          int y = msg.substring(commaIdx + 1).toInt();
          
          pitchOffset = y * (1.5 / 100.0); // Reduced to 1.5 deg max lean so it doesn't fall
          turnOffsetVal = x * (30.0 / 100.0); // Reduced turn speed

          // Update OLED String based on dominant axis
          if (abs(y) > abs(x)) {
            directionStr = (y > 0) ? "FWD" : "REV";
          } else if (abs(x) > 0) {
            directionStr = (x > 0) ? "RIGHT" : "LEFT";
          } else {
            directionStr = "STOP";
          }
        }
      }
      else if (msg.startsWith("SLI:")) {
        int speed = msg.substring(4).toInt();
        pitchOffset = speed * (1.5 / 100.0);
        turnOffsetVal = 0;
        
        if (speed == 0) directionStr = "STOP";
        else if (speed > 0) directionStr = "FWD " + String(speed) + "%";
        else directionStr = "REV " + String(abs(speed)) + "%";
      }
      else if (msg.startsWith("GYRO:")) {
        int commaIdx = msg.indexOf(',');
        if (commaIdx > 0) {
          int x = msg.substring(5, commaIdx).toInt();
          int y = msg.substring(commaIdx + 1).toInt();
          pitchOffset = y * (1.5 / 100.0); 
          turnOffsetVal = x * (30.0 / 100.0); 
          directionStr = "TILT";
        }
      }
      break;
  }
}

void setup() {
  Serial.begin(115200); // USB Debugging
  Serial2.begin(115200, SERIAL_8N1, RXD2, 17); // Hardware Serial to UNO

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println(F("BOOTING..."));
  display.display();

  WiFi.softAP(ssid, password);
  currentIP = WiFi.softAPIP().toString();

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  delay(1500); 
}

void loop() {
  webSocket.loop();

  // --- Read incoming Tilt Angle from Arduino UNO ---
  while (Serial2.available()) {
    String incomingData = Serial2.readStringUntil('\n');
    incomingData.trim(); // Remove whitespace/newlines
    
    if (incomingData.startsWith("T:")) {
      tiltAngle = incomingData.substring(2); // Extract just the number
    }
  }

  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate > 100) { 
    lastUpdate = currentMillis;

    // 1. Read Ultrasonic Sensor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH, 30000); 
    if (duration == 0) {
      distanceCm = 999; 
    } else {
      distanceCm = duration * 0.034 / 2; 
    }

    if (distanceCm < 20) {
      systemState = "OBSTACLE";
    } else {
      systemState = "IDLE";
    }

    // Follow Mode Logic
    if (currentMode == "FOLLOW") {
      turnOffsetVal = 0;
      if (distanceCm <= 15) { // Stop if close (No reverse)
        directionStr = "STOP";
        pitchOffset = 0.0;
      } else if (distanceCm > 15 && distanceCm < 60) { // Forward if far
        directionStr = "FWD";
        pitchOffset = 1.5; // Lean forward gently
      } else {
        directionStr = "IDLE";
        pitchOffset = 0.0;
      }
    }

    if (!isSystemOn) {
      pitchOffset = 0.0;
      turnOffsetVal = 0;
    }

    // Send precise movement offset command to Arduino UNO
    Serial2.print("CMD:");
    Serial2.print(pitchOffset);
    Serial2.print(",");
    Serial2.println(turnOffsetVal);

    // 2. Update OLED Display
    display.clearDisplay();
    
    // --- Layout Update ---
    // Title Bar
    display.fillRect(0, 0, 128, 12, SSD1306_WHITE);
    display.setTextColor(SSD1306_BLACK);
    display.setTextSize(1);
    display.setCursor(2, 2);
    if (webSocket.connectedClients() > 0) {
      display.print("[<->] "); 
    } else {
      display.print("[ x ] "); 
    }
    display.print(currentIP);

    // Main Content
    display.setTextColor(SSD1306_WHITE);
    
    // Mode Box
    display.setCursor(0, 16);
    display.print("MD:"); 
    display.print(currentMode);
    
    // Direction & State
    display.setCursor(0, 28);
    display.print("DR:"); 
    display.print(directionStr);
    display.print(" | ");
    if (systemState == "OBSTACLE") display.print("OBS!");
    else display.print("OK");

    // Distance
    display.setCursor(0, 40);
    display.print("DST:"); 
    if (distanceCm == 999) {
      display.print("MAX");
    } else { 
      display.print(distanceCm); 
      display.print("cm"); 
    }
    
    // Bottom line (Tilt)
    display.drawLine(0, 52, 128, 52, SSD1306_WHITE);
    display.setCursor(0, 55);
    display.print("TILT: "); 
    display.print(tiltAngle); 

    display.display();
  }
}