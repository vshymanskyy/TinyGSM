#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_DEBUG Serial //to use DBG fubction from tiny gsm library


#include <TinyGsmClient.h>
#include <SoftwareSerial.h>
#define rxPin 10
#define txPin 11

SoftwareSerial GSM_Port =  SoftwareSerial(rxPin, txPin);
TinyGsm modem(GSM_Port);//use serial1 in case of arduino mega

void setup() {
  GSM_Port.begin(9600);
  Serial.begin(115200);
  Serial.println("Call Status Monitoring");

}

void loop() {

//Similar to fona getCallStatus() Finction
int8_t call_status = modem.getCallStatus();
        switch (call_status) {
          case 0: Serial.println(F("Ready")); break;
          case 1: Serial.println(F("Could not get status")); break;
          case 3: Serial.println(F("Ringing (incoming)")); break;
          case 4: Serial.println(F("Ringing/in progress (outgoing)")); break;
          default: Serial.println(F("Unknown")); break;
        }

}
