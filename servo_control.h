#pragma once

struct Joints { float j1, j2, j3, j4, j5, j6; };  // joint angles (deg, math frame)

void   servoBegin();                        // init PCA9685, park at startup pose
void   servoSetTarget(const Joints& tgt);   // command a new joint target
void   servoUpdate();                       // step motion; call from loop()
bool   servoBusy();                         // true while a move is in progress
Joints servoCurrent();                      // current (estimated) joint angles
void   servoJog(int joint, float deltaDeg); // nudge one joint (manual)
void   servoSetHome(const Joints& ref);     // redefine current pose (open-loop reset)
void   servoGoHome();                       // ramp to default home pose
