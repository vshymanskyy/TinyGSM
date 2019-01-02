#define RI 2 //connect RI pin of sim 800 to interrupt pin 0 of arduino
#define mode HIGH
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <SoftwareSerial.h>
#define rxPin 10
#define txPin 11
// set up a new serial port
SoftwareSerial NETWORK_PORT =  SoftwareSerial(rxPin, txPin);
TinyGsm modem(NETWORK_PORT);//use serial1 in case of arduino mega
void setup() {
  // define pin modes for tx, rx:
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  NETWORK_PORT.begin(9600);
  Serial.begin(9600);
  attachInterrupt(digitalPinToInterrupt(RI), ISR_NEW_SMS, mode);

}

void loop() {
  //Read SMS normally
  Serial.println("Enter the SMS index to Read ");
  while(Serial.available()==0){
    
  }
  int i = Serial.parseInt();
  Serial.print("Reading message at :- ");Serial.println(i);
  String SMS=modem.readSMS(i);
  String ID=modem.getSenderID(i);
  Serial.print("Message From : ");Serial.println(ID);
  Serial.println(" and the message is ");
  Serial.println(SMS);
}
void ISR_NEW_SMS(){
  Serial.print("New SMS arrived at index :- ");
  unsigned long timerStart,timerEnd;
  String interrupt="";
  timerStart = millis();
  while(1) {
      if(NETWORK_PORT.available()) {
          char c = NETWORK_PORT.read();
          interrupt+=c;   
      }
      timerEnd = millis();
      if(timerEnd - timerStart > 1500)break;
    }
  if(interrupt.indexOf("CMTI:") > 0){
    i=modem.newMessageIndex(interrupt);
    Serial.println(i);
    String SMS=modem.readSMS(i);
    String ID=modem.getSenderID(i);
    Serial.print("Message From : ");Serial.println(ID);
    Serial.println(" and the message is ");
    Serial.println(SMS);
    }else{
        Serial.println("false alarm");
        return;
    }
  
}
