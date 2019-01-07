//------------------------------------------------------------------------//
#include <SoftwareSerial.h>
#define Seperator "_"
int rxPin = 2;
int txPin = 3; // Not used
SoftwareSerial SerialToRepeat(rxPin,txPin);
//------------------------------------------------------------------------//
void setup() {
    pinMode(4,OUTPUT);digitalWrite(4,LOW); // RX GND reference
    Serial.begin(9600);
    SerialToRepeat.begin(9600);
}
//------------------------------------------------------------------------//
void loop() {
    if (SerialToRepeat.available()) {
        byte valueOut = SerialToRepeat.read();
        // Add space to single and dbl digit
        if(valueOut<10)  Serial.print(" ");
        if(valueOut<100) Serial.print(" ");
        // Repeat and seperate
        Serial.print(valueOut);
        Serial.print(Seperator);
    }
}
//------------------------------------------------------------------------//