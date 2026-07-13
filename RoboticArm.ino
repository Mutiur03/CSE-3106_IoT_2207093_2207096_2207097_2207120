#include <Wire.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "servo_control.h"
#include "webpage.h"

AsyncWebServer server(80);
AsyncWebSocket  ws("/ws");

static uint32_t lastBroadcast = 0;

static String stateJson(const char* msg = nullptr, bool ok = true) {
  StaticJsonDocument<256> d;
  Joints c = servoCurrent();
  JsonArray ja = d.createNestedArray("j");
  ja.add(c.j1); ja.add(c.j2); ja.add(c.j3); ja.add(c.j4); ja.add(c.j5); ja.add(c.j6);
  if (msg) { d["msg"] = msg; d["ok"] = ok; }
  String s; serializeJson(d, s); return s;
}

static void broadcast(const char* msg = nullptr, bool ok = true) {
  ws.textAll(stateJson(msg, ok));
}

static void handleCommand(AsyncWebSocketClient* client, const String& body) {
  StaticJsonDocument<128> d;
  if (deserializeJson(d, body)) return;
  const char* cmd = d["cmd"] | "";

  if (!strcmp(cmd, "jog")) {
    servoJog(d["joint"] | 0, d["delta"] | 0.0f);
    broadcast("jog");
  } else if (!strcmp(cmd, "stop")) {
    servoSetHome(servoCurrent());
    broadcast("stopped");
  } else if (!strcmp(cmd, "sethome")) {
    servoGoHome();
    broadcast("moving to home");
  }
}

static bool applyTargetFromJson(JsonDocument& d, String& err) {
  Joints t = servoCurrent();

  if (d["j"].is<JsonArray>()) {
    JsonArray a = d["j"].as<JsonArray>();
    if (a.size() != 6) { err = "j array must have 6 elements"; return false; }
    t.j1 = a[0]; t.j2 = a[1]; t.j3 = a[2]; t.j4 = a[3]; t.j5 = a[4]; t.j6 = a[5];
  } else {
    t.j1 = d["j1"] | t.j1;  t.j2 = d["j2"] | t.j2;  t.j3 = d["j3"] | t.j3;
    t.j4 = d["j4"] | t.j4;  t.j5 = d["j5"] | t.j5;  t.j6 = d["j6"] | t.j6;
  }

  t.j1 = constrain(t.j1, J1_MIN, J1_MAX);
  t.j2 = constrain(t.j2, J2_MIN, J2_MAX);
  t.j3 = constrain(t.j3, J3_MIN, J3_MAX);
  t.j4 = constrain(t.j4, J4_MIN, J4_MAX);
  t.j5 = constrain(t.j5, J5_MIN, J5_MAX);
  t.j6 = constrain(t.j6, J6_MIN, J6_MAX);

  servoSetTarget(t);
  return true;
}

static void onApiMoveBody(AsyncWebServerRequest* req, uint8_t* data, size_t len,
                          size_t index, size_t total) {
  if (index == 0) {
    req->_tempObject = new String();
    ((String*)req->_tempObject)->reserve(total);
  }
  String* buf = (String*)req->_tempObject;
  for (size_t i = 0; i < len; i++) *buf += (char)data[i];

  if (index + len != total) return;

  StaticJsonDocument<256> d;
  DeserializationError jerr = deserializeJson(d, *buf);
  delete buf; req->_tempObject = nullptr;

  if (jerr) { req->send(400, "application/json", "{\"ok\":false,\"msg\":\"bad json\"}"); return; }

  String err;
  if (!applyTargetFromJson(d, err)) {
    req->send(400, "application/json", String("{\"ok\":false,\"msg\":\"") + err + "\"}");
    return;
  }
  broadcast("api move");
  req->send(200, "application/json", stateJson("moving", true));
}

static void onWsEvent(AsyncWebSocket* s, AsyncWebSocketClient* c, AwsEventType type,
                      void* arg, uint8_t* data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    c->text(stateJson());
  } else if (type == WS_EVT_DATA) {
    AwsFrameInfo* info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      String body; body.reserve(len);
      for (size_t i = 0; i < len; i++) body += (char)data[i];
      handleCommand(c, body);
    }
  }
}

static void i2cScan() {
  Wire.begin();
  Serial.println("I2C scan...");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  found device at 0x%02X\n", addr);
      found++;
    }
  }
  if (!found) Serial.println("  NO I2C devices found! Check SDA(21)/SCL(22) wiring.");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  i2cScan();
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

  server.on("/api/state", HTTP_GET, [](AsyncWebServerRequest* r) {
    r->send(200, "application/json", stateJson());
  });
  server.on("/api/move", HTTP_POST,
            [](AsyncWebServerRequest* r) {},
            nullptr, onApiMoveBody);
  server.on("/api/home", HTTP_POST, [](AsyncWebServerRequest* r) {
    servoGoHome();
    broadcast("moving to home");
    r->send(200, "application/json", stateJson("moving to home", true));
  });

  server.begin();
}

void loop() {
  servoUpdate();
  ws.cleanupClients();
  if (millis() - lastBroadcast > 150) {
    lastBroadcast = millis();
    if (ws.count()) broadcast();
  }
}
