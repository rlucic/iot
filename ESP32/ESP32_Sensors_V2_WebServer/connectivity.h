#pragma once
#include <Arduino.h>



void wifiSetup();
void wifiLoop();

bool wifiIsConnected();
String wifiStatusString();
void wifiPrintStatus(Stream& out);
