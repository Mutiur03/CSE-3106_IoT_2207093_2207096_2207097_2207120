# ESP32 Robotic Arm — 4-DOF, web-controlled, on-board IK

Web UI (served by the ESP32) → WebSocket → ESP32 solves inverse kinematics →
PCA9685 → servos.

Joints: **J1 base yaw** (MG996 360° continuous), **J2 shoulder** (MG996 180°),
**J3 elbow** (MG996 360° continuous), **J4 wrist** (SG90). Gripper excluded for now.

## Files
| File | Purpose |
|------|---------|
| `RoboticArm.ino` | WiFi, web server, WebSocket, command routing |
| `config.h` | **all calibration** — pins, link lengths, limits, servo trim |
| `kinematics.cpp/.h` | forward + inverse kinematics |
| `servo_control.cpp/.h` | PCA9685 drive, positional + open-loop timed motion |
| `webpage.h` | the control page (HTML/JS in flash) |

## Wiring
```
ESP32          PCA9685
 3V3  ───────── VCC   (logic power)
 GND  ───────── GND
 21(SDA) ────── SDA
 22(SCL) ────── SCL

PCA9685 V+  ◄── EXTERNAL 5–6V supply (servo power, NOT from ESP32)
PCA9685 GND ─── EXTERNAL supply GND  ── AND ── ESP32 GND   (COMMON GROUND, required)

Servo channel map (PCA9685 outputs):
  0 = J1 base    1 = J2 shoulder    2 = J3 elbow    3 = J4 wrist
```

### ⚠ Power — read this
- MG996 stalls at **~1–2.5 A each**. Four servos can spike **>5 A**. **Never**
  run servo V+ from the ESP32 5V pin — it will brown-out and reset, or die.
- Use a separate **5–6V ≥5A** supply into PCA9685 **V+**.
- **Common ground** between that supply, the PCA9685, and the ESP32 — or servos
  jitter / signals float.
- Add a big cap (**1000µF**) across V+/GND at the PCA9685 to absorb spikes.

## Libraries (Arduino IDE → Library Manager)
- **ESPAsyncWebServer** + **AsyncTCP** (me-no-dev)
- **ArduinoJson** (Benoit Blanchon)
- **Adafruit PWM Servo Driver Library**

Board: ESP32 Dev Module (install "esp32 by Espressif" boards).

## Flash & connect
1. Open `RoboticArm.ino` (keep all files in one folder).
2. Select your ESP32 board + port, Upload.
3. Open Serial Monitor @115200 — it prints the URL.
   - Default = **Access Point** `RoboArm` / pass `12345678`. Connect your
     phone/laptop to that WiFi, open **http://192.168.4.1/**.
   - To join home WiFi instead: set `WIFI_MODE_AP 0` + your SSID/PASS in `config.h`.

## Calibration order (do once)
1. **Positional servos (J2, J4).** Jog to a known pose. If a joint moves the
   wrong way, flip `J2_DIR`/`J4_DIR`. If zero is off, adjust `J2_OFFSET_DEG`/
   `J4_OFFSET_DEG` so math-0° = arm horizontal.
2. **Link lengths.** Measure pivot-to-pivot with a ruler; set `L0/L1/L2/L3`.
   Current values are STL bounding-box estimates.
3. **Continuous servos (J1, J3) speed.** Jog +90°, time how long the joint
   actually takes, then set `J1_DEG_PER_SEC`/`J3_DEG_PER_SEC = 90 / seconds`.
   These are **open-loop** — the angle is guessed from time and **drifts**.
   Physically home the joint and press **Set home** in the UI when it strays.
4. **Verify IK.** Enter a target you can measure (e.g. X=150,Y=0,Z=120,pitch=0),
   press Move, measure the tip. Tune `L*` until it lands.

## Known limits
- J1 & J3 open-loop → no repeatability without homing. Add **AS5600** encoders
  later to close the loop (see chat) if you need real accuracy.
- No collision/self-limit checking beyond per-joint angle clamps.
- Single tool pitch DOF — can't reach every orientation (4-DOF arm).
