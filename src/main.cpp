#include <Arduino.h>
#include <cmath>
#include <algorithm>
#include <ESP32Servo.h>
using namespace std;


Servo LM;
Servo RM;
Servo WM;
constexpr int LM_Pin = 12;  // connect ESC signal wire here
constexpr int RM_Pin = 14;
constexpr int WM_Pin = 27;

constexpr int ch1_Pin = 22;
constexpr int ch2_Pin = 21;
constexpr int ch3_Pin = 19;
constexpr int ch5_Pin = 18;

int ch1;
int ch2;
int ch3;
int ch5;

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
int NtolCH = 30;
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
    ch1 = pulseIn(ch1_Pin, HIGH, 50000);
    ch2 = pulseIn(ch2_Pin, HIGH, 50000);
    delay(1); //DO NOT DELETE!!!
    ch3 = pulseIn(ch3_Pin, HIGH, 50000);
    ch5 = pulseIn(ch5_Pin, HIGH, 50000);


    //Neutralize all motors if arbitrary signal not present
    if(ch1 == 0 || ch2 == 0 || ch3 == 0 || ch5 == 0){
        RM.writeMicroseconds(1500);
        LM.writeMicroseconds(1500);
        WM.writeMicroseconds(1500);
        Serial.println("No signal");
        return;
    }

    //Enforce deadzone for channels
    (ch1 > (1500-NtolCH) && ch1 < (1500+NtolCH)) ? (ch1 = 1500):(ch1 = ch1);
    (ch2 > (1500-NtolCH) && ch2 < (1500+NtolCH)) ? (ch2 = 1500):(ch2 = ch2);
    (ch3 > (1500-NtolCH) && ch3 < (1500+NtolCH)) ? (ch3 = 1500):(ch3 = ch3);
    (ch5 > (1500-NtolCH) && ch5 < (1500+NtolCH)) ? (ch5 = 1500):(ch5 = ch5);

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
    

    int RMotorSpeed = static_cast<int>(1500 + motorSpeeds.y * 500.0); // Extract right motor speed and scale to -255 to 255
    int LMotorSpeed = static_cast<int>(1500 + motorSpeeds.x * 500.0); // Extract left motor speed and scale to -255 to 255
    
    //Clamp speeds to prevent writeMicroseconds() error
    //Should not exceed but being precautionary
    (RMotorSpeed > 2000) ? (RMotorSpeed = 2000):(RMotorSpeed = RMotorSpeed);
    (RMotorSpeed < 1000) ? (RMotorSpeed = 1000):(RMotorSpeed = RMotorSpeed);
    (LMotorSpeed > 2000) ? (LMotorSpeed = 2000):(LMotorSpeed = LMotorSpeed);
    (LMotorSpeed < 1000) ? (LMotorSpeed = 1000):(LMotorSpeed = LMotorSpeed);

    //DIAGNOSTICS
    ////////////////////////
    Serial.print("ch1: " + String(ch1) + " ch2: " + String(ch2) + " ch3: " + String(ch3) + " ch5: " + String(ch5) + " || ");
    Serial.print("Left Speed: " + String(LMotorSpeed) + " Right Speed: " + String(RMotorSpeed) + " || ");
    Serial.print("x: " + String(x) + " y: " + String(y) + " || ");
    Serial.println("Transform x: " + String(motorSpeeds.x) + " Transform y: " + String(motorSpeeds.y));
    ////////////////////////

    //Check if robot is flipped
    if (ch5 < 1500) isFlipped = false;
    else isFlipped = true;

    
    //Write to all motors
    if (isFlipped == false){
        //Assign speeds with deadzone
        (RMotorSpeed > -Ntolerance && RMotorSpeed < Ntolerance) ? (RM.writeMicroseconds(1500)):(RM.writeMicroseconds(RMotorSpeed));
        (LMotorSpeed > -Ntolerance && LMotorSpeed < Ntolerance) ? (LM.writeMicroseconds(1500)):(LM.writeMicroseconds(LMotorSpeed));

        //Write to weapon motor
        WM.writeMicroseconds(weaponSpeed);
    }
    else{
        //Flip speeds
        RMotorSpeed += -2*(RMotorSpeed - 1500);
        LMotorSpeed += -2*(LMotorSpeed - 1500);
        weaponSpeed += -2*(weaponSpeed - 1500);

        //Assign FLIPPED speeds with deadzone
        (RMotorSpeed > -Ntolerance && RMotorSpeed < Ntolerance) ? (RM.writeMicroseconds(1500)):(RM.writeMicroseconds(RMotorSpeed));
        (LMotorSpeed > -Ntolerance && LMotorSpeed < Ntolerance) ? (LM.writeMicroseconds(1500)):(LM.writeMicroseconds(LMotorSpeed));

        //Write to weapon motor
        WM.writeMicroseconds(weaponSpeed);
    }    
}