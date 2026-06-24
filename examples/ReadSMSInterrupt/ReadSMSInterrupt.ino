#define RI 2 //connect RI pin of SIM800 to interrupt pin 0 of arduino
#define Mode HIGH

#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_DEBUG Serial //to use DBG fubction from tiny gsm library

#include <TinyGsmClient.h>
#include <SoftwareSerial.h>
#define rxPin 10
#define txPin 11

SoftwareSerial GSM_Port =  SoftwareSerial(rxPin, txPin);
TinyGsm modem(GSM_Port);//use serial1 in case of arduino mega

int sms_index=0;

void setup() {
  pinMode(RI, INPUT);
  Serial.begin(115200);
  Serial.println("Initializing Serial... ");

  GSM_Port.begin(9600);
  Serial.println("Initializing GSM module...");
  delay(100);

  GSM_Port.write("AT\r"); //added to test the communication
  delay(1000);
  Serial.println("TinyGSM Read SMS With Interrupt Example Test");
  attachInterrupt(digitalPinToInterrupt(RI), ISR_NEW_SMS, Mode);

}

void loop() {
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
   
   //if interrupt occurs, Fetch the SMS from indexed MSSG number.   
   if(sms_index>0){
    
    String SMS=modem.readSMS(sms_index);
    String ID=modem.getSenderID(sms_index);
    Serial.print("Message From : ");Serial.println(ID);
    Serial.println(" and the message is ");
    Serial.println(SMS);
    sms_index=0;
    }

}



void ISR_NEW_SMS(){
  Serial.print("New SMS arrived at index :- ");
  
  unsigned long timerStart,timerEnd;
  String interrupt="";
  
  while(GSM_Port.available()) {
          char c = GSM_Port.read();
          interrupt+=c;   
      }
   
  if(interrupt.indexOf("CMTI:") > 0){
    sms_index=modem.newMessageInterrupt(interrupt);
    Serial.println(sms_index);
    interrupt="";
    }else{
        Serial.println("false alarm");
        return;
    }
}
