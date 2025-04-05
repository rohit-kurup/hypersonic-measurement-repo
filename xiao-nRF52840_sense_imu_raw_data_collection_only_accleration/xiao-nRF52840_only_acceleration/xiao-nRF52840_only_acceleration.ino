#include "LSM6DS3.h"
#include "Wire.h"

// Initialize IMU with I2C mode and address
LSM6DS3 imu(I2C_MODE, 0x6A);  // I2C address 0x6A

// Define an array to store 20 sets of acceleration data
float accelData[40][3];  // 20 samples, each with 3 values (X, Y, Z)
int sampleCount = 0;  // Counter for the number of collected samples

void initIMU() {
  // Initialize with default settings
  if (imu.begin() != 0) {
    Serial.println("IMU initialization failed!");
    while(1);
  }
  
  // Set Accelerometer to 1.66 kHz, ±16g (increase ODR for faster sampling)
  imu.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x9C);  // ODR = 1.66 kHz, ±16g
  
  Serial.println("IMU configured for Accelerometer (1.66 kHz) only!");
}

void setup() {
  Serial.begin(2000000);  // High baud rate for fast communication
  while (!Serial);  // Wait for serial monitor
  
  initIMU();  // Configure IMU
  Serial.println("IMU Ready for Accelerometer Data Collection!");
}

void loop() {
  // Collect accelerometer data (X, Y, Z)
  accelData[sampleCount][0] = imu.readFloatAccelX();
  accelData[sampleCount][1] = imu.readFloatAccelY();
  accelData[sampleCount][2] = imu.readFloatAccelZ();
  
  sampleCount++;  // Increment the sample count
  
  // If we have collected 20 samples, print them
  if (sampleCount >= 40) {
    // Print the collected data
    Serial.println("Collected 40 Samples:");
    for (int i = 0; i < 40; i++) {
      Serial.print("Sample ");
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(accelData[i][0]);
      Serial.print(", ");
      Serial.print(accelData[i][1]);
      Serial.print(", ");
      Serial.println(accelData[i][2]);
    }

    // Reset the sample count to start collecting the next set of 20 samples
    sampleCount = 0;
  }
  
  // No delay to get the fastest possible rate
  // delay(1);  // Optional for slightly slower printing, but avoid delay for max speed
}
