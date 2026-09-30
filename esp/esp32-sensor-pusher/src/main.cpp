#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

#ifndef API_BASE_URL
#define API_BASE_URL "https://your-tunnel.example.com"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID "Wokwi-GUEST"
#endif

#ifndef WIFI_PASSWORD
#define WIFI_PASSWORD ""
#endif

#ifndef WIFI_CHANNEL
#define WIFI_CHANNEL 6
#endif

static const int LED_PIN = 2;
static const int SENSOR_ID = 1;
static const unsigned long POST_INTERVAL_MS = 5000;
static const int MAX_SENSORS = 16;

static unsigned long lastPostAt = 0;
static int activeSensorId = SENSOR_ID;
static float currentValue = 20.0f;

static void blink(int times) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(60);
    digitalWrite(LED_PIN, LOW);
    delay(60);
  }
}

static void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  Serial.printf("[wifi] connecting to %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL);

  unsigned long startedAt = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startedAt < 20000UL) {
    blink(1);
    delay(200);
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[wifi] connected, ip %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[wifi] connection failed");
  }
}

static void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  unsigned long startedAt = millis();
  while (time(nullptr) < 1600000000 && millis() - startedAt < 5000UL) {
    delay(250);
  }
  Serial.println(time(nullptr) >= 1600000000 ? "[time] synced" : "[time] not synced");
}

static void isoTimestamp(char *buffer, size_t size) {
  time_t now = time(nullptr);
  if (now < 1600000000) {
    snprintf(buffer, size, "1970-01-01T00:00:00Z");
    return;
  }
  strftime(buffer, size, "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
}

static int fetchSensors(int *ids, int max) {
  if (WiFi.status() != WL_CONNECTED) {
    return 0;
  }

  HTTPClient http;
  http.setTimeout(10000);
  http.begin(String(API_BASE_URL) + "/sensors");
  int code = http.GET();
  if (code != 200) {
    Serial.printf("[GET /sensors] unexpected status %d\n", code);
    http.end();
    return 0;
  }

  JsonDocument doc;
  if (deserializeJson(doc, http.getStream()) != DeserializationError::Ok) {
    Serial.println("[GET /sensors] invalid json");
    http.end();
    return 0;
  }

  int count = 0;
  for (JsonObject sensor : doc["sensors"].as<JsonArray>()) {
    int id = sensor["id"].as<int>();
    if (id > 0 && count < max) {
      ids[count++] = id;
    }
  }
  Serial.printf("[GET /sensors] found %d sensor(s)\n", count);
  http.end();
  return count;
}

static float randomReading() {
  currentValue += (random(0, 2401) - 1200) / 100.0f;
  currentValue = constrain(currentValue, -20.0f, 80.0f);
  return currentValue;
}

static void postReading() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
    if (WiFi.status() != WL_CONNECTED) {
      return;
    }
  }

  char timestamp[24];
  isoTimestamp(timestamp, sizeof(timestamp));

  char payload[160];
  snprintf(payload, sizeof(payload),
           "{\"sensor-id\":%d,\"value\":%.2f,\"timestamp\":\"%s\"}",
           activeSensorId, randomReading(), timestamp);

  String url = String(API_BASE_URL) + "/sensors/" + String(activeSensorId) + "/readings";

  HTTPClient http;
  http.setTimeout(10000);
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST((uint8_t *)payload, strlen(payload));

  Serial.printf("[POST] %s\n         %s\n         -> %d %s\n",
                url.c_str(), payload, code, http.getString().c_str());
  http.end();
  blink(1);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n[boot] esp32 sensor pusher");

  connectWifi();
  syncTime();

  int ids[MAX_SENSORS];
  int count = fetchSensors(ids, MAX_SENSORS);
  if (count > 0) {
    activeSensorId = ids[random(0, count)];
  }
  Serial.printf("[boot] posting as sensor %d every %lu ms\n",
                activeSensorId, POST_INTERVAL_MS);
}

void loop() {
  if (millis() - lastPostAt < POST_INTERVAL_MS) {
    delay(100);
    return;
  }
  lastPostAt = millis();
  postReading();
}
