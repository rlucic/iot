#include "config.h"
#include "connectivity.h"
#include "sensors.h"
#include "the_webserver.h"

static unsigned long lastPollMs = 0;
static SensorReadings lastReadings;


void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("ESP32 Sensors V2 + Web Server Starting...");

  wifiSetup();
  sensorsSetup();

  webServerSetup();

  wifiPrintStatus(Serial);
  //print chip information
  ChipInfo ci = setChipInfo();
  printChipInfo(Serial, ci);
}

void loop() {
  wifiLoop();
  the_webServerLoop();

  const unsigned long now = millis();
  if (now - lastPollMs >= SENSOR_POLL_MS) {
    lastPollMs = now;

    wifiPrintStatus(Serial);

    lastReadings = sensorsReadAll();
    webServerUpdateReadings(lastReadings);

    sensorsPrint(lastReadings, Serial);
  }
}

ChipInfo setChipInfo(){
  ChipInfo r;
  for (int i = 0; i < 17; i = i + 8) {
    r.chip_id |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  r.chip_cores = ESP.getChipCores();
  r.chip_model = ESP.getChipModel();
  r.chip_revision = ESP.getChipRevision();

  return r;
}

void printChipInfo(Stream& out, ChipInfo& ci){
    out.println("");
    out.println("--> Start printing the chip info");
    out.printf("ESP32 Chip model = %s Rev %d\n", ci.chip_model, ci.chip_revision);
    out.printf("This chip has %d cores\n", ci.chip_cores);
    out.print("Chip ID: ");
    out.println(ci.chip_id);
    out.println("End printing the chip info <--");
    out.println("");
}