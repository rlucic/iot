// This #include statement was automatically added by the Particle IDE.
#include <Adafruit_DHT.h>

// This #include statement was automatically added by the Particle IDE.
#include <spark-dallas-temperature.h>

// This #include statement was automatically added by the Particle IDE.
#include <OneWire.h>
// // This #include statement was automatically added by the Particle IDE.
// #include <SparkTime.h>


/*****************************************************************************
Reads DHT11 and publishes events with temperature and humidity

See https://docs.particle.io/tutorials/topics/maker-kit to learn how!

more exact location: https://docs.particle.io/tutorials/projects/maker-kit/
and from: https://github.com/adafruit/Adafruit_SSD1306/blob/master/examples/ssd1306_128x64_i2c/ssd1306_128x64_i2c.ino

for sending JSON in the data to thingspeak:
complete sample: https://www.hackster.io/kayakpete/multiple-sensors-particle-photon-thingspeak-3ff8a4
using String: https://community.particle.io/t/how-to-pass-multiple-data-items-from-particle-device-to-webhook/25487/4
using snprintf: https://community.particle.io/t/how-to-set-up-a-json-for-multiple-variables-in-a-webhook-integration/33172/7

******************************************************************************/

// Data wire is plugged into pin D4 of the Particle
#define ONE_WIRE_BUS 4

#define DHTPIN 2     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11		// DHT 11
//#define DHTTYPE DHT22		// DHT 22 (AM2302)
//#define DHTTYPE DHT21		// DHT 21 (AM2301)
DHT dht(DHTPIN, DHTTYPE);
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

String event_Name = "ambient-reading";
String event_Name_Loop = "LoopingEvent";
String event_Name_faultReading = "dht11-fault-reading";
String event_Name_temp = "dht11-temp-reading";
String event_Name_digtemp = "digtemp";
String event_Name_dtemp = "temp";
String event_Name_hum = "dht11-humid-reading";
String app_name = "Ambient readings";

int i = 0;
int j = 0;

// UDP UDPClient;
// SparkTime rtc;

// unsigned long currentTime;
// unsigned long lastTime = 0UL;
// String dateStr, timeStr;


void setup() {

    Particle.publish(app_name,"Start " + event_Name + " setup",PRIVATE);

    delay(1000);

    //START THE SENZOR READ
    dht.begin();
    Particle.publish(app_name,"DHT11 senzor started!",PRIVATE);

        //initialize SparkTime
    // rtc.begin(&UDPClient, "north-america.pool.ntp.org");
    // rtc.setTimeZone(-5); // gmt offset

    //start the senzors
    sensors.begin();

    Particle.publish(app_name,"End " + event_Name + " setup",PRIVATE);
    delay(5000);

}

void loop() {

//    currentTime = rtc.now();



// Reading temperature or humidity takes about 250 milliseconds!
// Sensor readings may also be up to 2 seconds 'old' (its a
// very slow sensor)
    float h = dht.getHumidity();
// Read temperature as Celsius
    float t = dht.getTempCelcius();
// Read temperature as Farenheit
    float f = dht.getTempFarenheit();

    sensors.requestTemperatures(); // Send the command to get temperature readings from the Dallas 20B20
    float digt = sensors.getTempCByIndex(0);

// Check if any reads failed and exit early (to try again).
 	if (isnan(h) || isnan(t) || isnan(f)) {
//		Serial.println("Failed to read from DHT sensor!");
 	    Particle.publish(event_Name_faultReading,"Failed to read from DHT sensor!" ,PRIVATE);
 		return;
 	}
  else{
        //String eventString = "Temp: " + String(t, 2) + "C" + " | " + "Rel Hum: " + String(h, 2) + "%";

        i++;
        if(i>=3){
            Particle.publish(event_Name_temp, "Temp: " + String(t, 2) + "C", PRIVATE);
            delay(1100);
            Particle.publish(event_Name_hum,  "Rel Hum: " + String(h, 2) + "%", PRIVATE);
            delay(1100);
            Particle.publish(event_Name_digtemp, String(digt,2), PRIVATE);
            i=0;
        }
        j++;
        if(j>45){
          delay(1100);
          Particle.publish(event_Name_dtemp, String(digt,2), PRIVATE);
          j=0;
        }
 	}

    delay(10000);
}
