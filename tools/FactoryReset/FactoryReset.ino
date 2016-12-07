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

// Set serial for debug console (to the Serial Monitor, speed 115200)
#define SerialMonitor Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerialMonitor.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

#include <TinyGsmClient.h>
#include <StreamDebugger.h>
StreamDebugger DebugAT(SerialAT, SerialMonitor);
TinyGsmClient gsm(DebugAT);

void setup() {
  // Set console baud rate
  SerialMonitor.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(3000);

  if (!gsm.begin()) {
    SerialMonitor.println(F("***********************************************************"));
    SerialMonitor.println(F(" Cannot initialize module!"));
    SerialMonitor.println(F("   Use File -> Examples -> TinyGSM -> tools -> AT_Debug"));
    SerialMonitor.println(F("   to find correct configuration"));
    SerialMonitor.println(F("***********************************************************"));
    return;
  }

  bool ret = gsm.factoryDefault();

  SerialMonitor.println(F("***********************************************************"));
  SerialMonitor.print  (F(" Return settings to Factory Defaults: "));
  SerialMonitor.println((ret) ? "OK" : "FAIL");
  SerialMonitor.println(F("***********************************************************"));
}

void loop() {

}
