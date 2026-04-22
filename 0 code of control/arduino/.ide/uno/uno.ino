#include "Wire.h"
#include <MPU6050_light.h>

MPU6050 mpu(Wire);

// --- L298N MOTOR PINS ---
#define ENA 9
#define IN1 8
#define IN2 7
#define IN3 6
#define IN4 5
#define ENB 10

// --- PID TUNING VARIABLES ---
// Adjust these numbers based on your physical floor tests!
float Kp = 25.0; // Start here. Increase until it jitters.
float Ki = 0.5;  // Keep very low. Helps prevent drifting.
float Kd = 1.2;  // Increase to smooth out the jitter.

float targetAngle = 0.0; 
float turnOffset = 0.0;
float error, previousError, P, I, D, PID_value;
unsigned long previousTime;

// Timer for sending display data to ESP32
unsigned long lastDisplayUpdate = 0; 

// --- FRICTION DEADBAND ---
// Increase this if your motors whine but don't spin at low angles
int minPWM = 40; 

void setup() {
  Serial.begin(9600); 
  Wire.begin();

  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  byte status = mpu.begin();
  while(status != 0) { } 
  
  // --- INSTANT BOOT CALIBRATION ---
  // REPLACE THESE NUMBERS WITH YOUR EXACT CALIBRATION DATA!
 mpu.setGyroOffsets(-3.56, -0.87, -1.25);
mpu.setAccOffsets(-0.04, -0.01, 0.07);  

  delay(100); 
}

void loop() {
  // 1. Listen for Web Dashboard Commands from ESP32
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    switch(cmd) {
      case 'F': targetAngle = 2.0; turnOffset = 0; break; 
      case 'B': targetAngle = -2.0; turnOffset = 0; break; 
      case 'L': targetAngle = 0.0; turnOffset = -30; break;
      case 'R': targetAngle = 0.0; turnOffset = 30; break;  
      case 'S': targetAngle = 0.0; turnOffset = 0; break;   
    }
  }

  // 2. Read Sensor Data
  mpu.update();
  unsigned long currentTime = millis();
  float dt = (currentTime - previousTime) / 1000.0; 
  previousTime = currentTime;
  float currentAngle = mpu.getAngleY(); // Ensure your MPU is mounted so Y is Pitch

  // 3. Send live angle to ESP32 OLED every 100ms
  if (currentTime - lastDisplayUpdate > 100) {
    Serial.println(currentAngle); 
    lastDisplayUpdate = currentTime;
  }

  // 4. Compute PID Math
  error = currentAngle - targetAngle;
  P = Kp * error;
  I += Ki * error * dt;
  D = Kd * ((error - previousError) / dt);
  PID_value = P + I + D;
  previousError = error;

  // 5. Apply Deadband Compensation
  int baseSpeed = 0;
  if (PID_value > 0) baseSpeed = PID_value + minPWM;
  else if (PID_value < 0) baseSpeed = abs(PID_value) + minPWM;
  
  // Mix in steering controls for turning
  int speedLeft = constrain(baseSpeed + turnOffset, 0, 255);
  int speedRight = constrain(baseSpeed - turnOffset, 0, 255);

  // 6. Drive the Motors
  if (abs(error) > 30) {
    // CRASH DETECTION: Shut off motors if it falls over
    analogWrite(ENA, 0); analogWrite(ENB, 0);
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    I = 0; // Reset integral memory
  } else if (PID_value > 0) {
    // Fall Forward -> Drive Forward
    analogWrite(ENA, speedLeft); analogWrite(ENB, speedRight);
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  } else if (PID_value < 0) {
    // Fall Backward -> Drive Backward
    analogWrite(ENA, speedLeft); analogWrite(ENB, speedRight);
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  }
}