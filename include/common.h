// common.h
#ifndef COMMON_H
#define COMMON_H

#include <WiFi.h>
#include <HTTPClient.h>

// WiFi Credentials - Replace with your actual credentials
const char* WIFI_SSID = "BitHacks";
const char* WIFI_PASSWORD = "BitHacks2025!";

// Firebase Realtime Database URL
const char* FIREBASE_URL = "https://project-zoomies-34419.firebaseio.com";
const char* FIREBASE_PATH = "/animal_activity"; // Path in Firebase for data

// Function to connect to WiFi
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

#endif // COMMON_H
