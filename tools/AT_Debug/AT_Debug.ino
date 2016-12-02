/**************************************************************
 *
 * To run this tool you need StreamDebugger library:
 *   https://github.com/vshymanskyy/StreamDebugger
 *   or from http://librarymanager/all#StreamDebugger
 *
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 *
 **************************************************************/

#include <TinyGsmClient.h>
#include <StreamDebugger.h>

char apn[]  = "YourAPN";
char user[] = "";
char pass[] = "";

// Use Hardware Serial on Mega, Leonardo, Micro
#define GsmSerial Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial GsmSerial(2, 3); // RX, TX

StreamDebugger DebugSerial(GsmSerial, Serial);
TinyGsmClient gsm(DebugSerial);

void setup() {
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  // Set GSM module baud rate
  GsmSerial.begin(115200);
  delay(3000);

  gsm.networkConnect(apn, user, pass);

  // Access AT commands from Serial
  DebugSerial.directAccess();
}

void loop() {

}

