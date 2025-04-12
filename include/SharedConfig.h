#ifndef SHAREDCONFIG_H
#define SHAREDCONFIG_H

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASS";

// Firebase details
const String FIREBASE_URL = "https://project-zoomies-34419.firebaseio.com";

// Gemini API details
const String GEMINI_API_KEY = "YOUR_GEMINI_API_KEY";
const String GEMINI_ENDPOINT = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + GEMINI_API_KEY;

class SharedConfig {
public:
    void connectWiFi() {
        Serial.print("Connecting to WiFi");
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        while (WiFi.status() != WL_CONNECTED) {
            delay(500);
            Serial.print(".");
        }
        Serial.println("\nConnected. IP: " + WiFi.localIP().toString());
    }

    HTTPClient http;
};

#endif
