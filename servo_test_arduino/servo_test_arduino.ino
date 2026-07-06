// Test 180 positional servo on Arduino (Uno/Nano/Mega).
// Uses built-in Servo.h — no extra libraries needed.
// Wire: servo signal -> pin 9, servo GND -> GND, servo V+ -> external 5V.

#include <Servo.h>

Servo myServo;
const int PIN = 9;

void setup() {
  Serial.begin(9600);
  myServo.attach(PIN, 500, 2500);
  Serial.println("180 servo test");
  myServo.write(0);
  delay(1000);
}

void loop() {
  // int angles[] = {0, 45, 90, 135, 180, 135, 90, 45};
  // for (int i = 0; i < 8; i++) {
    Serial.print("-> ");
  //   Serial.print(angles[i]);
  //   Serial.println(" deg");
  //   myServo.write(angles[i]);
    delay(1500);
  // }

}
