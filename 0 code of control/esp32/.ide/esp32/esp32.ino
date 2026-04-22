#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// --- OLED SETUP ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- ULTRASONIC PINS ---
#define TRIG_PIN 5
#define ECHO_PIN 18

// --- UART PINS (To Logic Level Converter LV side) ---
#define RXD2 16
#define TXD2 17

// --- WIFI SETUP ---
const char* ssid = ".";
const char* password = "963852741";
WebServer server(80);

unsigned long lastSensorTime = 0;
unsigned long lastWiFiCheckTime = 0;
String currentStatus = "IDLE";

void setup() {
  Serial.begin(115200); 
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); 
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize OLED
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED allocation failed");
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("AeroBalance Booting**");
  display.display();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // --- WEB SERVER ENDPOINT ---
  server.on("/cmd", []() {
    server.sendHeader("Access-Control-Allow-Origin", "https://whoiam27.github.io"); 
    server.sendHeader("Access-Control-Allow-Methods", "GET");
    
    String val = server.arg("val");
    Serial2.print(val); 
    currentStatus = "CMD: " + val;
    server.send(200, "text/plain", "OK");
  });

  server.begin();
}

void loop() {
  server.handleClient(); // Listen for web dashboard clicks

  // 1. Auto-Reconnect Wi-Fi if it drops (Checks every 5 seconds)
  if (millis() - lastWiFiCheckTime > 5000) {
    if (WiFi.status() != WL_CONNECTED) {
      WiFi.reconnect(); // Silently try to reconnect without freezing the robot
    }
    lastWiFiCheckTime = millis();
  }

  // 2. Read Sensors and Update Screen (Every 100ms)
  if (millis() - lastSensorTime > 100) {
    long duration, distance;
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    // REDUCED TIMEOUT: 15000us max wait (prevents Wi-Fi crashing)
    duration = pulseIn(ECHO_PIN, HIGH, 15000); 
    
    if (duration == 0) {
      distance = -1; // Sensor didn't hear an echo (out of range)
    } else {
      distance = (duration / 2) / 29.1;
    }

    // Safety Override: Obstacle closer than 15cm
    if (distance > 0 && distance < 15) {
      Serial2.print('S'); 
      currentStatus = "OBSTACLE!";
    }

    updateOLED(currentStatus, distance);
    lastSensorTime = millis();
  }
}

// --- OLED RENDER FUNCTION ---
void updateOLED(String statusMsg, int dist) {
  display.clearDisplay();
  
  // Top Section: Title & IP
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("AeroBalance System");
  display.println("---------------------");
  
  if(WiFi.status() == WL_CONNECTED) {
    display.print("IP: "); display.println(WiFi.localIP());
  } else {
    display.println("IP: Reconnecting...");
  }
  
  // Middle Section: Robot Action
  display.setCursor(0, 32);
  display.setTextSize(2);
  display.println(statusMsg);
  
  // Bottom Left: Distance Data
  display.setTextSize(1);
  display.setCursor(0, 55);
  if(dist >= 0) {
    display.print("Dist: "); display.print(dist); display.print("cm");
  } else {
    display.print("Dist: ---");
  }

  // Bottom Right: Wi-Fi Status Symbol
  display.setCursor(85, 55); // Positioned at bottom right
  if(WiFi.status() == WL_CONNECTED) {
    display.print("[Wi-Fi]"); // Shows when connected
  } else {
    display.print("[  X  ]"); // Shows when disconnected
  }
  
  display.display();
}