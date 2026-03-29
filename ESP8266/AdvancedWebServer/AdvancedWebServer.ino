/** Resources:

- How to Send Data from NodeMCU ESP8266 to ThingSpeak | Complete IoT Tutorial for Beginners
https://www.youtube.com/watch?v=BK-cfWglVz0

Loaded on ESP8266 1
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <StreamString.h>
#include <Adafruit_AHTX0.h>

#ifndef STASSID
#define STASSID "BELL481"
#define STAPSK "7AEF5D494AF9"
#endif

#define LED_WHITE D1
#define LED_GREEN D4


const char *ssid = STASSID;
const char *password = STAPSK;
long last_submission = millis();
int data = 0;
const char *APIKey = "3DSGH4PYEK2TQM7V";
String thspeak = "http://api.thingspeak.com/update";


ESP8266WebServer server(80);

Adafruit_AHTX0 aht;
float temp = 0.0;
float hum = 0.0;
float new_temp = 0.0;
float new_hum = 0.0;

void handleRoot() {
  digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  //read the temp and rel hum
  read_the_sensor();

  StreamString temp;
  temp.reserve(500);  // Preallocate a large chunk to avoid memory fragmentation
  temp.printf("\
<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size: 26; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>ESP8266 Sensors read</h1>\
    <p>Uptime                   :%02d:%02d:%02d</p>\
    <p>Current temperature      : %0.2f°C</p>\
    <p>Current relative humidity: %0.2f%</p>\
    <br><br><br>\
    <p>Host name: %s</p>\
    <p>Host IP  : %s</p>\
  </body>\
</html>",
              hr, min % 60, sec % 60, new_temp, new_hum, WiFi.getHostname(), WiFi.localIP().toString().c_str());
  
  server.send(200, "text/html", temp.c_str());
  //digitalWrite(LED_GREEN, 0);
}


/*
* Handler for the /info link
*/
void handleInfo(){
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  //read the temp and rel hum
  read_the_sensor();
 
  StreamString temp;
  temp.reserve(500);  // Preallocate a large chunk to avoid memory fragmentation
  temp.printf("\
<html>\
  <head>\
    <meta http-equiv='refresh' content='10'/>\
    <title>ESP8266 Sensors read</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; font-size: 26; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Information page with sensors values</h1>\
    <p>Uptime                   :%02d:%02d:%02d</p>\
    <p>Current temperature      : %0.2f°C</p>\
    <p>Current relative humidity: %0.2f%</p>\
    <br><br><br>\
    <p>Host name: %s</p>\
    <p>Host IP  : %s</p>\
  </body>\
</html>",
              hr, min % 60, sec % 60, new_temp, new_hum, WiFi.getHostname(), WiFi.localIP().toString().c_str());
  
  server.send(200, "text/html", temp.c_str());
  //digitalWrite(LED_GREEN, 0);
}

/*
* Handler for all the 404 (not found) 
*/
void handleNotFound() {
  digitalWrite(LED_GREEN, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) { message += " " + server.argName(i) + ": " + server.arg(i) + "\n"; }

  server.send(404, "text/plain", message);
  digitalWrite(LED_GREEN, 0);
}

void read_the_sensor(){
  sensors_event_t  hum, the_temp;
  aht.getEvent(&hum, &the_temp);

  new_temp = the_temp.temperature;
  if(isnan(new_temp))  {
    Serial.println("Failed to read from sensor!!!");
  }
  //Serial.println(new_temp);

  new_hum = hum.relative_humidity;
  if(isnan(new_hum)){
    Serial.println("Failed to read from sensor!!!");
  }
}


void setup(void) {
  pinMode(LED_GREEN, OUTPUT);
  digitalWrite(LED_GREEN, 0);
  // Connect to the wifi network
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) { Serial.println("MDNS responder started"); }
  // END Connect to the wifi network

  // Set the web server links handlers
  server.on("/", handleRoot);
  // server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.on("/info", handleInfo);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
  // END set the web server links handlers

  //The default I2C address is 0x38. It cannot be changed.
  aht.begin();
  
}

void loop(void) {

  server.handleClient();
  
  if(millis()-last_submission > 900000){
    send_data_to_thingspeak();
    last_submission = millis();
    //data++;
  }
  MDNS.update();
}

void send_data_to_thingspeak(){
  Serial.println(">>> Sending data to ThingSpeak...");
  WiFiClient wificlient;
  HTTPClient http;

  String serverPath = thspeak + "?api_key=" + String(APIKey) + "&field1=" + new_temp + "&field2=" + new_hum;
  Serial.println(serverPath);
  http.begin(wificlient, serverPath);
  int httpResponse = http.GET();
  if(httpResponse > 0){
    Serial.print("HTTP response code: ");
    Serial.println(httpResponse);
  } else {
    Serial.print("HTTP error code: ");
    Serial.println(httpResponse);    
  }
  Serial.println("<<< Sending data to ThingSpeak...");
  //delay(30000);

}
