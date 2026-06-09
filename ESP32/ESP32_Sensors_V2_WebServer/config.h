#pragma once

// =====================
// WiFi Credentials
// =====================
static const char* WIFI_SSID     = "BELL481";
static const char* WIFI_PASSWORD = "7AEF5D494AF9";

static const char* SKETCH_NAME = "ESP32_Sensors_V2_WebServer.ino";

// =====================
// ESP32 Pins
// =====================
static const int I2C_SDA_PIN  = 21;
static const int I2C_SCL_PIN  = 22;
static const int ONE_WIRE_PIN = 4;   // DS18B20 data pin

// =====================
// Sensor / App Settings
// =====================
static const float SEA_LEVEL_HPA = 1013.25;     // used for BMP280 altitude estimate
static const unsigned long SENSOR_POLL_MS = 3000;

// =====================
// WiFi Reconnect Policy
// =====================
static const unsigned long WIFI_RECONNECT_INTERVAL_MS = 30000;

// =====================
// Web Server
// =====================
static const uint16_t HTTP_PORT = 80;

struct ChipInfo{
  const char* chip_model ="";
  uint32_t chip_revision =0;
  uint8_t chip_cores =0;
  uint32_t chip_id =0;
};