#include <Arduino.h>
#include <cmath>
#include <algorithm>
#include <ESP32Servo.h>
#include "esp_timer.h"
using namespace std;


Servo LM;
Servo RM;
//Servo WM;
constexpr int LM_Pin = 4;  // connect ESC signal wire here
constexpr int RM_Pin = 2;
//constexpr int WM_Pin = 6;

constexpr int ch1_Pin = 13;
constexpr int ch2_Pin = 12;
constexpr int ch3_Pin = 14;

int64_t PTime;
int LPastSpeed;
int RPastSpeed;

double maxAcc = 0.001; //steps (0-500) per microsecond
//0.001 => 0->500 in 0.5 second

int ch1;
int ch2;
int ch3;

struct Vec
{
    double x;
    double y;
};

Vec transformFlipped(const Vec& v)
{
    // Step 0: preserve zero
    if (v.y == 0.0 && v.x == 0.0)
    {
        return {0.0, 0.0};
    }

    // Step 1: rotate by -45 degrees
    constexpr double invSqrt2 = 0.7071067811865475244; // 1 / sqrt(2)

    double xr =  (v.y + v.x) * invSqrt2;
    double yr = (-v.y + v.x) * invSqrt2;

    // Step 2: normalize by dominant component (L∞ norm)
    double maxAbs = std::max(std::fabs(xr), std::fabs(yr));

    xr /= maxAbs;
    yr /= maxAbs;

    // Step 3: remap axes to match required ordering
    // (this is the critical correction)
    return { xr, -yr };
}


// Transforms the controller input vector to right/left motor speeds
Vec transform(const Vec& v)
{
    // Step 1: rotate by -45 degrees
    const double invSqrt2 = 1.0 / sqrt(2.0);

    double xr =  (v.x + v.y) * invSqrt2;
    double yr = (-v.x + v.y) * invSqrt2;

    // Step 2: normalize by max absolute component
    double maxAbs = max(fabs(xr), fabs(yr));

    if (maxAbs == 0.0)
    {
        return {0.0, 0.0};
    }

    double nx = xr / maxAbs;
    double ny = yr / maxAbs;

    // Step 3: fix lower-half ordering
    if (v.y < 0.0)
    {
        std::swap(nx, ny);
    }

    return { nx, ny };
}


/*Pins to AVOID:
GPIO 23, 24, 25, 29

Pins for CAUTION:
GPIO 0, 1, 2, 3, 15*/

//Pin setup
int Ntolerance = 25;
int NtolCH = 30;
int MScale = 500;
bool isFlipped = false;

void setup()
{
    Serial.begin(115200);
    Serial.println();

    LM.attach(LM_Pin, 1000, 2000);  // min/max pulse width in µs
    RM.attach(RM_Pin, 1000, 2000);
    //WM.attach(WM_Pin, 1000, 2000);

    LM.writeMicroseconds(1500);     // neutral
    RM.writeMicroseconds(1500);
    //WM.writeMicroseconds(1500);

    delay(1000);                    // wait for ESC to arm
    Serial.println("Initialized!");

    PTime = esp_timer_get_time();
    LPastSpeed = 1500;
    RPastSpeed = 1500;
    
}

void loop()
{ 

    //Channels always around 1000 and 2000
    ch1 = pulseIn(ch1_Pin, HIGH, 50000);
    ch2 = pulseIn(ch2_Pin, HIGH, 50000);
    delay(1); //DO NOT DELETE!!!
    ch3 = pulseIn(ch3_Pin, HIGH, 50000);


    //Neutralize all motors if arbitrary signal not present
    if(ch1 == 0 || ch2 == 0 || ch3 == 0){
        RM.writeMicroseconds(1500);
        LM.writeMicroseconds(1500);
        //WM.writeMicroseconds(1500);
        Serial.println("No signal");
        return;
    }

    //Enforce deadzone for channels
    (ch1 > (1500-NtolCH) && ch1 < (1500+NtolCH)) ? (ch1 = 1500):(ch1 = ch1);
    (ch2 > (1500-NtolCH) && ch2 < (1500+NtolCH)) ? (ch2 = 1500):(ch2 = ch2);
    (ch3 > (1500-NtolCH) && ch3 < (1500+NtolCH)) ? (ch3 = 1500):(ch3 = ch3);

    //Clamp values to exactly 1000-2000
    (ch1 < 1000) ? (ch1 = 1000):(ch1=ch1);
    (ch2 < 1000) ? (ch2 = 1000):(ch2=ch2);
    (ch3 < 1000) ? (ch3 = 1000):(ch3=ch3);

    (ch1 > 2000) ? (ch1 = 2000):(ch1=ch1);
    (ch2 > 2000) ? (ch2 = 2000):(ch2=ch2);
    (ch3 > 2000) ? (ch3 = 2000):(ch3=ch3);

    // Map x/y channels
    int x = map(ch1, 1000, 2000, -100, 100); 
    int y = map(ch2, 1000, 2000, -100, 100);
    //int weaponSpeed = ch3;
    

    //Check if robot is flipped
    if (ch3 < 1500) isFlipped = false;
    else isFlipped = true;

    Serial.print(String(ch3) + " || ");
    // Calculate motor speeds based on orientation, horizontal, and vertical inputs
    Vec motorSpeeds = {0, 0};
    if(!isFlipped){
        Vec v = {static_cast<double>(x), static_cast<double>(y)};
        motorSpeeds = transform(v);
        Serial.println("Left: " + String(motorSpeeds.x) + " Right: " + String(motorSpeeds.y));
    }
    else{
        Vec v = {static_cast<double>(x), static_cast<double>(y)};
        motorSpeeds = transformFlipped(v);
        Serial.println("(Flipped) -- Left: " + String(motorSpeeds.x) + " Right: " + String(motorSpeeds.y));
    }

    double RMotorSpeed = static_cast<double>(1500.0 + motorSpeeds.y * MScale); // Extract right motor speed and scale to -255 to 255
    double LMotorSpeed = static_cast<double>(1500.0 + motorSpeeds.x * MScale); // Extract left motor speed and scale to -255 to 255
    
    //Clamp speeds to prevent writeMicroseconds() error
    //Should not exceed but being precautionary
    (RMotorSpeed > 2000.0) ? (RMotorSpeed = 2000.0):(RMotorSpeed = RMotorSpeed);
    (RMotorSpeed < 1000.0) ? (RMotorSpeed = 1000.0):(RMotorSpeed = RMotorSpeed);
    (LMotorSpeed > 2000.0) ? (LMotorSpeed = 2000.0):(LMotorSpeed = LMotorSpeed);
    (LMotorSpeed < 1000.0) ? (LMotorSpeed = 1000.0):(LMotorSpeed = LMotorSpeed);


    //Slew Speeds to prevent brownout
    int64_t dt = esp_timer_get_time() - PTime;
    double dsR = static_cast<double>(RMotorSpeed)-static_cast<double>(RPastSpeed);
    double dsL = static_cast<double>(LMotorSpeed)-static_cast<double>(LPastSpeed);

    if (dsR > 0 && dsR/dt > maxAcc){
        RMotorSpeed = RPastSpeed + maxAcc*dt;
    }
    else if(dsR < 0 && dsR/dt < -maxAcc){
        RMotorSpeed = RPastSpeed - maxAcc*dt;
    }

    if (dsL > 0 && dsL/dt > maxAcc){
        LMotorSpeed = LPastSpeed + maxAcc*dt;
    }
    else if(dsL < 0 && dsL/dt < -maxAcc){
        LMotorSpeed = LPastSpeed - maxAcc*dt;
    }

    PTime = esp_timer_get_time();
    RPastSpeed = RMotorSpeed;
    LPastSpeed = LMotorSpeed;

    //DIAGNOSTICS
    ////////////////////////
    // Serial.print("ch1: " + String(ch1) + " ch2: " + String(ch2) + " ch3: " + String(ch3) + " || ");
    // Serial.print("Left Speed: " + String(LMotorSpeed) + " Right Speed: " + String(RMotorSpeed) + " || ");
    // Serial.print("Flipped: " + String(isFlipped) + " || ");
    // //Serial.print("x: " + String(x) + " y: " + String(y) + " || ");
    // Serial.println("Transform x: " + String(motorSpeeds.x) + " Transform y: " + String(motorSpeeds.y));

    ////////////////////////

    
    //Assign speeds with deadzone
    (RMotorSpeed > -Ntolerance && RMotorSpeed < Ntolerance) ? (RM.writeMicroseconds(1500)):(RM.writeMicroseconds(RMotorSpeed));
    (LMotorSpeed > -Ntolerance && LMotorSpeed < Ntolerance) ? (LM.writeMicroseconds(1500)):(LM.writeMicroseconds(LMotorSpeed));

}