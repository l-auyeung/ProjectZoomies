#include <Wire.h>
#include <WiFiConfig.h>
#include <LiquidCrystal_I2C.h>

#define I2C_SDA 8 //pin 8
#define I2C_SCL 9 //pin 9

LiquidCrystal_I2C lcd(0x27, 16, 2);
WeatherReport report;

void setup() {
  Serial.begin(115200);
  
  // Initialize the LCD
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();

  // Set up communication between transmitter
  wifi_setup();
  set_report_receiver(&report);

}

void loop() {

  // Clear and set cursor at (0,0)
  lcd.clear();
  lcd.setCursor(0,0);

  // Print temp
  lcd.print("Temp:");
  lcd.print(report.temperature);
  lcd.print("C");

  //Print Humidity
  lcd.setCursor(0,1);
  lcd.print("Hum:");
  lcd.print(report.humidity);
  lcd.print("%,");

  //Print time of day
  if (report.light == 0) {
    lcd.print("Day");       
  }else
    lcd.print("Night");

    delay(1000);
}
