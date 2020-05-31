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
//search function variables init.
const int SIZE = 5; //MAX SIZE = 8, i.e. 9x9 or x=0~8 and y=0~8
int START = 0; //xy
int DEST_default[(SIZE+1)*(SIZE+1)];

int Facing = 0;
int NowPos = START;

//DON'T CHANGE THIS
int Route[2];
int DEST[(SIZE+1)*(SIZE+1)]; 

/*facing
	    1 |(y-dir)
        |
	2------------0 (x-dir)
        |
        |3
*/

////////////////////////////////////////////////////////
//red spot related and 2-car mode/wifi
bool is2car=false;
bool isRedspotFound=false;
int RedspotPos=-1; //in 2car mode, -1 mean another car still not found
int Car=1;

struct MqttMsg {
  String topic;
  String payload;
};

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
  //wait wifi setup
  wait_esp8266_connected();
  /////////////////////////////////////////
  //OLED setup
  display.begin();
  oledStartPrint();
  
  //LED 
  pinMode(pinLED, OUTPUT);
  
  //array init
  arrInitToSize(DEST);
  for (int i=0; i<(SIZE+1)*(SIZE+1); i++) {
    DEST_default[i]=i/(SIZE+1)+(i%(SIZE+1))*10;
  }

  //search setup
  setupWizardOLED();

}

void loop()
{  
  oledPos();
  
  DEST[elementIndexinArray(NowPos,DEST)]=-1;
  if (!is2car) { //1-car mode
    //start from 0,0
    MotorControl_locFind(50);
    MotorControl_locFind(51);
    MotorControl_locFind(1);
    MotorControl_locFind(2);
    MotorControl_locFind(52);
    
    MotorControl_locFind(53);
    MotorControl_locFind(3);
    MotorControl_locFind(4);
    MotorControl_locFind(54);
    MotorControl_locFind(55);
    MotorControl_locFind(5);
  } else { //2-car
    if (NowPos==0) { //start from 0,0
      MotorControl_locFind(50);
      MotorControl_locFind(51);
      MotorControl_locFind(1);
      MotorControl_locFind(2);
      MotorControl_locFind(52);
    } else { //start from 5,5
      MotorControl_locFind(5);
      MotorControl_locFind(4);
      MotorControl_locFind(54);
      MotorControl_locFind(53);
      MotorControl_locFind(3);
    }
  }
  
  //search is some point still not be found
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
//////////////////////////////////////////////////////
//Search function
int shortestDist(int startPos, int facing) {
  int min_cost=100;
  int min_dest_i=0;
  for (int i=0; i<(sizeof(DEST)/sizeof(DEST[0])); i++) {
    if (DEST[i]>=0) {
      int dist_x=DEST[i]/10-startPos/10;
      int dist_y=DEST[i]%10-startPos%10;
      int cost_to_dest=abs(dist_x)+abs(dist_y);
      
      //if turning occur in journey middle
      if (dist_x!=0 and dist_y!=0) {
        cost_to_dest++;
      }
      
      //if turning at starting
      if (facing==0 and dist_x<=0)	{
        cost_to_dest++;
      }
      if (facing==1 and dist_y<=0)	{
        cost_to_dest++;
      }
      if (facing==2 and dist_x>=0)	{
        cost_to_dest++;
      }
      if (facing==3 and dist_y>=0)	{
        cost_to_dest++;
      }
      
      if (cost_to_dest < min_cost)  {
        min_cost = cost_to_dest;
        min_dest_i = i;
      }
    }
  }
  int min_dest=DEST[min_dest_i];
  DEST[min_dest_i]=-1; //i.e. remove that element
  return min_dest;
}

void findRoute(int destPos) {
  int now_x=NowPos/10;
  int now_y=NowPos%10;
  int dest_x=destPos/10;
  int dest_y=destPos%10;
  int dist_x=dest_x-now_x;
  int dist_y=dest_y-now_y;
  
  //if no turning is needed
  if (now_x==dest_x or now_y==dest_y) {
    Route[0]=10*dest_x+dest_y;
    Route[1]=-1;
  }
  else {
    int tpx=0; //pos of turning pt.
    int tpy=0;
    
    //dest in forward
    if ((Facing==0 and dist_x>0) or (Facing==2 and dist_x<0))	{
      tpx=dest_x;
      tpy=now_y;
    }
  	if ((Facing==1 and dist_y>0) or (Facing==3 and dist_y<0))	{
      tpx=now_x;
      tpy=dest_y;
    }
  	
  	//dest in backward
  	if ((Facing==0 and dist_x<0) or (Facing==2 and dist_x>0))	{
      tpx=now_x;
      tpy=dest_y;
    }
  	if ((Facing==1 and dist_y<0) or (Facing==3 and dist_y>0))	{
      tpx=dest_x;
      tpy=now_y;
    }
      
    Route[0]=10*tpx+tpy;
    Route[1]=10*dest_x+dest_y;
  }
}

void posForwardUpdate2() {
  switch (Facing) {
    case 0: //+x 
      NowPos+=10;
      break;
    case 1: //+y
      NowPos+=1;
      break;
    case 2: //-x
      NowPos-=10;
      break;
    case 3: //-y
      NowPos-=1;
      break;
  }
  DEST[elementIndexinArray(NowPos,DEST)]=-1;
}
//////////////////////////////////////////////////////
//redspot and wifi
void redspotPosDecide() {
  if (is2car) {
    if (RedspotPos==-1) { //my car detect red spot
      RedspotPos=NowPos;
      //publish to mqtt
      Serial2.print("PUB,");
      Serial2.print(RedspotPos);
      Serial2.print("\n");
    }
  } else {
    RedspotPos=NowPos;
  }
}

void redspotPosReceive() {
  if (Serial2.available()) {
    isRedspotFound=true;
    
    String serial_str;
    serial_str = Serial2.readStringUntil('\n');

    MqttMsg mqtt_msg = parse_mqtt_msg(serial_str);
    RedspotPos=mqtt_msg.payload.toInt();
  }
}

void carNumSend() {
    //setup topic in esp wifi
    display.setCursor(0, 0);
    display.setTextColor(WHITE);  
    display.setTextSize(2);
    display.println("WAITING");
    display.println("CONNECT");
    display.println("TO WIFI");
    display.println("BOARD");
    
    bool successSend=false;
    while (!successSend) {
      Serial2.print("CAR");
      Serial2.print(Car);
      Serial2.print("\n");
        
      Serial.print("CAR");
      Serial.print(Car);
      Serial.print(" Sent\n");
      
      //check send success or fail
      if (Serial2.available()) {
        successSend=true;
      }
      delay(100);
    }
    display.fillScreen(BLACK);
}

void wait_esp8266_connected(){
  String serial_str("");
  do {
    while (Serial2.available()) {
      serial_str = Serial2.readStringUntil('\n');
      Serial.println(serial_str);
      Serial.println(serial_str != "CONNECTED\r");
    }
    if (serial_str != "CONNECTED\r")
      break;
  }while (serial_str != "CONNECTED\r");
}

MqttMsg parse_mqtt_msg(String str){
  
  MqttMsg mqtt_msg;

  // create char* that keeps str for subsequent operations
  char* c_str = new char[str.length() + 1];
  strcpy(c_str, str.c_str());

  // The C library function char *strtok(char *str, const char *delim) 
  // breaks string str into a series of tokens using the delimiter delim.
  char* sub_str = strtok(c_str, ",");
  
  if (strcmp(sub_str, "MSG") != 0) {
    return mqtt_msg; // str does not contain MQTT message
  }
  
  sub_str = strtok(NULL, ",");
  if (sub_str == NULL) {
    return mqtt_msg; // str not complete
  }
  mqtt_msg.topic = String(sub_str);
  
  sub_str = strtok(NULL, ",");
  if (sub_str == NULL) {
    return mqtt_msg; // str not complete
  }
  mqtt_msg.payload = String(sub_str);

  return mqtt_msg;
}

//////////////////////////////////////////////////////
//array related function

int arrayReverse(int arr[]) {
  int i=0;
  int j=elementIndexinArray(-1,arr)-1;
  int temp;
  while (i<j) {
    temp=arr[i];
    arr[i]=arr[j];
    arr[j]=temp;
    i++;
    j--;
  }
}

int findElementinArray2D(int element, int arr[][2], int rowFind, int rowReturn) {
  int n=(SIZE+1)*(SIZE+1);
  for (int i=0; i<n ;i++) {
    if (element==arr[i][rowFind]) {
      return arr[i][rowReturn];
    }
  }
  return -1; //not find
}

int elementIndexinArray(int element,int arr[]) {
  int n=(SIZE+1)*(SIZE+1);
  for (int i=0; i<n ;i++) {
    if (element==arr[i]) {
      return i;
    }
  }
  return -1; //not find
}

int elementIndexinArray2D(int element, int arr[][2], int rowFind) {
  int n=(SIZE+1)*(SIZE+1);
  for (int i=0; i<n ;i++) {
    if (element==arr[i][rowFind]) {
      return i;
    }
  }
  return -1; //not find
}

int isElementinArray(int element, int arr[]) {
  int n=(SIZE+1)*(SIZE+1);
  for (int i=0; i<n; i++) {
    if (element==arr[i]) {
      return true;
    }
  }
  return false;
}

int isElementinArray2D(int element, int arr[][2], int rowFind) {
  int n=(SIZE+1)*(SIZE+1);
  for (int i=0; i<n; i++) {
    if (element==arr[i][rowFind]) {
      return true;
    }
  }
  return false;
}

void arrayAppend(int arr[], int element) {
  int i=0;
  while(i<(SIZE+1)*(SIZE+1) and arr[i]>=0) { 
    i++;   
  }
  if (arr[i]<0) {
    arr[i]=element;
  }
}

void arrayAppend2D(int arr[][2], int element1, int element2) { 
  int i=0;
  while(i<(SIZE+1)*(SIZE+1) and arr[i][0]>=0) { 
    i++;   
  }
  if (arr[i][0]<0) {
    arr[i][0]=element1;
    arr[i][1]=element2;
  }
}

void arrInitToSize(int arr[]) {
  for (int i=0; i<(SIZE+1)*(SIZE+1); i++) {
    arr[i]=-1;
  }
}

void arrInitToSize2D(int arr[][2]) {
  for (int i=0; i<(SIZE+1)*(SIZE+1); i++) {
    for (int j=0; j<2; j++) {
      arr[i][j]=-1;
    }
  }
}

bool isArrEmpty2D(int arr[][2], int rowFind) {
  for (int i=0; i<(SIZE+1)*(SIZE+1); i++) {
    if (arr[i][rowFind]!=-1) {
      return false;
    }
  }
  return true;
}


///////////////////////////////////////////
//setup wizard screen on OLED
bool setupWizard_default(int tDelay) {  //phase 1
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("A> START");
    display.println("B> MENU");
    
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print("A      B");
    display.setTextColor(WHITE); 
    
    delay(tDelay);
    InputCapture();
    while (!photoR and !photoL) {
      InputCapture();
    }
    
    display.fillScreen(BLACK);
    if (photoL) {
      return false;
    } else {
      return true;
    }
}

void setupWizard_colourCalibrat(int tDelay) {
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print("   OK");
    display.setTextColor(WHITE);
    
    delay(tDelay);
    InputCapture();
    while (!photoR and !photoL) {
      InputCapture();
      ColorInput();
      oledColourPrint();
      delay(200);
    }
    
    int diff=50;
    colorRedThreshold[0]=colorValueC-diff;
    colorRedThreshold[1]=colorValueC+diff;
    colorRedThreshold[2]=colorValueR-diff;
    colorRedThreshold[3]=colorValueR+diff;
    colorRedThreshold[4]=colorValueB-diff;
    colorRedThreshold[5]=colorValueB+diff;
    colorRedThreshold[6]=colorValueG-diff;
    colorRedThreshold[7]=colorValueG+diff;
    
    display.fillRect(0,45,100,15,BLACK);
    display.setTextColor(GREEN); 
    display.setCursor(0,45);
    display.print("  DONE");
    display.setTextColor(WHITE);
    delay(2000);
    display.fillScreen(BLACK);
}

void setupWizard_mode(int tDelay) {
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print("SEL   +1");
    display.setTextColor(WHITE);
    
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("  MODE");
    display.println("1-car");
    display.println("2-car");
    
    display.setTextColor(GREEN);
    display.setCursor(82, 15);
    display.print("*");
    
    delay(tDelay);
    InputCapture();
    while (!photoR) {
      InputCapture();
      if (photoL) {
        is2car=!is2car;
        display.fillRect(82,0,15,45,BLACK);
        if (!is2car) {
          display.setCursor(82, 15);
        } else {
          display.setCursor(82, 30);
        }
        display.print("*");
        delay(tDelay);
      }
    }
    display.setTextColor(WHITE);    
    display.fillScreen(BLACK);
}

void setupWizard_pos2(int tDelay) {
    display.fillScreen(BLACK);
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print("SEL   +1");
    display.setTextColor(WHITE); 
    
    //phase 2
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("START   START");
    display.println("POS     Facing");
    
    int searchSetup_facing=0;
    int searchSetup_pos=0;
    
    display.setTextSize(2);
    display.setCursor(0, 15);
    display.println("0,0  0");
    display.println("5,5  2");
    
    int selection=0;
    display.setTextColor(GREEN);
    display.setCursor(82, 15);
    display.print("*");
    
    delay(tDelay);
    InputCapture();
    while (!photoR) {
      InputCapture();
      if (photoL) {
        selection=(selection+1)%2;
        display.fillRect(82,0,15,45,BLACK);
        display.setCursor(82, 15+selection*15);
        display.print("*");
        delay(tDelay);
      }
    }
    display.setTextColor(WHITE);
    display.fillScreen(BLACK);
    
    if (selection==0) { //0,0 0 Car1
      Facing=0;
      NowPos=0;
      Car=1;
    } else { //5,5 2 Car2
      Facing=2;
      NowPos=55;
      Car=2;
    }
    carNumSend();
}

void setupWizard_result2(int tDelay) {
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.setTextSize(2);
    display.print("  START");
    display.setTextColor(WHITE); 
    
    display.setCursor(0,0);
    display.setTextSize(1);
    display.println("");
    display.println("MODE");
    display.println("");
    display.println("FACE");
    display.println("");
    display.println("POS");
    
    //mode
    display.setCursor(30,0);
    display.setTextSize(2);
    if (is2car) {
      display.print("2-car");
    } else {
      display.print("1-car");
    }
    
    //facing
    display.setCursor(30,15);
    display.print(Facing);
    
    //pos
    display.setCursor(30,30);
    display.print(NowPos/10);
    display.print(",");
    display.print(NowPos%10);
    
    delay(tDelay);
    InputCapture();
    while (!photoR and !photoL) {
      InputCapture();
    }    
    display.fillScreen(BLACK);
}

void setupWizardOLED() {
    int tDelay=250;
    if (!setupWizard_default(tDelay)) {      
      //setupWizard_colourCalibrat(tDelay);
      setupWizard_mode(tDelay);   //2 car mode? 
      if (is2car) {
        setupWizard_pos2(tDelay); 
      }
    } 
    setupWizard_result2(tDelay);
    
    if (is2car) {
      if (Facing==0) {
        for (int i=0; i<(SIZE+1)*(SIZE+1)/2; i++) {
          DEST[i]=DEST_default[i];
        }
      } else {
        for (int i=0; i<(SIZE+1)*(SIZE+1)/2; i++) {
          DEST[i]=DEST_default[i+(SIZE+1)*(SIZE+1)/2];
        }
      }
    } else {
      for (int i=0; i<(SIZE+1)*(SIZE+1); i++) {
        DEST[i]=DEST_default[i];
      }
    }
    display.fillScreen(BLACK);
}
//////////////////////////////////////////////////////
//Motor control
void MotorControl_locFind(int pos) { //accept straight line travel only
    int forwardDelayt=150;
    int forwardSpd=10;
    int turnDelayt=200;
    int turnSpd=10;
    
    int facingTurnTo=-1; //check next face to where and then turning
    if ((pos/10-NowPos/10)>0) {
      facingTurnTo=0;
    } else {
      if ((pos/10-NowPos/10)<0) {
        facingTurnTo=2;
      } else {   
        if ((pos%10-NowPos%10)>0) {
          facingTurnTo=1;
        } else {   
          if ((pos%10-NowPos%10)<0) {
            facingTurnTo=3;
          } 
        }
      }
    }  
    
    //act only when moving is needed
    if (facingTurnTo!=-1) {
      //1. turning
      if (Facing!=facingTurnTo) {
        if ((facingTurnTo-Facing+4)%4==1) {
          TurnBlacklineL(turnDelayt, turnSpd, true);
        } else {
          if (abs(facingTurnTo-Facing)==2) {           
            TurnBlacklineR(turnDelayt, turnSpd, true);
            TurnBlacklineR(turnDelayt, turnSpd, true);
            //MotorFollowBlackline();
          } else {
            TurnBlacklineR(turnDelayt, turnSpd, true);
          }
        }
      }
      
      redspotPosReceive();
      if (isRedspotFound) {
        stopCar();
        redspotPosDecide();
        oledRedspotFound();
      }
      //2. go straight
      int forwardStep=abs(pos/10-NowPos/10)+abs(pos%10-NowPos%10);
      for (int i=0; i<forwardStep;i++) { //Task ii: check obstacle here
        ForwardBlackline(forwardDelayt, forwardSpd);
        redspotPosReceive();
        if (isRedspotFound) {
          stopForwardCar();
          redspotPosDecide();
          oledRedspotFound();
        }
      }
      stopForwardCar();
    }
    oledPos();  
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
      stopCar;
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

void realSpdForward(int speed) {
    if (real_spd<speed) {
      SpdForward(10);
    } else {
      SpdForward(speed);
    }
}
//////////////////////////////////////////
//Motor control by blackline

void ForwardBlackline(int tDelay, int speed) {
    InputCapture();
    ColorInput();
    encoderValueL=0;
    encoderValueR=0;
    for (int i=0; (i<tDelay) or (!isBlackFL() and !isBlackFR()); i++) {
      InputCapture();
      if ((i%10)<speed or i<tDelay) {
        MotorFollowBlackline();
        if (i>tDelay) {
          ColorInput();
        }
      }
      else {
        if (!isBlackTL() or !isBlackTR()) { //avoid reduce spd in triming
          MotorFollowBlackline();
        } else {
          stopCar();
        }
      }
      delay(1);
    }
    posForwardUpdate2();
}

void TurnBlacklineL(int tDelay, int speed, bool isFaceChange) {  //max spd 10
    InputCapture();
    encoderValueL=0;
    encoderValueR=0;
    
    for (int i=0; (i<tDelay) or (!isBlackTL() or !isBlackTR()); i++) {
      InputCapture();
      //i%10 = PWM, tDelay = force accel, ignore sensor until delay
      if ((i%10)<speed or i<tDelay) {
        MotorTurnOnsiteL();
      } else {
        stopCar();
      }
      delay(1);
    } 
    stopCar();
    delay(50);
    InputCapture();
    while (!isBlackTL() or !isBlackTR()) { //overshoot
      TurnBlacklineR(0,7, false); 
    }
    
    if (isFaceChange) { //change face
      Facing=(Facing+1)%4;
    }
}

void TurnBlacklineR(int tDelay, int speed, bool isFaceChange) {  //max spd 10
    InputCapture();
    encoderValueL=0;
    encoderValueR=0;
    for (int i=0; (i<tDelay) or (!isBlackTL() or !isBlackTR()); i++) {
      InputCapture();
      
      if ((i%10)<speed or i<tDelay) {
        MotorTurnOnsiteR();
      } else {
        stopCar();
      }
      delay(1);
    } 
    stopCar();
    delay(50);
    InputCapture();
    while (!isBlackTL() or !isBlackTR()) { //overshoot
      TurnBlacklineL(0,7, false); 
    }
    
    if (isFaceChange) { //change face
      Facing=(Facing+3)%4;
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
}

void oledPrint() {
    display.fillScreen(BLACK);
    display.setCursor(0, 0);
    display.setTextColor(WHITE);  
    display.setTextSize(2);
    display.print("TOP R/L: ");

    display.setCursor(0, 15);
    display.print(blackLineAnalogTR);
    display.print(",");
    display.print(blackLineAnalogTL);

    display.setCursor(0, 30);
    display.print("FAR R/L: ");

    display.setCursor(0, 45);
    display.print(blackLineAnalogFR);
    display.print(",");
    display.print(blackLineAnalogFL);
    
    delay(1000);
}

void oledPos() {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("Facing:");
    
    display.setCursor(0, 30);
    display.print("NOW POS:");
    
    display.setCursor(0, 13);
    display.setTextSize(2);
    display.fillRect(0,13,20,15,BLACK);
    display.print(Facing);
    
    display.setCursor(0, 43);
    display.fillRect(0,43,90,15,BLACK);
    display.print(NowPos/10);
    display.print(",");
    display.print(NowPos%10);
}

void oledRedspotFound() {
  onLED();
  delay(100);
  offLED();
  
  display.setTextSize(2);
  display.fillScreen(GREEN);
  display.setCursor(0,0);
  
  display.println("Redspot");
  display.print("found in  ");
  display.print(RedspotPos/10);
  display.print(",");
  display.print(RedspotPos%10);
  delay(5000);
  display.fillScreen(GREEN);
  
  while (1) {
    oledPos();
    delay(200);
  }
}

void oledRedspotNotFound() {
  display.setTextSize(2);
  display.fillScreen(RED);
  display.setCursor(0,0);
  
  display.println("Redspot");
  display.println("NOT ");
  display.print("found");
  
  for (int i=0; i<15; i++) {
    onLED();
    delay(200);
    offLED();
    delay(200);
  }
  
  display.fillScreen(RED);
  for (int i=0; i<15; i++) {
    oledPos();
    onLED();
    delay(100);
    offLED();
    delay(100);
  }
  
  display.fillScreen(RED);
  while(1) {
    ColorInput();
    oledColourPrint();
    onLED();
    delay(200);
    offLED();
    delay(200);
  }
}

void oledColourPrint() {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("C:");
    display.setCursor(47, 0);
    display.print("R:");
    
    display.setCursor(0, 23);
    display.print("B:");
    display.setCursor(47, 23);
    display.print("G:");
    
    display.fillRect(0,8,100,15,BLACK);
    display.fillRect(0,30,100,15,BLACK);
    display.setTextSize(2); 
       
    display.setCursor(0, 8);    
    display.print(colorValueC);
    display.setCursor(47, 8);    
    display.print(colorValueR);
    
    display.setCursor(0, 30);    
    display.print(colorValueB);
    display.setCursor(47, 30);    
    display.print(colorValueG);
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
//colour sensor
void ColorInput() {
//  if (colorCnt == 0) {
    //read Clear value
    colorValueC = pulseIn(colorpin_out, LOW);
    //Set Red filter
    digitalWrite(colorpin_s2, 0);
    digitalWrite(colorpin_s3, 0);
    colorCnt++;
//  }
//  else if (colorCnt == 1) {
    //read Red value
    colorValueR = pulseIn(colorpin_out, LOW);
    //Set Blue filter
    digitalWrite(colorpin_s2, 0);
    digitalWrite(colorpin_s3, 1);
    colorCnt++;
//  }
//  else if (colorCnt == 2) {
    //read Blue value
    colorValueB = pulseIn(colorpin_out, LOW);
    //Set Green filter
    digitalWrite(colorpin_s2, 1);
    digitalWrite(colorpin_s3, 1);
    colorCnt++;
//  }
// else {
    //read Green value
    colorValueG = pulseIn(colorpin_out, LOW);
    //Set Clear filter
    digitalWrite(colorpin_s2, 1);
    digitalWrite(colorpin_s3, 0);
    colorCnt = 0;
//  }
  
  detectRed();
}
void detectRed() {
  colorRedThreshold[4]=colorValueR+colorValueC;
  colorRedThreshold[6]=colorValueR+colorValueC;
  //Check Red Color (R=min)
  if ((colorRedThreshold[0] < colorValueC && colorValueC < colorRedThreshold[1]) &&
      (colorRedThreshold[2] < colorValueR && colorValueR < colorRedThreshold[3]) &&
      (colorRedThreshold[4] < colorValueB && colorValueB < colorRedThreshold[5]) &&
      (colorRedThreshold[6] < colorValueG && colorValueG < colorRedThreshold[7])) {
    colorCheckCnt++;
  } else {
    colorCheckCnt = 0;
  }
  //Continous detection before notification
  if (colorCheckCnt > 0) {
    colorCheckCnt = 1;
    isRedspotFound=true;
  } else {
    isRedspotFound=false;
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
