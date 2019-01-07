//-------------------------------------------------------------------------------//
// Global
float ScreenHeight = 200;// 768*0.9;
float ScreenWidth =  600;//1024*0.9;
boolean SomethingWentWrong = false;
boolean AutoUpdate = true;
// Serial
boolean SerialEnabled = true;
String inString = "";
import processing.serial.*;
Serial myPort;
// Hellicoptor
int Thrust = 0;
int Pitch  = 63; // 0->127 ->> DownMax->UpMax ->> Neutral at 63
int Yaw    = 63; // 0->127 ->> DownMax->UpMax ->> Neutral at 63
int YawAdj = 63; // 0->127 ->> DownMax->UpMax ->> Neutral at 63
int lastThrust = Thrust;
int lastPitch  = Pitch;
int lastYaw    = Yaw;
int lastYawAdj = YawAdj;

//-------------------------------------------------------------------------------//
void setup() {
    size(floor(ScreenWidth),floor(ScreenHeight));
    // create a font with the third font available to the system:
    PFont myFont = createFont(PFont.list()[2], 20);
    textFont(myFont);
    // Serial
    if(SerialEnabled) {
        println("Available ports are:");
        println(Serial.list());
        // myPort = new Serial(this, Serial.list()[0], 9600);
        myPort = new Serial(this, Serial.list()[0], 115200);
    }
}
//-------------------------------------------------------------------------------//
void draw() {
    //-----------Draw Prepare------------//
    background(0);
    //---------- Global Settings Echo ----------//
    stroke(69,88,148);
    fill(  69,88,148);
    text("Thrust:" + Thrust + "  Pitch:" + Pitch + "  Yaw:" + Yaw + "  YawAdj:" + YawAdj,20,20);
    text("AutoUpdate:" + AutoUpdate,20,40);
    if(SomethingWentWrong) {
        stroke(255,0,0);
        fill(  255,0,0);
        text("SomethingWentWrong!",20,50);
    }
    //---------- Movement ----------//
    if(AutoUpdate && ((lastThrust!=Thrust)||(lastPitch!=Pitch)||(lastYaw!=Yaw)||(lastYawAdj!=YawAdj))) {
        SendCMDs();
        lastThrust = Thrust;
        lastPitch  = Pitch;
        lastYaw    = Yaw;
        lastYawAdj = YawAdj;
        delay(500);
    }
}
//-------------------------------------------------------------------------------//
//void mousePressed(){
//}
//-------------------------------------------------------------------------------//
void SendCMDs() {
    String ToSend = "";
    if(lastThrust != Thrust) ToSend += "H" + (char)127 + (char)(Thrust-lastThrust+64) + (char)127 + "H";// + (char)0;
    if(lastPitch  != Pitch ) ToSend += "H" + (char)126 + (char)(Pitch-lastPitch+64)   + (char)126 + "H";// + (char)0;
    if(lastYaw    != Yaw   ) ToSend += "H" + (char)125 + (char)(Yaw-lastYaw+64)       + (char)125 + "H";// + (char)0;
    if(lastYawAdj != YawAdj) ToSend += "H" + (char)124 + (char)(YawAdj-lastYawAdj+64) + (char)124 + "H";// + (char)0;
    myPort.write(ToSend);
    println("Serial Send: " + ToSend);
}
//-------------------------------------------------------------------------------//
void keyPressed() {
    // Hellicoptor related
    if((key == ' ')&&SerialEnabled&&!SomethingWentWrong) SendCMDs();
    else if(key == 'H') myPort.write(" HHHHH "); // Toggle Helicopter enable
    else if(key == 'k') {
        Thrust = 0;
        Pitch  = 63; // 0->127 ->> DownMax->UpMax ->> Neutral at 63
        Yaw    = 63; // 0->127 ->> DownMax->UpMax ->> Neutral at 63
        YawAdj = 63; // 0->127 ->> DownMax->UpMax ->> Neutral at 63
        myPort.write(" KKKKK "); // Kill CMD
        println("Kill cmd sent"); // Kill CMD
    }
    else if((key == CODED)&&(keyCode == UP)  &&(Thrust<(126-8))) Thrust += 8; // Thrust Max is 125
    else if((key == CODED)&&(keyCode == DOWN)&&(Thrust>7))       Thrust -= 8; // Thrust Min is 0

    else if((key == CODED)&&(keyCode == RIGHT)&&(Yaw<(127-4))) Yaw += 4; // Yaw is [0-127] with 63 Neutral
    else if((key == CODED)&&(keyCode == LEFT)&&(Yaw>3))        Yaw -= 4; // Yaw is [0-127] with 63 Neutral

    else if((key == 'F')&&(Pitch<(127-4))) Pitch += 4; // BKWD: Pitch is [0-127] with 63 Neutral
    else if((key == 'f')&&(Pitch>3))       Pitch -= 4; // FWRD: Pitch is [0-127] with 63 Neutral

    else if((key == '<')&&(YawAdj<(127-4))) YawAdj += 4; // YawAdj is [0-127] with 63 Neutral
    else if((key == '>')&&(YawAdj>3))       YawAdj -= 4; // YawAdj is [0-127] with 63 Neutral
    
    // Inchworm related
    else if(key == 'I') myPort.write(" IIIII "); // Toggle Inchworm enable
    else if(key == '8') myPort.write(" IFFFI ");
    else if(key == '2') myPort.write(" IBBBI ");
    else if(key == '4') myPort.write(" ILLLI ");
    else if(key == '6') myPort.write(" IRRRI ");

    // Other related
    else if(key == 'W') SomethingWentWrong = false;
    else if(key == 'A') AutoUpdate = !AutoUpdate;
}
//-------------------------------------------------------------------------------//
void serialEvent(Serial myPort) {
    if(SerialEnabled) {
        char inChar = (char)myPort.read();
        if(inChar != '\n') {
            inString += inChar;  // Keep reading until you hit a \n
        } else {
            if(inString.equals("ERROR")) SomethingWentWrong = true; 
            // Regular echo
            println("SerialRead: " + inString);
            // Clear for next use
            inString = "";
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

