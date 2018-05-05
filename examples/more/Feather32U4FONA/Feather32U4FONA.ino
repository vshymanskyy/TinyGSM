/**************************************************************

   This sketch connects to a website and downloads a page.
   It can be used to perform HTTP/RESTful API calls.

   TinyGSM Getting Started guide:
     http://tiny.cc/tiny-gsm-readme

 **************************************************************/

// Select your modem:
#define TINY_GSM_MODEM_UBLOX

// Increase RX buffer if needed
//#define TINY_GSM_RX_BUFFER 512

#include <TinyGsmClient.h>

// Uncomment this if you want to see all AT commands
//#define DUMP_AT_COMMANDS

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

// Use Hardware Serial on MKR GSM
#define SerialAT SerialGSM

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "YourAPN";
const char user[] = "";
const char pass[] = "";

// Server details
const char server[] = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
const int  port = 80;

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);

const int baud = 921600;

void OnPowerOn(void* sender, EventArgs* e)
{
  // FOR THE MKR GSM 1400
  // reset / powerup the modem
  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);
}

void OnModemReady(void* sender, EventArgs* e)
{
  if (baud > 115200) {
    modem.setBaud(baud);

    SerialAT.end();
    delay(100);
    SerialAT.begin(baud);
  }
}

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  while (!SerialMon) {}

  modem.PowerOn    += OnPowerOn;
  modem.ModemReady += OnModemReady;

  // Set GSM module baud rate
  SerialAT.begin( (baud > 115200) ? 115200 : baud);

  // FOR THE MKR GSM 1400
  // reset / powerup the modem
  pinMode(GSM_RESETN, OUTPUT);
  digitalWrite(GSM_RESETN, HIGH);
  delay(100);
  digitalWrite(GSM_RESETN, LOW);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing modem..."));
  modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print(F("Modem: "));
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

void loop() {
  SerialMon.print(F("Waiting for network..."));
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
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

  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));

  // Do nothing forevermore
  while (true) {
    delay(1000);
  }
}

