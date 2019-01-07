boolean EnableINT_Disable = false;  // Will poll serial and not use INT, so don't need ISR dissable
boolean BurstPortB = true;
int IRledPin = 8;
boolean Xmit_S107 = false;
boolean Xmit_Inch = false;
// FSM
int numBuff = 5;
char FSM[5] = {0,0,0,0,0};
// Heli
int Speeds[4] = {63,63,0,63};
int SpeedsInit[4] = {63,63,0,63};
char ThrustID = 127; // Note delta value must be [1,123] with +64 offset thus [-63,59]
char PitchID  = 126; // So these IDs should be fine
char YawID    = 125;
char YawAdjID = 124;
// Inch
char InchDirCmd = '0';
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
    if(BurstPortB) DDRB |= B1111;
    else           pinMode(IRledPin,OUTPUT);
//    Serial.begin(115200);
    Serial.begin(9600);
    Serial.println("IR Emitter Turned On");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
    pollSerial(); // By polling the Serial, we can avoid all that ISR dissabling stuff
    if(Xmit_S107) Xmit_S107Code(Speeds);
    if(Xmit_Inch) Xmit_InchCode(InchDirCmd);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void pollSerial() {
    if(Serial.available() > 0) {
        char tempc = Serial.read();
        CrunchFSM(tempc);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CrunchFSM(char newChar) {
    if(newChar != 0) {
        for(int i=numBuff-1;i>=0;i--) {
            if(i==0) FSM[i] = newChar;
            else     FSM[i] = FSM[i-1];
        }
        if((FSM[0]==FSM[1])&&(FSM[1]==FSM[2])&&(FSM[2]==FSM[3])&&(FSM[3]==FSM[4])&&(FSM[4]=='H')) {
            Xmit_S107 = !Xmit_S107;
            Serial.print("IRToy: Helicopter Constant Update Enb:");
            Serial.println(Xmit_S107?"T":"F");
        } else if((FSM[0]==FSM[1])&&(FSM[1]==FSM[2])&&(FSM[2]==FSM[3])&&(FSM[3]==FSM[4])&&(FSM[4]=='I')) {
            Xmit_Inch = !Xmit_Inch;
            Serial.print("IRToy: Inchworm Constant Update Enb:");
            Serial.println(Xmit_Inch?"T":"F");
        } else if((FSM[0]==FSM[1])&&(FSM[1]==FSM[2])&&(FSM[2]==FSM[3])&&(FSM[3]==FSM[4])&&(FSM[4]=='K')) {
                for(int i=0;i<4;i++) Speeds[i] = SpeedsInit[i];
                InchDirCmd = '0';
                Serial.println("KillEngaged");
        } else if((FSM[0]==FSM[1])&&(FSM[1]==FSM[3])&&(FSM[3]==FSM[4])&&(FSM[4]=='D')) {
            Debug(FSM[2]);
        } else {
            ProcessB5Pattern(FSM);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ProcessB5Pattern(char Pattern[]) {
    // char ID;
    // Helicopter
    if((Pattern[0] == Pattern[4])&&(Pattern[0] == 'H')&&(Pattern[1] == Pattern[3])) {
        int Delta = (int)Pattern[2] - 64;
        char ID = Pattern[1];
        if(ID == YawID){
            Speeds[0] += Delta;
            if(Speeds[0] < 0)   Speeds[0] = 0;
            if(Speeds[0] > 127) Speeds[0] = 127;
        } else if(ID == PitchID) {
            Speeds[1] += Delta;
            if(Speeds[1] < 0)   Speeds[1] = 0;
            if(Speeds[1] > 127) Speeds[1] = 127;
        } else if(ID == ThrustID) {
            Speeds[2] += Delta;
            if(Speeds[2] < 0)   Speeds[2] = 0;
            if(Speeds[2] > 125) Speeds[2] = 125;
        } else if(ID == YawAdjID){
            Speeds[3] += Delta;
            if(Speeds[3] < 0)   Speeds[3] = 0;
            if(Speeds[3] > 127) Speeds[3] = 127;
        } else {
            String PrintOut = "";
            PrintOut += "  Got Invalid ID(" ;
            PrintOut += ID;
            Serial.println(PrintOut + ") for:" + Delta);
            Serial.println(" ");
        }
    // Inchworm
    } else if((Pattern[0] == Pattern[4])&&(Pattern[0] == 'I')&&(Pattern[1] == Pattern[2])&&(Pattern[2] == Pattern[3])) {
        if(Xmit_Inch) InchDirCmd = Pattern[2];   // If auto update enabled, overwrite
        else for(int i=0;i<2;i++) Xmit_InchCode(Pattern[2]);
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Debug(char select) {
    if(select == 'b') {
        BurstPortB = !BurstPortB;
        Serial.print("BurstPortB:");
        Serial.println(BurstPortB?"T":"F");
    }
    Serial.println("DebugDone");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Xmit_S107Code(int ValueArray[]) {
    uint16_t pinID = IRledPin;
    // Header
    pulseIR_38KHz(4000,pinID);
    delayMicroseconds(2000);
    // Body
    sendCharArray(ValueArray,4,300,600,300,pinID);
    // Footer
    pulseIR_38KHz(360,pinID);
    digitalWrite(pinID, LOW);
    delay(65);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Xmit_InchCode(char Code) {
    uint16_t pinID = IRledPin;
    int CodeArray[1] = {0};
    if      (Code == 'F') CodeArray[0] = 131;
    else if (Code == 'B') CodeArray[0] = 69; // heheh
    else if (Code == 'R') CodeArray[0] = 38;
    else if (Code == 'L') CodeArray[0] = 193;
    // Header
    pulseIR_38KHz(4000,pinID); delayMicroseconds(400);
    pulseIR_38KHz(2000,pinID); delayMicroseconds(800);
    // Body
    sendCharArray(CodeArray,1,400,1400,500,pinID);
    // Footer
    pulseIR_38KHz(400,pinID); delayMicroseconds(1400);
    pulseIR_38KHz(400,pinID); delayMicroseconds(1400);
    pulseIR_38KHz(400,pinID);
    digitalWrite(pinID, LOW);
    delay(65);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void sendCharArray(int CharArray[], uint16_t size, uint16_t PulseOn, uint16_t DelayTrue, uint16_t DelayFalse, uint16_t pinID) {
    int tempc = 0;
    for(int i=0;i < size;i++) {
        tempc = CharArray[i];
        for(int j=7;j>=0;j--) {
            pulseIR_38KHz(PulseOn,pinID);
            if (((tempc>>j)%2) == 1) delayMicroseconds(DelayTrue);
            else                     delayMicroseconds(DelayFalse);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void pulseIR_38KHz(long microsecs, uint16_t pinID) {
    if(EnableINT_Disable) cli();  // Dissable Interrupts (like serial event)
    if(BurstPortB) {
        while (microsecs > 0) {
            // 38KHz = 26.316us -> 26us, so cut in 1/2 for IR pulse -> 13us high, 13us low
            PORTB = B1111;  // Takes ~1/2us
            delayMicroseconds(13);
            PORTB = B0000;  // Takes ~1/2us
            delayMicroseconds(12);
            microsecs -= 26;
        }
    } else {
        while (microsecs > 0) {
            // 38KHz = 26.316us -> 26us, so cut in 1/2 for IR pulse -> 13us high, 13us low
            digitalWrite(pinID, HIGH);  // Takes ~3us
            delayMicroseconds(10);
            digitalWrite(pinID, LOW);   // Takes ~3us
            delayMicroseconds(10);
            microsecs -= 26;
        }
    }
    if(EnableINT_Disable) sei();  // Enable Interupts
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
