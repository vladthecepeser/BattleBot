#include <Arduino.h>
#include <cmath>
#include <algorithm>
#include "IBusPico.h"

IBusPico IBus;

struct Vec
{
    double x;
    double y;
};

// Transforms the controller input vector to right/left motor speeds
Vec transform(const Vec& v)
{
    // Step 1: rotate by -45 degrees
    const double invSqrt2 = 1.0 / sqrt(2.0);

    double xr =  (v.x + v.y) * invSqrt2;
    double yr = (-v.x + v.y) * invSqrt2;

    // Step 2: normalize by max absolute component
    double maxAbs = max(abs(xr), fabs(yr));

    // Guard against zero (just in case)
    if (maxAbs == 0.0)
    {
        return {0.0, 0.0};
    }

    return { xr / maxAbs, yr / maxAbs };
}


/*Pins to AVOID:
GPIO 23, 24, 25, 29

Pins for CAUTION:
GPIO 0, 1, 2, 3, 15*/

//Pin setup
int RMotor_IN1 = 4;
int RMotor_IN2 = 5;
int LMotor_IN1 = 6;
int LMotor_IN2 = 7; 

void setup()
{
    Serial.begin(115200);
    Serial.println();
  
    Serial1.begin(115200);   // iBus baudrate
    IBus.begin(Serial1);

    pinMode(RMotor_IN1, OUTPUT);
    pinMode(RMotor_IN2, OUTPUT);
    pinMode(LMotor_IN1, OUTPUT);
    pinMode(LMotor_IN2, OUTPUT);
}


void loop()
{ 
    IBus.loop();             // must be called often

    //Channels always between 1000 and 2000
    int ch1 = IBus.readChannel(0); // Right Left/Right
    int ch2 = IBus.readChannel(1); // Right Up/Down
    int ch3 = IBus.readChannel(2); // Left Up/Down
    int ch5 = IBus.readChannel(4); // potentiometer (VRA)
  
    

    int x = map(ch1, 1000, 2000, -100, 100); // Map ch1 to a heading value between -100 and 100
    int y = map(ch2, 1000, 2000, -100, 100);


    // Calculate motor speeds based on horizontal and vertical inputs
    Vec v = {static_cast<double>(x), static_cast<double>(y)};
    Vec motorSpeeds = transform(v);
    

    int RMotorSpeed = static_cast<int>(motorSpeeds.x * 255.0); // Extract right motor speed and scale to -100 to 100
    int LMotorSpeed = static_cast<int>(motorSpeeds.y * 255.0); // Extract left motor speed and scale to -100 to 100
    
    
    //
    if (RMotorSpeed > 0) {
        analogWrite(RMotor_IN1, RMotorSpeed); // Forward
        digitalWrite(RMotor_IN2, LOW);
    } else if (RMotorSpeed < 0) {
        digitalWrite(RMotor_IN1, LOW); // Backward
        analogWrite(RMotor_IN2, RMotorSpeed);
    }

    
    if (LMotorSpeed > 0) {
        analogWrite(LMotor_IN1, LMotorSpeed); // Forward
        digitalWrite(LMotor_IN2, LOW);
    } else if (LMotorSpeed < 0) {
        digitalWrite(LMotor_IN1, LOW); // Backward
        analogWrite(LMotor_IN2, LMotorSpeed);
    }

    
}