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
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

#include <TinyGsmClient.h>
TinyGsm modem(SerialAT);

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(5000);
}

void loop() {
  // Detect module baud rate
  uint32_t rate = 0;
  uint32_t rates[] = { 115200, 9600, 57600, 19200, 74400, 74880 };

  SerialMon.println("Autodetecting baud rate");
  for (unsigned i = 0; i < sizeof(rates)/sizeof(rates[0]); i++) {
    SerialMon.print(String("Trying baud rate ") + rates[i] + "... ");
    SerialAT.begin(rates[i]);
    delay(10);
    if (modem.autoBaud(2000)) {
      rate = rates[i];
      SerialMon.println(F("OK"));
      break;
    } else {
      SerialMon.println(F("fail"));
    }
  }

  if (!rate) {
    SerialMon.println(F("***********************************************************"));
    SerialMon.println(F(" Module does not respond!"));
    SerialMon.println(F("   Check your Serial wiring"));
    SerialMon.println(F("   Check the module is correctly powered and turned on"));
    SerialMon.println(F("***********************************************************"));
    delay(30000L);
    return;
  }

  // Access AT commands from Serial Monitor
  SerialMon.println(F("***********************************************************"));
  SerialMon.println(F(" You can now send AT commands"));
  SerialMon.println(F(" Enter \"AT\" (without quotes), and you should see \"OK\""));
  SerialMon.println(F(" If it doesn't work, select \"Both NL & CR\" in Serial Monitor"));
  SerialMon.println(F("***********************************************************"));

  while(true) {
    if (SerialAT.available()) {
      SerialMon.write(SerialAT.read());
    }
    if (SerialMon.available()) {
      SerialAT.write(SerialMon.read());
    }
    delay(0);
  }
}

