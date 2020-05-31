/*
 * 
 * 
 * 
 * 3. SETUP AT OLED
 *    AND DISPLAY
 * 
 * 
 * 
 * 
 */
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

void setupWizard_posDestMenu(int tDelay) {
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
    
    switch (selection) {
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

void setupWizard_carNo(int tDelay) {
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print("SEL   +1");
    display.setTextColor(WHITE);
    
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("CAR NO.");
    display.println("Car 1");
    display.println("Car 2");
    
    display.setTextColor(GREEN);
    display.setCursor(82, 15);
    display.print("*");
    
    delay(tDelay);
    InputCapture();
    while (!photoR) {
      InputCapture();
      if (photoL) {
        display.fillRect(82,0,15,45,BLACK);
        if (Car==2) {
          Car=1;
          display.setCursor(82, 15);
        } else {
          Car=2;
          display.setCursor(82, 30);
        }
        display.print("*");
        delay(tDelay);
      }
    }
    display.setTextColor(WHITE);    
    display.fillScreen(BLACK);
    carNumSend();
}

void setupWizard_startScreen(int tDelay) {
  display.fillScreen(BLACK);
    
  display.setTextColor(CYAN);
  display.setCursor(0, 0);
  display.println("HIDE ANY");
  display.println("PHOTO");
  display.println("SENSOR");
  display.println("TO START");
  display.setTextColor(WHITE);
  
  delay(tDelay);
  InputCapture();
  while (!photoR and !photoL) {
    InputCapture();
  }
  display.fillScreen(BLACK);
}

void setupWizard_taskSel(int tDelay) {
    display.setTextColor(CYAN); 
    display.setTextSize(2);
    display.setCursor(0,45);
    display.print("SEL   +1");
        
    display.setCursor(0, 0);
    display.println("TASK");
    task_sel=1;
    display.setCursor(60, 0);
    display.print(task_sel);
    
    display.setTextColor(WHITE);
    display.setCursor(0, 15);
    display.println("Find w/o");
    display.println("Obs.");
    
    delay(tDelay);
    InputCapture();
    while (!photoR and !isRemoteTaskSel) {
      prevActMotor();
      InputCapture();
      remoteCmdReceive(); //check cmd from remote computer
      if (photoL) {
        int numTotalTask=9;             //9 selection
        task_sel=(task_sel%numTotalTask)+1; 
        
        //display no. of task
        display.setTextColor(CYAN);
        display.fillRect(60,0,30,15,BLACK);
        display.setCursor(60, 0);
        display.print(task_sel);
                
        //display task name
        display.fillRect(0,15,100,30,BLACK);
        display.fillRect(30,45,30,5,BLACK);
        display.setTextColor(WHITE);
        display.setCursor(0, 15);
        switch (task_sel) {
          case 1:
            display.println("Find w/o");
            display.println("Obs.");
            break;
          case 2:
            display.println("Find w/");
            display.println("Obs.");
            break;
          case 3:
            display.println("Locate");
            display.println("Redspot");
            break;
          case 4:
            display.println("Wireless");
            display.println("CHARGE");
            break;
          case 5:
            display.println("Light");
            display.println("Track");
            break;
          case 6:
            display.println("Measure");
            display.println("IR");
            break;
          case 7:
            display.println("Measure");
            display.println("LIGHT");
            break;
          case 8:
            display.println("Measure");
            display.println("COLOUR");
            break;
          case 9:
            display.println("Measure");
            display.println("PWR,DIST");
            break;
        }
       
        delay(tDelay);
      }
    }
    display.setTextColor(WHITE);    
    display.fillScreen(BLACK);
}

void setupWizard_modeSel(int tDelay) { //sel 1-car mode or 2-car mode
    display.setTextColor(CYAN); 
    display.setCursor(0,45);
    display.print("SEL   +1");
    display.setTextColor(WHITE);
    
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.println("  MODE");
    display.println("ONE CAR");
    display.println("TWO CAR");
    
    display.setTextColor(GREEN);
    display.setCursor(82, 15);
    display.print("*");
    
    delay(tDelay);
    InputCapture();
    while (!photoR) {
      InputCapture();
      if (photoL) {
        display.fillRect(82,0,15,45,BLACK);
        is2car=!is2car;
        if (is2car) {
          display.setCursor(82, 30);
        } else {
          display.setCursor(82, 15);
        }
        display.print("*");
        delay(tDelay);
      }
    }
    display.setTextColor(WHITE);    
    display.fillScreen(BLACK);
}

void setupWizardOLED() {
    int tDelay=250;
    
    setupWizard_taskSel(tDelay);
    if (!isRemoteTaskSel) { 
      
    if (task_sel==1 or task_sel==3) { //only task 1 or 3 have 2-car mode
      setupWizard_modeSel(tDelay); //CHANGE is2car 
    }
    //sel car 1 or 2
    if (is2car) {
      setupWizard_carNo(tDelay);
    }
    
    switch (task_sel) {
      case 1: //find w/o obs.        
        if (is2car) {
          //2-car mode
          if (Car==1) {
            //1. apply default?
            if (!setupWizard_default(tDelay)) {
              //no-> set pos/dest
              setupWizard_posDestMenu(tDelay);
            } else {
              //ys-> show result and start
              applyDestDefault();
            }
          } else if (Car==2) {
            //car 2 can set its pos only
            setupWizard_pos(tDelay);
          }
          setupWizard_result(tDelay, "  READY");
        } else {
            if (!setupWizard_default(tDelay)) {
              setupWizard_posDestMenu(tDelay);
            } else {
              applyDestDefault();
            }
            setupWizard_result(tDelay, "  START");
        }
        
        break;
        
      case 2: //find w/ obs.
        if (!setupWizard_default(tDelay)) {
          //no-> set pos/dest
          setupWizard_posDestMenu(tDelay);
        } else {
          //ys-> show result and start
          applyDestDefault();
        }
        setupWizard_result(tDelay, "  START");
        break;
        
      case 3: //find redspot
        if (is2car) {
          if (Car==1) {
            NowPos=0;
            Facing=0;
            //update the dest list
            for (int i=0; i<(SIZE+1)*(SIZE+1)/2; i++) {
              DEST[i]=DEST_locPtInterest[i];
            }
          } else if (Car==2) {
            NowPos=55;
            Facing=2;
            for (int i=0; i<(SIZE+1)*(SIZE+1)/2; i++) {
              DEST[i]=DEST_locPtInterest[i+(SIZE+1)*(SIZE+1)/2];
            }
          }
        } else {
          for (int i=0; i<(SIZE+1)*(SIZE+1); i++) {
            DEST[i]=DEST_locPtInterest[i];
          }
        } 
        setupWizard_result2(tDelay);
        break;
      case 4: //wireless charging
        setupWizard_startScreen(tDelay);
        break; 
    //case 5-9 go to loop directly
    } 
    }
    display.fillScreen(BLACK);
}

void applyDestDefault() {
  for (int i=0; i<(sizeof(DEST_default)/sizeof(DEST_default[0])); i++) {
        DEST[i]=DEST_default[i];
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

void oledDestReach(int destReached) { //last for 30s
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
    
    for (int i=0; i<150; i++) { 
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
    delay(30000);
    display.fillScreen(GREEN);
    display.setTextColor(WHITE);
  }  
}

void oledLight() {
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
  delay(30000);  //last for 30s
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
  
  for (int i=0; i<75; i++) { //last for 30s
    onLED();
    delay(200);
    offLED();
    delay(200);
  }
  
  display.fillScreen(RED);
  for (int i=0; i<150; i++) { //last for 15s
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
///////////////////////////////////////////////////////
