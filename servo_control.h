// ============================================================================
//  servo_control.h  -  Drives 4 joints through a PCA9685.
//  Positional joints (J2,J4): pulse set directly from angle.
//  Continuous joints (J1,J3): OPEN-LOOP timed spin (no feedback) - angle is
//  ESTIMATED by integrating spin time. Re-home by hand when drift builds up.
//  All motion is non-blocking: call servoUpdate() every loop().
// ============================================================================
#pragma once
#include "kinematics.h"

void  servoBegin();                 // init PCA9685, park at current estimate
void  servoSetTarget(const Joints& tgt);   // command a new joint target
void  servoUpdate();                // step motion; call from loop()
bool  servoBusy();                  // true while a move is in progress
Joints servoCurrent();              // current (estimated) joint angles
void  servoJog(int joint, float deltaDeg);  // nudge one joint (manual)
void  servoSetHome(const Joints& ref);      // redefine current pose (open-loop reset)
