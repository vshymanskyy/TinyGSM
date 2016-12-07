/**************************************************************
 *
 * This script tries to auto-detect the baud rate
 * and allows direct AT commands access
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
TinyGsmClient gsm(SerialAT);

void setup() {
  // Set console baud rate
  SerialMonitor.begin(115200);
  delay(5000);
}

void loop() {
  // Detect module baud rate
  uint32_t rate = 0;
  uint32_t rates[] = { 115200, 9600, 57600, 19200, 74400, 74880 };

  SerialMonitor.println("Autodetecting baud rate");
  for (unsigned i = 0; i < sizeof(rates)/sizeof(rates[0]); i++) {
    SerialMonitor.print(String("Trying baud rate ") + rates[i] + "... ");
    SerialAT.begin(rates[i]);
    delay(10);
    if (gsm.autoBaud(3000)) {
      rate = rates[i];
      SerialMonitor.println(F("OK"));
      break;
    } else {
      SerialMonitor.println(F("fail"));
    }
  }

  if (!rate) {
    SerialMonitor.println(F("***********************************************************"));
    SerialMonitor.println(F(" Module does not respond!"));
    SerialMonitor.println(F("   Check your Serial wiring"));
    SerialMonitor.println(F("   Check the module is correctly powered and turned on"));
    SerialMonitor.println(F("***********************************************************"));
    delay(30000L);
    return;
  }

  // Access AT commands from Serial Monitor
  SerialMonitor.println(F("***********************************************************"));
  SerialMonitor.println(F(" You can now send AT commands"));
  SerialMonitor.println(F(" Enter \"AT\" (without quotes), and you should see \"OK\""));
  SerialMonitor.println(F(" If it doesn't work, select \"Both NL & CR\" in Serial Monitor"));
  SerialMonitor.println(F("***********************************************************"));

  while(true) {
    if (SerialAT.available()) {
      SerialMonitor.write(SerialAT.read());
    }
    if (SerialMonitor.available()) {
      SerialAT.write(SerialMonitor.read());
    }
    delay(0);
  }
}

