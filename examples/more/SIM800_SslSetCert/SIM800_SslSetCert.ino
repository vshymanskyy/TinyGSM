/**************************************************************
 *
 * This sketch uploads SSL certificates to the SIM8xx
 *
 * TinyGSM Getting Started guide:
 *   https://tiny.cc/tinygsm-readme
 *
 **************************************************************/

// This example is specific to SIM8xx
#define TINY_GSM_MODEM_SIM800

// Select your certificate:
#include "DSTRootCAX3.h"
//#include "DSTRootCAX3.der.h"
//#include "COMODORSACertificationAuthority.h"

// Select the file you want to write into
// (the file is stored on the modem)
#define CERT_FILE "C:\\USER\\CERT.CRT"

#include <TinyGsmClient.h>

// Set serial for debug console (to the Serial Monitor, speed 115200)
#define SerialMon Serial

// Use Hardware Serial for AT commands
#define SerialAT Serial1

// Uncomment this if you want to see all AT commands
// #define DUMP_AT_COMMANDS


#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(6000);

  SerialMon.println(F("Initializing modem..."));
  modem.init();

  modem.sendAT(GF("+FSCREATE=" CERT_FILE));
  if (modem.waitResponse() != 1) return;

  const int cert_size = sizeof(cert);

  modem.sendAT(GF("+FSWRITE=" CERT_FILE ",0,"), cert_size, GF(",10"));
  if (modem.waitResponse(GF(">")) != 1) {
    return;
  }

  for (int i = 0; i < cert_size; i++) {
    char c = pgm_read_byte(&cert[i]);
    modem.stream.write(c);
  }

  modem.stream.write(GSM_NL);
  modem.stream.flush();

  if (modem.waitResponse(2000) != 1) return;

  modem.sendAT(GF("+SSLSETCERT=\"" CERT_FILE "\""));
  if (modem.waitResponse() != 1) return;
  if (modem.waitResponse(5000L, GF(GSM_NL "+SSLSETCERT:")) != 1) return;
  const int retCode = modem.stream.readStringUntil('\n').toInt();


  SerialMon.println();
  SerialMon.println();
  SerialMon.println(F("****************************"));
  SerialMon.print(F("Setting Certificate: "));
  SerialMon.println((0 == retCode) ? "OK" : "FAILED");
  SerialMon.println(F("****************************"));
}

void loop() {
  if (SerialAT.available()) {
    SerialMon.write(SerialAT.read());
  }
  if (SerialMon.available()) {
    SerialAT.write(SerialMon.read());
  }
  delay(0);
}
