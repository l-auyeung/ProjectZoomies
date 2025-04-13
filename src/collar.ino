#include <Wire.h>
#include <Adafruit_LSM6DSOX.h>
#include <HTTPClient.h>
#include <WiFi.h>

// Enum defining collar state
enum MotionState {
  IDLE,
  ACTIVE
};

// WiFi credentials
const char* WIFI_SSID = "BitHacks";
const char* WIFI_PASS = "BitHacks2025!";

// Firebase endpoints
const String firebaseHistoryUrl = "https://project-zoomies-34419-default-rtdb.firebaseio.com/sensor.json";   // POST - historical data
const String firebaseCurrentUrl = "https://project-zoomies-34419-default-rtdb.firebaseio.com/current.json";  // PUT - latest data only

// Sampling configuration
const unsigned long uploadInterval = 1000; 
const unsigned int sensorReadDelay = 100;    // 100ms delay between sensor readings

// Custom I2C pins for ESP32-S3
#define SDA_PIN 8
#define SCL_PIN 9

// Global Objects
Adafruit_LSM6DSOX lsm6;
HTTPClient http;

// Averaging variables
unsigned long previousUploadMillis = 0;
float activitySum = 0;
unsigned long samplesCount = 0;

// Connect to WiFi with reconnection handling
void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to WiFi");
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries < 20) {
    delay(500);
    Serial.print(".");
    retries++;
  }
  if(WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected. IP: " + WiFi.localIP().toString());
  } else {
    Serial.println("\nFailed to connect WiFi");
  }
}

// Ensure WiFi is connected
void ensureWiFiConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Attempting reconnection...");
    connectWiFi();
  }
}

// Determine state based on average activity level
MotionState determineState(float avgActivityLevel) {
  return (avgActivityLevel < 9.5 || avgActivityLevel > 10.5) ? ACTIVE : IDLE;
}

// Send HTTP request (POST or PUT based on method parameter)
bool sendDataToFirebase(const String& url, const String& jsonPayload, const String& method) {
  ensureWiFiConnected();
  
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
  
  int responseCode;
  if (method == "POST") {
    responseCode = http.POST(jsonPayload);
  } else if (method == "PUT") {
    responseCode = http.PUT(jsonPayload);
  } else {
    Serial.println("Invalid HTTP method");
    http.end();
    return false;
  }

  bool success = false;
  if (responseCode > 0) {
    Serial.println(method + " successful. Response code: " + String(responseCode));
    success = true;
  } else {
    Serial.println(method + " failed. Error code: " + String(responseCode));
  }

  http.end();
  return success;
}

// Prepare JSON payload
String prepareJsonPayload(float avgActivity, MotionState state) {
  String json = "{";
  json += "\"activityLevel\":" + String(avgActivity, 2) + ",";
  json += "\"state\":\"" + String(state == ACTIVE ? "ACTIVE" : "IDLE") + "\",";
  json += "\"timestamp\": {\".sv\": \"timestamp\"}";
  json += "}";
  return json;
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Program starts");

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!lsm6.begin_I2C()) {
    Serial.println("LSM6DSOX initialization failed!");
    while (1) delay(10);
  }
  Serial.println("LSM6DSOX sensor initialized successfully!");

  connectWiFi();

  previousUploadMillis = millis();
}

void loop() {
  sensors_event_t accel, gyro, temp;
  lsm6.getEvent(&accel, &gyro, &temp);

  float activityLevel = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  );

  activitySum += activityLevel;
  samplesCount++;

  unsigned long currentMillis = millis();
  if (currentMillis - previousUploadMillis >= uploadInterval) {
    float avgActivity = activitySum / samplesCount;
    MotionState avgState = determineState(avgActivity);

    Serial.println("--- 30-Second Summary ---");
    Serial.print("Average Activity Level: "); Serial.println(avgActivity);
    Serial.print("Determined State: "); Serial.println(avgState == ACTIVE ? "ACTIVE" : "IDLE");

    String jsonPayload = prepareJsonPayload(avgActivity, avgState);
    sendDataToFirebase(firebaseHistoryUrl, jsonPayload, "POST");
    sendDataToFirebase(firebaseCurrentUrl, jsonPayload, "PUT");

    activitySum = 0;
    samplesCount = 0;
    previousUploadMillis = currentMillis;
  }

  delay(sensorReadDelay);
}
