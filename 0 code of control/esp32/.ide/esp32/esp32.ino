#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WebSocketsServer.h> // Make sure you have the WebSockets library installed (by Markus Sattler)

// --- OLED Display Settings ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- Ultrasonic Sensor Pins (ESP32) ---
const int trigPin = 5;  
const int echoPin = 18; 

// Variables for distance calculation
int distanceCm = 0;
String systemState = "IDLE"; // "IDLE" or "OBSTACLE"

// WiFi & WebSocket Settings
const char* ssid = "AeroBalance";
const char* password = ""; // Open network
WebSocketsServer webSocket = WebSocketsServer(80);

// Global State Variables from Website
String currentIP = "Not Connected";
String currentMode = "STANDBY";
String directionStr = "STOP";
String tiltAngle = "0, 0";
bool isSystemOn = false;

unsigned long lastUpdate = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch(type) {
    case WStype_DISCONNECTED:
      break;
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
        String coords = msg.substring(4);
        if (coords == "0,100") directionStr = "FWD";
        else if (coords == "0,-100") directionStr = "REV";
        else if (coords == "-100,0") directionStr = "LEFT";
        else if (coords == "100,0") directionStr = "RIGHT";
        else if (coords == "0,0") directionStr = "STOP";
        else directionStr = coords; // For joystick intermediate values
      }
      else if (msg.startsWith("GYRO:")) {
        tiltAngle = msg.substring(5);
        directionStr = "TILT";
      }
      else if (msg.startsWith("SLI:")) {
        String speed = msg.substring(4);
        if (speed == "0") directionStr = "STOP";
        else if (speed.toInt() > 0) directionStr = "FWD " + speed + "%";
        else directionStr = "REV " + String(abs(speed.toInt())) + "%";
      }
      break;
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize Ultrasonic Pins
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); 
  }

  // Boot Screen
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 20);
  display.println(F("BOOTING..."));
  display.display();

  // Setup WiFi Access Point
  WiFi.softAP(ssid, password);
  currentIP = WiFi.softAPIP().toString();

  // Start WebSocket Server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  
  delay(1500); 
}

void loop() {
  // Handle WebSocket clients
  webSocket.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate > 100) { // Update display and sensor every 100ms
    lastUpdate = currentMillis;

    // 1. Read Ultrasonic Sensor
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
    if (duration == 0) {
      distanceCm = 999; // Out of range
    } else {
      distanceCm = duration * 0.034 / 2; 
    }

    // Determine state
    if (distanceCm < 20) {
      systemState = "OBSTACLE";
    } else {
      systemState = "IDLE";
    }

    // 2. Update OLED Display
    display.clearDisplay();
    
    // IP Address
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("IP: "); 
    display.print(currentIP);
    
    // Mode
    display.setCursor(0, 10);
    display.print("Mode: "); 
    display.print(currentMode);
    
    // Direction
    display.setCursor(0, 20);
    display.print("Dir: "); 
    display.print(directionStr);
    
    // Tilt Angle
    display.setCursor(0, 30);
    display.print("Tilt: "); 
    display.print(tiltAngle);
    
    // Distance
    display.setCursor(0, 40);
    display.print("Dist: "); 
    if (distanceCm == 999) display.print("OUT OF RANGE");
    else { display.print(distanceCm); display.print(" cm"); }
    
    // Idle or Obstacle
    display.setCursor(0, 50);
    display.print("State: "); 
    display.print(systemState);

    display.display();
  }
}