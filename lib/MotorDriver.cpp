#include "MotorDriver.h"
#include <Arduino.h>

MotorDriver::MotorDriver(int leftMotorPin1, int leftMotorPin2, int leftPWMPin,
                         int rightMotorPin1, int rightMotorPin2, int rightPWMPin)
    : leftMotorPin1(leftMotorPin1), leftMotorPin2(leftMotorPin2), leftPWMPin(leftPWMPin),
      rightMotorPin1(rightMotorPin1), rightMotorPin2(rightMotorPin2), rightPWMPin(rightPWMPin), speed(0) {
    pinMode(leftMotorPin1, OUTPUT);
    pinMode(leftMotorPin2, OUTPUT);
    pinMode(leftPWMPin, OUTPUT);
    pinMode(rightMotorPin1, OUTPUT);
    pinMode(rightMotorPin2, OUTPUT);
    pinMode(rightPWMPin, OUTPUT);
}

void MotorDriver::setSpeed(int newSpeed) {
    speed = constrain(newSpeed, 0, 255);
}

void MotorDriver::setMotorSpeed(int leftSpeed, int rightSpeed) {
    // Set direction for left motor
    if (leftSpeed >= 0) {
        digitalWrite(leftMotorPin1, HIGH);
        digitalWrite(leftMotorPin2, LOW);
    } else {
        digitalWrite(leftMotorPin1, LOW);
        digitalWrite(leftMotorPin2, HIGH);
        leftSpeed = -leftSpeed;
    }

    // Set direction for right motor
    if (rightSpeed >= 0) {
        digitalWrite(rightMotorPin1, HIGH);
        digitalWrite(rightMotorPin2, LOW);
    } else {
        digitalWrite(rightMotorPin1, LOW);
        digitalWrite(rightMotorPin2, HIGH);
        rightSpeed = -rightSpeed;
    }

    // Set PWM speed
    analogWrite(leftPWMPin, constrain(leftSpeed, 0, 255));
    analogWrite(rightPWMPin, constrain(rightSpeed, 0, 255));
}

void MotorDriver::driveStraight() {
    setMotorSpeed(speed, speed);
}

void MotorDriver::driveSLine(int frequency) {
    unsigned long currentTime = millis();
    int offset = sin(currentTime * frequency * 0.001) * speed / 2;
    setMotorSpeed(speed + offset, speed - offset);
}

void MotorDriver::driveCircle(int radius) {
    int leftSpeed = speed;
    int rightSpeed = speed * (radius - 1) / radius;
    setMotorSpeed(leftSpeed, rightSpeed);
}

void MotorDriver::driveZigzag(int angle) {
    static bool direction = true;
    static unsigned long lastChange = 0;
    unsigned long currentTime = millis();

    if (currentTime - lastChange > 1000) { // Change direction every second
        direction = !direction;
        lastChange = currentTime;
    }

    int offset = direction ? angle : -angle;
    setMotorSpeed(speed + offset, speed - offset);
}
