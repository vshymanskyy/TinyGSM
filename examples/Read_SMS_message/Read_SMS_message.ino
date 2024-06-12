#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_DEBUG Serial //to use DBG fubction from tiny gsm library
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <SoftwareSerial.h>
#define rxPin 10
#define txPin 11
SoftwareSerial NETWORK_PORT =  SoftwareSerial(rxPin, txPin);
TinyGsm modem(NETWORK_PORT);//use serial1 in case of arduino mega
void setup() {
  // define pin modes for tx, rx:
  pinMode(rxPin, INPUT);
  pinMode(txPin, OUTPUT);
  NETWORK_PORT.begin(9600);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  /*
  set mode 0 for reading latest unread message first
  set mode 1 for reading oldest unread message first
  in newMessageIndex(mode);
  */
int index=modem.newMessageIndex(0);
if(index>0){
String SMS=modem.readSMS(index);
String ID=modem.getSenderID(index);
DBG("new message arrived from :");
DBG(ID);DBG("Says");DBG(SMS);
}
delay(100);
}
