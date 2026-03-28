/***
GPIO Viewer Tutorial : https://youtu.be/UxkOosaNohU
Latest Features : https://youtu.be/JJzRXcQrl3I
GPIO Viewer Documentation : https://github.com/thelastoutpostworkshop/gpio_viewer (Download the library from here)

The following libraries need to be added to Arduino IDE:
- ESP Async WebServer by ESP32Async (and NOT ESPAsyncWebServer !!!!!). Installed from the Libraries area.
- Async TCP by ESP32Async (and NOT AsyncTCP !!!!!). Installed from the Libraries area.

The ESP32 type for the viewer: ESP32-VROOM-32D (30 pins)

sensor hardware: HW-497
Reed sensor tutorial: https://www.youtube.com/watch?v=9qyQhf1gVgE
***/

// Since version 1.5.6, the library detects pin functions like ADC and Touch, this has been causing problems on some boards, like the XiaoESP32-S3-Sense. You can disable pin detection by uncommenting the following line:
// #define NO_PIN_FUNCTIONS

#include <gpio_viewer.h> // Must me the first include in your project
#include <Arduino.h>
//#include <ESP32Servo.h>

#define LED 2
#define EXT_LED 25
#define REED_PIN 26  //connect to the S pin of the Reed sensor

GPIOViewer gpio_viewer;
//Servo myServo;
float count = 0;

void setup()
{
  Serial.begin(115200);

  // Comment the next line, If your code aleady include connection to Wifi in mode WIFI_STA (WIFI_AP and WIFI_AP_STA are not supported)
  gpio_viewer.connectToWifi("BELL481", "7AEF5D494AF9");
  // gpio_viewer.setPort(5555);                 // You can set the http port, if not set default port is 8080

  // Your own setup code start here

  // Must be at the end of your setup
  // gpio_viewer.setSamplingInterval(25); // You can set the sampling interval in ms, if not set default is 100ms
  // gpio_viewer.setSkipPeripheralPins(false); // Default true: skip I2C/SPI/UART pins owned by peripherals

  pinMode(LED, OUTPUT);
  pinMode((EXT_LED), OUTPUT);
  pinMode(REED_PIN , INPUT);
  //myServo.attach(SERVO_PIN);
  //myServo.write(0);
  delay(500);
  gpio_viewer.begin();
}

// You don't need to change your loop function
void loop() {

  boolean state = digitalRead(REED_PIN);
  // Serial.print("Reed state: ");
  // Serial.println(state);

  if(state == LOW){
    digitalWrite(LED, HIGH);
    digitalWrite(EXT_LED, LOW);
    count++;
    Serial.print("Magnetic proximity detected for ");
    Serial.print(count/2);
    Serial.println(" seconds.");
    delay(500);
  }
  else{
    if (count > 0){
      Serial.println("-----------");
      count=0;
    }
    digitalWrite(LED, LOW);
    digitalWrite(EXT_LED, HIGH);
    delay(500);
  }

}

