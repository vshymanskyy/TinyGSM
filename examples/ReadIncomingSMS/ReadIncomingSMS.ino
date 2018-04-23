#include <Arduino.h>

#define TINY_GSM_MODEM_SIM800

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Use Hardware Serial for the sim800
#define SerialAT Serial1

#include <TinyGsmClient.h>

TinyGsm sim800(SerialAT);

void setup(void){
  // Set console baud rate
  SerialMon.begin(115200);
  // Set GSM module baud rate
  SerialAT.begin(9600);
 
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing sim800..."));
  sim800.restart();
  delay(5000);
 
  String sim800Info = sim800.getModemInfo();
  SerialMon.print(F("sim800 info: "));
  SerialMon.println(sim800Info);

  //sim800.enableRING();
  
  sim800.enableSMSIndication();
  /* Register the callback for incoming SMS */
  sim800.registerUnsolitCallback(String("+CMTI:"),incomingSMSCallback);
  delay(3000);

  // Unlock your SIM card with a PIN if necessary
  //sim800.simUnlock("1234");
}

/* Callback for processing incoming SMS */
void incomingSMSCallback(String &unsolicited_msg){
    
    /* Get the SMS from the unsolicited message */
    
    SMS sms_msg; /* SMS object to store the message */
    bool rsp = sim800.readSMSUnsolicited(unsolicited_msg,sms_msg);

    if(rsp == false){
        Serial.println("readSMS failed");
        return;
    }

    /* Print number, datetime and content */
    Serial.print("Number: ");
    Serial.println(sms_msg.number);
    Serial.print("Datetime: ");
    Serial.println(sms_msg.datetime);
    Serial.print("Content: ");
    Serial.println(sms_msg.content);
}
  
void loop(void){
    /* Call this as often as posible */
    sim800.process();
}