#include <SoftwareSerial.h>
#include <DHT.h>

#define TX_PIN 7 //chân D7 arduino có chức năng của TX
#define RX_PIN 6 // chân D6 arduino có chức năng của RX
#define DHTPIN 5 // Define the DHT11 sensor pin
#define DHTTYPE DHT11 // Define the sensor type

DHT dht(DHTPIN, DHTTYPE);
SoftwareSerial bluetooth(RX_PIN, TX_PIN); // RX_PIN Arduino nối vào chân TX của HC_06/HC05 và TX_PIN Arduino nối với RX của HC_06/HC_05

void setup() {
  Serial.begin(9600);
  bluetooth.begin(9600);
  
  pinMode(RX_PIN, INPUT);
  pinMode(TX_PIN, OUTPUT);
  
  dht.begin(); // Initialize the DHT sensor
  
  Serial.println("Bluetooth On. Transmitting DHT11 sensor data...");
}

void loop() {
  // Wait a few seconds between measurements.
  delay(2000);
  
  // Read humidity and temperature values
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Print the data to Serial Monitor
  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.println(" *C");
  
  // Send the data via Bluetooth
  bluetooth.print("Humidity: ");
  bluetooth.print(h);
  bluetooth.print(" %\t");
  bluetooth.print("Temperature: ");
  bluetooth.print(t);
  bluetooth.println(" *C");
}
