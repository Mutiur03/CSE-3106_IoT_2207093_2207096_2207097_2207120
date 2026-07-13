#pragma once

#define USE_WIFI_AP 1 // 1 = Access Point, 0 = join home WiFi

#define AP_SSID "RoboArm"
#define AP_PASS "12345678" 

#define STA_SSID "Sanda"
#define STA_PASS "Sandar_tel03"

#define PCA9685_ADDR 0x40
#define SERVO_FREQ_HZ 50 

#define CH_J1_BASE 0     // MG996  360 continuous  (yaw)
#define CH_J2_SHOULDER 1 // MG996  180 positional  (pitch)
#define CH_J3_ELBOW 2    // MG996  360 continuous  (pitch)
#define CH_J4_WRIST 3    // SG90   180 positional  (pitch)
#define CH_J5_GRIP  4    // SG90   180 positional  (grip/roll)
#define CH_J6_AUX   5    // SG90   180 positional  (auxiliary)

#define PULSE_MIN_US 500
#define PULSE_MAX_US 2500

#define CONT_STOP_US 1500
#define CONT_SPEED_US 150 

#define J1_DEG_PER_SEC 45.0f
#define J3_DEG_PER_SEC 45.0f

#define J2_DIR (+1)
#define J2_OFFSET_DEG 90.0f 
#define J4_DIR (+1)
#define J4_OFFSET_DEG 90.0f

#define J5_DIR (+1)
#define J5_OFFSET_DEG 90.0f
#define J6_DIR (+1)
#define J6_OFFSET_DEG 90.0f

#define J3_DIR (+1)
#define J3_OFFSET_DEG 90.0f 

#define J1_DIR (+1)

#define J1_MIN (-180.0f)
#define J1_MAX ( 180.0f)
#define J2_MIN ( -30.0f)
#define J2_MAX ( 150.0f)
#define J3_MIN ( -90.0f)
#define J3_MAX (  90.0f)
#define J4_MIN ( -90.0f)
#define J4_MAX (  90.0f)
#define J5_MIN ( -90.0f)
#define J5_MAX (  90.0f)
#define J6_MIN ( -90.0f)
#define J6_MAX (  90.0f)

#define MOVE_STEP_DEG 1.0f
#define MOVE_STEP_MS  15
