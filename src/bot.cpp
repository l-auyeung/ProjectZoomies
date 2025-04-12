#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "common.h"

// Gemini API Key - Replace with your actual Gemini API key
const char* GEMINI_API_KEY = "YOUR_GEMINI_API_KEY"; // IMPORTANT: Securely manage your API key.

// Robot States Enum
enum RobotState {
  STOP,
  EVADE,
  SPIN
};

RobotState currentRobotState = STOP; // Initial state

void setup() {
  Serial.begin(115200);
  connectWiFi();
  Serial.println("Bot ESP32 started");
}

void loop() {
  float activityLevel = getActivityLevelFromFirebase();
  Serial.print("Activity Level from Firebase: ");
  Serial.println(activityLevel);

  if (activityLevel != -1.0) { // -1.0 indicates error fetching data
    RobotState newState = getRobotStateFromGemini(activityLevel);
    if (newState != currentRobotState) {
      currentRobotState = newState;
      Serial.print("Robot State changed to: ");
      Serial.println(stateToString(currentRobotState));
      // Implement robot actions based on the new state here
      performRobotAction(currentRobotState);
    } else {
      Serial.print("Robot State remains: ");
      Serial.println(stateToString(currentRobotState));
    }
  }

  delay(5000); // Check every 5 seconds
}

float getActivityLevelFromFirebase() {
  HTTPClient http;
  String firebaseEndpoint = String(FIREBASE_URL) + FIREBASE_PATH + "/activity_level.json"; // Get specific activity_level node

  http.begin(firebaseEndpoint.c_str());
  int httpResponseCode = http.GET();

  if (httpResponseCode > 0) {
    if (httpResponseCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.print("Firebase Payload: ");
      Serial.println(payload);
      float activity = payload.toFloat(); // Directly convert the string value to float
      http.end();
      return activity;
    } else {
      Serial.print("Error fetching activity level from Firebase, code: ");
      Serial.println(httpResponseCode);
    }
  } else {
    Serial.print("Error on HTTP request to Firebase: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }
  http.end();
  return -1.0; // Return -1.0 to indicate error
}

RobotState getRobotStateFromGemini(float activityLevel) {
  HTTPClient http;
  String geminiEndpoint = "https://generativelanguage.googleapis.com/v1beta/models/gemini-2.0-flash:generateContent?key=" + String(GEMINI_API_KEY);

  http.begin(geminiEndpoint.c_str());
  http.addHeader("Content-Type", "application/json");

  // Construct Gemini API request payload
  String promptText = "The animal activity level is " + String(activityLevel) + ". Based on this level, choose one of the following robot states: EVADE, SPIN, STOP. Output ONLY the state name as a string in JSON format.";
  String httpRequestData;

  // JSON Schema for Gemini response
  String responseSchemaJson =
    "{"
    "  \"type\": \"OBJECT\","
    "  \"properties\": {"
    "    \"robot_state\": {"
    "      \"type\": \"STRING\","
    "      \"enum\": [\"EVADE\", \"SPIN\", \"STOP\"]"
    "    }"
    "  },"
    "  \"required\": [\"robot_state\"]"
    "}";


  // Construct the full JSON request payload for Gemini
  DynamicJsonDocument doc(1024); // Adjust buffer size if needed
  JsonObject contentsArr = doc.createNestedArray("contents").createNestedObject();
  JsonObject partsArr = contentsArr.createNestedArray("parts").createNestedObject();
  partsArr["text"] = promptText;

  JsonObject generationConfig = doc.createNestedObject("generationConfig");
  generationConfig["response_mime_type"] = "application/json";
  JsonObject responseSchema = generationConfig.createNestedObject("response_schema");
  deserializeJson(responseSchema, responseSchemaJson); // Parse the schema JSON string

  serializeJson(doc, httpRequestData);

  Serial.print("Gemini Request Payload: ");
  Serial.println(httpRequestData);

  int httpResponseCode = http.POST(httpRequestData);

  if (httpResponseCode > 0) {
    if (httpResponseCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.print("Gemini Response Payload: ");
      Serial.println(payload);

      DynamicJsonDocument responseDoc(1024); // Adjust buffer size as needed
      DeserializationError error = deserializeJson(responseDoc, payload);

      if (error) {
        Serial.print("JSON Deserialization Error: ");
        Serial.println(error.c_str());
        http.end();
        return STOP; // Default to STOP in case of parsing error
      }

      if (responseDoc["candidates"] && responseDoc["candidates"][0]["content"]["parts"][0]["text"]) {
        String stateStr = responseDoc["candidates"][0]["content"]["parts"][0]["text"].as<String>();
        stateStr.trim(); // Remove potential whitespace

        if (stateStr.equalsIgnoreCase("EVADE")) return EVADE;
        else if (stateStr.equalsIgnoreCase("SPIN")) return SPIN;
        else if (stateStr.equalsIgnoreCase("STOP")) return STOP;
        else {
          Serial.println("Unknown state from Gemini: " + stateStr);
          return STOP; // Default to STOP for unknown state
        }
      } else {
        Serial.println("Unexpected Gemini response format (no text in parts).");
        return STOP; // Default to STOP if response format is unexpected
      }

    } else {
      Serial.print("Error from Gemini API, code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.print("Error Payload: ");
      Serial.println(payload);
    }
  } else {
    Serial.print("Error on HTTP request to Gemini API: ");
    Serial.println(http.errorToString(httpResponseCode).c_str());
  }
  http.end();
  return STOP; // Default to STOP in case of any error
}


void performRobotAction(RobotState state) {
  switch (state) {
    case STOP:
      Serial.println("Robot action: STOP");
      // Implement STOP robot movement here
      break;
    case EVADE:
      Serial.println("Robot action: EVADE");
      // Implement EVADE robot movement here
      break;
    case SPIN:
      Serial.println("Robot action: SPIN");
      // Implement SPIN robot movement here
      break;
  }
}

String stateToString(RobotState state) {
  switch (state) {
    case STOP: return "STOP";
    case EVADE: return "EVADE";
    case SPIN: return "SPIN";
    default: return "UNKNOWN";
  }
}
