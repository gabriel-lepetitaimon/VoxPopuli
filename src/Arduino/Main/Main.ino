  ////// FRAME ALPHABET

  //RX 
 
  #define FRAME_END 0xFF
  #define SLEEP_CMD 0xC8
  #define ACTIVE_MODE_CMD 0x96
  #define MUTE_MODE_CMD 0x64
  #define BATT_STATUS_CMD 0x10
  #define LED_CMD 0x21
  
  //TX
  #define BUTTON_FRAME 'B'
  #define BUTTON_LEFT_TX 'L'
  #define BUTTON_UP_TX 'U'
  #define BUTTON_RIGHT_TX 'R'
  #define BUTTON_DOWN_TX 'D' 
  #define BUTTON_ACTION_TX 'A'
  byte buttonLabel[]= {BUTTON_LEFT_TX,BUTTON_UP_TX,BUTTON_RIGHT_TX,BUTTON_DOWN_TX,BUTTON_ACTION_TX};
  
  ////// PIN DEFINITION

  #define PIN_ON_SLEEP 9
  
  #define BUTTON_LEFT 8
  #define BUTTON_UP 7
  #define BUTTON_RIGHT 6
  #define BUTTON_DOWN 5 
  #define BUTTON_ACTION 4
  int buttonPin[]= {BUTTON_LEFT,BUTTON_UP,BUTTON_RIGHT,BUTTON_DOWN,BUTTON_ACTION};
  
  #define LED 10
  
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
  //digitalWrite(LED, LOW);
  
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
    if (BufferRx[IbRx-1]==FRAME_END){
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
  if (BufferRx[0]== LED_CMD){
    toggleLED(BufferRx[1]); 
  }
  
}


  // TRANSMISSION STUFF  --- PIN ON SLEEP TO MAKE

void transmitTx (){
  //while (digitalRead(PIN_ON_SLEEP)==HIGH){
    for( int i=0 ; i<IbTx ; i++){
      Serial.write(BufferTx[i]);
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

void toggleLED(int value){
  int realValue=value;
  if(value==254){
    realValue=255;
  }
  analogWrite(LED, realValue);
  int length = 5;
  byte OKLed[] ={'O','K','L','E','D'};
  TxToBuffer (OKLed,length);  
}

  


  // BUTTON HANDLING

void buttonProcess(){

  delay(1);
  
  int tempBuffer[5];
  
  for (int i=0 ; i<5;i++){
    
    tempBuffer[i]=digitalRead(buttonPin[i]);
    if (tempBuffer[i]!= ButtonBuff [i]){
      ButtonBuff [i] = tempBuffer[i];
      byte buttonTx[]={BUTTON_FRAME,buttonLabel[i],ButtonBuff[i],FRAME_END};
      TxToBuffer(buttonTx,4);
    }
  }

}


