#include <Arduino.h>
#include "utils.h"
#include "wifiConnect.h"
#include <ArduinoJson.h>


/**Sensors and connections
Dallas 18B20: 
- The board has a resistor already;
- Uses digital pin 4 to communicate with the sensors
- Can have more than one sensor in parralel on the same bus

AHT10:
- Uses default I2C pins
  - sensor SDA: pin 
  - sensor SCL: 
- Can use different I2C sensors in parralel as long as they have different addresses

BMP280:
- Uses default I2C pins, in parralel with AHT10

**/

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting setup ....");
  unsigned status;

  loadJSONConfig();
  connectToWiFi();

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  i2cScan();

  /* BMP 280 section*/
  status = bmp.begin(0x76);
  if (!status) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring or "
                      "try a different address!"));
    Serial.print("SensorID was: 0x"); Serial.println(bmp.sensorID(),16);
    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    Serial.print("        ID of 0x60 represents a BME 280.\n");
    Serial.print("        ID of 0x61 represents a BME 680.\n");
    while (1) delay(10);
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */  

  /* AHT10 area*/
  if (!aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1)
      delay(10);
  }
  Serial.println("AHT10 or AHT20 found");

  // DS18B20 init + detect
  ds18b20.begin();
  delay(100);

  dsDetectAndPrint1WireAddresses();
}

void loop() {
  SensorData data;

  data.uptime_s = millis()/1000;

  Serial.println("<--- AHT10 section --->");
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data

  Serial.println("Temperature C: " + String(temp.temperature) + "ºC"); 
  Serial.println("Humidity: " + String(humidity.relative_humidity) + "% rH");   
  data.aht10_temp_c = temp.temperature;
  data.aht10_humidity = humidity.relative_humidity;

  Serial.println("<--- Dallas 18B20 section --->");
  ds18b20.requestTemperatures(); 
  float temperatureC = ds18b20.getTempCByIndex(0);
  float temperatureF = ds18b20.getTempFByIndex(0);
  Serial.println("Temperature C: " + String(temperatureC) + "ºC");
  Serial.println("Temperature F: " + String(temperatureF) + "ºF");
  data.ds18b20_temp_c = ds18b20.getTempCByIndex(0);

  Serial.println("<--- BMP280 section --->");
  Serial.println("Temperature: " + String(bmp.readTemperature()) + "ºC");
  Serial.println("Pressure: " + String(bmp.readPressure()/100) + "hPa");
  Serial.println("Approx altitude = " + String(bmp.readAltitude(1013.25)) + " m. Not really reliable!");
  data.bmp280_temp_c = bmp.readTemperature();
  data.bmp280_pressure_hpa = bmp.readPressure()/100;
  data.bmp280_altitude_m = bmp.readAltitude(1013.25);
  data.avg_temp_c = (data.aht10_temp_c + data.bmp280_temp_c + data.ds18b20_temp_c)/3;
  StaticJsonDocument<512> jsonDoc;
  updateJsonFromStruct(jsonDoc, data);

  serializeJson(jsonDoc, Serial);

  Serial.println();
  delay(5000);
}

void updateJsonFromStruct(JsonDocument& doc, const SensorData& data) {
    // top-level
    doc["uptime_s"] = data.uptime_s;
    doc["sample_age_ms"] = data.sample_age_ms;

    // wifi
    JsonObject wifi = doc["wifi"].to<JsonObject>();
    wifi["connected"] = data.wifi_connected;
    wifi["status"] = data.wifi_status;
    wifi["ip"] = data.wifi_ip;
    wifi["rssi"] = data.wifi_rssi;

    // avg_values
    JsonObject avg_values = doc["avg_values"].to<JsonObject>();
    avg_values["avg_temp_c"] = data.avg_temp_c;

    // aht10
    JsonObject aht10 = doc["aht10"].to<JsonObject>();
    aht10["ok"] = data.aht10_ok;
    aht10["temp_c"] = data.aht10_temp_c;
    aht10["humidity"] = data.aht10_humidity;

    // bmp280
    JsonObject bmp280 = doc["bmp280"].to<JsonObject>();
    bmp280["ok"] = data.bmp280_ok;
    bmp280["temp_c"] = data.bmp280_temp_c;
    bmp280["pressure_hpa"] = data.bmp280_pressure_hpa;
    bmp280["altitude_m"] = data.bmp280_altitude_m;

    JsonObject ds18b20 = doc["ds18b20"].to<JsonObject>();
    ds18b20["temp_c"] = data.ds18b20_temp_c;

}




