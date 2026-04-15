// #include <Arduino.h>
// #include <cmath>
// #include <algorithm>

// struct Vec
// {
//     double x;
//     double y;
// };

// // Transforms the controller input vector to right/left motor speeds
// Vec transform(const Vec& v)
// {
//     // Step 1: rotate by -45 degrees
//     const double invSqrt2 = 1.0 / sqrt(2.0);

//     double xr =  (v.x + v.y) * invSqrt2;
//     double yr = (-v.x + v.y) * invSqrt2;

//     // Step 2: normalize by max absolute component
//     double maxAbs = max(abs(xr), fabs(yr));

//     // Guard against zero (just in case)
//     if (maxAbs == 0.0)
//     {
//         return {0.0, 0.0};
//     }

//     return { xr / maxAbs, yr / maxAbs };
// }

// void setup()
// {
//     Serial.begin(115200);
//     Serial.println();
// }


// void loop(){
    
//     Serial.print("Enter coordinate x: ");

//     while (!Serial.available()) {
//         delay(1);     // Blocking wait
//     }
//     Serial.println();
//     int x = Serial.read();


//     Serial.print("Enter coordinate y: ");
//     while (!Serial.available()) {
//         delay(1);     // Blocking wait
//     }
//     Serial.println();
//     int y = Serial.read();

//     Serial.print("You entered: (");
//     Serial.print(x);
//     Serial.print(", ");
//     Serial.print(y);
//     Serial.println(").");
//     Serial.println();

//     Serial.print("This transforms to: ");
//     Vec v = {static_cast<double>(x), static_cast<double>(y)};
//     Vec motorSpeeds = transform(v);

//     Serial.print(motorSpeeds.x);
//     Serial.print(", ");
//     Serial.print(motorSpeeds.y);
//     Serial.println(").");
//     Serial.println();
// }