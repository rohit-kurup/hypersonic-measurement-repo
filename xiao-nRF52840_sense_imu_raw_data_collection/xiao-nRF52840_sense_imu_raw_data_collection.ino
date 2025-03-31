// including the libraries needed for this program 
#include "LSM6DS3.h" // library that is required to talk to the inbuilt IMU 
#include "Wire.h" // contains tools for I2C comminication 

// Use the built-in IMU with correct initialization
LSM6DS3 imu(I2C_MODE, 0x6A);  // I2C address 0x6A for XIAO Sense

void setup() {
  Serial.begin(115200);
  while (!Serial);  // Wait for serial connection
  
  Serial.println("Initializing IMU...");
  
  if (imu.begin() != 0) {
    Serial.println("IMU Error! Check:");
    Serial.println("1. Board selected: XIAO nRF52840 Sense");
    Serial.println("2. No other IMU libraries installed");
    while(1);  // Halt if initialization fails
  }
  
  Serial.println("IMU Ready!");
  Serial.println("Timestamp(ms),AccX,AccY,AccZ,GyroX,GyroY,GyroZ");
}

void loop() {
  // Print timestamp
  Serial.print(millis());
  Serial.print(",");
  
  // Accelerometer data
  Serial.print(imu.readFloatAccelX());
  Serial.print(",");
  Serial.print(imu.readFloatAccelY());
  Serial.print(",");
  Serial.print(imu.readFloatAccelZ());
  Serial.print(",");
  
  // Gyroscope data
  Serial.print(imu.readFloatGyroX());
  Serial.print(",");
  Serial.print(imu.readFloatGyroY());
  Serial.print(",");
  Serial.println(imu.readFloatGyroZ());
  
  delay(50);  // ~20Hz update rate
}