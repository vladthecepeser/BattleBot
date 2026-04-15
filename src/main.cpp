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
int WMotor_IN1 = 8;
int WMotor_IN2 = 9;
int Ntolerance = 25;
bool isFlipped = false;

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
    pinMode(WMotor_IN1, OUTPUT);
    pinMode(WMotor_IN2, OUTPUT);
}


void loop()
{ 
    IBus.loop(); // must be called often

    //Channels always between 1000 and 2000
    int ch1 = IBus.readChannel(0); // Right Left/Right
    int ch2 = IBus.readChannel(1); // Right Up/Down
    int ch3 = IBus.readChannel(2); // Left Up/Down (Motor Speed)
    int ch5 = IBus.readChannel(4); // potentiometer (VRA)
  
    
    // Map channels
    int x = map(ch1, 1000, 2000, -100, 100); 
    int y = map(ch2, 1000, 2000, -100, 100);
    int weaponSpeed = map(ch3, 1000, 2000, 0, 255); //Weapon one-directional


    // Calculate motor speeds based on horizontal and vertical inputs
    Vec v = {static_cast<double>(x), static_cast<double>(y)};
    Vec motorSpeeds = transform(v);
    

    int RMotorSpeed = static_cast<int>(motorSpeeds.x * 255.0); // Extract right motor speed and scale to -255 to 255
    int LMotorSpeed = static_cast<int>(motorSpeeds.y * 255.0); // Extract left motor speed and scale to -255 to 255
    
    //Clamp speeds to prevent analogWrite() error
    (RMotorSpeed > 255) ? (RMotorSpeed = 255):(RMotorSpeed = RMotorSpeed);
    (RMotorSpeed < -255) ? (RMotorSpeed = -255):(RMotorSpeed = RMotorSpeed);
    (LMotorSpeed > 255) ? (LMotorSpeed = 255):(LMotorSpeed = LMotorSpeed);
    (LMotorSpeed < -255) ? (LMotorSpeed = -255):(LMotorSpeed = LMotorSpeed);


    //Check if robot is flipped
    if (ch5 < 1500) isFlipped = false;
    else isFlipped = true;

    
    //Write to all motors
    if (isFlipped == false){
        //Write the speed to the right motor
        if (RMotorSpeed > -Ntolerance && RMotorSpeed < Ntolerance){
            digitalWrite(RMotor_IN1, LOW);
            digitalWrite(RMotor_IN2, LOW);}
        else if (RMotorSpeed > 0){
            analogWrite(RMotor_IN1, RMotorSpeed); // Forward
            digitalWrite(RMotor_IN2, LOW);}
        else if (RMotorSpeed < 0) {
            digitalWrite(RMotor_IN1, LOW); // Backward
            analogWrite(RMotor_IN2, -RMotorSpeed);}
        
        //Write the speed to the left motor
        if (LMotorSpeed > -Ntolerance && LMotorSpeed < Ntolerance){
            digitalWrite(LMotor_IN1, LOW);
            digitalWrite(LMotor_IN2, LOW);}
        else if (LMotorSpeed > 0) {
            analogWrite(LMotor_IN1, LMotorSpeed); // Forward
            digitalWrite(LMotor_IN2, LOW);} 
        else if (LMotorSpeed < 0) {
            digitalWrite(LMotor_IN1, LOW); // Backward
            analogWrite(LMotor_IN2, -LMotorSpeed);}

        //Write to weapon motor
        analogWrite(WMotor_IN1, weaponSpeed); 
        digitalWrite(WMotor_IN2, LOW);
    }
    else{
        if (RMotorSpeed > -Ntolerance && RMotorSpeed < Ntolerance){
            digitalWrite(RMotor_IN1, LOW);
            digitalWrite(RMotor_IN2, LOW);}
        else if (RMotorSpeed > 0){
            analogWrite(RMotor_IN1, LOW); // Forward (flipped)
            digitalWrite(RMotor_IN2, RMotorSpeed);}
        else if (RMotorSpeed < 0) {
            digitalWrite(RMotor_IN1, -RMotorSpeed); // Backward (flipped)
            analogWrite(RMotor_IN2, LOW);}
        
        //Write the speed to the left motor
        if (LMotorSpeed > -Ntolerance && LMotorSpeed < Ntolerance){
            digitalWrite(LMotor_IN1, LOW);
            digitalWrite(LMotor_IN2, LOW);}
        else if (LMotorSpeed > 0) {
            analogWrite(LMotor_IN1, LOW); // Forward (flipped)
            digitalWrite(LMotor_IN2, LMotorSpeed);} 
        else if (LMotorSpeed < 0) {
            digitalWrite(LMotor_IN1, -LMotorSpeed); // Backward (flipped)
            analogWrite(LMotor_IN2, LOW);}

        //Write to weapon motor (flipped)
        analogWrite(WMotor_IN1, LOW); 
        digitalWrite(WMotor_IN2, weaponSpeed);
    }

    
}