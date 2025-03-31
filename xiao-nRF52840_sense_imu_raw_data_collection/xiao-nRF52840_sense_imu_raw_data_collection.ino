#include "LSM6DS3.h"
#include "Wire.h"

// RGB LED Pins (XIAO nRF52840 Sense)
const int ledR = 26;  // Red
const int ledG = 30;  // Green
const int ledB = 6;   // Blue

// IMU Setup
LSM6DS3 imu(I2C_MODE, 0x6A);

void setLed(bool r, bool g, bool b) {
  // Note: Inverted because it's a common anode LED
  digitalWrite(ledR, !r);
  digitalWrite(ledG, !g);
  digitalWrite(ledB, !b);
}

void setup() {
  // Initialize LED pins
  pinMode(ledR, OUTPUT);
  pinMode(ledG, OUTPUT);
  pinMode(ledB, OUTPUT);
  setLed(0, 0, 1);  // Start with blue (initializing)

  Serial.begin(115200);
  while (!Serial);  // Wait for serial

  Serial.println("Initializing IMU...");

  // Attempt IMU initialization
  bool imuReady = false;
  for (int attempts = 0; attempts < 5; attempts++) {
    if (imu.begin() == 0) {
      imuReady = true;
      break;
    }
    // Blink red during retries
    setLed(1, 0, 0); delay(100);
    setLed(0, 0, 0); delay(100);
  }

  if (!imuReady) {
    // Solid red for failure
    setLed(1, 0, 0);
    Serial.println("IMU FAILED!");
    while(1);
  }

  // Green light for success
  setLed(0, 1, 0);
  Serial.println("IMU Ready!");
  Serial.println("Timestamp(ms),AccX,AccY,AccZ,GyroX,GyroY,GyroZ");
}

void loop() {
  // Purple during reading (red + blue)
  setLed(1, 0, 1);
  
  // Take readings
  float accX = imu.readFloatAccelX();
  float accY = imu.readFloatAccelY();
  float accZ = imu.readFloatAccelZ();
  float gyroX = imu.readFloatGyroX();
  float gyroY = imu.readFloatGyroY();
  float gyroZ = imu.readFloatGyroZ();

  // Print data
  Serial.print(millis()); Serial.print(",");
  Serial.print(accX); Serial.print(",");
  Serial.print(accY); Serial.print(",");
  Serial.print(accZ); Serial.print(",");
  Serial.print(gyroX); Serial.print(",");
  Serial.print(gyroY); Serial.print(",");
  Serial.println(gyroZ);

  // Visual feedback based on movement
  if (abs(accX) > 1.5 || abs(accY) > 1.5 || abs(gyroZ) > 100) {
    // Yellow (red+green) for significant movement
    setLed(1, 1, 0);
  } else {
    // Solid green for normal operation
    setLed(0, 1, 0);
  }

  delay(50);  // ~20Hz
}