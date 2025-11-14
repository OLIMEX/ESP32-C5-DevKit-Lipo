/*
  Demo for Olimex MOD-L3GD20 (L3GD20 3-axis gyroscope)
  Board: Olimex ESP32-C5-DevKit-Lipo

  I2C pins (UEXT):
    - SDA -> GPIO2
    - SCL -> GPIO3
  
  Requirements:
    - Arduino IDE with ESP32 board support installed
    - Board selected: "ESP32C5 Dev Module" 

  The demo:
    - Initializes L3GD20 over I2C
    - Configures output data rate and scale
    - Reads gyroscope X/Y/Z angular rate continuously
    - Prints data over Serial
*/

#include <Wire.h>

// L3GD20 default I2C address (SDO = HIGH)
#define L3GD20_ADDR 0x6A

// L3GD20 Registers
#define WHO_AM_I       0x0F
#define CTRL_REG1      0x20
#define CTRL_REG4      0x23
#define OUT_X_L        0x28

void writeReg(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(L3GD20_ADDR);
  Wire.write(reg);
  Wire.write(value);
  Wire.endTransmission();
}

uint8_t readReg(uint8_t reg) {
  Wire.beginTransmission(L3GD20_ADDR);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom(L3GD20_ADDR, (uint8_t)1);
  return Wire.read();
}

void readGyro(int16_t& x, int16_t& y, int16_t& z) {
  // Auto-increment by setting MSB of register address
  Wire.beginTransmission(L3GD20_ADDR);
  Wire.write(OUT_X_L | 0x80);
  Wire.endTransmission(false);

  Wire.requestFrom(L3GD20_ADDR, (uint8_t)6);

  uint8_t xl = Wire.read();
  uint8_t xh = Wire.read();
  uint8_t yl = Wire.read();
  uint8_t yh = Wire.read();
  uint8_t zl = Wire.read();
  uint8_t zh = Wire.read();

  x = (int16_t)(xh << 8 | xl);
  y = (int16_t)(yh << 8 | yl);
  z = (int16_t)(zh << 8 | zl);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Initializing L3GD20...");

  // Start I2C on Olimex ESP32-C5 pins SDA=2, SCL=3
  Wire.begin(2, 3);

  // WHO_AM_I check (should return 0xD4 or 0xD7)
  uint8_t whoami = readReg(WHO_AM_I);
  Serial.print("WHO_AM_I = 0x");
  Serial.println(whoami, HEX);

  if (whoami != 0xD4 && whoami != 0xD7) {
    Serial.println("ERROR: L3GD20 not detected!");
    while (true) delay(1000);
  }

  // CTRL_REG1:
  // DR=95Hz, BW=25, Power on, XYZ enabled
  writeReg(CTRL_REG1, 0b00001111);

  // CTRL_REG4:
  // Full scale = 250 dps
  writeReg(CTRL_REG4, 0b00000000);

  Serial.println("L3GD20 initialized.\n");
}

void loop() {
  int16_t gx, gy, gz;
  readGyro(gx, gy, gz);

  // Convert raw to degrees/sec using ±250 dps = 8.75 mdps/LSB
  float x_dps = gx * 0.00875;
  float y_dps = gy * 0.00875;
  float z_dps = gz * 0.00875;

  Serial.print("Gyro (dps): ");
  Serial.print("X=");
  Serial.print(x_dps, 2);
  Serial.print("  Y=");
  Serial.print(y_dps, 2);
  Serial.print("  Z=");
  Serial.println(z_dps, 2);

  delay(200);
}
