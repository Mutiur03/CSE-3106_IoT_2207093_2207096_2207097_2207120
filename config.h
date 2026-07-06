// ============================================================================
//  config.h  -  All tunable constants for the robotic arm.
//  Edit these to calibrate.  Nothing else in the firmware hard-codes geometry.
// ============================================================================
#pragma once

// ---------------------------------------------------------------------------
//  WiFi
//  MODE_AP  : ESP32 makes its own network. Connect phone/laptop to it, then
//             open http://192.168.4.1/  (no router needed).
//  MODE_STA : ESP32 joins your home WiFi. Set SSID/PASS below. Find the IP
//             it prints on the serial monitor.
// ---------------------------------------------------------------------------
#define USE_WIFI_AP         1          // 1 = Access Point, 0 = join home WiFi

#define AP_SSID             "RoboArm"
#define AP_PASS             "12345678" // >=8 chars, or "" for open network

#define STA_SSID            "YOUR_WIFI"
#define STA_PASS            "YOUR_PASS"

// ---------------------------------------------------------------------------
//  PCA9685 servo driver (I2C).  ESP32 default I2C pins: SDA=21, SCL=22.
// ---------------------------------------------------------------------------
#define PCA9685_ADDR        0x40
#define SERVO_FREQ_HZ       50         // 50 Hz standard for analog servos

// PCA9685 channel each joint is wired to
#define CH_J1_BASE          0          // MG996  360 continuous  (yaw)
#define CH_J2_SHOULDER      1          // MG996  180 positional  (pitch)
#define CH_J3_ELBOW         2          // MG996  360 continuous  (pitch)
#define CH_J4_WRIST         3          // SG90   180 positional  (pitch)

// ---------------------------------------------------------------------------
//  Servo pulse limits (in microseconds).  Analog servos: ~500..2500us.
//  Measure/trim per servo if the horn does not hit full travel.
// ---------------------------------------------------------------------------
#define PULSE_MIN_US        500
#define PULSE_MAX_US        2500

// Continuous-rotation servos: 1500us = stop. Offset sets spin speed.
// Larger offset = faster spin = more open-loop error. Keep modest.
#define CONT_STOP_US        1500
#define CONT_SPEED_US       150        // pulse = 1500 +/- this while moving (bigger = faster spin)

// Measured rotation rate of the continuous servos at CONT_SPEED_US, in
// degrees of JOINT travel per second. CALIBRATE: command a 90-deg move,
// time it, adjust. This is the whole accuracy of the open-loop joints.
#define J1_DEG_PER_SEC      45.0f
#define J3_DEG_PER_SEC      45.0f

// ---------------------------------------------------------------------------
//  Positional-servo angle calibration.
//  servo_pulse = map(offset + dir * math_angle_deg  ->  PULSE_MIN..MAX)
//  dir flips if the joint moves the wrong way. offset aligns math-zero to the
//  servo's usable range. Tune with the JOG buttons in the web UI.
// ---------------------------------------------------------------------------
#define J2_DIR              (+1)
#define J2_OFFSET_DEG       90.0f      // math 0deg (horizontal) -> servo 90
#define J4_DIR              (+1)
#define J4_OFFSET_DEG       90.0f

// J3 elbow positional calibration (now 180 deg servo)
#define J3_DIR              (+1)
#define J3_OFFSET_DEG       90.0f      // math 0deg -> servo 90

// J1 continuous direction
#define J1_DIR              (+1)

// ---------------------------------------------------------------------------
//  Link lengths (mm) - pivot to pivot. ESTIMATED from the STL bounding boxes;
//  refine with a ruler. IK adapts automatically.
// ---------------------------------------------------------------------------
#define L0_BASE_H           90.0f      // table  -> shoulder pivot (height)
#define L1_UPPER            140.0f     // shoulder pivot -> elbow pivot
#define L2_FORE             100.0f     // elbow pivot    -> wrist pivot
#define L3_TOOL             45.0f      // wrist pivot    -> tool tip

// ---------------------------------------------------------------------------
//  Joint software limits (degrees, in MATH frame, measured from horizontal
//  for the pitch joints; yaw is about the vertical axis).
// ---------------------------------------------------------------------------
#define J1_MIN  (-180.0f)
#define J1_MAX  ( 180.0f)
#define J2_MIN  (-30.0f)
#define J2_MAX  (150.0f)
#define J3_MIN  (-90.0f)
#define J3_MAX  ( 90.0f)
#define J4_MIN  (-90.0f)
#define J4_MAX  ( 90.0f)

// Elbow configuration: true = elbow-up solution, false = elbow-down.
#define ELBOW_UP            true

// Motion pacing for positional joints: degrees per step and step delay (ms).
// Lower = smoother/slower. This gives coordinated, non-jerky moves.
#define MOVE_STEP_DEG       1.0f
#define MOVE_STEP_MS        15
