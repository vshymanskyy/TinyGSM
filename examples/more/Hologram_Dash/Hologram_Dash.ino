/**************************************************************
 *
 * This sketch connects to a website and downloads a page.
 * It can be used to perform HTTP/RESTful API calls.
 *
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 *
 **************************************************************/

// Hologram Dash uses UBLOX U2 modems
#define TINY_GSM_MODEM_UBLOX

// Increase RX buffer if needed
//#define TINY_GSM_RX_BUFFER 512

#include <TinyGsmClient.h>

// Uncomment this if you want to see all AT commands
//#define DUMP_AT_COMMANDS

// Uncomment this if you want to use SSL
//#define USE_SSL

// Set serial for debug console (to the Serial Monitor, speed 115200)
#define SerialMon Serial

// We'll be using SerialSystem in Passthrough mode
#define SerialAT SerialSystem

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "YourAPN";
const char user[] = "";
const char pass[] = "";

// Server details
const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm mdm(debugger);
#else
  TinyGsm mdm(SerialAT);
#endif

#ifdef USE_SSL
  TinyGsmClientSecure client(mdm);
  const int  port = 443;
#else
  TinyGsmClient client(mdm);
  const int  port = 80;
#endif

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // Set up Passthrough
  HologramCloud.enterPassthrough();
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing modem..."));
  mdm.restart();

  String modemInfo = mdm.getModemInfo();
  SerialMon.print(F("Modem: "));
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN
  //mdm.simUnlock("1234");
}

void loop() {
  SerialMon.print(F("Waiting for network..."));
  if (!mdm.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!mdm.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print(F("Connecting to "));
  SerialMon.print(server);
  if (!client.connect(server, port)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  // Make a HTTP GET request:
  client.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    // Print available data
    while (client.available()) {
      char c = client.read();
      SerialMon.print(c);
      timeout = millis();
    }
  }
  SerialMon.println();

  // Shutdown

  client.stop();
  SerialMon.println(F("Server disconnected"));

  mdm.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));

  // Do nothing forevermore
  while (true) {
    delay(1000);
  }
}

