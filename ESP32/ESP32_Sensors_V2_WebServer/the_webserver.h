#pragma once
#include <Arduino.h>
#include "sensors.h"

void webServerSetup();
void the_webServerLoop();

// Call this whenever you take new readings so the web server can serve the latest.
void webServerUpdateReadings(const SensorReadings& r);
