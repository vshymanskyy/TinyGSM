/**************************************************************
 *
 * This sketch connects to a website and downloads a page.
 * It can be used to perform HTTP/RESTful API calls.
 *
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 *
 **************************************************************/
#include <string.h>
// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_ESP8266

#include <TinyGsmClient.h>

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "";
const char user[] = "";
const char pass[] = "";

// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

//#define DUMP_AT_COMMANDS
//#define TINY_GSM_DEBUG Serial

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(4, 3); // RX, TX

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, Serial);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

const char server[] = "cdn.rawgit.com";
const char resource[] = "/vshymanskyy/tinygsm/master/extras/logo.txt";

const int port = 80;

void setup() {
  // Set console baud rate
  Serial.begin(19200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(19200);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println(F("Initializing modem..."));
  modem.restart();

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

void loop() {
  Serial.print(F("Waiting for network..."));
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" OK");

  Serial.print(F("Connecting to "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" OK");

  modem.httpUinit();
  if(!modem.httpInit()){
    Serial.println(" fail ");
    delay(5000);
    return;
  }
  
  String url = String(server) + String(resource);
  if(!modem.httpRequest(URL,url.c_str())){
    Serial.println(" fail ");
    delay(5000);
    return;
  }

  int status = modem.httpMethod(GET);
  Serial.print("Status: ");
  Serial.println(status);
  if(status != OK){
    Serial.println(" fail");
    return;
  }
  String tmp = modem.httpResponse();
  Serial.println(tmp);

  modem.gprsDisconnect();
  Serial.println("GPRS disconnected");

  // Do nothing forevermore
  while (true) {
    delay(1000);
  }
}

