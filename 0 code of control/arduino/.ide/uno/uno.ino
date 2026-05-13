// ==========================================
// SELF-BALANCING ROBOT - UNO BRAIN
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

Quaternion q;           
VectorFloat gravity;    
float ypr[3];           

// ==========================================
// TUNING VALUES
// ==========================================
double originalSetpoint = 180.0; 
double setpoint = originalSetpoint;

double Kp = 20; 
double Kd = 0;  
double Ki = 0;  

double input, output;
PID pid(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

// ==========================================
// L298N MOTOR DRIVER PINS
// ==========================================
int ENA = 9;
int IN1 = 7;
int IN2 = 8;
int IN3 = 5;
int IN4 = 6;
int ENB = 10;

volatile bool mpuInterrupt = false;     
unsigned long lastTiltSend = 0; // Timer for sending data to ESP32

void dmpDataReady() {
    mpuInterrupt = true;
}

void setup() {
  Serial.begin(115200);

  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    Wire.begin();
    TWBR = 24; 
  #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
    Fastwire::setup(400, true);
  #endif

  mpu.initialize();
  devStatus = mpu.dmpInitialize();
  
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  mpu.setZAccelOffset(0); 
      
  if (devStatus == 0) {
      mpu.setDMPEnabled(true);
      attachInterrupt(0, dmpDataReady, RISING);
      mpuIntStatus = mpu.getIntStatus();
      dmpReady = true;
      packetSize = mpu.dmpGetFIFOPacketSize();
      
      pid.SetMode(AUTOMATIC);
      pid.SetSampleTime(10);
      pid.SetOutputLimits(-255, 255);
  }

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  
  Stop();
}

void loop() {
    if (!dmpReady) return;
    fifoCount = mpu.getFIFOCount();
    
    if (fifoCount < packetSize) {
        pid.Compute();
        
        // Send the angle to the ESP32 cleanly every 100ms
        if (millis() - lastTiltSend > 100) {
            Serial.print("T:");
            Serial.println(input);
            lastTiltSend = millis();
        }
        
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
    else {
        mpuIntStatus = mpu.getIntStatus();
        
        if ((mpuIntStatus & 0x10) || fifoCount == 1024) {
            mpu.resetFIFO();
        } 
        else if (mpuIntStatus & 0x02) {
            mpu.getFIFOBytes(fifoBuffer, packetSize);
            mpu.dmpGetQuaternion(&q, fifoBuffer); 
            mpu.dmpGetGravity(&gravity, &q); 
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity); 
            
            input = ypr[1] * 180/M_PI + 180;
        }
    }
}

void Forward() {
    digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
    analogWrite(ENA, output); analogWrite(ENB, output);
}

void Reverse() {
    digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
    analogWrite(ENA, output * -1); analogWrite(ENB, output * -1);
}

void Stop() {
    digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
    analogWrite(ENA, 0); analogWrite(ENB, 0);
}