  ////// FRAME ALPHABET

  //RX 
 
  #define FRAME_END 0xFF
  #define SLEEP_CMD 0xC8
  #define ACTIVE_MODE_CMD 0x96
  #define MUTE_MODE_CMD 0x64
  #define ASK_MODE_CMD 0x50
  #define BATT_STATUS_CMD 0x10
  #define LED_CMD 0x20
  #define LED1_CMD 0x21
  #define LED2_CMD 0x22
  #define LED3_CMD 0x23
  
  
  //TX
  
  
  #define BUTTON_LEFT_TX 0xB0
  #define BUTTON_UP_TX 0xB1
  #define BUTTON_RIGHT_TX 0xB2
  #define BUTTON_DOWN_TX 0xB3 
  #define BUTTON_ACTION_TX 0xB4
  byte buttonLabel[]= {BUTTON_LEFT_TX,BUTTON_UP_TX,BUTTON_RIGHT_TX,BUTTON_DOWN_TX,BUTTON_ACTION_TX};

  #define BATT_STATUS_TX 0x10
  
  ////// PIN DEFINITION

  #define PIN_ON_SLEEP 9
  
  #define BUTTON_LEFT 8
  #define BUTTON_UP 7
  #define BUTTON_RIGHT 6
  #define BUTTON_DOWN 5 
  #define BUTTON_ACTION 4
  int buttonPin[]= {BUTTON_LEFT,BUTTON_UP,BUTTON_RIGHT,BUTTON_DOWN,BUTTON_ACTION};
  
  #define LED1 10
  #define LED2 11
  #define LED3 3
  
  #define ADC_PIN A0
  
  ////// GLOBAL VARIABLES

  //FLAGS AND COUNTERS
  
  bool skipProcessFlag = false;
  bool Active_Mute = false;
  int IbRx=0;
  int IbTx=0;

  // BUFFERS
  
  int BufferRx[20];
  int BufferTx[20];
  int ButtonBuff[] = {1,1,1,1,1};

  // STATUS

  int LED1Status=0;
  int LED2Status=0;
  int LED3Status=0;

  
  //// SETUP

void setup() {

  //Serial operations
  Serial.begin(9600);
  
  ///////Pin modes

  pinMode (PIN_ON_SLEEP, INPUT);

  //Led
  
  pinMode (LED1, OUTPUT);
  pinMode (LED2, OUTPUT);
  pinMode (LED3, OUTPUT);
  
  //Buttons
  
  pinMode (BUTTON_LEFT, INPUT_PULLUP);
  pinMode (BUTTON_UP, INPUT_PULLUP);
  pinMode (BUTTON_RIGHT, INPUT_PULLUP);
  pinMode (BUTTON_DOWN, INPUT_PULLUP);
  pinMode (BUTTON_ACTION, INPUT_PULLUP);
     
}



  ///// MAIN LOOP


void loop() {
  
  skipProcessFlag==false;
  while(Serial.available() > 0) {
    
    BufferRx[IbRx]= Serial.read();
    IbRx++;
    if (BufferRx[IbRx-1]==FRAME_END){
      processMessage();
      IbRx=0;
    }
    
  }
  if (skipProcessFlag==false){
    
    if (Active_Mute==true){
      buttonProcess();
    }
    while (digitalRead(PIN_ON_SLEEP)!=1){
    
    }
    
    transmitTx(); 
  }
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
  if (BufferRx[0]== ASK_MODE_CMD){
    byte modeStatus[]={ASK_MODE_CMD,Active_Mute, FRAME_END};
    TxToBuffer(modeStatus,3); 
  }
  if (BufferRx[0]== SLEEP_CMD){
    setSleepMode(BufferRx[1]); 
  }
  if (BufferRx[0]== BATT_STATUS_CMD){
    batteryStatus(); 
  }
  if (BufferRx[0]== LED_CMD){
    toggleLED(BufferRx[1]); 
  }
  if (BufferRx[0]== LED1_CMD){
    toggleLED1(BufferRx[1]); 
  }
  if (BufferRx[0]== LED2_CMD){
    toggleLED2(BufferRx[1]); 
  }
  if (BufferRx[0]== LED3_CMD){
    toggleLED3(BufferRx[1]); 
  }
  
}


  // TRANSMISSION STUFF  --- PIN ON SLEEP HANDLED IN MAIN LOOP

void transmitTx (){
    for( int i=0 ; i<IbTx ; i++){
      Serial.write(BufferTx[i]);
    }
  
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
}

void setMuteMode(){
  Active_Mute = false; 
}

  // SLEEPMODE ---- WORK IN PROGRESS DON'T USE ----

void setSleepMode(int time){
  int length = 5;
  byte sleepMsg1[] ={'A','T','S','M',4};
  TxToBuffer (sleepMsg1,length);
  transmitTx;
  byte sleepMsg2[] ={'A','T','S','M',4};
  
}

  // BATTERYSTATUS ---- ADC on 10 bit FIX to make ---

void batteryStatus(){
  
  int sensorValue;
  sensorValue = analogRead(ADC_PIN);
  sensorValue=sensorValue/(1023)*(255);
  if (sensorValue==255){
    sensorValue=254;
  }
  
  byte battStatusTx[]={BATT_STATUS_TX,sensorValue,FRAME_END};
  TxToBuffer(battStatusTx,3);

}

  // LED MANAGEMENT

void toggleLED(int value){
  if (value==255){
    byte statusLED[]={LED_CMD,LED1Status,LED2Status,LED3Status,FRAME_END};
    TxToBuffer(statusLED,5);
  }
  else{
    int realValue=value;
    if(value==254){
      realValue=255;
    }
    analogWrite(LED1, realValue);
    analogWrite(LED2, realValue);
    analogWrite(LED3, realValue);
    LED1Status=value;
    LED2Status=value;
    LED3Status=value;
    
  }
  
    
}

void toggleLED1(int value){
  if (value==255){
    byte statusLED1[]={LED1_CMD,LED1Status,FRAME_END};
    TxToBuffer(statusLED1,3);
  }
  else{
   int realValue=value;
    if(value==254){
      realValue=255;
    }
  analogWrite(LED1, realValue);
  LED1Status=value; 
  }
  
    
}

void toggleLED2(int value){
  if (value==255){
    byte statusLED2[]={LED2_CMD,LED2Status,FRAME_END};
    TxToBuffer(statusLED2,3);
  }
  else{
   int realValue=value;
    if(value==254){
      realValue=255;
    }
  analogWrite(LED2, realValue);
  LED2Status=value; 
  }
    
}

void toggleLED3(int value){
  if (value==255){
    byte statusLED3[]={LED1_CMD,LED3Status,FRAME_END};
    TxToBuffer(statusLED3,3);
  }
  else{
   int realValue=value;
    if(value==254){
      realValue=255;
    }
  analogWrite(LED3, realValue);
  LED3Status=value; 
  } 
}
  


  // BUTTON HANDLING

void buttonProcess(){

  delay(1);
  
  int tempBuffer[5];
  
  for (int i=0 ; i<5;i++){
    
    tempBuffer[i]=digitalRead(buttonPin[i]);
    if(buttonLabel[i] == BUTTON_ACTION_TX)
      tempBuffer[i] = !tempBuffer[i];
      
    if (tempBuffer[i]!= ButtonBuff [i]){
      byte buttonTx[]={buttonLabel[i], ButtonBuff[i],FRAME_END};
      ButtonBuff [i] = tempBuffer[i];
      TxToBuffer(buttonTx,3);
    }
  }

}


