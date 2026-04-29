// ==========================================
// SELF-BALANCING ROBOT - FULL UNIFIED CODE
// ==========================================

#include "I2Cdev.h"
#include <PID_v1.h> 
#include "MPU6050_6Axis_MotionApps20.h" 

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  #include "Wire.h"
#endif

MPU6050 mpu;

// ==========================================
// MPU Control/Status Variables
// ==========================================
bool dmpReady = false;  
uint8_t mpuIntStatus;   
uint8_t devStatus;      
uint16_t packetSize;    
uint16_t fifoCount;     
uint8_t fifoBuffer[64]; 

// Orientation/Motion Variables
Quaternion q;           
VectorFloat gravity;    
float ypr[3];           

// ==========================================
// 🛑 YOUR SPECIFIC TUNING VALUES 🛑
// ==========================================
// 180 is perfectly flat. If it drifts forward, lower this (e.g., 179.5)
double originalSetpoint = 180.0; 
double setpoint = originalSetpoint;

// START TUNING HERE:
double Kp = 15; // Set this first (Increase until it pushes back and wobbles)
double Kd = 0;  // Set this second (Increase to stop the wobble)
double Ki = 0;  // Set this LAST (Only add 1 or 2 to fix slow drifting)

double input, output;
PID pid(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

// ==========================================
// L298N MOTOR DRIVER PINS
// ==========================================
int ENA = 9;
int IN1 = 8;
int IN2 = 7;
int IN3 = 6;
int IN4 = 5;
int ENB = 10;

volatile bool mpuInterrupt = false;     

void dmpDataReady() {
    mpuInterrupt = true;
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("Initializing I2C devices..."));

  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    TWBR = 24; // 400kHz I2C clock
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  mpu.initialize();
  Serial.println(F("Testing device connections..."));
  Serial.println(mpu.testConnection() ? F("MPU6050 connection successful") : F("MPU6050 connection failed"));
  
  devStatus = mpu.dmpInitialize();
  
  // Reset Gyro Offsets to 0 for a clean baseline
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  mpu.setZAccelOffset(0); 
      
  if (devStatus == 0) {
      Serial.println(F("Enabling DMP..."));
      mpu.setDMPEnabled(true);
      
      Serial.println(F("Enabling interrupt detection (Arduino external interrupt 0)..."));
      // THIS REQUIRES THE SENSOR 'INT' PIN TO BE PLUGGED INTO ARDUINO D2
      attachInterrupt(0, dmpDataReady, RISING);
      mpuIntStatus = mpu.getIntStatus();
      
      Serial.println(F("DMP ready! Waiting for first interrupt..."));
      dmpReady = true;
      packetSize = mpu.dmpGetFIFOPacketSize();
      
      // Setup PID parameters
      pid.SetMode(AUTOMATIC);
      pid.SetSampleTime(10);
      pid.SetOutputLimits(-255, 255);
  } else {
      Serial.print(F("DMP Initialization failed (code "));
      Serial.print(devStatus);
      Serial.println(F(")"));
  }

  // Initialize Motor Output Pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  // Ensure motors are off at startup
  Stop();
}

void loop() {
    // Stop if MPU failed to load
    if (!dmpReady) return;
    
    // Check how much data the sensor has ready
    fifoCount = mpu.getFIFOCount();
    
    // ==========================================
    // THE BYPASS: If no data is ready, do the balancing math
    // ==========================================
    if (fifoCount < packetSize) {
        pid.Compute();
        
        Serial.print("Angle Input: "); 
        Serial.print(input); 
        Serial.print(" \t => \t Motor Power: "); 
        Serial.println(output);
        
        // Safety Cutoff: Only run motors if between 135 and 225 degrees AND not 0
        if (input > 135 && input < 225 && input != 0.00) { 
            if (output > 0) {
                Forward(); 
            } else if (output < 0) {
                Reverse(); 
            }
        } else {
            Stop(); 
        }
    } 
    // ==========================================
    // If data IS ready, read it directly! (No INT pin needed)
    // ==========================================
    else {
        mpuIntStatus = mpu.getIntStatus();
        
        // Check for FIFO overflow
        if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
            mpu.resetFIFO();
            Serial.println(F("FIFO overflow!"));
        } 
        else if (mpuIntStatus & 0x02) {
            // Read the packet
            mpu.getFIFOBytes(fifoBuffer, packetSize);
            
            // Calculate Yaw, Pitch, Roll
            mpu.dmpGetQuaternion(&q, fifoBuffer); 
            mpu.dmpGetGravity(&gravity, &q); 
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity); 
            
            // Convert to 0-360 degrees
            input = ypr[1] * 180/M_PI + 180;
        }
    }
}

// ==========================================
// CUSTOM MOTOR MOVEMENT FUNCTIONS
// ==========================================

void Forward() {
    // Set motor direction forward
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    
    // Apply PID power to speed pins
    analogWrite(ENA, output);
    analogWrite(ENB, output);
}

void Reverse() {
    // Set motor direction backward
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    
    // Apply PID power (Multiply by -1 to convert negative output to positive for analogWrite)
    analogWrite(ENA, output * -1);
    analogWrite(ENB, output * -1);
}

void Stop() {
    // Turn off all motor pins
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    
    // Set speed to 0
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
}