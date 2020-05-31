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

//Black Line Threshold
int BlackThresholdTR = 250;
int BlackThresholdTL = 250;
int BlackThresholdFR = 400;
int BlackThresholdFL = 400;

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
//LED pin
int pinLED = 33;

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

//spd control
int prev_act = 0;
int spd_period = 10;
int real_spd=0; //spd per 1 spd period (10 spd -> 6 r_spd for period 10)
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
  
  //LED 
  pinMode(pinLED, OUTPUT);
  
}

void loop()
{  
  MotorControl_wirelessCharging();
  stopCar(); 
  while(1){
      InputCapture();  
      UltraSonic();
      oledVolt();
  }
}

//////////////////////////////////////////////////////
//Motor control
void MotorControl_wirelessCharging() { 
    int power_threshold=100;
    
    InputCapture();
    UltraSonic();
    
    encoderValueL=0;
    encoderValueR=0;
    
    while (powerValue<power_threshold) {
      while (distance<1 or distance>40) {
        realSpdForward_noResetEncoder(10); //full spd
        InputCapture();
        UltraSonic();
      }
      //spd cal
      int spdBrakeDist=5; //distance in cm to reduce 1 spd 
      int spd=distance/spdBrakeDist;
      
      realSpdForward_noResetEncoder(spd);
      InputCapture();
      UltraSonic();
    }
    stopCar();
    delay(100);
    InputCapture();
    while (powerValue<power_threshold) {
      realSpdForward_noResetEncoder(1);
      InputCapture();
    }
    
    stopCar();
}

void MotorStraightForward() {
    if (encoderValueL== encoderValueR) {
      forwardCar();
    }
    if (encoderValueL > encoderValueR) {
      forwardR();
      stopL();
    }
    if (encoderValueL < encoderValueR) {
      forwardL();
      stopR();
    }
}

void MotorStraightBackward() {
    if (encoderValueL== encoderValueR) {
      backwardCar();
    }
    if (encoderValueL > encoderValueR) {
      backwardL();
      stopR();
    }
    if (encoderValueL < encoderValueR) {
      backwardR();
      stopL();
    }
}

void MotorExactTurn(int turnL, int turnR) {
  while (encoderValueL!=turnL or encoderValueR!=turnR) {
    if (encoderValueL==turnL) {
      stopL();
    }
    else {
      if (encoderValueL<turnL) {
        forwardL();
      }
      else {
        backwardL();
      }
    }
    
    if (encoderValueR==turnR) {
      stopL();
    }
    else {
      if (encoderValueR<turnR) {
        forwardR();
      }
      else {
        backwardR();
      }
    }
  }
}

void MotorTurnOnsiteL() {
  if ((abs(encoderValueL)== abs(encoderValueR))) {
    turnCarOnsiteL();
  }
  else{
    if (abs(encoderValueL) > abs(encoderValueR)) {
      forwardR();
      stopL();
    }
    if (abs(encoderValueL) < abs(encoderValueR)) {
      backwardL();
      stopR();
    }
  }
}

void MotorTurnOnsiteR() {
  if ((abs(encoderValueL)== abs(encoderValueR))) {
    turnCarOnsiteR();
  }
  else{
    if (abs(encoderValueL) < abs(encoderValueR)) {
      forwardL();
      stopR();
    }
    if (abs(encoderValueL) > abs(encoderValueR)) {
      backwardR();
      stopL();
    }
  }
}

void MotorFollowBlackline() {
  if (isBlackTL() and isBlackTR()) {
    MotorStraightForward();
    prev_act=0;
  } else {
    encoderValueL=0;
    encoderValueR=0;
  }
  if (isBlackTL() and !isBlackTR()) {
    forwardR();
    stopL();
    prev_act=1;
  }
  if (!isBlackTL() and isBlackTR()) {
    forwardL();
    stopR();
    prev_act=2;
  }
  if (!isBlackTL() and !isBlackTR()) {
    switch (prev_act) {
      case 0:
        MotorStraightForward();
        break;
      case 1:
        forwardR();
        stopL();
        break;
      case 2:
        forwardL();
        stopR();
        break;
    }
  }
}

void MotorFollowBackwardBlackline() {
  if (isBlackTL() and isBlackTR()) {
    MotorStraightBackward();
    prev_act=0;
  } else {
    encoderValueL=0;
    encoderValueR=0;
  }
  if (isBlackTL() and !isBlackTR()) {
    backwardL();
    stopR();
    prev_act=1;
  }
  if (!isBlackTL() and isBlackTR()) {
    backwardR();
    stopL();
    prev_act=2;
  }
  if (!isBlackTL() and !isBlackTR()) {
    switch (prev_act) {
      case 0:
        MotorStraightForward();
        break;
      case 1:
        backwardL();
        stopR();
        break;
      case 2:
        backwardR();
        stopL();
        break;
    }
  }
}
//////////////////////////////////////////
//Motor speed control 
//period = 10ms ... spd_period
//max spd=10
void SpdForward(int speed) {
  encoderValueL=0;
  encoderValueR=0;
  for (int i=0; i<spd_period; i++) {
    if (i<speed) {
      MotorStraightForward();
    } else {
      stopCar();
    }
    delay(1);
  }
  real_spd=encoderValueL+encoderValueR;
}

void SpdTurnOnsiteL(int speed) {
  encoderValueL=0;
  encoderValueR=0;
  for (int i=0; i<spd_period; i++) {
    if (i<speed) {
      MotorTurnOnsiteL();
    } else {
      stopCar();
    }
    delay(1);
  }
}

void SpdTurnOnsiteR(int speed) {
  encoderValueL=0;
  encoderValueR=0;
  for (int i=0; i<spd_period; i++) {
    if (i<speed) {
      MotorTurnOnsiteR();
    } else {
      stopCar();
    }
    delay(1);
  }
}

void SpdTurnL(int speed) {
  for (int i=0; i<spd_period; i++) {
    if (i<speed) {
      turnCarL();
    } else {
      stopCar();
    }
    delay(1);
  }
}

void SpdTurnR(int speed) {
  for (int i=0; i<spd_period; i++) {
    if (i<speed) {
      turnCarR();
    } else {
      stopCar();
    }
    delay(1);
  }
}

void SpdFollowBlackline(int speed) { 
  encoderValueL=0;
  encoderValueR=0;
  for (int i=0; i<spd_period; i++) {
    InputCapture();
    if (i<speed) {
      MotorFollowBlackline();
    } else {
      //MotorFollowBlackline();
      if (!isBlackTL() or !isBlackTR()) { //avoid reduce spd in triming
        MotorFollowBlackline();
      } else {
        stopCar();
      }
    }
    delay(1);
  }
}

void SpdForward_noResetEncoder(int speed) {
  int prev_encoderL=encoderValueL;
  int prev_encoderR=encoderValueR;
  
  for (int i=0; i<spd_period; i++) {
    if (i<speed) {
      MotorStraightForward();
    } else {
      stopCar();
    }
    delay(1);
  }
  real_spd=(encoderValueL-prev_encoderL)+(encoderValueR-prev_encoderR);
}

void realSpdForward_noResetEncoder(int speed) {
    if (real_spd<speed) {
      SpdForward_noResetEncoder(10);
    } else {
      SpdForward_noResetEncoder(speed);
    }
}

void realSpdForward(int speed) {
    if (real_spd<speed) {
      SpdForward(10);
    } else {
      SpdForward(speed);
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
    display.fillScreen(BLACK);
    
    display.setTextColor(CYAN);
    display.setCursor(0, 0);
    display.println("HIDE ANY");
    display.println("PHOTO");
    display.println("SENSOR");
    display.println("TO START");
    display.setTextColor(WHITE);
    
    InputCapture();
    while (!photoR and !photoL) {
      InputCapture();
    }
    display.fillScreen(BLACK);
}



void oledVolt() {
    display.setTextSize(1);
    display.setCursor(0, 0);    
    display.println("ANA");
    display.println("LOG");
    display.println(" ");
    display.println("VOLT");
    display.println(" ");
    display.println("DIST");
    
    display.setTextSize(2);
    display.setCursor(25, 0);
    display.fillRect(25,0,59,15,BLACK);
    display.print(powerValue);
    
    display.setCursor(25, 17);
    display.fillRect(25,17,59,15,BLACK);
    display.print(powerVoltage());
    display.setTextSize(1);
    display.setCursor(84, 24);
    display.print("V");
   
    display.setTextSize(2);
    display.setCursor(25, 34);
    display.fillRect(25,34,59,15,BLACK);
    display.print(distance);
    display.setTextSize(1);
    display.setCursor(84, 40);
    display.print("cm");
}
//////////////////////////////////////////////////////
//power sensor
float powerVoltage() {    
    float vin = (powerValue * 5.0) / 200.0; 
    if (vin<0.01) {
      vin=0.0; }  
    return vin;    
}

//////////////////////////////////////////////////////
//IR sensor
bool isBlackTR() {
  if (blackLineAnalogTR>=BlackThresholdTR) {
    return true;
  }
  else {
    return false;
  }
}

bool isBlackTL() {
  if (blackLineAnalogTL>=BlackThresholdTL) {
    return true;
  }
  else {
    return false;
  }
}

bool isBlackFR() {
  if (blackLineAnalogFR>=BlackThresholdFR) {
    return true;
  }
  else {
    return false;
  }
}

bool isBlackFL() {
  if (blackLineAnalogFL>=BlackThresholdFL) {
    return true;
  }
  else {
    return false;
  }
}
//////////////////////////////////////////////////////
//Interrupt subroutine
void countL() {
 
  if (digitalRead(interruptL2)) {
    encoderValueL--;
  }
  else {
    encoderValueL++;
  }
}
void countR() {
  
  if (digitalRead(interruptR2)) {
    encoderValueR++;
  }
  else {
    encoderValueR--;
  }
}
//////////////////////////////////////////////////////
//LED
void onLED() {
  digitalWrite(pinLED, 1); 
}

void offLED() {
  digitalWrite(pinLED, 0); 
}
//////////////////////////////////////////////////////
//UltraSonic subroutine
void UltraSonic() {
  if (UltraSonicDone) {
    UltraSonicDone = 0;
    UltraSonicStartTime = millis();
    digitalWrite(trigPin, LOW);  // Added this line
  }
  if (millis() > UltraSonicStartTime + 2) {
    digitalWrite(trigPin, HIGH);
  }
  if (millis() > UltraSonicStartTime + 12) {
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH, 3000); //set 3000ns as timout
    distance = (duration / 2) / 29.1;
    UltraSonicDone = 1;
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
