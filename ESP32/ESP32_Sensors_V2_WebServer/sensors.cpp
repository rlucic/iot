#include "sensors.h"
#include "config.h"

#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <OneWire.h>
#include <DallasTemperature.h>

static Adafruit_AHTX0 aht;
static Adafruit_BMP280 bmp;

static OneWire oneWire(ONE_WIRE_PIN);
//static DallasTemperature ds18b20(&oneWire);

static bool g_ahtReady = false;
static bool g_bmpReady = false;
static bool g_dsReady  = false;

static void i2cScan() {
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

// static void dsDetectAndPrintAddresses() {
//   Serial.println("\n[1-Wire] Checking DS18B20 sensors...");

//   int deviceCount = ds18b20.getDeviceCount();
//   if (deviceCount == 0) {
//     Serial.println("❌ No DS18B20 sensors found!");
//     g_dsReady = false;
//     return;
//   }

//   Serial.print("✅ DS18B20 sensors found: ");
//   Serial.println(deviceCount);

//   DeviceAddress addr;
//   for (int i = 0; i < deviceCount; i++) {
//     if (ds18b20.getAddress(addr, i)) {
//       Serial.print("Sensor ");
//       Serial.print(i);
//       Serial.print(" Address: ");
//       for (uint8_t j = 0; j < 8; j++) {
//         if (addr[j] < 16) Serial.print("0");
//         Serial.print(addr[j], HEX);
//       }
//       Serial.println();
//     } else {
//       Serial.print("⚠️ Unable to read address for device ");
//       Serial.println(i);
//     }
//   }

//   g_dsReady = true;
// }

void sensorsSetup() {
  Serial.println("[Sensors] Setup...");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  i2cScan();

  g_ahtReady = aht.begin();
  if (!g_ahtReady) Serial.println("❌ Failed to initialize AHT10!");
  else Serial.println("✅ AHT10 initialized");

  g_bmpReady = bmp.begin(0x76);
  if (!g_bmpReady) {
    Serial.println("⚠️ BMP280 not found at 0x76, trying 0x77...");
    g_bmpReady = bmp.begin(0x77);
  }
  if (!g_bmpReady) Serial.println("❌ BMP280 not found!");
  else Serial.println("✅ BMP280 initialized");

  if (g_bmpReady) {
    bmp.setSampling(
      Adafruit_BMP280::MODE_NORMAL,
      Adafruit_BMP280::SAMPLING_X2,
      Adafruit_BMP280::SAMPLING_X16,
      Adafruit_BMP280::FILTER_X16,
      Adafruit_BMP280::STANDBY_MS_500
    );
  }

  // ds18b20.begin();
  // dsDetectAndPrintAddresses();

  Serial.println("\n[Sensors] Setup complete.\n");
}

SensorReadings sensorsReadAll() {
  SensorReadings r;

  if (g_ahtReady) {
    sensors_event_t humidityEv, tempEv;
    aht.getEvent(&humidityEv, &tempEv);
    r.ahtOk = true;
    r.ahtTempC = tempEv.temperature;
    r.humidity = humidityEv.relative_humidity;
  }

  if (g_bmpReady) {
    r.bmpOk = true;
    r.bmpTempC = bmp.readTemperature();
    r.pressureHpa = bmp.readPressure() / 100.0;
    r.altitudeM = bmp.readAltitude(SEA_LEVEL_HPA);
  }

  // setting up the average temperature
  if(g_ahtReady && g_bmpReady){
    r.avgTempC = (r.ahtTempC + r.bmpTempC)/2;
    // Serial.print(" ####>> Avg Temperature: ");
    // Serial.println(r.avgTempC);
  }

  // if (g_dsReady) {
  //   ds18b20.requestTemperatures();
  //   float t = ds18b20.getTempCByIndex(0);
  //   if (t != DEVICE_DISCONNECTED_C) {
  //     r.dsOk = true;
  //     r.dsTempC = t;
  //   } else {
  //     r.dsOk = false;
  //   }
  // }

  return r;
}

void sensorsPrint(const SensorReadings& r, Stream& out) {
  out.println("---- AHT10 ----");
  if (r.ahtOk) {
    out.print("Temperature: "); out.print(r.ahtTempC); out.println(" °C");
    out.print("Humidity   : ");    out.print(r.humidity); out.println(" %");
  } else {
    out.println("AHT10: ❌ not available");
  }

  out.println("---- BMP280 ----");
  if (r.bmpOk) {
    out.print("Temperature: "); out.print(r.bmpTempC); out.println(" °C");
    out.print("Pressure   : ");    out.print(r.pressureHpa); out.println(" hPa");
    out.print("Altitude   : ");    out.print(r.altitudeM); out.println(" m");
  } else {
    out.println("BMP280: ❌ not available");
  }

  // out.println("---- DS18B20 ----");
  // if (r.dsOk) {
  //   out.print("Temperature: "); out.print(r.dsTempC); out.println(" °C");
  // } else {
  //   out.println("DS18B20: ❌ disconnected / not found");
  // }

  out.println("========================\n");
}

