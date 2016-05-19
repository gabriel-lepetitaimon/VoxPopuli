  ////// FRAME ALPHABET

  //RX 
 
  #define FRAME_END 0xFF
  #define SLEEP_CMD 0xC8
  #define ACTIVE_MODE_CMD 0x96
  #define MUTE_MODE_CMD 0x64
  #define BATT_STATUS_CMD 50
  #define LED_ON_CMD 0x14
  #define LED_OFF_CMD 0x0A
  
  //TX

  #define NOT_PRESSED_TX '-'
  #define BUTTON_LEFT_TX 'L'
  #define BUTTON_UP_TX 'U'
  #define BUTTON_RIGHT_TX 'R'
  #define BUTTON_DOWN_TX 'D' 
  #define BUTTON_ACTION_TX 'A'
  
  ////// PIN DEFINITION

  #define PIN_ON_SLEEP 9
  
  #define BUTTON_LEFT 8
  #define BUTTON_UP 7
  #define BUTTON_RIGHT 6
  #define BUTTON_DOWN 5 
  #define BUTTON_ACTION 4
  
  #define LED 13
  
  ////// GLOBAL VARIABLES

  //FLAGS AND COUNTERS
  
  bool SkipProcessFlag = false;
  bool Active_Mute = false;
  int IbRx=0;
  int IbTx=0;

  // BUFFERS
  
  int BufferRx[20];
  int BufferTx[20];
  int ButtonBuff[] = {1,1,1,1,0};

void setup() {

  //Serial operations
  Serial.begin(9600);
  
  ///////Pin modes

  //Led
  
  pinMode (LED, OUTPUT);
  digitalWrite(LED, LOW);
  
  //Buttons
  
  pinMode (BUTTON_LEFT, INPUT_PULLUP);
  pinMode (BUTTON_UP, INPUT_PULLUP);
  pinMode (BUTTON_RIGHT, INPUT_PULLUP);
  pinMode (BUTTON_DOWN, INPUT_PULLUP);
  pinMode (BUTTON_ACTION, INPUT_PULLUP);
     
}



  ///// MAIN LOOP


void loop() {
  
  while(Serial.available() > 0) {
    
    BufferRx[IbRx]= Serial.read();
    IbRx++;
    if (BufferRx[IbRx]=FRAME_END){
      processMessage();
      IbRx=0;
    }
  }
  
  if (Active_Mute==true){
    buttonProcess();
  }
  
  transmitTx();
}



  /////FUNCTIONS

  // RX PROCESSING --- CAN BE BETTER HANDLED ---

void processMessage(){

  if (BufferRx[0]== ACTIVE_MODE_CMD){
    setActiveMode(); 
  }
  if (BufferRx[0]== MUTE_MODE_CMD){
    setMuteMode(); 
  }
  if (BufferRx[0]== SLEEP_CMD){
    setSleepMode(BufferRx[1]); 
  }
  if (BufferRx[0]== BATT_STATUS_CMD){
    batteryStatus(); 
  }
  if (BufferRx[0]== LED_ON_CMD){
    toggleLED(true); 
  }
  if (BufferRx[0]== LED_OFF_CMD){
    toggleLED(false); 
  }  
}


  // TRANSMISSION STUFF  --- PIN ON SLEEP TO MAKE

void transmitTx (){
  //while (digitalRead(PIN_ON_SLEEP)==HIGH){
    for( int i=0 ; i<IbTx ; i++){
      Serial.write(BufferTx[i]);
    }
    if(IbTx!=0){
      Serial.write('\r');
    }
  //}
  IbTx=0;
}

void TxToBuffer ( byte msg[], int length){
  for (int i=0; i<length ; i++){
    BufferTx[IbTx]=msg[i];
    IbTx++;
  }
}

  // ACTIVE/MUTE HANDLING

void setActiveMode(){
  Active_Mute =true;
  SkipProcessFlag = false;
}

void setMuteMode(){
  Active_Mute = false;
  SkipProcessFlag = false;  
}

  // SLEEPMODE ---- WORK IN PROGRESS DON'T USE ----

void setSleepMode(int time){
  int length = 5;
  byte sleepMsg1[] ={'A','T','S','M',4};
  TxToBuffer (sleepMsg1,length);
  transmitTx;
  byte sleepMsg2[] ={'A','T','S','M',4};
  
}

  // BATTERYSTATUS ---- NOT IMPLEMENTED YET ----

void batteryStatus(){
  
}

  // LED MANAGEMENT --- PWM HANDLING TO MAKE ----

void toggleLED(bool on_off){
  if (on_off==true){
    digitalWrite(LED, HIGH);
  int length = 7;
  byte OKLedOn[] ={'O','K','L','E','D','O','N'};
  TxToBuffer (OKLedOn,length);  
}

  if (on_off==false){
    digitalWrite(LED, LOW);
  int length = 8;
  byte OKLedOff[] ={'O','K','L','E','D','O','F','F'};
  TxToBuffer (OKLedOff,length);
  }
  
}

  // BUTTON HANDLING

void buttonProcess(){

  delay(1);
  
  bool pressedButtonFlag = false;
  byte buttonTx[5];
  int tempBuffer[5];
  
  tempBuffer[0]=digitalRead(BUTTON_LEFT);
  if (tempBuffer[0]==0){
    buttonTx[0]=BUTTON_LEFT_TX;
  }
  else buttonTx[0]=NOT_PRESSED_TX;
  
  tempBuffer[1]=digitalRead(BUTTON_UP);
  if (tempBuffer[1]==0){
    buttonTx[1]=BUTTON_UP_TX;
  }
  else buttonTx[1]=NOT_PRESSED_TX;
  
  tempBuffer[2]=digitalRead(BUTTON_RIGHT);
  if (tempBuffer[2]==0){
    buttonTx[2]=BUTTON_RIGHT_TX;
  }
  else buttonTx[2]=NOT_PRESSED_TX;
  
  tempBuffer[3]=digitalRead(BUTTON_DOWN);
  if (tempBuffer[3]==0){
    buttonTx[3]=BUTTON_DOWN_TX;
  }
  else buttonTx[3]=NOT_PRESSED_TX;
  
  tempBuffer[4]=digitalRead(BUTTON_ACTION);
  if (tempBuffer[4]==1){
    buttonTx[4]=BUTTON_ACTION_TX;
  }
  else buttonTx[4]=NOT_PRESSED_TX;
  
  for(int i = 0 ; i<5; i++){
    if (tempBuffer[i]!= ButtonBuff [i]){
      ButtonBuff [i] = tempBuffer[i];
      pressedButtonFlag = true;
    }
  }

  if (pressedButtonFlag == true){
    TxToBuffer(buttonTx,5);  
  }
   
  
}




