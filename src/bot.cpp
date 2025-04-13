#include "SharedConfig.h"
#include "MotorDriver.h"

SharedConfig config;

enum BotState {
    EVADE,
    SPIN,
    STOP,
    UNKNOWN
};

BotState currentState = UNKNOWN;
MotorDriver motorDriver(2, 4, 5,  // Left motor pins: IN1, IN2, PWM
                        3, 7, 6); // Right motor pins: IN1, IN2, PWM

void setup() {
    Serial.begin(115200);
    config.connectWiFi();
    motorDriver.setSpeed(150); // Set initial speed
    motorDriver.driveStraight(); // Initialize motors to stop
}

BotState parseGeminiResponse(const String& payload) {
    StaticJsonDocument<512> doc;
    deserializeJson(doc, payload);
    
    String state = doc["candidates"][0]["content"]["parts"][0]["text"];
    state.trim();
    
    if (state == "evade") return EVADE;
    if (state == "spin") return SPIN;
    if (state == "stop") return STOP;

    return UNKNOWN;
}

BotState getActivityBasedState(float activityLevel) {
    StaticJsonDocument<500> requestDoc;

    String prompt = "Given the animal's activity level (" + String(activityLevel) + "), choose the bot state from ['evade','spin','stop']:\n";
    requestDoc["contents"][0]["parts"][0]["text"] = prompt;
    requestDoc["generationConfig"]["response_mime_type"] = "application/json";
    requestDoc["generationConfig"]["response_schema"]["type"] = "STRING";
    String requestPayload;
    serializeJson(requestDoc, requestPayload);

    config.http.begin(GEMINI_ENDPOINT);
    config.http.addHeader("Content-Type", "application/json");

    int code = config.http.POST(requestPayload);
    String response = config.http.getString();

    config.http.end();

    if (code == 200) {
        return parseGeminiResponse(response);
    } else {
        Serial.println("Gemini API Error: " + String(code));
        return UNKNOWN;
    }
}

void loop() {
    // GET activity data from Firebase
    String endpoint = FIREBASE_URL + "/collar/activity.json?orderBy=\"$key\"&limitToLast=1";
    config.http.begin(endpoint);

    int code = config.http.GET();
    String payload = config.http.getString();

    config.http.end();

    if (code == 200) {
        StaticJsonDocument<256> doc;
        deserializeJson(doc, payload);
        
        JsonObject root = doc.as<JsonObject>();
        float latestActivity = 0.0;
        for (JsonPair kv : root) {
            latestActivity = kv.value()["activityLevel"];
        }

        Serial.println("Latest Activity Level: " + String(latestActivity));

        // Get state from Gemini
        BotState newState = getActivityBasedState(latestActivity);
        if (newState != currentState && newState != UNKNOWN) {
            currentState = newState;
            switch(currentState) {
                case EVADE:
                    Serial.println("State: EVADE");
                    motorDriver.driveZigzag(50);
                    break;
                case SPIN:
                    Serial.println("State: SPIN");
                    motorDriver.driveCircle(10);
                    break;
                case STOP:
                    Serial.println("State: STOP");
                    motorDriver.driveStraight();
                    break;
                default:
                    Serial.println("Unknown state received.");
                    break;
            }
        } else {
            Serial.println("No state change or unknown state.");
        }
    } else {
        Serial.println("Error fetching activity from Firebase: " + String(code));
    }

    delay(5000); // fetch new data every 5 seconds
}
