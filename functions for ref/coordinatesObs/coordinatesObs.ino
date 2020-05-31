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
float  distance=0.0;
int obsDistThreshold=18;
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
int DEST_default[] = {45}; //MAX 10 destination

int Facing = 0;
int NowPos = START;

//DON'T CHANGE THIS
int Route[2];
int DEST[] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}; 
int Obstacle[(SIZE+1)*(SIZE+1)];

/*facing
	    1 |(y-dir)
        |
	2------------0 (x-dir)
        |
        |3
*/
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
  
  //search setup
  setupWizardOLED();
  
  //array init
  arrInitToSize(Obstacle);
}

void loop()
{  
  oledPos();
  //Obstacle[0]=1;
  //Obstacle[1]=10;
  int destReached=-1; //-1= not reach, otherwise is dest coordinate
  int cand_pos=shortestDist(NowPos,Facing);
  while (cand_pos!=-1) { //when DEST not empty 
    distance=0;
    destReached=-1;
    int routeUCS[(SIZE+1)*(SIZE+1)]; 
    int routeShortUCS[(SIZE+1)*(SIZE+1)];  
    findRouteUCS(cand_pos,routeUCS);
    routeShorten(routeUCS,routeShortUCS);

    for (int i=0; (i<(SIZE+1)*(SIZE+1)) and (routeShortUCS[i]!=-1) and !isObstacle(); i++) {
      MotorControl_Obstacle(routeShortUCS[i]);  
      oledPos();
    }
    
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
  oledDestReach(destReached);
  
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

void findRouteUCS(int destPos, int routeUCS[]) {
  int frontier[(SIZE+1)*(SIZE+1)][2];
  int prev_pos[(SIZE+1)*(SIZE+1)][2];
  int prev_pos_facing[(SIZE+1)*(SIZE+1)];
  arrInitToSize2D(frontier); //init to -1
  arrInitToSize2D(prev_pos);
  arrInitToSize(prev_pos_facing);
  
  frontier[0][0]=0;       //cumulative cost
  frontier[0][1]=NowPos;  //pos
  
  prev_pos[0][0]=NowPos;  //start from
  prev_pos[0][1]=-1;      //its prev pos. (=NONE)
  prev_pos_facing[0]=Facing;  //facing
  
  while (!isArrEmpty2D(frontier,0)) { //while frontier not empty
    int minCostIndex=findMinCostIndex(frontier);
    int prev_cost=frontier[minCostIndex][0];
    int now_pos=frontier[minCostIndex][1];
    int now_facing=prev_pos_facing[elementIndexinArray2D(now_pos,prev_pos,0)];
    frontier[minCostIndex][0]=-1; //remove min cost frontier
  
    int neighbour[]={-1,-1,-1,-1};
    int moving_cost[]={-1,-1,-1,-1};
    int next_facing[]={-1,-1,-1,-1};
    findNeighbour(now_pos,now_facing,neighbour,moving_cost,next_facing);

    for (int i=0; i<4; i++) {
      if (neighbour[i]>=0) {//valid neighbour
        int next_cost=prev_cost+moving_cost[i];
        //cost to neighbour less than another route or neighbour is not prev pos 
        if (next_cost<findElementinArray2D(neighbour[i],frontier,1,0) or !isElementinArray2D(neighbour[i],prev_pos,0)) {
          arrayAppend2D(frontier,next_cost,neighbour[i]);
          //update prev_pos
          if (isElementinArray2D(neighbour[i], prev_pos,0)) { 
            int j=elementIndexinArray2D(neighbour[i],prev_pos,0);
            prev_pos[j][0]=neighbour[i];
            prev_pos[j][1]=now_pos;
            prev_pos_facing[j]=next_facing[i];
          } else {
            arrayAppend2D(prev_pos,neighbour[i],now_pos);
            arrayAppend(prev_pos_facing,next_facing[i]);
          }
        }
      }
    }
  }

  //linking dest in prev_pos to start pos
  arrInitToSize(routeUCS);
  routeTraceback(routeUCS,NowPos,destPos,prev_pos);
}

void posForwardUpdate() {
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
}

void routeShorten(int arr[], int arrReturn[]) {
  if (arr[0]!=-1) {
    arrInitToSize(arrReturn);
    int chng1=NowPos-arr[0];
    int chng2=arr[0]-arr[1];
    for (int i=0; i<(SIZE+1)*(SIZE+1)-2; i++) {
      if (chng1!=chng2) {
        arrayAppend(arrReturn,arr[i]);
      }
      chng1=arr[i]-arr[i+1];
      chng2=arr[i+1]-arr[i+2];
    }
  }
}
//////////////////////////////////////////////////////
//UCS related function
int routeTraceback(int route[], int start_pos, int dest_pos, int prev_pos[][2]) {  
  if (isElementinArray2D(dest_pos,prev_pos,0)) {
    int now_pos=dest_pos;
    while (now_pos!=start_pos) {
      arrayAppend(route,now_pos);
      now_pos=findElementinArray2D(now_pos,prev_pos,0,1);
    }
    arrayReverse(route);
  }
}

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

int findMinCostIndex(int arr[][2]) {
  int n=(SIZE+1)*(SIZE+1);
  int temp;
  int min_i=0;
  while (arr[min_i][0]<0) {
    min_i++;
  }
  temp=arr[min_i][0];
  for(int i=min_i; i<n; i++) {
      if(temp>arr[i][0] and arr[i][0]>=0) { //-1 mean remove
         temp=arr[i][0];
         min_i=i;
      }
   }
   return min_i;
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

void findNeighbour(int pos, int facing, int neighbour[], int moving_cost[], int next_facing[]) {
  int temp[]={10,1,-10,-1}; //x++, y++, x--, y--
  for (int i=0; i<4; i++) {
    int next_pos=pos+temp[i];
    if (isValidPos(next_pos) and next_pos>=0) {      
      int cost=abs(facing-i);
      if (cost==3) {
        cost=1;
      }
      neighbour[i]=next_pos;
      moving_cost[i]=cost;
      next_facing[i]=i;
    }
  }
}

bool isValidPos(int pos) { //check within the map and not obstacle
  if (pos>=0 and (pos/10<=SIZE and pos%10<=SIZE)) {
    if (isElementinArray(pos,Obstacle)) {
      return false;
    } else {
      return true;
    }
  } else {
    return false;
  }
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
    
    if (photoL) {
      return false;
    } else {
      return true;
    }
}

int setupWizard_menu(int tDelay) {
    display.fillScreen(BLACK);
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print("SEL   +1");
    display.setTextColor(WHITE);
    
    display.setCursor(0, 0);
    display.println("Pos");
    display.println("Dest");
    display.println("BOTH");
    
    int selection=0;
    display.setTextColor(GREEN);
    display.setCursor(75, 0);
    display.print("*");
    
    delay(tDelay);
    InputCapture();
    while (!photoR) {
      InputCapture();
      if (photoL) {
        selection=(selection+1)%3;
        display.fillRect(75,0,15,45,BLACK);
        display.setCursor(75, selection*15);
        display.print("*");
        delay(tDelay);
      }
    }
    display.setTextColor(WHITE);
    return selection;
}

void setupWizard_pos(int tDelay) {
    display.fillScreen(BLACK);
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print("NX    +1");
    display.setTextColor(WHITE); 
    
    //phase 2
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("START");
    display.println("Facing");
    display.setCursor(0,23);
    display.println("START");
    display.println("POS");
    
    int searchSetup_facing=0;
    int searchSetup_pos=0;
    
    display.setTextSize(2);
    display.setCursor(50, 0);
    display.print(searchSetup_facing);
    display.setCursor(50, 23);
    display.print(searchSetup_pos/10);
    display.print(",");
    display.print(searchSetup_pos%10);
    
    delay(tDelay);
    display.fillRect(50,16,10,3,GREEN);
    InputCapture();
    while (!photoR) { //deciding facing
      if (photoL) { //+1 selected          
        searchSetup_facing=(searchSetup_facing+1)%4;
        display.fillRect(50,0,10,15,BLACK);
        display.setCursor(50, 0);
        display.print(searchSetup_facing);
        delay(tDelay);
      }
      InputCapture();
    }
    display.fillRect(50,16,10,3,BLACK);
    
    delay(tDelay);
    display.fillRect(50,39,10,3,GREEN);
    InputCapture();
    while (!photoR) { //deciding x pos
      if (photoL) {       
        searchSetup_pos=searchSetup_pos%10+((searchSetup_pos/10+1)%(SIZE+1))*10;
        display.fillRect(50,23,10,15,BLACK);
        display.setCursor(50, 23);
        display.print(searchSetup_pos/10);
        delay(tDelay);
      }
      InputCapture();
    }
    display.fillRect(50,39,10,3,BLACK);
    
    delay(tDelay);
    display.fillRect(74,39,10,3,GREEN);
    InputCapture();
    while (!photoR) { //deciding y pos
      if (photoL) {       
        searchSetup_pos=(searchSetup_pos/10)*10+(searchSetup_pos%10+1)%(SIZE+1);
        display.fillRect(74,23,10,15,BLACK);
        display.setCursor(74, 23);
        display.print(searchSetup_pos%10);
        delay(tDelay);
      }
      InputCapture();
    }
    display.fillRect(74,39,10,3,BLACK);
    
    Facing=searchSetup_facing;
    NowPos=searchSetup_pos;
}

void setupWizard_dest(int tDelay) {
    display.fillScreen(BLACK);
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print("NX    +1");
    display.setTextColor(WHITE); 

    //phase 3
    int no_dest=1;
    
    display.fillRect(0,0,100,45,BLACK);
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("No. of");
    display.println("Dest.");
    display.setCursor(70, 17);
    display.print(no_dest);
    
    delay(tDelay);
    display.fillRect(70,33,10,3,GREEN);
    InputCapture();
    while (!photoR) { //deciding no. of destination, max 10
      if (photoL) {       
        no_dest=(no_dest+1)%11;
        display.fillRect(70,17,25,15,BLACK);
        display.setCursor(70, 17);
        display.print(no_dest);
        delay(tDelay);
      }
      InputCapture();
    }
    display.fillRect(70,33,10,3,BLACK);
    
    //phase 4      
    delay(tDelay);
    for (int i=0; i<no_dest; i++) { //select pos of destination
      display.fillScreen(BLACK);
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.println("Dest pos");
      display.setTextSize(2);
      display.println(" Dest");
      
      display.setTextColor(CYAN); 
      display.setCursor(0,45);
      display.print("NX    +1");
      display.setTextColor(WHITE); 
      
      int dest_pos=0;
      display.fillRect(70,7,25,15,BLACK);
      display.fillRect(30,25,90,15,BLACK);
      display.setCursor(70, 7);
      display.print(i+1);
      
      display.setCursor(30, 25);
      display.print(dest_pos/10);
      display.print(",");
      display.print(dest_pos%10);
      
      //deciding x
      delay(tDelay);
      display.fillRect(30,41,10,3,GREEN);
      InputCapture();
      while (!photoR) {
        if (photoL) {       
          dest_pos=dest_pos%10+((dest_pos/10+1)%(SIZE+1))*10;
          display.fillRect(30,25,10,15,BLACK);
          display.setCursor(30, 25);
          display.print(dest_pos/10);
          delay(tDelay);
        }
        InputCapture();
      }
      display.fillRect(30,41,10,3,BLACK);
      
      //deciding y
      delay(tDelay);
      display.fillRect(54,41,10,3,GREEN);
      InputCapture();
      while (!photoR) {
        if (photoL) {       
          dest_pos=(dest_pos/10)*10+(dest_pos%10+1)%(SIZE+1);
          display.fillRect(54,25,10,15,BLACK);
          display.setCursor(54, 25);
          display.print(dest_pos%10);
          delay(tDelay);
        }
        InputCapture();
      }
      display.fillRect(54,41,10,3,BLACK);
      
      //confirmation
      display.fillScreen(BLACK);
      display.setCursor(0,0);
      display.println("CONFIRM?");
      
      display.setCursor(30,20);
      display.print(dest_pos/10);
      display.print(",");
      display.print(dest_pos%10);
      
      display.setTextColor(CYAN); 
      display.setCursor(0,45);
      display.print("N      Y");
      display.setTextColor(WHITE); 
      
      delay(tDelay);
      InputCapture();
      while (!photoR and !photoL) {
        InputCapture();
      }
      
      if (photoR) {
        i--;
        display.setCursor(30,40);
        display.setTextColor(RED); 
        display.print("XXX");
        display.setTextColor(WHITE); 
        delay(500);
      } else {
        DEST[i]=dest_pos;
      }
    }
}

bool setupWizard_result(int tDelay, String lastStr) {
    display.fillScreen(BLACK);
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print("  NEXT  ");
    display.setTextColor(WHITE);
    
    //display pos info
    display.setCursor(0,0);
    display.print("POS INFO");    
    
    display.setTextSize(1);
    display.setCursor(0, 15);
    display.println("NOW");
    display.println("Facing");
    display.setCursor(0,30);
    display.println("NOW");
    display.println("POS");
    
    int searchSetup_facing=0;
    int searchSetup_pos=0;
    
    display.setTextSize(2);
    display.setCursor(50, 15);
    display.print(Facing);
    display.setCursor(50, 30);
    display.print(NowPos/10);
    display.print(",");
    display.print(NowPos%10);
    
    delay(tDelay);
    InputCapture();
    while (!photoR and !photoL) {
      InputCapture();
    }
    
    //display dest list
    display.fillRect(0,0,100,45,BLACK);
    display.setCursor(0,0);
    display.println("DestInfo");
    int i=0;
    
    for (; i<4 and DEST[i]>=0; i++) { //list page 1
      if (i%2==1) {
        display.fillRect(45,15+(i/2)*15,5,15,GREEN);
      }
      
      display.setCursor((i%2)*60, 15+(i/2)*15);
      display.print(DEST[i]/10);
      display.print(",");
      display.print(DEST[i]%10);
    }
    
    if (DEST[i]>=0) {
      delay(tDelay);
      InputCapture();
      while (!photoR and !photoL) {
        InputCapture();
      }
      display.fillScreen(BLACK);
      
      display.fillRect(0,0,100,45,BLACK);
      for (; i<10 and DEST[i]>=0; i++) { //list page 2
        if (i%2==1) {
          display.fillRect(45,(i/2-2)*15,5,15,GREEN);
        }
        
        display.setCursor((i%2)*60, (i/2-2)*15);
        display.print(DEST[i]/10);
        display.print(",");
        display.print(DEST[i]%10);
      }
    }
    
    display.fillRect(0,45,100,15,BLACK);
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print(lastStr);
    display.setTextColor(WHITE);
    
    delay(tDelay);
    InputCapture();
    while (!photoR and !photoL) {
      InputCapture();
    }
    display.fillScreen(BLACK);
    
    if (photoR) {
      return true;
    } else {
      return false;
    }
}

void setupWizardOLED() {
    int tDelay=250;
    if (!setupWizard_default(tDelay)) {
      switch (setupWizard_menu(tDelay)) {
        case 0:
          setupWizard_pos(tDelay);
          for (int i=0; i<(sizeof(DEST_default)/sizeof(DEST_default[0])); i++) {
            DEST[i]=DEST_default[i];
          }
          break;
        case 1:
          setupWizard_dest(tDelay); 
          break;
        case 2:
          setupWizard_pos(tDelay);
          setupWizard_dest(tDelay); 
          break;
      }     
      setupWizard_result(tDelay, "  START");     
    } else {
      for (int i=0; i<(sizeof(DEST_default)/sizeof(DEST_default[0])); i++) {
        DEST[i]=DEST_default[i];
      }
    }
    display.fillScreen(BLACK);
}
//////////////////////////////////////////////////////
//Motor control
void MotorControl_Obstacle(int pos) { //accept straight line travel only
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
      
      delay(100);
      for (int i=0; i<1; i++) {
        UltraSonic();
        delay(200);
      }
      UltraSonic();
      if (!isObstacle()) {
        //2. go straight
        int forwardStep=abs(pos/10-NowPos/10)+abs(pos%10-NowPos%10);
        for (int i=0; i<forwardStep and !isObstacle();i++) { //Task ii: check obstacle here
          ForwardBlackline(forwardDelayt, forwardSpd);
        }
        stopForwardCar();
      }
    } 
    
    //add obstacle to list
    if (isObstacle()) {
      addObstacle();
    }
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
    UltraSonic();
    encoderValueL=0;
    encoderValueR=0;
    for (int i=0; (i<tDelay) or (!isBlackFL() and !isBlackFR()); i++) {
      InputCapture();
      UltraSonic();
      if ((i%10)<speed or i<tDelay) {
        MotorFollowBlackline();
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
    posForwardUpdate();
}

void TurnBlacklineL(int tDelay, int speed, bool isFaceChange) {  //max spd 10
    InputCapture();
    UltraSonic();
    encoderValueL=0;
    encoderValueR=0;
    for (int i=0; (i<tDelay) or (!isBlackTL() or !isBlackTR()); i++) {
      InputCapture();
      UltraSonic();
      //i%10 = PWM, tDelay = force accel, ignore sensor until delay
      if ((i%10)<speed or i<tDelay) {
        MotorTurnOnsiteL();
      } else {
        stopCar();
      }
      delay(1);
    } 
    stopCar();
    delay(100);
    InputCapture();
    UltraSonic();
    while (!isBlackTL() or !isBlackTR()) { //overshoot
      TurnBlacklineR(0,7, false); 
    }
    
    if (isFaceChange) { //change face
      Facing=(Facing+1)%4;
    }
}

void TurnBlacklineR(int tDelay, int speed, bool isFaceChange) {  //max spd 10
    InputCapture();
    UltraSonic();
    encoderValueL=0;
    encoderValueR=0;
    for (int i=0; (i<tDelay) or (!isBlackTL() or !isBlackTR()); i++) {
      InputCapture();
      UltraSonic();
      if ((i%10)<speed or i<tDelay) {
        MotorTurnOnsiteR();
      } else {
        stopCar();
      }
      delay(1);
    } 
    stopCar();
    delay(100);
    InputCapture();
    UltraSonic();
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
    display.println(" ");
    display.println("FACE");
    display.println("NOW");
    display.println("POS");
    display.println(" ");
    display.println("DIST");
    
    display.setTextSize(2);
    display.setCursor(25, 0);
    display.fillRect(25,0,59,15,BLACK);
    display.print(Facing);
    
    display.setCursor(25, 17);
    display.fillRect(25,17,59,15,BLACK);
    display.print(NowPos/10);
    display.print(",");
    display.print(NowPos%10);
   
    display.setTextSize(2);
    display.setCursor(25, 34);
    display.fillRect(25,34,59,15,BLACK);
    display.print(distance);
    display.setTextSize(1);
    display.setCursor(84, 40);
    display.print("cm");
}

void oledDestReach(int destReached) {
  if (destReached<0) { //LED blink
    int dest=-1*(destReached+1);
    
    display.fillScreen(RED);
    display.setCursor(0,0);
    display.setTextSize(2);
    display.println("THE DEST");
    display.println("");
    display.println("CAN'T BE");
    display.println("REACHED");
    
    display.setCursor(30,15);
    display.print(dest/10);
    display.print(",");
    display.print(dest%10);
    
    for (int i=0; i<10; i++) {
      onLED();
      delay(100);
      offLED();
      delay(100);
    }
    display.fillScreen(RED);
  } else {
    display.fillScreen(GREEN);
    display.setTextColor(BLACK);
    display.setCursor(0,0);
    display.setTextSize(2);
    display.println("THE DEST");
    display.println("");
    display.println("REACHED");
    
    display.setCursor(30,15);
    display.print(destReached/10);
    display.print(",");
    display.print(destReached%10);
    delay(2000);
    display.fillScreen(GREEN);
    display.setTextColor(WHITE);
  }  
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

void addObstacle() {
  int temp[]={10,1,-10,-1}; //x++, y++, x--, y--
  int obs_pos=NowPos+temp[Facing];
  arrayAppend(Obstacle,obs_pos);
}

bool isObstacle() {
  if (distance<obsDistThreshold and distance>1) {
    return true;
  } else {
    return false;
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
