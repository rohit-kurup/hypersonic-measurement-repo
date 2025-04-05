#include <SPI.h>
#include <SdFat.h>

#define CS_SD 10  // SD Card Chip Select
#define CS_IMU 9  // IMU Chip Select (ICM-42688-P)

// IMU Registers
#define WHO_AM_I 0x75
#define PWR_MGMT_0 0x4E
#define GYRO_ODR 0x4F   // Correct register for Gyro ODR
#define ACCEL_ODR 0x50  // Correct register for Accel ODR
#define ACCEL_X_H 0x1F
#define GYRO_X_H  0x25
#define READ_FLAG 0x80

SdFat SD;
File imuFile;

const unsigned long LOG_DURATION = 15000;  // 60 seconds (1 min)
unsigned long startTime;
int entryCount = 0;
char buffer[5000];  // Buffer to store data before writing
int bufferIndex = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("Initializing SD card & IMU...");

    // Initialize SPI
    SPI.begin();
    
    // Setup CS Pins
    pinMode(CS_SD, OUTPUT);
    pinMode(CS_IMU, OUTPUT);
    digitalWrite(CS_SD, HIGH);
    digitalWrite(CS_IMU, HIGH);
    
    delay(10);

    // Initialize SD Card
    if (!SD.begin(CS_SD, SPI_FULL_SPEED)) {
        Serial.println("SD Card failed!");
        return;
    }
    Serial.println("SD Card initialized.");

    // Initialize IMU
    uint8_t whoAmI = readIMURegister(WHO_AM_I);
    Serial.print("IMU WHO_AM_I Register: 0x");
    Serial.println(whoAmI, HEX);

    if (whoAmI != 0x4C) {
        Serial.println("IMU NOT detected. Check wiring!");
        return;
    }
    Serial.println("IMU detected!");

    // ✅ Set 32kHz Data Rate
    Serial.println("Configuring IMU for 32kHz ODR...");
    writeIMURegister(PWR_MGMT_0, 0x0F);  // Enable Accel & Gyro
    writeIMURegister(GYRO_ODR, 0x01);    // Set Gyro ODR to 32kHz
    writeIMURegister(ACCEL_ODR, 0x01);   // Set Accel ODR to 32kHz
    delay(100);

    // ✅ Read back values to confirm settings
    Serial.print("PWR_MGMT_0: 0x"); Serial.println(readIMURegister(PWR_MGMT_0), HEX);
    Serial.print("GYRO_ODR: 0x"); Serial.println(readIMURegister(GYRO_ODR), HEX);
    Serial.print("ACCEL_ODR: 0x"); Serial.println(readIMURegister(ACCEL_ODR), HEX);

    Serial.println("IMU is ready. Starting data collection...");

    // Open CSV file once at the start
    imuFile = SD.open("imu_data.csv", FILE_WRITE);
    if (imuFile) {
        imuFile.println("Time (ms),Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z");
        Serial.println("CSV File initialized.");
    } else {
        Serial.println("Error opening imu_data.csv.");
    }

    // Start timer
    startTime = millis();
}

void loop() {
    unsigned long currentTime = millis();
    
    if (currentTime - startTime >= LOG_DURATION) {
        Serial.println("✅ Logging complete. Data saved to imu_data.csv.");

        // Flush remaining data in buffer to SD card
        imuFile.write(buffer, bufferIndex);
        imuFile.flush();
        imuFile.close();

        while (1);  // Stop execution
    }

    // Read IMU Data at 32kHz
    for (int i = 0; i < 32; i++) { // Read 32 samples in 1ms
        int16_t accelX = read16BitIMU(ACCEL_X_H);
        int16_t accelY = read16BitIMU(ACCEL_X_H + 2);
        int16_t accelZ = read16BitIMU(ACCEL_X_H + 4);
        int16_t gyroX = read16BitIMU(GYRO_X_H);
        int16_t gyroY = read16BitIMU(GYRO_X_H + 2);
        int16_t gyroZ = read16BitIMU(GYRO_X_H + 4);

        // Print data to Serial Monitor (32 times per ms)
        Serial.print(currentTime - startTime); Serial.print(",");
        Serial.print(accelX); Serial.print(",");
        Serial.print(accelY); Serial.print(",");
        Serial.print(accelZ); Serial.print(",");
        Serial.print(gyroX); Serial.print(",");
        Serial.print(gyroY); Serial.print(",");
        Serial.println(gyroZ);

        // Store data in buffer (instead of writing directly)
        bufferIndex += snprintf(buffer + bufferIndex, sizeof(buffer) - bufferIndex,
                                "%lu,%d,%d,%d,%d,%d,%d\n",
                                currentTime - startTime, accelX, accelY, accelZ, gyroX, gyroY, gyroZ);
    }

    entryCount += 32;  // Increment by 32 since we are sampling at 32kHz

    // Write to SD card every 5000 entries (batch writing)
    if (entryCount % 5000 == 0) {
        imuFile.write(buffer, bufferIndex);
        imuFile.flush();
        bufferIndex = 0;  // Reset buffer after writing
    }
}

// Function to write to IMU register
void writeIMURegister(uint8_t reg, uint8_t value) {
    digitalWrite(CS_IMU, LOW);
    SPI.transfer(reg & 0x7F);  // Write command
    SPI.transfer(value);
    digitalWrite(CS_IMU, HIGH);
}

// Read 16-bit values from IMU
int16_t read16BitIMU(uint8_t reg) {
    uint8_t highByte = readIMURegister(reg);
    uint8_t lowByte = readIMURegister(reg + 1);
    return (int16_t)((highByte << 8) | lowByte);
}

// Read a register from the IMU
uint8_t readIMURegister(uint8_t reg) {
    digitalWrite(CS_IMU, LOW);
    SPI.transfer(reg | READ_FLAG);
    uint8_t data = SPI.transfer(0x00);
    digitalWrite(CS_IMU, HIGH);
    return data;
}
