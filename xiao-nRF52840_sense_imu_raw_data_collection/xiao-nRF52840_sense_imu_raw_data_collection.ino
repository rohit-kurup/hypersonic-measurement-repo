#include "LSM6DS3.h"
#include "Wire.h"

LSM6DS3 imu(I2C_MODE, 0x6A);  // I2C address 0x6A

void initIMU() {
  // Initialize with default settings
  if (imu.begin() != 0) {
    Serial.println("IMU initialization failed!");
    while(1);
  }
  
  // Manually configure ranges via registers (advanced)
  // Accelerometer: 16g range (CTRL1_XL = 0b10010100)
  // 1001 = 16g, 0100 = 104Hz ODR
  imu.writeRegister(LSM6DS3_ACC_GYRO_CTRL1_XL, 0x94);
  
  // Gyroscope: 2000dps range (CTRL2_G = 0b10011100)
  // 1001 = 2000dps, 1100 = 104Hz ODR
  imu.writeRegister(LSM6DS3_ACC_GYRO_CTRL2_G, 0x9C);
  
  Serial.println("IMU configured for:");
  Serial.println("- Accelerometer: ±16g");
  Serial.println("- Gyroscope: ±2000dps");
  Serial.println("- Output rate: 104Hz");
}

void setup() {
  Serial.begin(2000000);
  while(!Serial);
  
  initIMU();
  Serial.println("IMU Ready!");
}

void loop() {
  static int sampleCount = 0;

  // Read accelerometer and gyroscope data
  imu.readFloatAccelX();
  imu.readFloatAccelY();
  imu.readFloatAccelZ();
  imu.readFloatGyroX();
  imu.readFloatGyroY();
  imu.readFloatGyroZ();

  // Print data at intervals (e.g., every 20 samples)
  if (sampleCount % 20 == 0) {
    Serial.print("Accel: ");
    Serial.print(imu.readFloatAccelX());
    Serial.print(", ");
    Serial.print(imu.readFloatAccelY());
    Serial.print(", ");
    Serial.print(imu.readFloatAccelZ());
    
    Serial.print(" | Gyro: ");
    Serial.print(imu.readFloatGyroX());
    Serial.print(", ");
    Serial.print(imu.readFloatGyroY());
    Serial.print(", ");
    Serial.println(imu.readFloatGyroZ());
  }

  sampleCount++;

  // Optional delay to avoid overwhelming the serial output buffer
  // delay(5);  // ~200Hz
}
