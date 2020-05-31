#include <Wire.h>
//Mega2560 pin 20 (SDA), pin 21 (SCL)
//SPI pin
// MOSI 51
//MISO 50
//SCK 52
//SS 53

//////////////////////////////////////
//OLED
#define Sclk 52 //--- connect this to the display module CLK pin (Serial Clock)
#define Mosi 51 //--- connect this to the display module DIN pin (Serial Data)
#define Rst  49 //--- connect this to the display module RES pin (Reset)
#define Dc   50 //--- connect this to the display module D/C  pin (Data or Command)
#define Cs 48 //--- connect this to the display module CS  pin (Chip Select)

// Color definitions
#define BLACK           0x0000
#define BLUE            0x0006
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN             0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0  
#define WHITE           0xFFFF
#define BACKGROUND      0x0000

#include <Adafruit_SSD1331.h>
Adafruit_SSD1331 display = Adafruit_SSD1331(Cs, Dc, Mosi, Sclk, Rst);
///////////////////////////////////////////////////////
//Color sensor
#define colorpin_out 26 //brown
#define colorpin_s2 27 //orange
#define colorpin_s3 28 //yellow
#define colorpin_s1 29 // purple
#define colorpin_s0 30 //blue
#define colorpin_LED 31 //LED
int colorValueC = 0;
int colorValueR = 0;
int colorValueG = 0;
int colorValueB = 0;
int colorCnt = 0;
int colorCheckCnt = 0;

///////////////////////////////////////////////////////
//Black Line
//see black tape is 1, LED off
//see white is 0, LED on
#define blackLinePinAnalogTR A12   //Top Right
#define blackLinePinTR 42         //Top Right
#define blackLinePinAnalogTL A13   //Top Left
#define blackLinePinTL 43         //Top Left
#define blackLinePinAnalogFR A14  //Far Right (near the wheel)
#define blackLinePinFR 44         //Far Right (near the wheel)
#define blackLinePinAnalogFL A15  //Far Left (near the wheel)
#define blackLinePinFL 45         //Far Left (near the wheel)

bool blackLineTR = 0; //top right
bool blackLineTL = 0; // top left
bool blackLineFR = 0; //far right
bool blackLineFL = 0; //far left
int  blackLineAnalogTR = 0; //top right
int  blackLineAnalogTL = 0; //top left
int  blackLineAnalogFR = 0; //far right
int  blackLineAnalogFL = 0; //far left

///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
// Photo Diode
#define photoPinAnalogR  A8//Photo Right
#define photoPinR 38  //Photo Right
#define photoPinAnalogL  A9//Photo Left
#define photoPinL  39  //Photo Left

bool photoR = 0; //right
bool photoL = 0; //left
int photoAnalogR = 0; //right
int photoAnalogL = 0; //left
int photoThresholdR, photoThresholdL; 
int photoBlackR, photoBlackL;
int photoLevelR, photoLevelL;
///////////////////////////////////////////////////////
//Power detection
#define powerValuePin A7
int  powerValue = 0;

///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//Ultra Sonic
#define trigPin 13
#define echoPin 12

unsigned long UltraSonicStartTime = 0;
int UltraSonicDone = 1;
int    duration;
float  distance;
///////////////////////////////////////////////////////

///////////////////////////////////////////////////////
//interrupt pin
int interruptL1 = 2;
int interruptL2 = 3;

int interruptR1 = 18;
int interruptR2 = 19;
///////////////////////////////////////////////////////

//////////////////////////////////////////////////////
//motor driver LM298 control pin
int pin1R = 8;
int pin2R = 9;
int pin1L = 10;
int pin2L = 11;
//////////////////////////////////////////////////////

//////////////////////////////////////////////////////
// encoder parameter
//encoderValue
long encoderValueR = 0;
long encoderValueL = 0;

//Reset encoderValue
int encoderValueR_Reset = 0;
int encoderValueL_Reset = 0;

//encoderValue Diff
long encoderValueR_Diff = 0;
long encoderValueL_Diff = 0;
//////////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  /////////////////////////////////////////
  //Color sensor setup
  pinMode(colorpin_LED, OUTPUT);
  pinMode(colorpin_out, INPUT);
  pinMode(colorpin_s0, OUTPUT);
  pinMode(colorpin_s1, OUTPUT);
  pinMode(colorpin_s2, OUTPUT);
  pinMode(colorpin_s3, OUTPUT);
  // turn on LED
  digitalWrite(colorpin_LED, 1);
  //100% freq
  digitalWrite(colorpin_s0, 1);
  digitalWrite(colorpin_s1, 1);
  //detect red
  digitalWrite(colorpin_s2, 1);
  digitalWrite(colorpin_s3, 0);
  /////////////////////////////////////////

  /////////////////////////////////////////
  //Black Line sensor
  pinMode(blackLinePinTR, INPUT);
  pinMode(blackLinePinTL, INPUT);
  pinMode(blackLinePinFR, INPUT);
  pinMode(blackLinePinFL, INPUT);
  /////////////////////////////////////////

  /////////////////////////////////////////
  //Photo sensor
  pinMode(photoPinR, INPUT);
  pinMode(photoPinL, INPUT);
  /////////////////////////////////////////

  /////////////////////////////////////////
  //Ultra Sonic sensor
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  /////////////////////////////////////////

  /////////////////////////////////////////
  //Motor
  pinMode(pin1L, OUTPUT);
  pinMode(pin2L, OUTPUT);
  pinMode(pin1R, OUTPUT);
  pinMode(pin2R, OUTPUT);
  /////////////////////////////////////////

  /////////////////////////////////////////
  //Encoder
  //https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
  pinMode(interruptL1, INPUT_PULLUP);
  pinMode(interruptL2, INPUT_PULLUP);
  pinMode(interruptR1, INPUT_PULLUP);
  pinMode(interruptR2, INPUT_PULLUP);
  //setup interrupt
  attachInterrupt(digitalPinToInterrupt(interruptL1), countL, FALLING);
  attachInterrupt(digitalPinToInterrupt(interruptR1), countR, FALLING);
  /////////////////////////////////////////
  //OLED setup
  display.begin();
  oledStartPrint();
  delay(1000);
  //photo calibration
  photoCalibration();
}

void loop()
{
    oledPrint();
    for (int i=0; i<300000; i++) {
      MotorLightTrack();
      oledDataPrint();
      //delay(1);
    }
    
    stopCar();
    while(1){
      InputCapture();  
      oledDataPrint();
    }
  
}

//////////////////////////////////////////////////////
//Motor control
void MotorControl() {
    forwardCar();
    int counter=0;
    while(counter<30000){
      InputCapture();
      if (blackLineAnalogFL>150 and blackLineAnalogFR>150) {
        counter++;
        //delay(1);
      }
    }
    stopCar();
    while(1){
      InputCapture();  
      oledPrint();
    }
  }

void MotorLightTrack() {
    InputCapture();
    
    if (photoLevelL>0 or photoLevelR>0) {
      if (photoLevelL>photoLevelR) { //R brighter
        turnCarR();
      }
      if (photoLevelL<photoLevelR) { //L brighter
        turnCarL();
      }
      if (photoLevelL==photoLevelR) {
        forwardCar();
      }
    }
    else {
      if (photoLevelL==photoLevelR) {
        stopCar();
      }
      else {
        backwardCar();
      }
    }
}

void MotorStraightForward() {
    if (encoderValueL== encoderValueR) {
      forwardCar();
    }
    if (encoderValueL > encoderValueR) {
      turnCarL();
    }
    if (encoderValueL < encoderValueR) {
      turnCarR();
    }
  
}

//////////////////////////////////////////////////////
//OLED print
void oledStartPrint() {
    display.fillScreen(BLACK);
    display.setCursor(0, 0);
    display.setTextColor(WHITE);  
    display.setTextSize(2);
    display.print("HKUEEE");

    display.setCursor(0, 15);
    display.print("IDP19/20");

    display.setCursor(0, 30);
    display.print("Group 12");
   
    delay(1000);
}

void oledPrint() {
    display.fillScreen(BLACK);
    display.setCursor(0, 0);
    display.setTextColor(WHITE);  
    display.setTextSize(1);
    display.print("PhotoR/L");
        
    display.setCursor(0, 30);
    display.print("Lvl R/L");
}

void oledDataPrint() {
    display.setTextSize(2);
    
    display.fillRect(0,15,100,15,BLACK);
    display.setCursor(0, 15);
    display.print(photoAnalogR);
    display.print(",");
    display.print(photoAnalogL);

    display.fillRect(0,45,100,15,BLACK);
    display.setCursor(0, 45);
    display.print(photoLevelR);
    display.print(",");
    display.print(photoLevelL);
    
}

//////////////////////////////////////////////////////
//photo sensor
void photoCalibration() {
    display.fillScreen(BLACK);
    display.setCursor(0, 0);
    display.setTextColor(CYAN);  
    display.setTextSize(2);
    display.println("CLOSE");
    display.println("TORCH");
    delay(2000);
    InputCapture();
    photoBlackL=photoAnalogL;
    photoBlackR=photoAnalogR;
    
    display.setTextColor(MAGENTA);
    display.setTextSize(1);
    display.println("");
    display.println("black R/L");
    display.setTextSize(2);
    display.print(photoBlackR);
    display.print(",");
    display.print(photoBlackL);
    delay(1000);
    //
    display.fillScreen(BLACK);
     display.setTextColor(CYAN);
    display.setCursor(0, 0);
    display.println("OPEN");
    display.println("TORCH");
    delay(2000);
    InputCapture();
    photoThresholdL=photoAnalogL;
    photoThresholdR=photoAnalogR;
    
     display.setTextColor(MAGENTA);
    display.setTextSize(1);
    display.println("");
    display.println("analog R/L");
    display.setTextSize(2);
    display.print(photoAnalogR);
    display.print(",");
    display.print(photoAnalogL);
    delay(1000);
    //
    display.fillScreen(BLACK);
    display.setCursor(0, 0);
    display.print("DONE");
    delay(1000);
}

void photoNormalize() {
  //level 0=light, level 10=black
  photoLevelL=10*(photoAnalogL-photoThresholdL)/(photoBlackL-photoThresholdL);
  photoLevelR=10*(photoAnalogR-photoThresholdR)/(photoBlackR-photoThresholdR);
  
}
//////////////////////////////////////////////////////
//Interrupt subroutine
void countL() {
  if (encoderValueL_Reset==1) {
    encoderValueL=0;
    encoderValueL_Reset=0;
  }
  
  if (digitalRead(interruptL2)) {
    encoderValueL--;
  }
  else {
    encoderValueL++;
  }
}
void countR() {
  if (encoderValueR_Reset==1) {
    encoderValueR=0;
    encoderValueR_Reset=0;
  }
  
  if (digitalRead(interruptR2)) {
    encoderValueR++;
  }
  else {
    encoderValueR--;
  }
}
//////////////////////////////////////////////////////
//Input routine
void InputCapture() {
  //black
  blackLineTR = digitalRead(blackLinePinTR);
  blackLineTL = digitalRead(blackLinePinTL);
  blackLineFR = digitalRead(blackLinePinFR);
  blackLineFL = digitalRead(blackLinePinFL);
  blackLineAnalogTR = analogRead(blackLinePinAnalogTR);
  blackLineAnalogTL = analogRead(blackLinePinAnalogTL);
  blackLineAnalogFR = analogRead(blackLinePinAnalogFR);
  blackLineAnalogFL = analogRead(blackLinePinAnalogFL);
  //photo diode
  photoR     = digitalRead(photoPinR);
  photoL     = digitalRead(photoPinL);
  photoAnalogR = analogRead(photoPinAnalogR);
  photoAnalogL = analogRead(photoPinAnalogL);
  photoNormalize();
  //Power Sensor
  powerValue = analogRead(powerValuePin);
}
//////////////////////////////////////////////////////
// Motor subrountine
void forwardR() {
  digitalWrite(pin1R, 1);
  digitalWrite(pin2R, 0);
}
void backwardR() {
  digitalWrite(pin1R, 0);
  digitalWrite(pin2R, 1);
}
void stopR() {
  digitalWrite(pin1R, 0);
  digitalWrite(pin2R, 0);
}
void forwardL() {
  digitalWrite(pin1L, 1);
  digitalWrite(pin2L, 0);
}
void backwardL() {
  digitalWrite(pin1L, 0);
  digitalWrite(pin2L, 1);
}
void stopL() {
  digitalWrite(pin1L, 0);
  digitalWrite(pin2L, 0);
}
void turnCarR() {
  digitalWrite(pin1R, 0);
  digitalWrite(pin2R, 0);
  digitalWrite(pin1L, 1);
  digitalWrite(pin2L, 0);
}
void turnCarL() {
  digitalWrite(pin1R, 1);
  digitalWrite(pin2R, 0);
  digitalWrite(pin1L, 0);
  digitalWrite(pin2L, 0);
}
void turnCarL_Backward() {
  digitalWrite(pin1R, 0);
  digitalWrite(pin2R, 1);
  digitalWrite(pin1L, 0);
  digitalWrite(pin2L, 0);
}
void turnCarR_Backward() {
  digitalWrite(pin1R, 0);
  digitalWrite(pin2R, 0);
  digitalWrite(pin1L, 0);
  digitalWrite(pin2L, 1);
}
void turnCarOnsiteL() {
  digitalWrite(pin1R, 1);
  digitalWrite(pin2R, 0);
  digitalWrite(pin1L, 0);
  digitalWrite(pin2L, 1);
}
void turnCarOnsiteR() {
  digitalWrite(pin1R, 0);
  digitalWrite(pin2R, 1);
  digitalWrite(pin1L, 1);
  digitalWrite(pin2L, 0);
}
void forwardCar() {
  digitalWrite(pin1R, 1);
  digitalWrite(pin2R, 0);
  digitalWrite(pin1L, 1);
  digitalWrite(pin2L, 0);
}
void backwardCar() {
  digitalWrite(pin1R, 0);
  digitalWrite(pin2R, 1);
  digitalWrite(pin1L, 0);
  digitalWrite(pin2L, 1);
}
void stopForwardCar() {
  backwardCar();
  delay(150);
  stopL();
  stopR();
}
void stopBackwardCar() {
  forwardCar();
  delay(100);
  stopL();
  stopR();
}
void stopTurnCarOnsiteL() {
  turnCarOnsiteR();
  delay(100);
  stopL();
  stopR();
}
void stopTurnCarOnsiteR() {
  turnCarOnsiteL();
  delay(100);
  stopL();
  stopR();
}
void stopCar() {
  stopL();
  stopR();
}

//////////////////////////////////////////////////////
