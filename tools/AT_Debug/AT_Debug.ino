/**************************************************************
 *
 * To run this tool you need StreamDebugger library:
 *   https://github.com/vshymanskyy/StreamDebugger
 *   or from http://librarymanager/
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

StreamDebugger GsmSerial(Serial1, Serial);
GsmClient gsm(GsmSerial);

void setup() {
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  // Set GSM module baud rate
  GsmSerial.begin(115200);
  delay(3000);

  gsm.networkConnect(apn, user, pass);

  // Access AT commands from Serial
  GsmSerial.directAccess();
}

void loop() {

}

