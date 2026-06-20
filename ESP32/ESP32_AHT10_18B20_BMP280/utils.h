#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_AHTX0.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>

static const int I2C_SDA_PIN  = 21;
static const int I2C_SCL_PIN  = 22;
static const int ONE_WIRE_PIN = 4;    //DS18B20 data pin

static OneWire oneWire(ONE_WIRE_PIN);
static DallasTemperature ds18b20(&oneWire);
static Adafruit_BMP280 bmp; // I2C
static Adafruit_AHTX0 aht;

static bool g_dsReady  = false;

//extern String networkName;

// =====================
// WiFi Credentials
// =====================
extern String WIFI_SSID;
extern String WIFI_PASSWORD;
// =====================
// WiFi Reconnect Policy
// =====================
static const unsigned long WIFI_RECONNECT_INTERVAL_MS = 30000;

struct SensorData  {
    String uptime_s;
    uint32_t sample_age_ms;

    // wifi
    bool wifi_connected;
    String wifi_status;
    String wifi_ip;
    int wifi_rssi;

    // avg_values
    float avg_temp_c;

    // 18B20 sensor values
    float ds18b20_temp_c;

    // aht10
    bool aht10_ok;
    float aht10_temp_c;
    float aht10_humidity;

    // bmp280
    bool bmp280_ok;
    float bmp280_temp_c;
    float bmp280_pressure_hpa;
    float bmp280_altitude_m;
};


void i2cScan();
void dsDetectAndPrint1WireAddresses();
bool loadJSONConfig();
void printStrings();