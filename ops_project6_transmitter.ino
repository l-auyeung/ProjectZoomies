#include <WiFiConfig.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>

// Photoresistor
const int photoresistor = 4;
const int light_threshold = 1000;

// Define I2C pins
#define I2C_SDA 6
#define I2C_SCL 7
WeatherReport report;
Adafruit_AHTX0 aht;

//MAC Address: 34:b7:da:f6:38:28
uint8_t receiverAddress[] = {0x34, 0xB7, 0xDA, 0xF6, 0x38, 0x28};

void setup() {

  // Begin Serial
  Serial.begin(115200);

  // Configure I2C for pins 6 and 7
  Wire.begin(I2C_SDA, I2C_SCL);
  aht.begin();

  // Setup Wifi
  wifi_setup();

  // Setup communication between transmitter and receiver
  peer_setup(receiverAddress);

}

void loop() {

  // Setup sensors
  sensors_event_t humidity, temp;

  // Read light level
  int lightValue = analogRead(photoresistor);

  //Check if it is day or night
  if(lightValue > 1000) {
    report.light = 0;     // Set lightValue to 0 for day
  }else
    report.light = 1;     // Set lightValue to 1 for night

  // Get data
  aht.getEvent(&humidity, &temp);
  report.humidity = humidity.relative_humidity;
  report.temperature = temp.temperature;

  //Send data to receiver
  send_report(receiverAddress, report);
  
  //print the values
      Serial.print("Temp: ");
    Serial.println(report.temperature);
    Serial.print("Hun: ");
    Serial.println(report.humidity);
    Serial.print("Light: ");
    Serial.println(report.light);

  // Every 10 seconds
  delay(10000);

}
