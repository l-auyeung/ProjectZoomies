#include <Adafruit_LSM6DSOX.h>
#include <Wire.h>
#include "SharedConfig.h"

SharedConfig config;
Adafruit_LSM6DSOX lsm6dsox;

void setup() {
    Serial.begin(115200);
    config.connectWiFi();
    Wire.begin();

    if (!lsm6dsox.begin_I2C()) {
        Serial.println("LSM6DSOX sensor not found!");
        while (1) delay(10);
    }
    Serial.println("LSM6DSOX initialized.");
}

void loop() {
    sensors_event_t accel, gyro, temp;
    lsm6dsox.getEvent(&accel, &gyro, &temp);

    // Simple activity metric (sum of absolute values as example)
    float activityLevel = abs(accel.acceleration.x) + abs(accel.acceleration.y) + abs(accel.acceleration.z);

    Serial.println("Activity Level: " + String(activityLevel));

    // Prepare JSON data
    StaticJsonDocument<200> doc;
    doc["activityLevel"] = activityLevel;
    doc["timestamp"] = millis(); // simple timestamp

    String jsonPayload;
    serializeJson(doc, jsonPayload);

    // POST data to Firebase
    String endpoint = FIREBASE_URL + "/collar/activity.json"; // chosen endpoint structure
    config.http.begin(endpoint);
    config.http.addHeader("Content-Type", "application/json");

    int responseCode = config.http.POST(jsonPayload);
    Serial.println("Firebase POST Response: " + String(responseCode));

    config.http.end();

    delay(5000); // send data every 5 seconds
}
