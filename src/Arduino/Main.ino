// FRAME ALPHABET
  
  #define FRAME_END 255
  #define SLEEP_CMD 200
  #define ACTIVE_MODE_CMD 150
  #define MUTE_MODE_CMD 100
  #define BATT_STATUS_CMD 50
  #define LED_ON_CMD 20
  #define LED_OFF_CMD 10

  // PIN DEFINITION

  #define PIN_ON_SLEEP 255
  

  // GLOBAL VARIABLES

  bool SkipProcessFlag = false;
  bool Active_Mute = false;
  int IbRx=0;
  int IbTx=0;

  // BUFFER
  
  int BufferRx[255];
  int BufferTx[255];

void setup() {
  Serial.begin(9600);
  pinMode (13, OUTPUT);
  digitalWrite(13, LOW);   
}

void loop() {
  while(Serial.available() > 0) {
    Serial.write("OKBro");
    BufferRx[IbRx]= Serial.read();
    IbRx++;
    if (BufferRx[IbRx]=FRAME_END){
      processMessage();
      IbRx=0;
    }
  }

  //transmitTx();
}

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

void transmitTx (){
  while (digitalRead(PIN_ON_SLEEP)==HIGH){
    for( int i=0 ; i<IbTx ; i++){
      Serial.write(BufferTx[i]);
    }
  }
  IbTx=0;
}

void TxToBuffer ( byte msg[], int length){
  for (int i=0; i<length ; i++){
    BufferTx[IbTx]=msg[i];
    IbTx++;
  }
}

void setActiveMode(){
  Active_Mute =true;
  SkipProcessFlag = false;
}

void setMuteMode(){
  Active_Mute = false;
  SkipProcessFlag = false;  
}

void setSleepMode(int time){
  int length = 5;
  byte sleepMsg1[] ={'A','T','S','M',4};
  TxToBuffer (sleepMsg1,length);
  transmitTx;
  byte sleepMsg2[] ={'A','T','S','M',4};
  
}

void batteryStatus(){
  
}

void toggleLED(bool on_off){
  if (on_off==true){
    digitalWrite(13, HIGH);
  }

  if (on_off==false){
    digitalWrite(13, LOW);
  }
  
}




