/*
 * 
 * 
 * 
 * 5. SENSOR
 * 
 * 
 * 
 * 
 */
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
//LED
void onLED() {
  digitalWrite(pinLED, 1); 
}

void offLED() {
  digitalWrite(pinLED, 0); 
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
  if (isValidPos(obs_pos)) {
    arrayAppend(Obstacle,obs_pos);
  }
  offLED();
}

void checkObstacle() { //can change isObstacle from F to T, but not T to F
  if (distance<obsDistThreshold and distance>1) {
    onLED();
    isObstacle=true;
  }
}

void checkObstacle_short() { //shorther threshold
  if (distance<obsDistThreshold_short and distance>1) {
    onLED();
    isObstacle=true;
  }
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
