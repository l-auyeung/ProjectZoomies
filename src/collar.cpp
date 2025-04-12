#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_LSM6DS33.h> // Or your LSM6DSOX library
#include "common.h"
#include <ArduinoJson.h> // For potential future JSON formatting if needed, not strictly needed for simple value push

Adafruit_LSM6DS33 lsm6ds; // Adjust if you are using a different library or class name.

void setup() {
  Serial.begin(115200);
  connectWiFi();

  Serial.println("Starting LSM6DSOX sensor");
  if (!lsm6ds.begin_I2C()) { // or begin_SPI(CS_PIN)
    Serial.println("Failed to initialize LSM6DSOX sensor!");
    while (1);
  }
  Serial.println("Found LSM6DSOX sensor!");
}

void loop() {
  sensors_event_t accel, gyro, temp;
  lsm6ds.getEvent(&accel, &gyro, &temp);

  // Calculate activity level (simple magnitude of acceleration)
  float activityLevel = sqrt(pow(accel.acceleration.x, 2) + pow(accel.acceleration.y, 2) + pow(accel.acceleration.z, 2));

  Serial.print("Activity Level: ");
  Serial.println(activityLevel);

  // Post activity level to Firebase
  postActivityLevel(activityLevel);

  delay(2000); // Send data every 2 seconds
}

void postActivityLevel(float activityLevel) {
  HTTPClient http;

  String firebaseEndpoint = String(FIREBASE_URL) + FIREBASE_PATH + ".json"; // Construct the endpoint URL

  http.begin(firebaseEndpoint.c_str());
  http.addHeader("Content-Type", "application/json");

  // Create JSON payload (even for a single value, Firebase expects JSON)
  String httpRequestData = "{\"activity_level\": " + String(activityLevel) + "}";

  Serial.print("Sending to Firebase: ");
  Serial.println(httpRequestData);

  int httpResponseCode = http.PUT(httpRequestData); // Use PUT to update the value at the path

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    if (httpResponseCode == HTTP_CODE_OK || httpResponseCode == HTTP_CODE_ACCEPTED) {
      String response = http.getString();
      Serial.println("Response from Firebase:");
      Serial.println(response);
    } else {
      Serial.print("Error posting to Firebase, code: ");
      Serial.println(httpResponseCode);
    }
  } else {
    Serial.print("Error on HTTP request: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }
  http.end();
}
