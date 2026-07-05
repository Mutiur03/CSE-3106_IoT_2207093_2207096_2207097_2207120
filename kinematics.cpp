#include <Arduino.h>
#include <math.h>
#include "config.h"
#include "kinematics.h"

static inline float deg(float r){ return r * 57.2957795f; }
static inline float rad(float d){ return d * 0.01745329252f; }

// ---------------------------------------------------------------------------
//  Inverse kinematics
//
//  1) J1 (yaw) aims the arm plane at the target:      j1 = atan2(y, x)
//  2) Work in that vertical plane using radial r and height z.
//  3) Back off the tool link L3 along the requested tool pitch to find the
//     WRIST pivot, then solve a standard planar 2-link (shoulder+elbow) to it.
//  4) J4 (wrist) sets the tool pitch relative to the forearm.
// ---------------------------------------------------------------------------
bool ikSolve(const Pose& p, Joints& out) {

  // --- 1) base yaw ---
  float j1 = deg(atan2f(p.y, p.x));

  // radial distance of target from the base (vertical) axis
  float r = sqrtf(p.x * p.x + p.y * p.y);
  float z = p.z;

  // --- 3) subtract tool link to get the wrist pivot in the (r,z) plane ---
  float pr = rad(p.pitch);
  float wr = r - L3_TOOL * cosf(pr);       // wrist radial
  float wz = z - L3_TOOL * sinf(pr);       // wrist height

  // vector from shoulder pivot (0, L0) to wrist pivot
  float dr = wr;                           // shoulder sits on the yaw axis
  float dz = wz - L0_BASE_H;
  float D2 = dr * dr + dz * dz;
  float D  = sqrtf(D2);

  // reachability
  if (D > (L1_UPPER + L2_FORE) || D < fabsf(L1_UPPER - L2_FORE)) return false;

  // --- 2) planar 2-link (law of cosines) ---
  float c3 = (D2 - L1_UPPER * L1_UPPER - L2_FORE * L2_FORE)
             / (2.0f * L1_UPPER * L2_FORE);
  c3 = constrain(c3, -1.0f, 1.0f);
  float s3 = sqrtf(1.0f - c3 * c3);
  if (ELBOW_UP) s3 = -s3;                  // pick elbow-up branch

  float j3 = deg(atan2f(s3, c3));          // elbow angle relative to upper link
  float j2 = deg(atan2f(dz, dr)
                 - atan2f(L2_FORE * s3, L1_UPPER + L2_FORE * c3)); // shoulder

  // --- 4) wrist keeps the requested tool pitch ---
  float j4 = p.pitch - (j2 + deg(atan2f(s3, c3)));

  // --- limit check ---
  if (j1 < J1_MIN || j1 > J1_MAX) return false;
  if (j2 < J2_MIN || j2 > J2_MAX) return false;
  if (j3 < J3_MIN || j3 > J3_MAX) return false;
  if (j4 < J4_MIN || j4 > J4_MAX) return false;

  out.j1 = j1; out.j2 = j2; out.j3 = j3; out.j4 = j4;
  return true;
}

// ---------------------------------------------------------------------------
//  Forward kinematics - joint angles back to tool-tip pose (for UI display).
// ---------------------------------------------------------------------------
Pose fkSolve(const Joints& j) {
  float a2 = rad(j.j2);
  float a23 = rad(j.j2 + j.j3);
  float toolPitch = j.j2 + j.j3 + j.j4;    // deg
  float ap = rad(toolPitch);

  // radial & height in the arm plane
  float r = L1_UPPER * cosf(a2) + L2_FORE * cosf(a23) + L3_TOOL * cosf(ap);
  float z = L0_BASE_H + L1_UPPER * sinf(a2) + L2_FORE * sinf(a23) + L3_TOOL * sinf(ap);

  float a1 = rad(j.j1);
  Pose p;
  p.x = r * cosf(a1);
  p.y = r * sinf(a1);
  p.z = z;
  p.pitch = toolPitch;
  return p;
}
