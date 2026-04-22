 #include <WiFi.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define TRIG_PIN 5
#define ECHO_PIN 18

#define RXD2 16 
#define TXD2 17 

// Your exact Wi-Fi details
const char* ssid = ".";
const char* password = "963852741";

// Create a WebSocket server on port 80
WebSocketsServer webSocket = WebSocketsServer(80);

unsigned long lastSensorTime = 0;
String currentStatus = "IDLE";
String currentAngle = "0.0";
bool obstacleDetected = false;

// --- WEBSOCKET DATA HANDLER ---
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String msg = String((char*)payload);
    
    // Check if the message is from the ARROW buttons (e.g., "ARR:0,100")
    if (msg.startsWith("ARR:")) {
      int colonIndex = msg.indexOf(':');
      int commaIndex = msg.indexOf(',');
      
      // Extract the X and Y numbers
      int x = msg.substring(colonIndex + 1, commaIndex).toInt();
      int y = msg.substring(commaIndex + 1).toInt();
      
      char cmd = 'S'; // Default to Stop
      
      // Translate coordinates to Arduino UNO commands
      if (y > 50) { cmd = 'F'; currentStatus = "Forward"; }
      else if (y < -50) { cmd = 'B'; currentStatus = "Backward"; }
      else if (x > 50) { cmd = 'R'; currentStatus = "Right"; }
      else if (x < -50) { cmd = 'L'; currentStatus = "Left"; }
      else { cmd = 'S'; currentStatus = "Stop"; }

      // Send the translated letter to the Arduino UNO
      if (!obstacleDetected || cmd == 'B' || cmd == 'S') {
        Serial2.print(cmd); 
      }
    }
  }
}

void setup() {
  Serial.begin(115200); 
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2); 
  Serial2.setTimeout(10); 
  
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

  // Start the WebSocket server instead of the standard WebServer
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  // Keep the WebSocket connection alive and listening
  webSocket.loop(); 

  // Read live angle from UNO
  if (Serial2.available()) {
    String incoming = Serial2.readStringUntil('\n');
    incoming.trim(); 
    if (incoming.length() > 0) currentAngle = incoming; 
  }

  // Ultrasonic Sensor & OLED Update
  if (millis() - lastSensorTime > 100) {
    long duration, distance;
    digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    duration = pulseIn(ECHO_PIN, HIGH, 30000); 
    distance = (duration / 2) / 29.1;

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
  
  display.setCursor(0, 25);
  display.setTextSize(2);
  if (obstacleDetected) {
    display.println("OBSTACLE!");
  } else {
    display.print("Dir:"); 
    if (currentStatus == "Backward") display.setTextSize(1);
    display.println(currentStatus);
  }
  
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.print("Dst:"); display.print(dist); display.print("cm ");
  display.print("Ang:"); display.print(currentAngle); display.print((char)247);
  display.display();
}