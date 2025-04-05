#include <SPI.h>
#include <ArduinoBLE.h>

// IMU Register Addresses
#define CS_PIN 10
#define WHO_AM_I_REG 0x46
#define PWR_MGMT_1 0x06
#define PWR_MGMT_0 0x4E

// IMU Data Registers (first register for burst read)
#define ACCEL_X_H 0x1F  

// BLE Service and Characteristics
BLEService imuService("1001");
BLECharacteristic imuCharacteristic("2001", BLERead | BLENotify, 240); // Use max size for efficiency

// IMU Data Buffer
uint8_t imuBuffer[240];  // Buffer for 20 samples (each 12 bytes)
int bufferIndex = 0;

// Function Prototypes (Declare them before usage)
void writeRegister(uint8_t reg, uint8_t value);
uint8_t readRegister(uint8_t reg);
void readIMUData(uint8_t *buffer);

void setup() {
    Serial.begin(115200);
    while (!Serial);

    pinMode(CS_PIN, OUTPUT);
    digitalWrite(CS_PIN, HIGH);
    SPI.begin();
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0)); // Increased SPI speed to 8 MHz

    uint8_t whoAmI = readRegister(WHO_AM_I_REG);
    Serial.println(whoAmI);
    if (whoAmI != 0x46) {
        Serial.println("❌ IMU NOT detected. Check wiring!");
        while (1);
    }

    writeRegister(PWR_MGMT_1, 0x01);
    delay(10);
    writeRegister(PWR_MGMT_0, 0x0F);
    delay(10);

    // Initialize BLE
    if (!BLE.begin()) {
        Serial.println("❌ BLE Initialization Failed!");
        while (1);
    }

    BLE.setLocalName("IMU Sensor");
    BLE.setAdvertisedService(imuService);
    imuService.addCharacteristic(imuCharacteristic);
    BLE.addService(imuService);

    // BLE Optimizations
    BLE.setConnectionInterval(6, 6); // Force lowest interval (7.5ms)

    BLE.advertise();
}

// Function Definitions
void writeRegister(uint8_t reg, uint8_t value) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(reg & 0x7F);
    SPI.transfer(value);
    digitalWrite(CS_PIN, HIGH);
}

uint8_t readRegister(uint8_t reg) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(reg | 0x80); // Read mode
    uint8_t value = SPI.transfer(0x00);
    digitalWrite(CS_PIN, HIGH);
    return value;
}

void readIMUData(uint8_t *buffer) {
    digitalWrite(CS_PIN, LOW);
    SPI.transfer(ACCEL_X_H | 0x80); // Start burst read

    for (int i = 0; i < 12; i++) {
        buffer[i] = SPI.transfer(0x00);
    }

    digitalWrite(CS_PIN, HIGH);
}

void loop() {
    BLEDevice central = BLE.central();

    if (central) {
        while (central.connected()) {
            if (bufferIndex < 240) {
                readIMUData(&imuBuffer[bufferIndex]); // Read 12 bytes
                bufferIndex += 12;
            }
            if (bufferIndex >= 240) {
                imuCharacteristic.writeValue(imuBuffer, 240);
                bufferIndex = 0;
            }
        }
    }
}
