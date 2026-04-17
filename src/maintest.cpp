#include <Arduino.h>
#include <cmath>
#include <algorithm>
#include "hardware/pwm.h"

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
const int RMotor_IN1 = 4;
const int RMotor_IN2 = 5;
const int LMotor_IN1 = 6;
const int LMotor_IN2 = 7; 
const int WMotor_IN1 = 8;
const int WMotor_IN2 = 9;
const int Ntolerance = 25;
bool isFlipped = false;

uint sliceR1;
uint sliceR2;
uint sliceL1;
uint sliceL2;
uint sliceW1;
uint sliceW2;

uint channelR1;
uint channelR2;
uint channelL1;
uint channelL2;
uint channelW1;
uint channelW2;

void setup()
{
    //PWM frequency = 125 MHz / clkdiv / wrap
    
    gpio_set_function(RMotor_IN1, GPIO_FUNC_PWM);
    gpio_set_function(RMotor_IN2, GPIO_FUNC_PWM);
    gpio_set_function(LMotor_IN1, GPIO_FUNC_PWM);
    gpio_set_function(LMotor_IN2, GPIO_FUNC_PWM);
    gpio_set_function(WMotor_IN1, GPIO_FUNC_PWM);
    gpio_set_function(WMotor_IN2, GPIO_FUNC_PWM);

    sliceR1   = pwm_gpio_to_slice_num(RMotor_IN1);
    sliceR2   = pwm_gpio_to_slice_num(RMotor_IN2);
    sliceL1   = pwm_gpio_to_slice_num(LMotor_IN1);
    sliceL2   = pwm_gpio_to_slice_num(LMotor_IN2);
    sliceW1   = pwm_gpio_to_slice_num(WMotor_IN1);
    sliceW2   = pwm_gpio_to_slice_num(WMotor_IN2);

    channelR1 = pwm_gpio_to_channel(RMotor_IN1);
    channelR2 = pwm_gpio_to_channel(RMotor_IN2);
    channelL1 = pwm_gpio_to_channel(LMotor_IN1);
    channelL2 = pwm_gpio_to_channel(LMotor_IN2);
    channelW1 = pwm_gpio_to_channel(WMotor_IN1);
    channelW2 = pwm_gpio_to_channel(WMotor_IN2);
    
    // 125 MHz / 20 = 6.25 MHz PWM clock
    pwm_set_clkdiv(sliceR1, 20.0f);
    pwm_set_clkdiv(sliceR2, 20.0f);
    pwm_set_clkdiv(sliceL1, 20.0f);
    pwm_set_clkdiv(sliceL2, 20.0f);
    pwm_set_clkdiv(sliceW1, 20.0f);
    pwm_set_clkdiv(sliceW2, 20.0f);

    // 6.25 MHz / 255 = ~24.5 kHz PWM
    pwm_set_wrap(sliceR1, 255);
    pwm_set_wrap(sliceR2, 255);
    pwm_set_wrap(sliceL1, 255);
    pwm_set_wrap(sliceL2, 255);
    pwm_set_wrap(sliceW1, 255);
    pwm_set_wrap(sliceW2, 255);

    // 50% duty cycle
    //pwm_set_chan_level(slice, channel, 500);

    pwm_set_enabled(sliceR1, true);
    pwm_set_enabled(sliceR2, true);
    pwm_set_enabled(sliceL1, true);
    pwm_set_enabled(sliceL2, true);
    pwm_set_enabled(sliceW1, true);
    pwm_set_enabled(sliceW2, true);

    Serial.begin(115200);
    Serial.println();

    pinMode(RMotor_IN1, OUTPUT);
    pinMode(RMotor_IN2, OUTPUT);
    pinMode(LMotor_IN1, OUTPUT);
    pinMode(LMotor_IN2, OUTPUT);
    pinMode(WMotor_IN1, OUTPUT);
    pinMode(WMotor_IN2, OUTPUT);
    Serial.println("Initialized!");
}


void loop()
{ 
    int RMotorSpeed;
    int LMotorSpeed;
    int weaponSpeed;

    for(int i = 200; i < 255; i++){
        RMotorSpeed = i;
        LMotorSpeed = i;
        weaponSpeed = i;
        Serial.println(i);
        
        //Write to all motors
        if (isFlipped == false){
            //Write the speed to the right motor
            if (RMotorSpeed > -Ntolerance && RMotorSpeed < Ntolerance){
                digitalWrite(RMotor_IN1, LOW);
                digitalWrite(RMotor_IN2, LOW);}
            else if (RMotorSpeed > 0){
                pwm_set_chan_level(sliceR1, channelR1, 500);
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
        delay(100);
    }
}