// ============================================================================
//  kinematics.h  -  Forward & inverse kinematics for a 4-DOF arm:
//  J1 yaw (base) + J2 shoulder pitch + J3 elbow pitch + J4 wrist pitch.
//  All angles in DEGREES, math frame. Distances in mm. Base at origin, Z up.
// ============================================================================
#pragma once

struct Pose {      // task-space target
  float x, y, z;   // tool-tip position (mm)
  float pitch;     // tool approach pitch from horizontal (deg). -90 = pointing down
};

struct Joints {    // joint-space angles (math frame, deg)
  float j1, j2, j3, j4;
};

// Solve IK. Returns true if target reachable & within limits; fills out.
bool ikSolve(const Pose& p, Joints& out);

// Forward kinematics: joint angles -> tool-tip pose. For UI readback.
Pose fkSolve(const Joints& j);
