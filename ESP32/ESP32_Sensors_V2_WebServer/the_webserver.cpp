#include "the_webserver.h"
#include "config.h"
#include "connectivity.h"

#include <WiFi.h>
#include <WebServer.h>

static WebServer server(HTTP_PORT);

static bool g_started = false;

static SensorReadings g_last;
static unsigned long g_lastMs = 0;
static float tt = 0.0;

static String jsonEscape(const String& s) {
  String o;
  o.reserve(s.length() + 8);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"') o += "\\\"";
    else if (c == '\\') o += "\\\\";
    else if (c == '\n') o += "\\n";
    else if (c == '\r') o += "\\r";
    else if (c == '\t') o += "\\t";
    else o += c;
  }
  return o;
}

static String buildJson() {
  // Manual JSON to avoid ArduinoJson dependency.
  String ip = wifiIsConnected() ? WiFi.localIP().toString() : String("");
  long rssi = wifiIsConnected() ? WiFi.RSSI() : 0;
  int sec = millis()/1000;
  int min = sec/60;
  int hr  = min/60;
  
  String up_time = String(hr) + "h " + String(min%60) + "m " + String(sec%60) + "s";
  //String up_time = String(hr) + ":" + String(min%60) + ":" + String(sec%60);
  //Serial.println(up_time);

  String j;
  j.reserve(512);
  j += "{";

  j += "\"uptime_s\":\"" + jsonEscape(up_time) +            "\",";
  j += "\"sample_age_ms\":" + String(millis() - g_lastMs) + ",";

  j += "\"wifi\":{";
  j += "\"connected\":" + String(wifiIsConnected() ? "true" : "false") + ",";
  j += "\"status\":\"" + jsonEscape(wifiStatusString()) + "\",";
  j += "\"ip\":\"" + jsonEscape(ip) + "\",";
  j += "\"rssi\":" + String(rssi);
  j += "},";

  j += "\"avg_values\":{";
  j += "\"avg_temp_c\":" + (isnan(g_last.avgTempC) ? String("null") : String(g_last.avgTempC, 2));
  j += "},";

  j += "\"aht10\":{";
  j += "\"ok\":" + String(g_last.ahtOk ? "true" : "false") + ",";
  j += "\"temp_c\":" + (isnan(g_last.ahtTempC) ? String("null") : String(g_last.ahtTempC, 2)) + ",";
  j += "\"humidity\":" + (isnan(g_last.humidity) ? String("null") : String(g_last.humidity, 1));
  j += "},";

  j += "\"bmp280\":{";
  j += "\"ok\":" + String(g_last.bmpOk ? "true" : "false") + ",";
  j += "\"temp_c\":" + (isnan(g_last.bmpTempC) ? String("null") : String(g_last.bmpTempC, 2)) + ",";
  j += "\"pressure_hpa\":" + (isnan(g_last.pressureHpa) ? String("null") : String(g_last.pressureHpa, 2)) + ",";
  j += "\"altitude_m\":" + (isnan(g_last.altitudeM) ? String("null") : String(g_last.altitudeM, 2));
  j += "}";

  // j += "\"ds18b20\":{";
  // j += "\"ok\":" + String(g_last.dsOk ? "true" : "false") + ",";
  // j += "\"temp_c\":" + (isnan(g_last.dsTempC) ? String("null") : String(g_last.dsTempC, 2));
  // j += "}";

  j += "}";
  //Serial.println(j);
  return j;
}

static String buildRootHtml() {
  // Simple HTML + JS that polls /api/readings every 5s.
  String h;
  h.reserve(2200);
  h += "<!doctype html><html><head>";
  h += "<meta charset='utf-8'/>";
  h += "<meta name='viewport' content='width=device-width, initial-scale=1'/>";
  h += "<title>ESP32 Sensors</title>";
  h += "<style>body{font-family:Segoe UI,Arial,sans-serif;margin:16px;font-size:50;}";
  h += ".card{border:1px solid #ddd;border-radius:10px;padding:12px;margin:10px 0;}";
  h += ".ok{color:#0a7;} .bad{color:#c33;} .mono{font-family:Consolas,monospace;}";
  h += "table{border-collapse:collapse;} td{padding:4px 10px 4px 0;}";
  h += "</style></head><body>";
  h += "<h2>ESP32 Sensor Dashboard</h2>";
  h += "<div class='card'><b>WiFi</b><div id='wifi'></div></div>";
  h += "<div class='card'><b>Average Values</b><div id='values'></div></div>";
  h += "<div class='card'><b>AHT10</b><div id='aht'></div></div>";
  h += "<div class='card'><b>BMP280</b><div id='bmp'></div></div>";
  // h += "<div class='card'><b>DS18B20</b><div id='ds'></div></div>";
  h += "<div class='card'><b>Raw</b><pre class='mono' id='raw'></pre></div>";
  // h += "<div class='card'><b>Chip info</b><pre class='mono' id='chip'></pre></div>";

  h += "<script>";
  h += "function cls(ok){return ok?'ok':'bad';}";
  h += "function fmt(v, suf){return (v===null||v===undefined)?'—':(v+ (suf||''));}";
  h += "async function refresh(){";
  h += "  try{const r=await fetch('/api/readings'); const j=await r.json();";
  h += "    document.getElementById('raw').textContent=JSON.stringify(j,null,2);";
  h += "    document.getElementById('wifi').innerHTML =";
  h += "      '<div class='+cls(j.wifi.connected)+'>'+ (j.wifi.connected?'Connected':'Disconnected') +'</div>' +";
  h += "      '<div>Status: '+j.wifi.status+'</div>' +";
  h += "      '<div>IP: '+ (j.wifi.ip||'—') +'</div>' +";
  h += "      '<div>RSSI: '+ (j.wifi.connected?j.wifi.rssi:'—') +'</div>' +";
  h += "      '<div>Uptime: '+j.uptime_s+'</div>' +";
  h += "      '<div>Sample age(ms): '+j.sample_age_ms+'</div>';";

  h += "    document.getElementById('values').innerHTML =";
  h += "      '<table><tr><td>Avg Temp</td><td>'+fmt(j.avg_values.avg_temp_c,' °C')+'</td></tr>' +";
  h += "      '<tr><td>Humidity</td><td>'+fmt(j.aht10.humidity,' %')+'</td></tr>' +";
  h += "      '<tr><td>Pressure</td><td>'+fmt(j.bmp280.pressure_hpa,' hPa')+'</td></tr></table>';";

  h += "    document.getElementById('aht').innerHTML =";
  h += "      '<div class='+cls(j.aht10.ok)+'>'+ (j.aht10.ok?'OK':'Not available') +'</div>' +";
  h += "      '<table><tr><td>Temp</td><td>'+fmt(j.aht10.temp_c,' °C')+'</td></tr>' +";
  h += "      '<tr><td>Humidity</td><td>'+fmt(j.aht10.humidity,' %')+'</td></tr></table>';";

  h += "    document.getElementById('bmp').innerHTML =";
  h += "      '<div class='+cls(j.bmp280.ok)+'>'+ (j.bmp280.ok?'OK':'Not available') +'</div>' +";
  h += "      '<table><tr><td>Temp</td><td>'+fmt(j.bmp280.temp_c,' °C')+'</td></tr>' +";
  h += "      '<tr><td>Pressure</td><td>'+fmt(j.bmp280.pressure_hpa,' hPa')+'</td></tr>' +";
  h += "      '<tr><td>Altitude</td><td>'+fmt(j.bmp280.altitude_m,' m')+' <-- NOT RELIABLE!</td></tr></table>';";

  // h += "    document.getElementById('ds').innerHTML =";
  // h += "      '<div class='+cls(j.ds18b20.ok)+'>'+ (j.ds18b20.ok?'OK':'Disconnected') +'</div>' +";
  // h += "      '<table><tr><td>Temp</td><td>'+fmt(j.ds18b20.temp_c,' °C')+'</td></tr></table>';";

  // h += "    document.getElementById('chip').innerHTML =";
  // h += "      '<table><tr><td>Chip Model</td><td>'+fmt(j.bmp280.temp_c,' °C')+'</td></tr>' +";
  // h += "      '<tr><td>Chip revision</td><td>'+fmt(j.bmp280.pressure_hpa,' hPa')+'</td></tr>' +";
  // h += "      '<tr><td>Chip ID</td><td>'+fmt(j.bmp280.pressure_hpa,' hPa')+'</td></tr>' +";
  // h += "      '<tr><td>Chip cores</td><td>'+fmt(j.bmp280.altitude_m,' m')+'</td></tr></table>';";

  h += "  }catch(e){document.getElementById('raw').textContent='Error: '+e;}";
  h += "} setInterval(refresh,5000); refresh();";
  h += "</script>";

  h += "</body></html>";
  return h;
}

static void handleRoot() {
  server.send(200, "text/html", buildRootHtml());
}

static void handleApiReadings() {
  server.send(200, "application/json", buildJson());
}

static void handleNotFound() {
  String msg = "Not Found\n\nURI: ";
  msg += server.uri();
  msg += "\nMethod: ";
  msg += (server.method() == HTTP_GET) ? "GET" : "OTHER";
  server.send(404, "text/plain", msg);
}

void webServerUpdateReadings(const SensorReadings& r) {
  g_last = r;
  g_lastMs = millis();
}

void webServerSetup() {
  // Register routes. server.on() maps URI to handler.
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/readings", HTTP_GET, handleApiReadings);
  server.onNotFound(handleNotFound);
}

void the_webServerLoop() {
  // Start server once WiFi is up.
  if (!g_started && wifiIsConnected()) {
    server.begin();
    g_started = true;
    Serial.print("[HTTP] Server started on http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
  }

  // If WiFi dropped, stop the server to avoid confusing clients.
  if (g_started && !wifiIsConnected()) {
    server.stop();
    g_started = false;
    Serial.println("[HTTP] Server stopped (WiFi down)");
  }

  if (g_started) {
    // Must be called frequently to process requests.
    server.handleClient();
  }
}
