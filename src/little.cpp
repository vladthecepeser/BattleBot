// #include <ESP32Servo.h>
// using namespace std;

// Servo esc;
// int escPin = 23;  // connect ESC signal wire here
// Servo esc2;
// int escPin2 = 22;



// void setup()
// {  
//   esc.attach(escPin, 1000, 2000); // min/max pulse width in µs
//   //esc2.attach(escPin2, 1000, 2000);
//   esc.writeMicroseconds(1500);    // neutral
//   //esc2.writeMicroseconds(1500);
//   delay(2000);                 // wait for ESC to arm
// }


// //supposed to be some kind of writer for the code

// void loop()
// {
//   esc.writeMicroseconds(1900);
//   //esc2.writeMicroseconds(1500);
//   delay(1000); 
  
//   esc.writeMicroseconds(1500);    // neutral
//   //esc2.writeMicroseconds(1500);
//   delay(1000); 

//   esc.writeMicroseconds(1100);
//   //esc2.writeMicroseconds(1500);
//   delay(1000);

// }


// #include <Arduino.h>

// void setup() {
//   //myServo.attach(23);  // Attaches the servo on pin 9 to the servo object
//   Serial.begin(115200); // Starts serial communication at 9600 baud
// }

// void loop() {
//   int ch1;
//   int ch2;
//   int ch3 = 0;
//   int ch5 = 0;

//   // Read the pulse width of the first channel
//   ch1 = pulseIn(23, HIGH); // Assumes channel 1 is connected to pin 23
//   ch2 = pulseIn(22, HIGH);
//   delay(1);
//   ch3 = pulseIn(21, HIGH);
//   ch5 = pulseIn(19, HIGH);

//   // Print the pulse width to the serial monitor
//   Serial.print(ch1);
//   Serial.print(" ");
//   Serial.print(ch2);
//   Serial.print(" ");
//   Serial.print(ch3);
//   Serial.print(" ");
//   Serial.print(ch5);
//   Serial.println();

//   // Control the servo with the pulse width value
//   //myServo.writeMicroseconds(pulseWidth);

//   delay(10); // Delay a little bit to improve stability
// }