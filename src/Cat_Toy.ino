#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP32Servo.h>
#include <HardwareSerial.h>
#include <DFRobotDFPlayerMini.h>

#define MOTOR_1A 10
#define MOTOR_2A 11
#define MOTOR_1B 12
#define MOTOR_2B 13
#define SERVO_PIN 4
const int PIN_MP3_RX = 44; // Connects to DFPlayer TX
const int PIN_MP3_TX = 43; // Connects to DFPlayer RX

const char* WIFI_SSID = "BitHacks";
const char* WIFI_PASS = "BitHacks2025!";
const char* firebaseHost = "project-zoomies-34419-default-rtdb.firebaseio.com";
const int firebasePort = 443;
const char* firebasePath = "/current.json";

WiFiClientSecure client;
Servo tailServo;
HardwareSerial mySerial1(1);
DFRobotDFPlayerMini player;

bool isActive = false;
bool isWagging = false;
unsigned long lastFetchTime = 0;
unsigned long lastMoveTime = 0;
unsigned long lastWagTime = 0;
unsigned long lastAudioTime = 0;
int wagDirection = 0;
int currentTrack = 1;
const int totalTracks = 6;

const unsigned long fetchInterval = 1000;
const unsigned long moveInterval = 400;
const unsigned long wagInterval = 60;
const unsigned long trackInterval = 10000; // 10 seconds between random track changes

void setup() {
  Serial.begin(115200);

  pinMode(MOTOR_1A, OUTPUT);
  pinMode(MOTOR_1B, OUTPUT);
  pinMode(MOTOR_2A, OUTPUT);
  pinMode(MOTOR_2B, OUTPUT);
  digitalWrite(MOTOR_1A, LOW);
  digitalWrite(MOTOR_1B, LOW);
  digitalWrite(MOTOR_2A, LOW);
  digitalWrite(MOTOR_2B, LOW);

  tailServo.attach(SERVO_PIN);
  tailServo.write(90);

  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.println("IP Address: " + WiFi.localIP().toString());

  client.setInsecure();
  randomSeed(analogRead(0));

  mySerial1.begin(9600, SERIAL_8N1, PIN_MP3_RX, PIN_MP3_TX);
  delay(1000);
  if (!player.begin(mySerial1)) {
    Serial.println("DFPlayer connection failed!");
  } else {
    player.volume(15);
    Serial.println("DFPlayer Ready");
  }
}

void loop() {
  unsigned long now = millis();

  if (now - lastFetchTime > fetchInterval) {
    lastFetchTime = now;
    if (client.connect(firebaseHost, firebasePort)) {
      client.print(String("GET ") + firebasePath + " HTTP/1.1\r\n" +
                   "Host: " + firebaseHost + "\r\n" +
                   "Connection: close\r\n\r\n");

      while (client.connected() && !client.available()) delay(10);
      while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") break;
      }

      String payload = "";
      while (client.available()) {
        payload += client.readString();
      }
      client.stop();

      Serial.println("\xF0\x9F\x93\xA9 Payload:");
      Serial.println(payload);

      if (payload == "null" || payload.length() < 10) {
        isActive = false;
        isWagging = false;
      } else {
        String searchKey = "\"state\":\"";
        int start = payload.indexOf(searchKey);
        if (start != -1) {
          start += searchKey.length();
          int end = payload.indexOf("\"", start);
          String state = payload.substring(start, end);
          Serial.print("\xF0\x9F\x93\xA1 State: ");
          Serial.println(state);

          if (state.equalsIgnoreCase("active")) {
            isActive = true;
            isWagging = true;
          } else {
            isActive = false;
            isWagging = false;
          }
        }
      }
    }
  }

  if (isWagging && now - lastWagTime > wagInterval) {
    lastWagTime = now;
    tailServo.write(wagDirection == 0 ? 0 : 180);
    wagDirection = 1 - wagDirection;
  } else if (!isWagging) {
    tailServo.write(90);
  }

  if (isActive && now - lastMoveTime > moveInterval) {
    lastMoveTime = now;
    int moveType = random(0, 3);
    switch (moveType) {
      case 0:
        digitalWrite(MOTOR_1A, HIGH); digitalWrite(MOTOR_2A, LOW);
        digitalWrite(MOTOR_1B, LOW); digitalWrite(MOTOR_2B, HIGH);
        break;
      case 1:
        digitalWrite(MOTOR_1A, LOW); digitalWrite(MOTOR_2A, LOW);
        digitalWrite(MOTOR_1B, LOW); digitalWrite(MOTOR_2B, HIGH);
        break;
      case 2:
        digitalWrite(MOTOR_1A, HIGH); digitalWrite(MOTOR_2A, LOW);
        digitalWrite(MOTOR_1B, LOW); digitalWrite(MOTOR_2B, LOW);
        break;
    }
  } else if (!isActive) {
    digitalWrite(MOTOR_1A, LOW);
    digitalWrite(MOTOR_1B, LOW);
    digitalWrite(MOTOR_2A, LOW);
    digitalWrite(MOTOR_2B, LOW);
  }

  // Random Track Playback (only while active)
  if (isActive && now - lastAudioTime > trackInterval) {
    lastAudioTime = now;
    int randomTrack = random(1, totalTracks + 1);
    player.play(randomTrack);
    Serial.print("\xF0\x9F\x8E\xB5 Playing Track ");
    Serial.println(randomTrack);
  }
}