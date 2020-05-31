/*
 * 
 * 
 * 1. MOTOR CONTROL
 * 
 * 
 * 
 * 
 */
void MotorControl(int pos) { //accept straight line travel only
    int forwardDelayt=150;
    int forwardSpd=11;
    int turnDelayt=200;
    int minTurn=20;
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
          TurnBlacklineL(turnDelayt, minTurn, turnSpd, true);
        } else {
          if (abs(facingTurnTo-Facing)==2) {           
            TurnBlacklineR(turnDelayt, minTurn, turnSpd, true);
            TurnBlacklineR(turnDelayt, minTurn, turnSpd, true);
            //MotorFollowBlackline();
          } else {
            TurnBlacklineR(turnDelayt, minTurn, turnSpd, true);
          }
        }
      }
      
      //2. go straight
      int forwardStep=abs(pos/10-NowPos/10)+abs(pos%10-NowPos%10);
      for (int i=0; i<forwardStep;i++) { //Task ii: check obstacle here
        ForwardBlackline(forwardDelayt, forwardSpd);
      }
      stopForwardCar();
    }  
}

void MotorControl_Obstacle(int pos) { //accept straight line travel only
    int forwardDelayt=150;
    int forwardSpd=11;
    int turnDelayt=200;
    int minTurn=20;
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
          TurnBlacklineL(turnDelayt, minTurn, turnSpd, true);
        } else {
          if (abs(facingTurnTo-Facing)==2) {           
            TurnBlacklineR(turnDelayt, minTurn, turnSpd, true);
            TurnBlacklineR(turnDelayt, minTurn, turnSpd, true);
            //MotorFollowBlackline();
          } else {
            TurnBlacklineR(turnDelayt, minTurn, turnSpd, true);
          }
        }
      }
      
      
      for (int i=0; i<3; i++) {
        UltraSonic();
        delay(100);
        checkObstacle_short(); //use shorter dist threshold when turning or starting to detect next pos obs, not next 2 one
      }
      if (!isObstacle) {
        //2. go straight
        int forwardStep=abs(pos/10-NowPos/10)+abs(pos%10-NowPos%10);
        for (int i=0; i<forwardStep and !isObstacle;i++) { //Task ii: check obstacle here
          ForwardBlackline(forwardDelayt, forwardSpd);
        }
        stopForwardCar();
      }
    } 
    
    //add obstacle to list
    if (isObstacle) {
      addObstacle();
    }
}

void MotorControl_locFind(int pos) { //accept straight line travel only
    int forwardDelayt=150;
    int forwardSpd=11;
    int forwardReduceSpd=6;
    int turnDelayt=200;
    int minTurn=20;
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
          TurnBlacklineL(turnDelayt, minTurn, turnSpd, true);
        } else {
          if (abs(facingTurnTo-Facing)==2) {           
            TurnBlacklineR(turnDelayt, minTurn, turnSpd, true);
            TurnBlacklineR(turnDelayt, minTurn, turnSpd, true);
            //MotorFollowBlackline();
          } else {
            TurnBlacklineR(turnDelayt, minTurn, turnSpd, true);
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
        //if (forwardStep-i>1) { //higher spd when step more than 1
          ForwardBlackline(forwardDelayt, forwardSpd);
        //} else { // otherwise reduce spd to prepare turning
         //ForwardBlackline_realSpd(forwardDelayt, forwardReduceSpd);
        //}
        
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

void MotorControl_wirelessCharging() { 
    int power_threshold=150;
    
    InputCapture();
    UltraSonic();
    oledVolt();
    
    encoderValueL=0;
    encoderValueR=0;
    
    while (powerValue<power_threshold) {
      while (distance<1 or distance>40) {
        realSpdForward_noResetEncoder(10); //full spd
        InputCapture();
        UltraSonic();
        offLED();
      }
      //spd cal
      int spdBrakeDist=5; //distance in cm to reduce 1 spd 
      int spd=distance/spdBrakeDist;
      
      onLED();
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
//////////////////////////////////////////////////////
//Motor control

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
  encoderValueL=0;
  encoderValueR=0;
  int encoderThreshold=3;
  while (abs(encoderValueL-turnL)>encoderThreshold or abs(encoderValueR-turnR)>encoderThreshold) {
    sPrintEncoder();
    if (!abs(encoderValueL-turnL)>encoderThreshold) {
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
    
    if (!abs(encoderValueR-turnR)>encoderThreshold) {
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
    delay(1);
  }
  stopCar();
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
//////////////////////////////////////////
//Motor speed control 
//period = 10ms ... spd_period
void SpdForward(int speed) {
  encoderValueL=0;
  encoderValueR=0;
  for (int i=0; i<spd_period; i++) {
    sPrintEncoder();
    if (i<speed) {
      MotorStraightForward();
    } else {
      stopCar();
    }
    delay(1);
  }
  real_spd=encoderValueL+encoderValueR;
}

void SpdBackward(int speed) {
  encoderValueL=0;
  encoderValueR=0;
  for (int i=0; i<spd_period; i++) {
    sPrintEncoder();
    if (i<speed) {
      MotorStraightBackward();
    } else {
      stopCar();
    }
    delay(1);
  }
  real_spd=abs(encoderValueL)+abs(encoderValueR);
}

void SpdTurnOnsiteL(int speed) {
  encoderValueL=0;
  encoderValueR=0;
  for (int i=0; i<spd_period; i++) {
    sPrintEncoder();
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
    sPrintEncoder();
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
    sPrintEncoder();
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

void realSpdBackward(int speed) {
    if (real_spd<speed) {
      SpdBackward(10);
    } else {
      SpdBackward(speed);
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
    sPrintEncoder();
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
//////////////////////////////////////////
//Motor control by blackline

void ForwardBlackline(int tDelay, int speed) {
    InputCapture();
    if (task_sel==2) {
      UltraSonic();
      checkObstacle();
    } else if (task_sel==3) {
      ColorInput();
    }
    
    encoderValueL=0;
    encoderValueR=0;
    for (int i=0; (i<tDelay) or (!isBlackFL() and !isBlackFR()); i++) {
      InputCapture();
      if (task_sel==2) {
        UltraSonic();
        checkObstacle();
      }
      if ((i%10)<speed or i<tDelay) {
        MotorFollowBlackline();
        if (task_sel==3 and i>tDelay) {
          ColorInput();
        }
      } else {
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

void ForwardBlackline_realSpd(int tDelay, int speed) {
    InputCapture();
    if (task_sel==2) {
      UltraSonic();
      checkObstacle();
    } else if (task_sel==3) {
      ColorInput();
    }
    
    encoderValueL=0;
    encoderValueR=0;
    int prev_encoderL=0;
    int prev_encoderR=0;
    for (int i=0; (i<tDelay) or (!isBlackFL() and !isBlackFR()); i++) {
      if (i%spd_period==0) { //cal spd for every 1 cycle
        real_spd=((encoderValueL-prev_encoderL)+(encoderValueR-prev_encoderR));
        Serial.println(real_spd);
        prev_encoderL=encoderValueL;
        prev_encoderR=encoderValueR;
      }
      //sensor
      InputCapture();
      if (task_sel==2) {
        UltraSonic();
        checkObstacle();
      }
      //compare spd
      if (real_spd<speed) {
        MotorFollowBlackline();
        if (task_sel==3 and i>tDelay) {
          ColorInput();
        }
      } else {
        stopCar();
      }
      delay(1);
      
    }
    posForwardUpdate();
}

void TurnBlacklineL(int tDelay, int minTurn, int speed, bool isFaceChange) {  //max spd 10
    InputCapture();
    encoderValueL=0;
    encoderValueR=0;
    for (int i=0; (i<tDelay) or (abs(encoderValueL)<minTurn) or (!isBlackTL() or !isBlackTR()); i++) {
      InputCapture();
      sPrintEncoder();
      //i%10 = PWM, tDelay = force accel, ignore sensor until delay
      if ((i%10)<speed or i<tDelay or abs(encoderValueL)<minTurn) {
        MotorTurnOnsiteL();
      } else {
        stopCar();
      }
      delay(1);
    } 
    stopCar();
    delay(100);
    InputCapture();
    while (!isBlackTL() or !isBlackTR()) { //overshoot
      TurnBlacklineR(0,0,7, false); 
    }
    
    if (isFaceChange) { //change face
      Facing=(Facing+1)%4;
    }
}

void TurnBlacklineR(int tDelay, int minTurn, int speed, bool isFaceChange) {  //max spd 10
    InputCapture();
    encoderValueL=0;
    encoderValueR=0;
    for (int i=0; (i<tDelay) or (abs(encoderValueR)<minTurn) or (!isBlackTL() or !isBlackTR()); i++) {
      InputCapture();
      sPrintEncoder();
      if ((i%10)<speed or i<tDelay or abs(encoderValueR)<minTurn) {
        MotorTurnOnsiteR();
      } else {
        stopCar();
      }
      delay(1);
    } 
    stopCar();
    delay(100);
    InputCapture();
    while (!isBlackTL() or !isBlackTR()) { //overshoot
      TurnBlacklineL(0,0,7, false); 
    }
    
    if (isFaceChange) { //change face
      Facing=(Facing+3)%4;
    }
}

void sPrintEncoder() {
  Serial.print(encoderValueL);
  Serial.print(",");
  Serial.print(encoderValueR);
  Serial.print("|");
}

void prevActMotor() {
  
    if (strcmp(prevAct_remote, "F")==0) { //forward
      realSpdForward(spd_remote);   
    } else if (strcmp(prevAct_remote, "L")==0) {  //left 
      SpdTurnL(spd_remote);
    } else if (strcmp(prevAct_remote, "R")==0) {  //right
      SpdTurnR(spd_remote);
    } else if (strcmp(prevAct_remote, "SL")==0) { //onsite L
      SpdTurnOnsiteL(spd_remote); 
    } else if (strcmp(prevAct_remote, "SR")==0) { //onsite R
      SpdTurnOnsiteR(spd_remote); 
    } else if (strcmp(prevAct_remote, "B")==0)  { //backward 
      realSpdBackward(spd_remote);
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
  delay(100);
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
