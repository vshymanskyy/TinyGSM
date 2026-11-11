/** ============================================================================
 * @example{lineno} WebClient.ino
 *
 * @brief This sketch manually constructs an HTTP request and sends it to a web
 * server using TinyGSM.
 *
 * This example requires the least amount of RAM and Flash on the Arduino
 * because it does not use any external libraries.
 * It can be used to perform HTTP/RESTful API calls.
 * ========================================================================== */

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM868
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_SIM7000
// #define TINY_GSM_MODEM_SIM7000SSL
// #define TINY_GSM_MODEM_SIM7080
// #define TINY_GSM_MODEM_SIM5360
// #define TINY_GSM_MODEM_SIM7600
// #define TINY_GSM_MODEM_A7672X
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_SARAR4
// #define TINY_GSM_MODEM_SARAR5
// #define TINY_GSM_MODEM_M95
// #define TINY_GSM_MODEM_BG95
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_MC60
// #define TINY_GSM_MODEM_MC60E
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_ESP32
// #define TINY_GSM_MODEM_XBEE
// #define TINY_GSM_MODEM_SEQUANS_MONARCH

// Set serial for debug console (to the Serial Monitor)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#if !defined(__AVR_ATmega328P__) && !defined(SerialAT)
#define SerialAT Serial1

// or Software Serial on Uno, Nano
#elif !defined(SerialAT)
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(2, 3);  // RX, TX
#endif

// Increase RX buffer to capture the entire response
// Chips without internal buffering (A6/A7, ESP8266, M590)
// need enough space in the buffer for the entire response
// else data will be lost (and the http library will fail).
#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 256
#endif

// See all AT commands, if wanted
// WARNING: At high baud rates, incoming data may be lost when dumping AT
// commands
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
// #define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 921600

// Uncomment this if you want to use SSL
// #define USE_SSL

// Define how you're planning to connect to the internet.
// This is only needed for this example, not in other code.
#define TINY_GSM_USE_CELLULAR_DATA true
#define TINY_GSM_USE_WIFI false

// set SIM card pin, if any
#define SIM_PIN ""

// Your SIM card credentials, if any
const char apn[]     = "YourAPN";
const char simUser[] = "";
const char simPass[] = "";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

#include <TinyGsmClient.h>

#if (defined(ARDUINO_NRF52840_FEATHER)) && !defined(ADAFRUIT_TINYUSB_H_)
#include <Adafruit_TinyUSB.h>  // for Serial
#endif

// Just in case someone defined the wrong thing..
#if TINY_GSM_USE_CELLULAR_DATA && not defined TINY_GSM_MODEM_HAS_CELLULAR_DATA
#undef TINY_GSM_USE_CELLULAR_DATA
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_CELLULAR_DATA false
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_CELLULAR_DATA
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_CELLULAR_DATA true
#define TINY_GSM_USE_WIFI false
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

#if defined(USE_SSL) && !defined(TINY_GSM_MODEM_HAS_SSL)
#undef USE_SSL
#endif

#ifdef USE_SSL
TinyGsmClientSecure client(modem);
// Server details to test TCP over SSL
const char server[]   = "vsh.pp.ua";
const char resource[] = "/TinyGSM/logo.txt";
const int  port       = 443;
#else
TinyGsmClient client(modem);
// Server details to test TCP without SSL
const char server[]   = "time.sodaq.net";
const char resource[] = "/";
const int  port       = 80;
#endif

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  SerialMon.println("Wait...");

  // Set GSM module baud rate
  TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  // SerialAT.begin(9600);
  delay(6000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();
  // modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);

#if TINY_GSM_USE_CELLULAR_DATA
  // Unlock your SIM card with a PIN if needed
  if (SIM_PIN && modem.getSimStatus() != SIM_READY) {
    // simUnlock will do nothing if the pin is empty
    modem.simUnlock(SIM_PIN);
  }
#endif
}

void loop() {
#if TINY_GSM_USE_WIFI
  // Wifi connection parameters must be set before waiting for the network
  SerialMon.print(F("Setting SSID/password..."));
  if (!modem.networkConnect(wifiSSID, wifiPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");
#endif

#if TINY_GSM_USE_CELLULAR_DATA && defined TINY_GSM_MODEM_XBEE
  // The XBee must run the dataConnect function BEFORE waiting for network!
  modem.dataConnect(apn, simUser, simPass);
#endif

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isNetworkConnected()) { SerialMon.println("Network connected"); }

#if TINY_GSM_USE_CELLULAR_DATA
  // Cellular data connection parameters are usually set after network
  // registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.dataConnect(apn, simUser, simPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if (modem.isDataConnected()) { SerialMon.println("Cellular data connected"); }
#endif

  SerialMon.print("Connecting to ");
  SerialMon.println(server);
  if (!client.connect(server, port)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  // Make a HTTP GET request:
  SerialMon.println("Performing HTTP GET request...");
  client.print(String("GET ") + resource + " HTTP/1.1\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");
  client.println();

  uint32_t timeout = millis();
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

#if TINY_GSM_USE_WIFI
  modem.networkDisconnect();
  SerialMon.println(F("WiFi disconnected"));
#endif
#if TINY_GSM_USE_CELLULAR_DATA
  modem.dataDisconnect();
  SerialMon.println(F("Cellular data disconnected"));
#endif

  // Do nothing forevermore
  while (true) { delay(1000); }
}
