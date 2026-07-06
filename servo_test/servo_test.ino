// Test all 4 servos at once via PCA9685.
// Positional (J2, J4): sweep 0 → 90 → 180 → 90
// Continuous (J1, J3): spin CW → stop → spin CCW → stop
// Watch Serial Monitor @115200 for I2C scan + status.

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm(0x40);

// PCA9685 channels
#define CH_J1  0   // base      - MG996 360 continuous
#define CH_J2  1   // shoulder  - MG996 180 positional
#define CH_J3  2   // elbow     - MG996 360 continuous
#define CH_J4  3   // wrist     - SG90  180 positional

#define PULSE_MIN  500
#define PULSE_MAX  2500

void i2cScan() {
  Serial.println("I2C scan...");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  found 0x%02X %s\n", addr, addr==0x40 ? "<-- PCA9685" : "");
      found++;
    }
  }
  if (!found) Serial.println("  NO devices! Check SDA(21) SCL(22) wiring + PCA9685 VCC.");
}

void setAngle(uint8_t ch, float deg) {
  deg = constrain(deg, 0, 180);
  float us = PULSE_MIN + (PULSE_MAX - PULSE_MIN) * (deg / 180.0f);
  pwm.writeMicroseconds(ch, (uint16_t)us);
}

void setContinuous(uint8_t ch, int dir) {
  // dir: -1 = reverse, 0 = stop, +1 = forward
  float us = 1500 + dir * 200;
  pwm.writeMicroseconds(ch, (uint16_t)us);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n=== SERVO TEST (all 4 via PCA9685) ===\n");

  Wire.begin();
  i2cScan();

  pwm.begin();
  pwm.setPWMFreq(50);
  delay(10);

  // stop continuous servos
  setContinuous(CH_J1, 0);
  setContinuous(CH_J3, 0);
  // center positional servos
  setAngle(CH_J2, 90);
  setAngle(CH_J4, 90);

  Serial.println("\nAll servos initialized. Starting test loop...\n");
  delay(1000);
}

void loop() {
  // --- Positional: J2 + J4 sweep ---
  Serial.println("[J2+J4] -> 45 deg");
  setAngle(CH_J2, 45);
  setAngle(CH_J4, 45);
  delay(1500);

  Serial.println("[J2+J4] -> 90 deg");
  setAngle(CH_J2, 90);
  setAngle(CH_J4, 90);
  delay(1500);

  Serial.println("[J2+J4] -> 135 deg");
  setAngle(CH_J2, 135);
  setAngle(CH_J4, 135);
  delay(1500);

  Serial.println("[J2+J4] -> 90 deg (center)");
  setAngle(CH_J2, 90);
  setAngle(CH_J4, 90);
  delay(1500);

  // --- Continuous: J1 + J3 spin ---
  Serial.println("[J1+J3] -> spin CW");
  setContinuous(CH_J1, +1);
  setContinuous(CH_J3, +1);
  delay(2000);

  Serial.println("[J1+J3] -> STOP");
  setContinuous(CH_J1, 0);
  setContinuous(CH_J3, 0);
  delay(1500);

  Serial.println("[J1+J3] -> spin CCW");
  setContinuous(CH_J1, -1);
  setContinuous(CH_J3, -1);
  delay(2000);

  Serial.println("[J1+J3] -> STOP");
  setContinuous(CH_J1, 0);
  setContinuous(CH_J3, 0);
  delay(2000);

  Serial.println("--- cycle done, repeating ---\n");
}
