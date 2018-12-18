#include "DHT.h" //including the dht22 library
#include "LiquidCrystal.h"
#define DHTPIN 9 //Declaring pin 9 of arduino for the dht22
#define DHTTYPE DHT22 //Defining which type of dht22 we are using (DHT22 or DHT11)

#define DEGREE_SYMBOL (char)223

//define pins for the LCD
// lcd pin --> Arduino digital pin
#define LCD_RS 7
#define LCD_E  6
#define LCD_D4 5
#define LCD_D5 4
#define LCD_D6 3
#define LCD_D7 2

DHT dht(DHTPIN, DHTTYPE); //Declaring a variable named dht
//LiquidCrystal lcd(12, 11, 10, 9, 8, 7); // Initializing the LCD pins.
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7); // Initializing the LCD pins.

void setup() { //Data written in it will only run once
  Serial.begin(9600); //setting the baud rate at 9600
  lcd.begin(16, 2); // Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display

  dht.begin(); //This command will start to receive the values from dht22

  lcd.setCursor(0, 0);
  lcd.print("HI=");
  lcd.setCursor(0, 1);
  lcd.print("T=");
  lcd.setCursor(10, 1);
  lcd.print("RH=");

  Serial.println("Finished setup!");
}


void loop() { //Data written in it will run again and again
  float hum = dht.readHumidity(); //Reading the humidity and storing in hum
  float temp = dht.readTemperature(); //Reading the temperature as Celsius and storing in temp
  float fah = dht.readTemperature(true);
  if (isnan(hum) || isnan(temp)) {
    // || isnan(fah)
    Serial.println("Failed to read from DHT sensor!");
    lcd.print("Failed to read");
    delay(2000); //Giving a delay of 2 seconds
    return;
  }
  float heat_index = dht.computeHeatIndex(fah, hum);
  float heat_indexC = dht.convertFtoC(heat_index);
  Serial.print("Humidity: "); //Writing the humidity on the serial monitor
  Serial.print(hum); //Writing the humidity value stored in hum on the Serial monitor
  Serial.print(" %\t");
  Serial.print("Temperature: "); //Writing the temperature on the serial monitor
  Serial.print(temp); //Writing the value of temperature on the serial monitor
  Serial.print(" *C ");
  Serial.print(fah);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(heat_indexC);
  Serial.println(" *C ");
  //  Serial.print(heat_index);
  //  Serial.println(" *F ");


  lcd.clear();
  lcd.setCursor(0, 0); //Setting the cursor at the start of the LCD
  lcd.print("*Reading sensor*");
  delay(1000);
  lcd.clear();

  lcd.setCursor(0, 0); //Setting the cursor at the start of the LCD
  lcd.print("T="); //Writing the temperature on the LCD
  lcd.print(String(temp,1)); //Writing the value of temperature on the LCD
  lcd.print("C");
  //  lcd.print(" | ");
  lcd.print(" ");
  lcd.print("TF="); //Writing the temperature on the LCD
  lcd.print(String(fah,1)); //Writing the value of temperature on the LCD
  lcd.print("F");
  //  lcd.print(DEGREE_SYMBOL);
  lcd.setCursor(0, 1); //setting the cursor at next line
  lcd.print("H="); //Writing the humidity on the LCD display
  lcd.print(String(hum,1)); //Writing the humidity value stored in hum on the LCD display
  lcd.print("%");
  lcd.print(" ");
  lcd.print("HR=");
  lcd.print(String(heat_indexC,1));
  lcd.print("C");
  delay(10000); //Giving a delay of 2 seconds
}
