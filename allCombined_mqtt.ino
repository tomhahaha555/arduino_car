/*
 * 
 * 2. NETWORKING/MQTT
 * 
 * 
 * 
 * 
 * 
 */
//////////////////////////////////////////////////////
//redspot and wifi
void redspotPosDecide() {
  if (is2car) {
    if (RedspotPos==-1) { //my car detect red spot
      RedspotPos=NowPos;
      //publish to mqtt
      Serial2.print("PUB,R/");
      Serial2.print(RedspotPos);
      Serial2.print("-\n");
    }
  } else {
    RedspotPos=NowPos;
  }
}

void redspotPosReceive() {
  // read msg received from mqtt
  if (Serial2.available()) {
    String serial_str;
    serial_str = Serial2.readStringUntil('\n');
    MqttMsg mqtt_msg = parse_mqtt_msg(serial_str);
    
    char* sub_str = strtok(mqtt_msg.payload.c_str(), "/");
    if (strcmp(sub_str, "R")==0) {
      isRedspotFound=true;
      sub_str = strtok(NULL, "-");
      RedspotPos=String(sub_str).toInt();    
    }
  }
}

void remoteCmdReceive() {
  //1.check remote control signal (single cmd and multiple cmd)
  //2.check task sel and parameters (only 1-car mode can choose)
  if (Serial2.available()) {
    String serial_str;
    serial_str = Serial2.readStringUntil('\n');
    MqttMsg mqtt_msg = parse_mqtt_msg(serial_str);
    
    char* sub_str = strtok(mqtt_msg.payload.c_str(), "/");
    prevAct_remote=sub_str;
    Serial.print(prevAct_remote);
    
    //1A.remote single cmd (e.g. "F/6/")
    if (strcmp(sub_str, "F")==0) { //forward
      sub_str = strtok(NULL, "/");
      spd_remote=String(sub_str).toInt();
      realSpdForward(spd_remote);   
    } else if (strcmp(sub_str, "L")==0) {  //left
      sub_str = strtok(NULL, "/");
      spd_remote=String(sub_str).toInt(); 
      SpdTurnL(spd_remote);
    } else if (strcmp(sub_str, "R")==0) {  //right
      sub_str = strtok(NULL, "/");
      spd_remote=String(sub_str).toInt(); 
      SpdTurnR(spd_remote);
    } else if (strcmp(sub_str, "SL")==0) { //onsite L
      sub_str = strtok(NULL, "/");
      spd_remote=String(sub_str).toInt();
      SpdTurnOnsiteL(spd_remote); 
    } else if (strcmp(sub_str, "SR")==0) { //onsite R
      sub_str = strtok(NULL, "/");
      spd_remote=String(sub_str).toInt();
      SpdTurnOnsiteR(spd_remote); 
    } else if (strcmp(sub_str, "B")==0)  { //backward
      sub_str = strtok(NULL, "/");
      spd_remote=String(sub_str).toInt(); 
      realSpdBackward(spd_remote);
    }

    //1B.remote multiple cmd (e.g. "MULTIR/F-20/L-40/F-30/" distance in cm, degree for L/R)
    else if (strcmp(sub_str, "MULTIR")==0)  {
      prevAct_remote="A";
      spd_remote=0;
      sub_str = strtok(NULL, "-");
      while (sub_str!=NULL) {      
        //read cmd
        char* action = sub_str;
        
        //read distance        
        sub_str = strtok(NULL, "/");
        int dist = String(sub_str).toInt();

        //execute
        int turnL;
        int turnR;
        int turn_10cm=50;
        if (strcmp(action, "F")==0) {
            turnL=dist/10*turn_10cm;
            turnR=dist/10*turn_10cm;
        } else if (strcmp(action, "L")==0) {
            turnL=-dist/10*turn_10cm;
            turnR=dist/10*turn_10cm;
        } else if (strcmp(action, "R")==0) {
            turnL=dist/10*turn_10cm;
            turnR=-dist/10*turn_10cm;
        } else if (strcmp(action, "B")==0) {
            turnL=-dist/10*turn_10cm;
            turnR=-dist/10*turn_10cm;
        }
        Serial.println(action);
        Serial.println(turnL);
        Serial.println(turnR);
        MotorExactTurn(turnL,turnR);
        sub_str = strtok(NULL, "-");
      }
    }

    //2.task sel and parameters 
    else if(strcmp(sub_str, "T")==0) {
      //task selection
      sub_str = strtok(NULL, "/");
      task_sel = String(sub_str).toInt();
      isRemoteTaskSel=true;
        
      if (task_sel==1) { //find w/o obs. (e.g. "T/1/1-11/20-23-44-" T/task num/facing-startPos/dest1-dest2...)
            //facing
            sub_str = strtok(NULL, "-");
            Facing=String(sub_str).toInt();

            //start pos
            sub_str = strtok(NULL, "/");
            NowPos=String(sub_str).toInt();

            //append DEST list
            int i=0;
            sub_str = strtok(NULL, "-");
            while (sub_str!=NULL) {      
              DEST[i]=String(sub_str).toInt();
              i++;
              sub_str = strtok(NULL, "-");
            }
        } else if (task_sel==2) {    //find w/ obs.  (e.g. "T/2/1-11/20/10-13-14-15-" T/task num/facing-startPos/dest/obs1-obs2...)                         
            //facing
            sub_str = strtok(NULL, "-");
            Facing=String(sub_str).toInt();

            //start pos
            sub_str = strtok(NULL, "/");
            NowPos=String(sub_str).toInt();

            //dest pos
            sub_str = strtok(NULL, "/");    
            DEST[0]=String(sub_str).toInt();

            //append Obs list
            int j=0;
            sub_str = strtok(NULL, "-");
            while (sub_str!=NULL) {      
              Obstacle[j]=String(sub_str).toInt();
              j++;
              sub_str = strtok(NULL, "-");
            }
        }
     }
  }  
}

void carNumSend() {
    //reset wifi
    Serial2.println("RST,");
  
    //display status in oled
    display.setCursor(0, 0);
    display.setTextColor(BLACK); 
    display.fillScreen(YELLOW); 
    display.setTextSize(2);
    display.println("WAITING");
    display.println("CONNECT");
    display.println("TO WIFI");
    display.println("BOARD");

    //wait wifi setup
    wait_esp8266_connected();

    //setup topic in esp wifi
    bool successSend=false;
    while (!successSend) {
      Serial2.print("CAR");
      Serial2.print(Car);
      Serial2.print("\n");
        
      Serial.print("CAR");
      Serial.print(Car);
      Serial.print(" Sent\n");
      
      //check send success or fail
      //flush the serial buffer
      while (Serial2.available()) {
        Serial2.flush();
        Serial2.read();
        successSend=true;
      }
      delay(100);
    }
    
    display.fillScreen(BLACK);
    display.setTextColor(WHITE);
}


void obsDestUpdate2car() {
  //update the path and dest of another car to obstacle
  if (Serial2.available()) {
    arrInitToSize(Obstacle); //init the obs list only new path detected
    
    String serial_str;
    serial_str = Serial2.readStringUntil('\n');
    MqttMsg mqtt_msg = parse_mqtt_msg(serial_str);
    
    char* sub_str = strtok(mqtt_msg.payload.c_str(), "/");
    int dest_remove=String(sub_str).toInt();
    DEST[elementIndexinArray(dest_remove,DEST)]=-1; //remove dest reached by another car
    
    int i=0;
    sub_str = strtok(NULL, "-");
    while (sub_str!=NULL) {      
      Obstacle[i]=String(sub_str).toInt();
      i++;
      sub_str = strtok(NULL, "-");
    }
  }
}

void pathDestPublish(int cand_pos, int route[]) {
  //publish path and candidate dest to mqtt server
  //e.g. PUB,23/1-2-21-22-23\n
  Serial2.print("PUB,");
  Serial2.print(cand_pos);
  Serial2.print("/");
  
  for (int i=0; i<(SIZE+1)*(SIZE+1) and route[i]!=-1; i++) {
    if (i!=0) {
      Serial2.print("-");
    }
    Serial2.print(route[i]);
  }
  
  //coordinates surround dest
  int temp[]={10,1,-10,-1}; //x++, y++, x--, y--
  for (int j=0; j<4; j++) {
    int next_pos=cand_pos+temp[j];
    if (next_pos>=0 and (next_pos/10<=SIZE and next_pos%10<=SIZE) and !isElementinArray(next_pos,route)) {
      Serial2.print("-");
      Serial2.print(next_pos);
    }
  }
  Serial2.print("\n");
}

void destLtPublish() {
  //1. publish dest list (w/ all dest) to car 2
  //e.g. PUB,DEST/20-23-15-44\n
  Serial2.print("PUB,DEST/");
  for (int i=0; i<(SIZE+1)*(SIZE+1) and DEST[i]!=-1; i++) {
    if (i!=0) {
      Serial2.print("-");
    }
    Serial2.print(DEST[i]);
  }
  Serial2.print("\n");
}

void waitCar1DestLt() {
  //2. wait until received dest list 
  //3. then publish ready to car 1
  display.setCursor(0, 0);
  display.setTextColor(BLACK); 
  display.fillScreen(YELLOW); 
  display.setTextSize(2);
  display.println("WAITING");
  display.println("CAR 1");
  
  while (DEST[0]==-1) {   //when dest list not updated
    if (Serial2.available()) {
      String serial_str;
      serial_str = Serial2.readStringUntil('\n');
      MqttMsg mqtt_msg = parse_mqtt_msg(serial_str);
      
      char* sub_str = strtok(mqtt_msg.payload.c_str(), "/");
      if (strcmp(sub_str, "DEST")==0) {
        int i=0;
        sub_str = strtok(NULL, "-");
        while (sub_str!=NULL) {
          DEST[i]=String(sub_str).toInt();
          i++;
          sub_str = strtok(NULL, "-");
        }
      }
    }
    delay(100);
  }
  
  //step 3
  Serial2.print("PUB,READY/");
  Serial2.print("\n");
    
  display.fillScreen(BLACK);
  display.setTextColor(WHITE);
}

void waitCar2Ready() {
  //4. wait until received ready from car 2
  display.setCursor(0, 0);
  display.setTextColor(BLACK); 
  display.fillScreen(YELLOW); 
  display.setTextSize(2);
  display.println("WAITING");
  display.println("CAR 2");
  
  bool isReady=false;
  while (!isReady) {   
    if (Serial2.available()) {
      String serial_str;
      serial_str = Serial2.readStringUntil('\n');      
      MqttMsg mqtt_msg = parse_mqtt_msg(serial_str);
      
      char* sub_str = strtok(mqtt_msg.payload.c_str(), "/");
      if (strcmp(sub_str, "READY")==0) {
        isReady=true;
      }
    }
    delay(100);
  }
    
  display.fillScreen(BLACK);
  display.setTextColor(WHITE);
}
///////////////////////////////////////////
//wifi function provided
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
///////////////////////////////////////////////
