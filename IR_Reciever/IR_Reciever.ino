#define IRpin_PIN PIND
#define IRpin 2
#define PulseTimeout 65000
#define SampleDelay_us 20
#define MaxSamples 200
String inStringRaw = "";
boolean Armed = false;
boolean HitMax = false;
boolean Clear = false;
uint16_t pulses[MaxSamples][2];
uint8_t currentSampleIdx = 0;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup(void) {
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    digitalWrite(3,HIGH);
    digitalWrite(4,LOW);
    Serial.begin(115200);
    Serial.println("Ready to be Armed!");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void serialEvent() {
    if(!Armed) {
        char inChar = Serial.read();
        if((inChar >= 'A')&&(inChar <= 'z')) inStringRaw += (char)inChar;
        else {
            if(inStringRaw.equals("Arm")) {
                Serial.println("Armed!");
                Armed = true;
            } else if(inStringRaw.equals("clc")) {
                Clear = true;
            }
            inStringRaw = "";
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop (void) {
    if(Clear) {
        Clear = false;
        HitMax = false;
        Armed = false;
        for(int i=0;i < MaxSamples;i++) {
            pulses[i][0]=0;
            pulses[i][1]=0;
        }
        currentSampleIdx = 0;
        Serial.println("Cleared!");
    } else if(Armed) Capture();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Capture(void) {
    uint16_t pulseCount;
    pulseCount = 0;
    while (IRpin_PIN & (1 << IRpin)) { // vout pin is still HIGH
        pulseCount++;
        delayMicroseconds(SampleDelay_us);
        if ((pulseCount >= PulseTimeout) && (currentSampleIdx != 0)) {
            printpulses();
            currentSampleIdx=0;
            return;
        }
    }
    // we didn't time out so lets stash the reading
    pulses[currentSampleIdx][0] = pulseCount;
    // same as above
    pulseCount = 0;
    while (! (IRpin_PIN & _BV(IRpin))) { // pin is still LOW
        pulseCount++;
        delayMicroseconds(SampleDelay_us);
       if ((pulseCount >= PulseTimeout) && (currentSampleIdx != 0)) {
           printpulses();
           currentSampleIdx=0;
           return;
       }
    }
    pulses[currentSampleIdx][1] = pulseCount;

    // we read one high-low pulse successfully, continue!
    currentSampleIdx++;
    if(currentSampleIdx >= MaxSamples) {
        HitMax = true;
        Serial.println("Ring Buffer Looped");
        currentSampleIdx = 0;
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void printpulses(void) {
    int size = (HitMax)? MaxSamples : currentSampleIdx;
    Serial.println("\n\nReceived: (TimeON \tTimeOFF)usec");
    for (uint8_t i = 0; i < size; i++) {
        Serial.print(pulses[i][1] * SampleDelay_us, DEC);
        Serial.print(" ");
        Serial.println(pulses[i+1][0] * SampleDelay_us, DEC);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////