#include "wifiConnect.h"
#include "utils.h"
#include <WiFi.h>
#include <WebServer.h>




void connectToWiFi() {
    Serial.println("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());

    int retries = 0;
    const int maxRetries = 20;

    while (WiFi.status() != WL_CONNECTED && retries < maxRetries) {
        delay(500);
        Serial.print(".");
        retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\n✅ WiFi connected!");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\n❌ Failed to connect to WiFi");
    }
}

void webServerSetup(){
  
}



