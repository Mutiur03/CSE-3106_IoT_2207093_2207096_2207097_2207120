// Test 180 positional servo via PCA9685 (same driver board you have).
// No ESP32Servo or LEDC needed — uses Adafruit PWM library only.
// Wiring: ESP32 SDA(21)->PCA SDA, SCL(22)->PCA SCL, 3V3->VCC, GND->GND
// Servo on PCA9685 channel 1 (J2 shoulder slot). External 5V on V+.

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm(0x40);

const int CH = 1;          // test on channel 1 (change to 0-3 to test others)
const int US_MIN = 500;
const int US_MAX = 2500;

void setAngle(int ch, int deg) {
  if (deg < 0) deg = 0;
  if (deg > 180) deg = 180;
  float us = US_MIN + (US_MAX - US_MIN) * (deg / 180.0);
  pwm.writeMicroseconds(ch, (int)us);
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin();
  Serial.println("I2C scan...");
  for (int addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print("  found 0x");
      Serial.println(addr, HEX);
    }
  }

  pwm.begin();
  pwm.setPWMFreq(50);
  delay(10);

  Serial.println("180 servo test via PCA9685");
  setAngle(CH, 90);
  delay(1000);
}

void loop() {
  int angles[] = {0, 45, 90, 135, 180, 135, 90, 45};
  for (int i = 0; i < 8; i++) {
    Serial.print("-> ");
    Serial.print(angles[i]);
    Serial.println(" deg");
    setAngle(CH, angles[i]);
    delay(1500);
  }
}
