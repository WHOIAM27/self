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

// --- PID & BALANCING VARIABLES ---
// You will still need to tune these three!
float Kp = 25.0; 
float Ki = 0.5;   
float Kd = 1.2;   

float targetAngle = 0.0; // The angle where it stands perfectly still
float turnOffset = 0.0;
float error, previousError, P, I, D, PID_value;
unsigned long previousTime;

// --- DEADBAND COMPENSATION ---
// Minimum PWM needed to overcome your standard gear motors' friction
int minPWM = 40; 

void setup() {
  Serial.begin(9600); // Listen to the ESP32 via Logic Level Converter
  Wire.begin();

  pinMode(ENA, OUTPUT); pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT); pinMode(ENB, OUTPUT);

  byte status = mpu.begin();
  while(status != 0) { } // Halt if MPU6050 wiring is wrong
  
  delay(1000);
  mpu.calcOffsets(true,true); // Keep robot totally still during power-on!
}

void loop() {
  // 1. Listen for Brain Commands
  if (Serial.available() > 0) {
    char cmd = Serial.read();
    switch(cmd) {
      case 'F': targetAngle = 2.0; turnOffset = 0; break;  // Lean forward
      case 'B': targetAngle = -2.0; turnOffset = 0; break; // Lean backward
      case 'L': targetAngle = 0.0; turnOffset = -30; break; // Spin left
      case 'R': targetAngle = 0.0; turnOffset = 30; break;  // Spin right
      case 'S': targetAngle = 0.0; turnOffset = 0; break;   // Stop
    }
  }

  // 2. Read MPU6050
  mpu.update();
  unsigned long currentTime = millis();
  float dt = (currentTime - previousTime) / 1000.0; 
  previousTime = currentTime;
  float currentAngle = mpu.getAngleY(); // Ensure your MPU is mounted so Y is the pitch axis

  // 3. Compute PID
  error = currentAngle - targetAngle;
  P = Kp * error;
  I += Ki * error * dt;
  D = Kd * ((error - previousError) / dt);
  PID_value = P + I + D;
  previousError = error;

  // 4. Deadband & Steering Math
  int baseSpeed = 0;
  if (PID_value > 0) baseSpeed = PID_value + minPWM;
  else if (PID_value < 0) baseSpeed = abs(PID_value) + minPWM;
  
  // Mix in turning offset for tank controls
  int speedLeft = constrain(baseSpeed + turnOffset, 0, 255);
  int speedRight = constrain(baseSpeed - turnOffset, 0, 255);

  // 5. Drive Motors with Safety Shutoff
  if (abs(error) > 30) {
    // It fell over. Stop motors.
    analogWrite(ENA, 0); analogWrite(ENB, 0);
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    I = 0; 
  } else if (PID_value > 0) {
    // Falling Forward -> Drive Forward to catch it
    analogWrite(ENA, speedLeft); analogWrite(ENB, speedRight);
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  } else if (PID_value < 0) {
    // Falling Backward -> Drive Backward to catch it
    analogWrite(ENA, speedLeft); analogWrite(ENB, speedRight);
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  }
}