#include "connectivity.h"
#include "config.h"
#include <WiFi.h>

static bool g_connected = false;
static unsigned long g_lastReconnectAttemptMs = 0;


// Minimal event handler: keep it lightweight (runs in another task).
// The Arduino-ESP32 WiFi API documents that event callbacks run on a separate FreeRTOS task.
static void onWiFiEvent(arduino_event_id_t event) {
  switch (event) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      g_connected = true;
      Serial.print("[WiFi] Got IP: ");
      Serial.println(WiFi.localIP());
      break;

    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      g_connected = false;
      Serial.println("[WiFi] Disconnected");
      break;

    default:
      break;
  }
}

void wifiSetup() {
  Serial.println("[WiFi] Setup...");

  WiFi.onEvent(onWiFiEvent);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("[WiFi] Connecting to SSID: ");
  Serial.println(WIFI_SSID);

}

void wifiLoop() {
  const unsigned long now = millis();

  g_connected = (WiFi.status() == WL_CONNECTED);

  // If down, attempt reconnect every WIFI_RECONNECT_INTERVAL_MS
  if (!g_connected && (now - g_lastReconnectAttemptMs >= WIFI_RECONNECT_INTERVAL_MS)) {
    g_lastReconnectAttemptMs = now;

    Serial.print("[WiFi] ");
    Serial.print(now);
    Serial.println("ms: Reconnecting...");

    WiFi.disconnect();
    WiFi.reconnect();
  }
}

bool wifiIsConnected() {
  return (WiFi.status() == WL_CONNECTED);
}

String wifiStatusString() {
  wl_status_t st = WiFi.status();
  switch (st) {
    case WL_CONNECTED:       return "WL_CONNECTED";
    case WL_NO_SSID_AVAIL:   return "WL_NO_SSID_AVAIL";
    case WL_CONNECT_FAILED:  return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST: return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED:    return "WL_DISCONNECTED";
    case WL_IDLE_STATUS:     return "WL_IDLE_STATUS";
    default:                 return "WL_UNKNOWN";
  }
}

void wifiPrintStatus(Stream& out) {
  out.print("[WiFi] Status: ");
  out.print(wifiStatusString());
  if (wifiIsConnected()) {
    out.print(" | IP: ");
    out.print(WiFi.localIP());
    out.print(" | RSSI: ");
    out.print(WiFi.RSSI());
    out.println(" dBm");
  } else {
    out.println();
  }
}



