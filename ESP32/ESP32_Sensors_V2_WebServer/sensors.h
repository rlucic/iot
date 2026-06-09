#pragma once
#include <Arduino.h>

struct SensorReadings {
  bool ahtOk = false;
  bool bmpOk = false;
  bool dsOk  = false;

  float avgTempC = NAN;

  float ahtTempC = NAN;
  float humidity = NAN;

  float bmpTempC = NAN;
  float pressureHpa = NAN;
  float altitudeM = NAN;

  float dsTempC = NAN;
};

void sensorsSetup();
SensorReadings sensorsReadAll();
void sensorsPrint(const SensorReadings& r, Stream& out);
