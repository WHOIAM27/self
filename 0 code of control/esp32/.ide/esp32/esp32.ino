#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define TRIG_PIN 5
#define ECHO_PIN 18

#define RXD2 16 // Receiving angle from UNO
#define TXD2 17 // Sending commands to UNO

const char* ssid = ".";
const char* password = "963852741";
WebServer server(80);

unsigned long lastSensorTime = 0;
String currentStatus = "Stop"; // Holds direction
String currentAngle = "0.0";   // Holds tilt degree
bool obstacleDetected = false;

void setup() {
  Serial.begin(115200); 
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); 
  Serial2.setTimeout(10); // Prevent ESP32 from waiting too long for serial data
  
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { for(;;); }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Connecting WiFi...");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }

  server.on("/cmd", []() {
    server.sendHeader("Access-Control-Allow-Origin", "https://whoiam27.github.io"); 
    server.sendHeader("Access-Control-Allow-Methods", "GET");
    
    String val = server.arg("val");
    Serial2.print(val); // Send to Arduino UNO
    
    // Translate for the OLED display
    if (val == "F") currentStatus = "Forward";
    else if (val == "B") currentStatus = "Backward";
    else if (val == "L") currentStatus = "Left";
    else if (val == "R") currentStatus = "Right";
    else if (val == "S") currentStatus = "Stop";
    
    server.send(200, "text/plain", "OK");
  });
  server.begin();
}

void loop() {
  server.handleClient(); 

  // 1. Read Angle from Arduino UNO
  if (Serial2.available()) {
    String incoming = Serial2.readStringUntil('\n');
    incoming.trim(); // Remove invisible newline characters
    if (incoming.length() > 0) {
      currentAngle = incoming; // Save the angle to display
    }
  }

  // 2. Read Ultrasonic Sensor (Every 100ms)
  if (millis() - lastSensorTime > 100) {
    long duration, distance;
    digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    duration = pulseIn(ECHO_PIN, HIGH, 30000); 
    distance = (duration / 2) / 29.1;

    // Safety Override
    if (distance > 0 && distance < 15) {
      Serial2.print('S'); 
      obstacleDetected = true;
    } else {
      obstacleDetected = false; 
    }

    updateOLED(distance);
    lastSensorTime = millis();
  }
}

void updateOLED(int dist) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AeroBalance Live");
  display.print("IP:"); display.println(WiFi.localIP());
  display.drawLine(0, 18, 128, 18, WHITE);
  
  // Highlight Obstacle if detected
  display.setCursor(0, 25);
  display.setTextSize(2);
  if (obstacleDetected) {
    display.println("OBSTACLE!");
  } else {
    display.print("Dir:"); 
    // Make text smaller if word is long (like 'Backward') to fit screen
    if (currentStatus == "Backward") display.setTextSize(1);
    display.println(currentStatus);
  }
  
  // Print Distance and Angle at the bottom
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.print("Dst:"); display.print(dist); display.print("cm ");
  display.print("Ang:"); display.print(currentAngle); display.print((char)247); // 247 is the degree symbol
  display.display();
}