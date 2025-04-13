#ifndef MOTORDRIVER_H
#define MOTORDRIVER_H

class MotorDriver {
public:
    MotorDriver(int leftMotorPin1, int leftMotorPin2, int leftPWMPin,
                int rightMotorPin1, int rightMotorPin2, int rightPWMPin);

    void setSpeed(int speed);
    void driveStraight();
    void driveSLine(int frequency);
    void driveCircle(int radius);
    void driveZigzag(int angle);

private:
    int leftMotorPin1, leftMotorPin2, leftPWMPin;
    int rightMotorPin1, rightMotorPin2, rightPWMPin;
    int speed; // Speed variable

    void setMotorSpeed(int leftSpeed, int rightSpeed);
};

#endif

