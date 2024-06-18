#include <SoftwareSerial.h>
#define TX_PIN 7  //chân D7 arduino có chức năng của TX
#define RX_PIN 6  // chân D6 arduino có chức năng của RX
#include "DHT.h"
#define dataPin 5  // Defines pin number to which the sensor is connected
#define DHTTYPE DHT11
String data;
int led = 4;
int flag = 0;
SoftwareSerial bluetooth(RX_PIN, TX_PIN);  //RX_PIN Arduino nối vào chân TX của HC_06 và TX_PIN Arduino nối với RX của HC_06
DHT dht(dataPin, DHTTYPE);                 // Creats a DHT object
void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  dht.begin();

  pinMode(led, OUTPUT);
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);

  digitalWrite(led, HIGH);
  Serial.println("Bluetooth On please press 1 or 0 blink LED");
}
void loop() {

  data = "read";
  if (bluetooth.available() > 0) {
    char c = bluetooth.read();
    switch (c) {
      case '1':
        data = "on";
        break;
      case '0':
        data = "off";
        break;
      case '2':
        data = "read";
        break;
      default:
        break;
    }
  }

  if (data.length() > 0) {
    if (data == "on" || data == "1") {
      digitalWrite(led, HIGH);
      Serial.println("LED On");
      //led1=on
      bluetooth.println("on");
      data = "";
    } else if (data == "off" || data == "0") {
      digitalWrite(led, LOW);
      Serial.println("LED Off");
      //led1=off
      bluetooth.println("off");
      data = "";
    } else if (data == "read") {
      float h = dht.readHumidity();
      // Read temperature as Celsius (the default)
      float t = dht.readTemperature();
      Serial.print("Humidity: ");
      Serial.print(h);
      Serial.println("%");
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.println("*C");
      
      Serial.println("send data ...");
      bluetooth.print("Humidity: ");
      bluetooth.print(h);
      bluetooth.print(" %\t");
      bluetooth.print("Temperature: ");
      bluetooth.print(t);
      bluetooth.println(" *C");

      delay(5000);
    }
  }
}