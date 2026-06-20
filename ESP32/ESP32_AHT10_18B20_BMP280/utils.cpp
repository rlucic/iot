#include "utils.h"
#include <OneWire.h>
#include <LittleFS.h>

String thspeak_s = "";
String thspeakAPIKey_s = "";
String WIFI_SSID = "";
String WIFI_PASSWORD = "";


void i2cScan() {
  Serial.println("\n[I2C] Scanning bus...");
  byte count = 0;

  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("✅ I2C device at 0x");
      if (address < 16) Serial.print("0");
      Serial.print(address, HEX);

      if (address == 0x38) Serial.print("  <-- AHT10 detected");
      if (address == 0x76) Serial.print("  <-- BMP280 detected");
      Serial.println();
      count++;
    }
  }

  if (count == 0) Serial.println("❌ No I2C devices found!");
  else Serial.println("✅ I2C scan complete.");
}

void dsDetectAndPrint1WireAddresses() {
  Serial.println("\n[1-Wire] Checking DS18B20 sensors...");
  ds18b20.begin();
  int deviceCount = ds18b20.getDeviceCount();
  if (deviceCount == 0) {
    Serial.println("❌ No DS18B20 sensors found!");
    g_dsReady = false;
    return;
  }

  Serial.print("✅ DS18B20 sensors found: ");
  Serial.println(deviceCount);

  DeviceAddress addr;
  for (int i = 0; i < deviceCount; i++) {
    if (ds18b20.getAddress(addr, i)) {
      Serial.print("Sensor ");
      Serial.print(i);
      Serial.print(" Address: ");
      for (uint8_t j = 0; j < 8; j++) {
        if (addr[j] < 16) Serial.print("0");
        Serial.print(addr[j], HEX);
      }
      Serial.println();
    } else {
      Serial.print("⚠️ Unable to read address for device ");
      Serial.println(i);
    }
  }

  g_dsReady = true;
}


bool loadJSONConfig() {
  Serial.begin(115200);
  delay(1000);
  if (!LittleFS.begin()) {
    Serial.println("Failed to mount file system");
    return false;
  }
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("❌ Failed to open config file");
    return false;
  }
  // Serial.print("Config file size: ");
  // Serial.println(configFile.size());
  

  StaticJsonDocument<300> doc;
  auto error = deserializeJson(doc, configFile);
  if (error) {
    Serial.println("Failed to parse config file");
    return false;
  }

  //transform char* to String
  WIFI_SSID = doc["networkName"].as<String>();
  WIFI_PASSWORD = doc["password"].as<String>();

  const char* thspeak = doc["thspeak"];
  const char* thspeakAPIKey = doc["thspeakAPIKey"];

  thspeak_s = doc["thspeak"].as<String>();
  thspeakAPIKey_s = doc["thspeakAPIKey"].as<String>();
  // Real world application would store these values in some variables for
  // later use.

  Serial.println("");
  Serial.println("Loaded network name: " + WIFI_SSID);
  Serial.println("ThingSpeak: " + thspeak_s);
  Serial.println("thspeakAPIKey: "+ thspeakAPIKey_s);
  Serial.println("");

  //printStrings();
  configFile.close();
  return true;
}

void printStrings(){
  Serial.println("");
  Serial.println(">>> Printing Strings: ");
  Serial.print("ThingSpeak as String: " + thspeak_s);
  Serial.println("<<< End Printing Strings");
}
