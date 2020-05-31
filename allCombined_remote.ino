/*
 * use realSpdForward(spd) or realSpdForward_noResetEncoder(spd) instead of forward() to make the car forward (spd:1~10)
 * see MotorControl_wirelessCharging() for controling car to stop in front of a wall
 * 
 */
#include <Wire.h>
//Mega2560 pin 20 (SDA), pin 21 (SCL)
//SPI pin
//MOSI 51
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
#define CYAN            0x07FF
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
int colorRedThreshold[] = {50,1000,  100,1000,  100,2000,  100,2000};//CRBG

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
float  distance=0.0;
int obsDistThreshold=20;
int obsDistThreshold_short=14;
bool isObstacle=false; //reset at addObstacle, change at checkObstacle
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

//for remote control
char* prevAct_remote="A";
int spd_remote=0;

//////////////////////////////////////////////////////
//search function variables init.
const int SIZE = 5; //MAX SIZE = 8, i.e. 9x9 or x=0~8 and y=0~8
int START = 0; //xy
int DEST_default[] = {15,20,23,44}; //for other tasks
int DEST_locPtInterest[(SIZE+1)*(SIZE+1)]; //for task of locate pt of interest

int Facing = 0;
int NowPos = START;

//DON'T CHANGE THIS
int Route[2];
int DEST[(SIZE+1)*(SIZE+1)]; 
int Obstacle[(SIZE+1)*(SIZE+1)];

/*facing
    1 |(y-dir)
      |
2------------0 (x-dir)
      |
      |3
*/
////////////////////////////////////////////////////////
//2-car mode/wifi
bool is2car=false;
bool isRedspotFound=false;
int RedspotPos=-1; //in 2car mode, -1 mean another car still not found
int Car=1;
bool isRemoteTaskSel=false;

struct MqttMsg {
  String topic;
  String payload;
};

//task selection
int task_sel;
//////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);
  Serial2.begin(115200);
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
  
  //reset, then wait wifi setup and send car no.
  carNumSend();
  
  //LED 
  pinMode(pinLED, OUTPUT);
  
  //array init
  arrInitToSize(DEST);
  arrInitToSize(Obstacle);
  for (int i=0; i<(SIZE+1)*(SIZE+1); i++) {
    DEST_locPtInterest[i]=i/(SIZE+1)+(i%(SIZE+1))*10;
  }

  //search setup
  setupWizardOLED();
  
  //wait car 1 and 2 ready in task 1
  if (is2car and task_sel==1) {
      if (Car==1) {
        destLtPublish();
        waitCar2Ready();
      } else if (Car==2) {
        waitCar1DestLt();
      }
  }
}

void loop()
{  
    if (task_sel<=3) {
      oledPos();
    }

    int tDelay=250;
    switch (task_sel) {
      case 1:
          task_coordinNoObs();
          break;
      case 2:
          task_coordinObs();
          break;
      case 3:
          task_locPt();
          break;
      case 4:
          task_wirelessCharging();
          break;
      case 5:
          photoCalibration();
          setupWizard_startScreen(tDelay);
          task_lightTrack();
          break;
      case 6:
          task_measureIR();
          break;
      case 7:
          task_measureLIGHT();
          break;
      case 8:
          task_measureCOLOUR();
          break;
      case 9:
          task_measurePWR_DIST();
          break;
    }
}

//////////////////////////////////////////////////
//main program of different task

void task_coordinNoObs() { //sel 1
  if (is2car and Car==2) {
    //wait until info (path and dest) from another car get
    while (Obstacle[0]==-1) { //obs list not updated
      obsDestUpdate2car();
    }
  }
  
  int dest_cannot_reach[(SIZE+1)*(SIZE+1)];
  arrInitToSize(dest_cannot_reach);
  
  int cand_pos=shortestDist(NowPos,Facing);
  while (cand_pos!=-1 or dest_cannot_reach[0]!=-1) { //when DEST not empty 
    //using Uniform Cost Search to find route
    int routeUCS[(SIZE+1)*(SIZE+1)]; 
    int routeShortUCS[(SIZE+1)*(SIZE+1)];     
    findRouteUCS(cand_pos,routeUCS);
    routeShorten(routeUCS,routeShortUCS);
    
    if (is2car and routeShortUCS[0]!=-1) {
      //if there is path, publish it
      pathDestPublish(cand_pos, routeUCS);
    }
    
    //execute the route
    for (int i=0; (i<(SIZE+1)*(SIZE+1)) and (routeShortUCS[i]!=-1); i++) {
      MotorControl(routeShortUCS[i]);  
      oledPos();
    }
    
    onLED();
    delay(100);
    offLED();
    
    //check another car to update obs(path) list after finish its own path
    if (is2car) {
      obsDestUpdate2car();
      //if there is no path, find another cand_pos
      if (routeShortUCS[0]==-1) {
        arrayAppend(dest_cannot_reach,cand_pos);
        cand_pos=shortestDist(NowPos,Facing);
      } else {
        cand_pos=shortestDist(NowPos,Facing); //find next cand_pos if there is path
        for (int i=0; dest_cannot_reach[i]!=-1; i++) {//restore the dest list
          arrayAppend(DEST, dest_cannot_reach[i]); 
        }
        arrInitToSize(dest_cannot_reach);
      }
      
    } else {
      //in one-car mode, just update the dest list
      cand_pos=shortestDist(NowPos,Facing);
    }
    
    //all dest cannot be accessed as block by another car's path
    if (is2car and cand_pos==-1 and dest_cannot_reach[0]!=-1) {
      for (int i=0; dest_cannot_reach[i]!=-1; i++) {//restore the dest list
        arrayAppend(DEST, dest_cannot_reach[i]); 
      }
      arrInitToSize(dest_cannot_reach);
      while (Serial2.available()==0) {//wait until obs list updated
        delay(1);
      }
      obsDestUpdate2car();
      cand_pos=shortestDist(NowPos,Facing);
    }
  }

  stopCar();
  while(1){
      InputCapture();  
      oledPos();
      delay(1000);
  }
}

void task_coordinObs() { //sel 2
  int destReached=-1; //-1= not reach, otherwise is dest coordinate
  int cand_pos=shortestDist(NowPos,Facing);
  while (cand_pos!=-1) { //when DEST not empty 
    distance=0;
    destReached=-1;
    int routeUCS[(SIZE+1)*(SIZE+1)]; 
    int routeShortUCS[(SIZE+1)*(SIZE+1)];  
    findRouteUCS(cand_pos,routeUCS);
    routeShorten(routeUCS,routeShortUCS);
/*
    Serial.println("");
    Serial.print(NowPos);
    Serial.print("|");
    for (int i=0; routeUCS[i]>=0; i++) {
      Serial.print(routeUCS[i]);
      Serial.print(",");
    }
    Serial.println("");
    for (int i=0; routeShortUCS[i]>=0; i++) {
      Serial.print(routeShortUCS[i]);
      Serial.print(",");
    }
    Serial.println("");
*/
    for (int i=0; (i<(SIZE+1)*(SIZE+1)) and (routeShortUCS[i]!=-1) and !isObstacle; i++) {
      MotorControl_Obstacle(routeShortUCS[i]);  
      oledPos();
    }
    
    isObstacle=false; //RESET FLAG HERE
    if (NowPos==cand_pos) { //dest reached
      onLED();
      delay(100);
      offLED();
      destReached=cand_pos;
      cand_pos=shortestDist(NowPos,Facing);
    } else {
      if (routeUCS[0]==-1) { //no route
        destReached=-1*cand_pos-1;
        cand_pos=shortestDist(NowPos,Facing);
      }
    }
  }

  stopCar();
  oledDestReach(destReached); //last for 30s
  
  while(1){
      InputCapture();
      UltraSonic();  
      oledPos();
      if (destReached<0) {
        onLED();
        delay(100);
        offLED();
        delay(100);
      }
  }
}

void task_locPt() { //sel 3
  //SIZE=5 only
  DEST[elementIndexinArray(NowPos,DEST)]=-1;
  if (!is2car) { //1-car mode
    //start from 0,0
    int MotorPath[]={50,51, 1,2, 52,53, 3,4, 54,55, 5, -1};
    for (int i=0; MotorPath[i]>=0; i++) {
      MotorControl_locFind(MotorPath[i]);
    }
  } else { //2-car
    if (NowPos==0) { //start from 0,0
      int MotorPath[]={50,52, 2,1, 41, -1};
      for (int i=0; MotorPath[i]>=0; i++) {
        MotorControl_locFind(MotorPath[i]);
      }
    } else { //start from 5,5
      int MotorPath[]={5,4, 54,53, 3, -1};
      for (int i=0; MotorPath[i]>=0; i++) {
        MotorControl_locFind(MotorPath[i]);
      }
    }
  }
  
  //search if some point still not found
  int cand_pos=shortestDist(NowPos,Facing);
  while (cand_pos!=-1) { //when DEST not empty 
    findRoute(cand_pos);        
    for (int i=0; (i<(sizeof(Route)/sizeof(Route[0]))) and (Route[i]!=-1); i++) {
      MotorControl_locFind(Route[i]);
    }
    cand_pos=shortestDist(NowPos,Facing);
  }
  
  stopCar();
  oledRedspotNotFound();
}

void task_wirelessCharging() { //sel 4
  MotorControl_wirelessCharging();
  stopCar(); 
  while(1){
      InputCapture();  
      UltraSonic();
      oledVolt();
  }
}

void task_lightTrack() { //sel 5
    //display
    display.fillScreen(BLACK);
    display.setCursor(0, 0);
    display.setTextColor(WHITE);  
    display.setTextSize(1);
    display.print("PhotoR/L");
        
    display.setCursor(0, 30);
    display.print("Lvl R/L");
    //start running
    for (int i=0; i<300000; i++) {
      MotorLightTrack();
      oledLight();
    }
    
    stopCar();
    while(1){
      InputCapture();  
      oledLight();
    }
}


void task_measureIR() {
  display.fillScreen(BLACK); 
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("NOW TOP R/L:");
    
    display.setCursor(0, 30);
    display.print("NOW FAR R/L:");
    
    while (1) {
      InputCapture();
      if (blackLineAnalogTR>999) {
        blackLineAnalogTR=999;
      }
      if (blackLineAnalogTL>999) {
        blackLineAnalogTL=999;
      }
      if (blackLineAnalogFR>999) {
        blackLineAnalogFR=999;
      }
      if (blackLineAnalogFL>999) {
        blackLineAnalogFL=999;
      }
      
      //display NOW FR/FL
      display.setCursor(0, 13);
      display.setTextSize(2);
      display.fillRect(0,13,100,15,BLACK);
      display.print(blackLineAnalogTR);
      display.print(",");
      display.print(blackLineAnalogTL);

      
      //display NOW TR/TL
      display.setCursor(0, 43);
      display.setTextSize(2);
      display.fillRect(0,43,100,15,BLACK);
      display.print(blackLineAnalogFR);
      display.print(",");
      display.print(blackLineAnalogFL);
    }
}

void task_measureLIGHT() {
    display.fillScreen(BLACK); 
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("NOW Photo R/L:");

    while (1) {
      InputCapture();
      if (photoAnalogR>999) {
        photoAnalogR=999;
      }
      if (photoAnalogL>999) {
        photoAnalogL=999;
      }
      
      //display NOW photo sensor
      display.setCursor(0, 13);
      display.setTextSize(2);
      display.fillRect(0,13,100,15,BLACK);
      display.print(photoAnalogR);
      display.print(",");
      display.print(photoAnalogL);
    }
}

void task_measureCOLOUR() {
  while (1) {
    ColorInput();
    oledColourPrint();
  }
}

void task_measurePWR_DIST() {
  while (1) {
    InputCapture();
    UltraSonic();
    oledVolt();
  }
}

//////////////////////////////////////////////
