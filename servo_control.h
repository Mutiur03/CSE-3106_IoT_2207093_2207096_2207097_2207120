// ============================================================================
//  servo_control.h  -  Drives 4 joints through a PCA9685.
//  Positional joints (J2,J3,J4): pulse set directly from angle.
//  Continuous joint (J1): OPEN-LOOP timed spin (no feedback) - angle is
//  ESTIMATED by integrating spin time. Re-home by hand when drift builds up.
//  All motion is non-blocking: call servoUpdate() every loop().
// ============================================================================
#pragma once

struct Joints { float j1, j2, j3, j4; };    // joint angles (deg, math frame)

void   servoBegin();                        // init PCA9685, park at startup pose
void   servoSetTarget(const Joints& tgt);   // command a new joint target
void   servoUpdate();                       // step motion; call from loop()
bool   servoBusy();                         // true while a move is in progress
Joints servoCurrent();                      // current (estimated) joint angles
void   servoJog(int joint, float deltaDeg); // nudge one joint (manual)
void   servoSetHome(const Joints& ref);     // redefine current pose (open-loop reset)
void   servoGoHome();                       // ramp to default home pose
