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
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(9600);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing sim800..."));
  sim800.restart();

  String sim800Info = sim800.getModemInfo();
  SerialMon.print(F("sim800 info: "));
  SerialMon.println(sim800Info);
  delay(3000);

  // Unlock your SIM card with a PIN if necessary
  //sim800.simUnlock("1234");
}

void incomingSMSCallback(String unsolicited_msg){
    /* Get the SMS index from the unsolicited message */
    uint8_t index = 3;
    SMS msg;
    bool rsp = sim800.readSMS(index,msg);
    if(rsp == false){
        return;
    }
    Serial.println("SMS received");
    Serial.print("Number: ");
    Serial.println(msg.number);
    Serial.print("Datetime: ");
    Serial.println(msg.datetime);
    Serial.print("Content: ");
    Serial.println(msg.content);
    
}

void loop(void){

}