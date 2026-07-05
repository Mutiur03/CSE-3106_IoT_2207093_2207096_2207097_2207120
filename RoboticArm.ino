// ============================================================================
//  RoboticArm.ino  -  ESP32 4-DOF arm controller.
//  ESP32 hosts a web page + WebSocket; solves IK on-board; drives servos
//  through a PCA9685. See config.h for all calibration.
//
//  Libraries (install via Arduino Library Manager):
//    - ESPAsyncWebServer   (me-no-dev)     + AsyncTCP (me-no-dev)
//    - ArduinoJson         (Benoit Blanchon)
//    - Adafruit PWM Servo Driver Library   (Adafruit)
//  Board: any ESP32 dev module.
// ============================================================================
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "kinematics.h"
#include "servo_control.h"
#include "webpage.h"

AsyncWebServer server(80);
AsyncWebSocket  ws("/ws");

static uint32_t lastBroadcast = 0;

// Build a JSON state string and (optionally) an operation message.
static String stateJson(const char* msg = nullptr, bool ok = true) {
  StaticJsonDocument<256> d;
  Joints c = servoCurrent();
  Pose   p = fkSolve(c);
  JsonArray ja = d.createNestedArray("j");
  ja.add(c.j1); ja.add(c.j2); ja.add(c.j3); ja.add(c.j4);
  JsonArray pa = d.createNestedArray("p");
  pa.add(p.x); pa.add(p.y); pa.add(p.z); pa.add(p.pitch);
  if (msg) { d["msg"] = msg; d["ok"] = ok; }
  String s; serializeJson(d, s); return s;
}

static void broadcast(const char* msg = nullptr, bool ok = true) {
  ws.textAll(stateJson(msg, ok));
}

// ---- WebSocket command handling ------------------------------------------
static void handleCommand(AsyncWebSocketClient* client, const String& body) {
  StaticJsonDocument<192> d;
  if (deserializeJson(d, body)) return;         // ignore malformed
  const char* cmd = d["cmd"] | "";

  if (!strcmp(cmd, "ik")) {
    Pose target { d["x"] | 0.0f, d["y"] | 0.0f, d["z"] | 0.0f, d["pitch"] | 0.0f };
    Joints sol;
    if (ikSolve(target, sol)) { servoSetTarget(sol); broadcast("moving to target"); }
    else                       broadcast("target unreachable", false);

  } else if (!strcmp(cmd, "jog")) {
    servoJog(d["joint"] | 0, d["delta"] | 0.0f);
    broadcast("jog");

  } else if (!strcmp(cmd, "stop")) {
    servoSetHome(servoCurrent());               // freeze at current estimate
    broadcast("stopped");

  } else if (!strcmp(cmd, "sethome")) {
    Joints home { 0, 45, -90, 0 };              // matches startup estimate
    servoSetHome(home);
    broadcast("home set");
  }
}

static void onWsEvent(AsyncWebSocket* s, AsyncWebSocketClient* c, AwsEventType type,
                      void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    c->text(stateJson());                       // push current state on connect
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      String body; body.reserve(len);
      for (size_t i = 0; i < len; i++) body += (char)data[i];
      handleCommand(c, body);
    }
  }
}

void setup() {
  Serial.begin(115200);
  servoBegin();

#if USE_WIFI_AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, strlen(AP_PASS) ? AP_PASS : nullptr);
  Serial.printf("AP \"%s\"  ->  http://%s/\n", AP_SSID, WiFi.softAPIP().toString().c_str());
#else
  WiFi.mode(WIFI_STA);
  WiFi.begin(STA_SSID, STA_PASS);
  Serial.print("joining WiFi");
  while (WiFi.status() != WL_CONNECTED) { delay(400); Serial.print('.'); }
  Serial.printf("\nconnected  ->  http://%s/\n", WiFi.localIP().toString().c_str());
#endif

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* r) {
    r->send_P(200, "text/html", INDEX_HTML);
  });
  server.begin();
}

void loop() {
  servoUpdate();                                // non-blocking motion stepper
  ws.cleanupClients();

  if (millis() - lastBroadcast > 150) {         // ~7 Hz telemetry to UI
    lastBroadcast = millis();
    if (ws.count()) broadcast();
  }
}
