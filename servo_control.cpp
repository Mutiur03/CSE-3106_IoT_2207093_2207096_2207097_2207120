#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <math.h>
#include "config.h"
#include "servo_control.h"

static Adafruit_PWMServoDriver pwm(PCA9685_ADDR);

static const Joints HOME = { 0, 45, 0, 0 };  // default rest pose (deg, math frame)
static Joints cur = HOME;                    // startup pose estimate
static Joints tgt = HOME;
static uint32_t lastMs = 0;

// ---- helpers --------------------------------------------------------------

// positional joint: math angle -> pulse, honoring dir + offset + limits
static void writePositional(uint8_t ch, float mathDeg, int dir, float offset) {
  float servoDeg = offset + dir * mathDeg;
  servoDeg = constrain(servoDeg, 0.0f, 180.0f);
  float us = PULSE_MIN_US + (PULSE_MAX_US - PULSE_MIN_US) * (servoDeg / 180.0f);
  pwm.writeMicroseconds(ch, (uint16_t)us);
}

// continuous joint: spin, stop, or hold
static void writeContinuous(uint8_t ch, int spinDir) {
  // spinDir: -1, 0 (stop), +1
  float us = CONT_STOP_US + spinDir * (float)CONT_SPEED_US;
  pwm.writeMicroseconds(ch, (uint16_t)us);
}

// step a value toward target by at most `maxStep`; returns true if reached
static bool stepTo(float& v, float target, float maxStep) {
  float d = target - v;
  if (fabsf(d) <= maxStep) { v = target; return true; }
  v += (d > 0 ? maxStep : -maxStep);
  return false;
}

// ---- API ------------------------------------------------------------------

void servoBegin() {
  Wire.begin();                         // ESP32 default SDA=21 SCL=22
  pwm.begin();
  pwm.setPWMFreq(SERVO_FREQ_HZ);
  delay(10);
  // park positional joints; stop continuous joints
  writePositional(CH_J2_SHOULDER, cur.j2, J2_DIR, J2_OFFSET_DEG);
  writePositional(CH_J3_ELBOW,    cur.j3, J3_DIR, J3_OFFSET_DEG);
  writePositional(CH_J4_WRIST,    cur.j4, J4_DIR, J4_OFFSET_DEG);
  writeContinuous(CH_J1_BASE, 0);
  lastMs = millis();
}

void servoSetTarget(const Joints& t) {
  tgt.j1 = constrain(t.j1, J1_MIN, J1_MAX);
  tgt.j2 = constrain(t.j2, J2_MIN, J2_MAX);
  tgt.j3 = constrain(t.j3, J3_MIN, J3_MAX);
  tgt.j4 = constrain(t.j4, J4_MIN, J4_MAX);
}

void servoJog(int joint, float d) {
  Joints t = tgt;
  switch (joint) { case 1: t.j1+=d; break; case 2: t.j2+=d; break;
                   case 3: t.j3+=d; break; case 4: t.j4+=d; break; }
  servoSetTarget(t);
}

void servoSetHome(const Joints& ref) { cur = ref; tgt = ref; }
void servoGoHome() { servoSetTarget(HOME); }   // ramp current -> HOME via update loop

Joints servoCurrent() { return cur; }

bool servoBusy() {
  const float tol = 0.5f;
  return fabsf(cur.j1-tgt.j1)>tol || fabsf(cur.j2-tgt.j2)>tol ||
         fabsf(cur.j3-tgt.j3)>tol || fabsf(cur.j4-tgt.j4)>tol;
}

void servoUpdate() {
  uint32_t now = millis();
  if (now - lastMs < (uint32_t)MOVE_STEP_MS) return;
  float dt = (now - lastMs) / 1000.0f;      // seconds since last update
  lastMs = now;

  // --- positional joints: smooth step, write angle ---
  stepTo(cur.j2, tgt.j2, MOVE_STEP_DEG);
  writePositional(CH_J2_SHOULDER, cur.j2, J2_DIR, J2_OFFSET_DEG);
  stepTo(cur.j3, tgt.j3, MOVE_STEP_DEG);
  writePositional(CH_J3_ELBOW, cur.j3, J3_DIR, J3_OFFSET_DEG);
  stepTo(cur.j4, tgt.j4, MOVE_STEP_DEG);
  writePositional(CH_J4_WRIST, cur.j4, J4_DIR, J4_OFFSET_DEG);

  // --- continuous joint: J1 base only ---
  const float tol = 1.0f;
  float e1 = tgt.j1 - cur.j1;
  if (fabsf(e1) > tol) {
    int dir = (e1 > 0 ? 1 : -1) * J1_DIR;
    Serial.printf("J1: err=%.1f dir=%d\n", e1, dir);
    writeContinuous(CH_J1_BASE, dir);
    cur.j1 += (e1 > 0 ? 1 : -1) * J1_DEG_PER_SEC * dt;
    if ((e1 > 0) != (tgt.j1 - cur.j1 > 0)) cur.j1 = tgt.j1;
  } else { writeContinuous(CH_J1_BASE, 0); cur.j1 = tgt.j1; }
}
