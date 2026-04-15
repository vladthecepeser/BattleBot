#include <IBusBM.h>
using namespace std;

void setup()
{
    Serial.begin(115200);
    Serial.println();
  
    Serial1.begin(115200);   // iBus baudrate
    IBus.begin(Serial1);

    /*Pins to AVOID:
    GPIO 23, 24, 25, 29
    
    Pins for CAUTION:
    GPIO 0, 1, 2, 3, 15*/
    int RMotor_IN1 = 4;
    int RMotor_IN2 = 5;
    int LMotor_IN1 = 6;
    int LMotor_IN2 = 7; 
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
  
    

    int horizontal = map(ch1, 1000, 2000, -100, 100); // Map ch1 to a heading value between -100 and 100
    int vertical = map(ch2, 1000, 2000, -100, 100);

    double LH = horizontal;
    double RH = -horizontal;

    double LV = vertical;
    double RV = vertical;

    int signL;
    if(abs(LH)-abs(LV) >0){
        signL = 1;
    }
    else if(abs(LH)-abs(LV) <0){
        signL = -1;
    }
    else{
        signL = 0;
    }

    int signR;
    if(abs(RH)-abs(RV) >0){
        signR = 1;
    }
    else if(abs(RH)-abs(RV) <0){
        signR = -1;
    }
    else{
        signR = 0;
    }

    double L = 0.5((LH + LV) + (LH - LV))*signL;
    double R = 0.5((RH + RV) + (RH - RV))*signR;

    


}