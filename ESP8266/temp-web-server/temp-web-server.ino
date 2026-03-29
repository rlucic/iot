/*
Uses NodeMCU 1.0 board and COM6 (COM port might change)

The connection to local network works by using the wifiMulti class
Loaded in the #1
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h> 
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>   // Include the WebServer library
/*adding libraries for the DS18B20 sensors and initialize the objects*/
#include <OneWire.h>
#include <DallasTemperature.h>

// All DS18B20 Sensors are connected to pin 2 on the ESP8266
#define ONE_WIRE_BUS 4
// Creating a oneWire instance(object)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire object reference to Dallas Temperature array of sensors
DallasTemperature sensors(&oneWire);

int numberOfDevices; //To store number of sensor connected

DeviceAddress tempDeviceAddress; // Variable to store a single sensor address

//define variables for the two temperatures
float temp_c;
float temp_f;
int loops = 0;

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'

ESP8266WebServer server(80);    // Create a webserver object that listens for HTTP request on port 80

void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();

void setup(void){
  int conn_attempts = 0;
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  //wifiMulti.addAP("BELL481-V", "fc49ee4c6ebd");   // add Wi-Fi networks you want to connect to
  wifiMulti.addAP("BELL481", "7AEF5D494AF9");
  // wifiMulti.addAP("ssid_from_AP_3", "your_password_for_AP_3");

  Serial.println("Connecting ...");
  int i = 0;
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250);
    Serial.print(i);
    Serial.print("-> ");
    i++;

  }
  Serial.println('\n');
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // Send the IP address of the ESP8266 to the computer

  if (MDNS.begin("esp8266-1")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
    MDNS.addService("_http", "_tcp", 80);
  } else {
    Serial.println("Error setting up MDNS responder!");
  }

  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/info", handleInfo);
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
  Serial.printf("Default hostname: %s\n", WiFi.hostname().c_str());

  pinMode(ONE_WIRE_BUS, INPUT); 
  setup_sensors_and_print_addresses();
}

/*
 * Part of the setup for the application.
 * Sets up the sensors.
 */
void setup_sensors_and_print_addresses(){
  Serial.println(" >> into setup_sensors_and_print_addresses ...");
  
  // The begin method finds the devices on the wire and stores the addresses in the sensors object
  sensors.begin();
  delay(2000);
  // Get the number of sensors connected to the the wire( digital pin 2)
  numberOfDevices = sensors.getDeviceCount();
  
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  // Loop through each sensor and print out address
  for(int i=0; i<numberOfDevices; i++) {
    
    // Search the data wire for address and store the address in "tempDeviceAddress" variable
    if(sensors.getAddress(tempDeviceAddress, i)) {
      
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(tempDeviceAddress);
      Serial.println();
    } else {
      Serial.print("Found ghost device at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
    
  }//Enf of for loop
  Serial.println(" << out from setup_sensors_and_print_addresses ...");
}


void loop(void){
Serial.println(">>> loop");
read_temperatures();
server.handleClient();
Serial.println("<<< loop");
//delay(5000);

}



void read_temperatures(){
  Serial.println(">>> read_temperatures");
  //start reading the available ds18b20 sensors
  sensors.requestTemperatures(); // Send the command to get temperatures from all sensors.
  
  // Loop through each device, print out temperature one by one
  for(int i=0; i<numberOfDevices; i++) {
    
    // Search the wire for address and store the address in tempDeviceAddress
    if(sensors.getAddress(tempDeviceAddress, i)){
      temp_c = sensors.getTempC(tempDeviceAddress); //Temperature in degree celsius
      Serial.print("Sensor number: ");
      Serial.print(i,DEC);
      temp_f = DallasTemperature::toFahrenheit(temp_c);
      // Print the temperature
      Serial.print("Temp °C: ");
      Serial.print(temp_c);
      Serial.print(" Temp F: ");
      Serial.print(temp_f); // Converts tempC to Fahrenheit
      Serial.println(F("°F"));
    }   
    
  }// End of for loop
Serial.println("<<< read_temperatures");
}

// function to print a sensor address
void printAddress(DeviceAddress deviceAddress) {
  
  for (uint8_t i = 0; i < 8; i++) {
    
    if (deviceAddress[i] < 16) 
      Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
      
  }//End of for loop

}

void handleRoot() {
  Serial.println("Responding a request...");
  String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<link rel=\"icon\" href=\"data:,\">";
  html += "<style>";
  html += "body { font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; background: white; color: #333; margin: 20px; }";
  html += "h1 { color: #4a5568; font-weight: 300; }";
  html += "</style></head>";
  html += "<body>";
  html += "<h1>Hello World</h1>";
  html += "<p>ESP32 mDNS Demo.</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleInfo() {
  Serial.println("Responding to an INFO request...");
  server.send(200, "text/plain", "Sending back the INFO");   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}