#include <Arduino.h>
#include <cmath>
#include <algorithm>
#include <ESP32Servo.h>
using namespace std;


Servo LM;
Servo RM;
Servo WM;
int LM_Pin = 22;  // connect ESC signal wire here
int RM_Pin = 21;
int WM_Pin = 19;

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
int Ntolerance = 25;
bool isFlipped = false;

void setup()
{
    Serial.begin(115200);
    Serial.println();

    LM.attach(LM_Pin, 1000, 2000);  // min/max pulse width in µs
    RM.attach(RM_Pin, 1000, 2000);
    WM.attach(WM_Pin, 1000, 2000);

    LM.writeMicroseconds(1500);     // neutral
    RM.writeMicroseconds(1500);
    WM.writeMicroseconds(1500);

    delay(1000);                    // wait for ESC to arm
    Serial.println("Initialized!");
}


void loop()
{ 

    //Channels always around 1000 and 2000
    int ch1 = pulseIn(23, HIGH);
    int ch2 = pulseIn(22, HIGH);
    delay(1); //DO NOT DELETE!!!
    int ch3 = pulseIn(21, HIGH);
    int ch5 = pulseIn(19, HIGH);
    
    //Clamp values to exactly 1000-2000
    (ch1 < 1000) ? (ch1 = 1000):(ch1=ch1);
    (ch2 < 1000) ? (ch2 = 1000):(ch2=ch2);
    (ch3 < 1000) ? (ch3 = 1000):(ch3=ch3);
    (ch5 < 1000) ? (ch5 = 1000):(ch5=ch5);

    (ch1 > 2000) ? (ch1 = 2000):(ch1=ch1);
    (ch2 > 2000) ? (ch2 = 2000):(ch2=ch2);
    (ch3 > 2000) ? (ch3 = 2000):(ch3=ch3);
    (ch5 > 2000) ? (ch5 = 2000):(ch5=ch5);
    

    // Map x/y channels
    int x = map(ch1, 1000, 2000, -100, 100); 
    int y = map(ch2, 1000, 2000, -100, 100);
    int weaponSpeed = ch3;


    // Calculate motor speeds based on horizontal and vertical inputs
    Vec v = {static_cast<double>(x), static_cast<double>(y)};
    Vec motorSpeeds = transform(v);
    

    int RMotorSpeed = static_cast<int>(1000 + motorSpeeds.x * 500.0); // Extract right motor speed and scale to -255 to 255
    int LMotorSpeed = static_cast<int>(1000 + motorSpeeds.y * 500.0); // Extract left motor speed and scale to -255 to 255
    
    //Clamp speeds to prevent writeMicroseconds() error
    //Should not exceed but being precautionary
    (RMotorSpeed > 2000) ? (RMotorSpeed = 2000):(RMotorSpeed = RMotorSpeed);
    (RMotorSpeed < 1000) ? (RMotorSpeed = 1000):(RMotorSpeed = RMotorSpeed);
    (LMotorSpeed > 2000) ? (LMotorSpeed = 2000):(LMotorSpeed = LMotorSpeed);
    (LMotorSpeed < 1000) ? (LMotorSpeed = 1000):(LMotorSpeed = LMotorSpeed);

    Serial.println("ch1: " + String(ch1) + "ch2: " + String(ch2) + "ch3: " + String(ch3) + "ch5: " + String(ch5));
    //Check if robot is flipped
    if (ch5 < 1500) isFlipped = false;
    else isFlipped = true;

    
    //Write to all motors
    if (isFlipped == false){
        //Assign speeds with deadzone
        (RMotorSpeed > -Ntolerance && RMotorSpeed < Ntolerance) ? (RM.writeMicroseconds(RMotorSpeed)):(RM.writeMicroseconds(1500));
        (LMotorSpeed > -Ntolerance && LMotorSpeed < Ntolerance) ? (LM.writeMicroseconds(LMotorSpeed)):(LM.writeMicroseconds(1500));

        //Write to weapon motor
        WM.writeMicroseconds(weaponSpeed);
    }
    else{
        //Flip speeds
        RMotorSpeed += -2*(RMotorSpeed - 1500);
        LMotorSpeed += -2*(LMotorSpeed - 1500);
        weaponSpeed += -2*(weaponSpeed - 1500);

        //Assign FLIPPED speeds with deadzone
        (RMotorSpeed > -Ntolerance && RMotorSpeed < Ntolerance) ? (RM.writeMicroseconds(RMotorSpeed)):(RM.writeMicroseconds(1500));
        (LMotorSpeed > -Ntolerance && LMotorSpeed < Ntolerance) ? (LM.writeMicroseconds(LMotorSpeed)):(LM.writeMicroseconds(1500));

        //Write to weapon motor
        WM.writeMicroseconds(weaponSpeed);
    }    
}