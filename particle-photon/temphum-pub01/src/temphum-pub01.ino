// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_DHT.h>

// This #include statement was automatically added by the Particle IDE.
#include <spark-dallas-temperature.h>

// This #include statement was automatically added by the Particle IDE.
#include <OneWire.h>
// // This #include statement was automatically added by the Particle IDE.
// #include <SparkTime.h>

/*****************************************************************************
Reads DHT22 and DallasTemperature 18B20 and publishes events with temperature and humidity

See https://docs.particle.io/tutorials/topics/maker-kit to learn how!

more exact location: https://docs.particle.io/tutorials/projects/maker-kit/
and from: https://github.com/adafruit/Adafruit_SSD1306/blob/master/examples/ssd1306_128x64_i2c/ssd1306_128x64_i2c.ino

for sending JSON in the data to thingspeak:
complete sample: https://www.hackster.io/kayakpete/multiple-sensors-particle-photon-thingspeak-3ff8a4
using String: https://community.particle.io/t/how-to-pass-multiple-data-items-from-particle-device-to-webhook/25487/4
using snprintf: https://community.particle.io/t/how-to-set-up-a-json-for-multiple-variables-in-a-webhook-integration/33172/7

******************************************************************************/

// Data wire is plugged into pin D4 of the Particle
// Pin bus for digital temperature Maxim/Dallas temperature
#define ONE_WIRE_BUS 4

#define DHTPIN 2     // what pin we're connected DHTxx to

// Uncomment whatever type you're using!
//#define DHTTYPE DHT11		// DHT 11
#define DHTTYPE DHT22		// DHT 22 (AM2302)
//#define DHTTYPE DHT21		// DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas 18B20 Temperature.
DallasTemperature sensors(&oneWire);

String event_Name = "ambient-reading";
String event_Name_Loop = "LoopingEvent";
String event_Name_dht11faultReading = "dht11-fault-reading";
String event_Name_dht22faultReading = "dht22-fault-reading";
String event_Name_dht11temp = "dht11-temp-reading";
String event_Name_dht22temp = "dht22-temp-reading";
String event_Name_digtemp = "digtemp";
String event_Name_dtemp = "temp";
String event_Name_dht22hum = "dht22-humid-reading";
String app_name = "Ambient readings";

Timer timer30sec(30000, timer30sec_handler);

//int i = 0;
int j = 0;

float digital_temp_calibration_value = 4.00;

// for using the time library
// UDP UDPClient;
// SparkTime rtc;
// unsigned long currentTime;
// unsigned long lastTime = 0UL;
// String dateStr, timeStr;


void setup() {

    Particle.publish(app_name,"Start " + event_Name + " setup",PRIVATE);

    delay(1000);

    //START THE SENSOR DHT22
    dht.begin();
    Particle.publish(app_name,"DHT22 sensor started!",PRIVATE);

    // for using the time library
    // initialize SparkTime
    // rtc.begin(&UDPClient, "north-america.pool.ntp.org");
    // rtc.setTimeZone(-5); // gmt offset

    //start the sensors
    sensors.begin();

    Particle.publish(app_name,"End " + event_Name + " setup",PRIVATE);
    delay(5000);
    //start the 30 seconds timer
    timer30sec.start();

}

void loop() {
// for using the time library
// currentTime = rtc.now();

//looks like the loop needs to have some code to execute
//otherwise the sensors are not read correctly
  delay(100);
}

/*
function to handle the timer for 30 seconds
*/
void timer30sec_handler(){
  //    delay(500);
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a
  // very slow sensor)
      float h = dht.getHumidity();
      String pubh = String(h,2);
  // Read temperature as Celsius
      float t = dht.getTempCelcius();
      String pubt = String(t,2);
  // Read temperature as Farenheit
  // float f = dht.getTempFarenheit();

  if (isnan(h) || isnan(t)) {
//		Serial.println("Failed to read from DHT sensor!");
      Particle.publish(event_Name_dht22faultReading,"Failed to read from DHT sensor!" ,PRIVATE);
    return;
  }

  // Send the command to get temperature readings from the Dallas 20B20
  //    sensors.requestTemperatures();
  //    float digt = sensors.getTempCByIndex(0);
  //    digt = digt - digital_temp_calibration_value;



      //String eventString = "Temp: " + String(t, 2) + "C" + " | " + "Rel Hum: " + String(h, 2) + "%";
      // String d_temp = "";
      // d_temp = String(digt,2);

      //Thingspeak publishing section for the Dallas 18B20
      // j++;
      // if(j>10 && !isnan(digt)){
      //    //publish to Thingspeak approx every 10 minutes
      //     Particle.publish(event_Name_dtemp, d_temp, PRIVATE);
      //     delay(1000);
      //   j=0;
      // }


      //Thingspeak publishing section for the DHT22
      j++;
      if(j>10 && !isnan(t)){
        //publish to Thingspeak approx every 5 minutes
        Particle.publish("temp", pubt, PRIVATE);
        Particle.publish(event_Name_dht22temp, pubt, PRIVATE);
        j=0;
      }

      Particle.publish(event_Name_dht22temp, "Temp: " + pubt + "C", PRIVATE);
      delay(100);
      Particle.publish(event_Name_dht22hum,  "Rel Hum: " + pubh + "%", PRIVATE);
      delay(100);

      String jsonString = "{\"field1\":";
      jsonString.concat("\"" + pubt + "\"");
      jsonString.concat(",\"field2\":\"" + pubh + "\"");
      jsonString.concat("}");

      Particle.publish("json", jsonString, PRIVATE);
}
