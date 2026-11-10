/** ============================================================================
 * @example{lineno} Multiple_Modules.ino
 *
 * @brief This example shows how to use two different modems together in the
 * same code.
 *
 * The key thing to notice is that instead of defining the modem type and
 * including the TinyGsmClient.h file, it includes the specific modem headers.
 * The global aliases `TinyGsm`, `TinyGsmClient`, and `TinyGsmClientSecure`
 * select only one modem type. Use each modem class and its scoped client
 * aliases when multiple modem headers are included.
 * ========================================================================== */

// NOTE: DO NOT set a modem define!

// WARNING:  In this example, both of the modems and the communication with the
// serial monitor are all using the same hardware serial port.  This WILL NOT
// WORK IN REALITY.  You must use different serial ports for each modem and the
// serial monitor.  To communicate with both modules and to output to the serial
// monitor your board must have **3** serial ports.  If your board does not have
// 3 hardware serial ports, you can use SoftwareSerial for one or both of the
// modems, but you must use different pins for each modem and the serial
// monitor. This example uses a single port for easier CI compilation testing.

// Set serial for debug console (to the Serial Monitor)
#define SerialMon Serial

// Set serial for AT commands (to the first module)
// In real life, you must use different serial ports for each modem and the
// serial monitor.

// Use Hardware Serial on Mega, Leonardo, Micro
#if !defined(__AVR_ATmega328P__) && !defined(SerialAT1)
#define SerialAT1 Serial1
// or Software Serial on Uno, Nano
#elif !defined(SerialAT1)
#include <SoftwareSerial.h>
SoftwareSerial SerialAT1(2, 3);  // RX, TX
#endif

// Set serial for AT commands (to the second module)
// In real life, you must use different serial ports for each modem and the
// serial monitor.

// Use Hardware Serial on Mega, Leonardo, Micro
#if !defined(__AVR_ATmega328P__) && !defined(SerialAT2)
#define SerialAT2 Serial1
// or Software Serial on Uno, Nano
#elif !defined(SerialAT2)
#include <SoftwareSerial.h>
SoftwareSerial SerialAT2(2, 3);  // RX, TX
#endif

// Increase RX buffer to capture the entire response
// Chips without internal buffering (A6/A7, ESP8266, M590)
// need enough space in the buffer for the entire response
// else data will be lost (and the http library will fail).
#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 1024
#endif

// See all AT commands, if wanted
// #define DUMP_AT_COMMANDS

// Define the serial console for debug prints, if needed
// #define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem1.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 921600

// set SIM card pin, if any
#define SIM_PIN ""

// Your SIM card credentials, if any
const char apn[]     = "YourAPN";
const char simUser[] = "";
const char simPass[] = "";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

// Server 1 details (this example site requires SSL)
const char server1[]   = "vsh.pp.ua";
const char resource1[] = "/TinyGSM/logo.txt";

// Server 2 details (in this example, identical to server 1, but could be
// different)
const char server2[]   = "vsh.pp.ua";
const char resource2[] = "/TinyGSM/logo.txt";

// NOTE: Do NOT include the generic TinyGsmClient headers for specific modems.
// Include for the SIM7080 modem
#include <TinyGsmClientSIM7080.h>
// Include for the ESP32
#include <TinyGsmClientESP32.h>

#if (defined(ARDUINO_NRF52840_FEATHER)) && !defined(ADAFRUIT_TINYUSB_H_)
#include <Adafruit_TinyUSB.h>  // for Serial
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger1(SerialAT1, SerialMon);
TinyGsmSim7080 modem1(debugger1);
StreamDebugger debugger2(SerialAT2, SerialMon);
TinyGsmESP32   modem2(debugger2);
#else
TinyGsmSim7080 modem1(SerialAT1);
TinyGsmESP32   modem2(SerialAT2);
#endif

TinyGsmSim7080::TinyGsmClientSecure client1(modem1);
const int                           port1 = 443;

TinyGsmESP32::TinyGsmClientSecure client2(modem2);
const int                         port2 = 443;

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  while (!SerialMon && millis() < 10000L) {}
  delay(10);

  SerialMon.println(GF("Wait..."));

  // Set GSM module baud rate
  SerialAT1.begin(115200);
  SerialAT2.begin(115200);

  SerialMon.print(GF("Modem 1 is compiled as "));
  SerialMon.print(GFP(decltype(modem1)::ModemConfig::MODEM_MANUFACTURER));
  SerialMon.print(GF(" "));
  SerialMon.print(GFP(decltype(modem1)::ModemConfig::MODEM_MODEL));
  SerialMon.print(GF(" ("));
  SerialMon.print((modem1).getConfiguredModem());
  SerialMon.println(GF(")"));
  SerialMon.print(GF("Modem 2 is compiled as "));
  SerialMon.print(GFP(decltype(modem2)::ModemConfig::MODEM_MANUFACTURER));
  SerialMon.print(GF(" "));
  SerialMon.print(GFP(decltype(modem2)::ModemConfig::MODEM_MODEL));
  SerialMon.print(GF(" ("));
  SerialMon.print((modem2).getConfiguredModem());
  SerialMon.println(GF(")"));

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  DBG(GF("Attempting to force baud rate of SIM7080 to 115200"));
  modem1.forceModemBaud(SerialAT1, 115200);
  DBG(GF("Attempting to force baud rate of ESP32 to 115200"));
  modem2.forceModemBaud(SerialAT2, 115200);

  SerialMon.println(GF("\n==============================="));
  SerialMon.println(GF("===============================\n"));

  SerialMon.println("Initializing cellular modem...");
  modem1.init();
  String modemInfo1 = modem1.getModemInfo();
  SerialMon.print(GF("Modem Info: "));
  SerialMon.println(modemInfo1);

  SerialMon.println(GF("\n==============================="));
  SerialMon.println(GF("===============================\n"));

  SerialMon.println("Initializing WiFi modem...");
  modem2.init();
  String modemInfo2 = modem2.getModemInfo();
  SerialMon.print(GF("Modem Info: "));
  SerialMon.println(modemInfo2);

  SerialMon.println(GF("\n==============================="));
  SerialMon.println(GF("===============================\n"));
}

void loop() {
  // Connect the ESP32 to WiFi
  SerialMon.print(GF("Setting SSID/password..."));
  if (!modem2.networkConnect(wifiSSID, wifiPass)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(GF(" success"));

  SerialMon.print(GF("Waiting for WiFi network..."));
  if (!modem2.waitForNetwork()) {
    SerialMon.println(GF(" fail"));
    delay(10000);
    return;
  }
  SerialMon.println(GF(" success"));
  if (modem2.isNetworkConnected()) {
    SerialMon.println(GF("WiFi network connected"));
  }

  SerialMon.println(GF("\n==============================="));
  SerialMon.println(GF("===============================\n"));

  // Wait for the SIM7080 to connect to the cellular network
  SerialMon.print(GF("Waiting for cellular network..."));
  if (!modem1.waitForNetwork()) {
    SerialMon.println(GF(" fail"));
    delay(10000);
    return;
  }
  SerialMon.println(GF(" success"));
  if (modem1.isNetworkConnected()) {
    SerialMon.println(GF("Cellular network connected"));
  }

  // Cellular data connection parameters are usually set after network
  // registration
  SerialMon.print(GF("Connecting to "));
  SerialMon.print(apn);
  if (!modem1.dataConnect(apn, simUser, simPass)) {
    SerialMon.println(GF(" fail"));
    delay(10000);
    return;
  }
  SerialMon.println(GF(" success"));
  if (modem1.isDataConnected()) {
    SerialMon.println(GF("Cellular data connected"));
  }

  SerialMon.println(GF("\n==============================="));
  SerialMon.println(GF("===============================\n"));

  // get some data from a server via HTTP GET request with the cellular
  // connection
  SerialMon.print(GF("Connecting to "));
  SerialMon.println(server1);
  if (!client1.connect(server1, port1)) {
    SerialMon.println(GF(" fail"));
    delay(10000);
    return;
  }
  SerialMon.println(GF(" success"));

  // Make a HTTP GET request:
  SerialMon.println(GF("Performing HTTP GET request..."));
  client1.print(String("GET ") + resource1 + " HTTP/1.1\r\n");
  client1.print(String("Host: ") + server1 + "\r\n");
  client1.print("Connection: close\r\n\r\n");

  uint32_t timeout = millis();
  while (client1.connected() && millis() - timeout < 50000L) {
    // Print available data
    while (client1.available()) {
      SerialMon.println(client1.readString());
      timeout = millis();
    }
  }
  SerialMon.println();

  // Shutdown
  client1.stop();
  SerialMon.println(GF("Server disconnected"));

  SerialMon.println(GF("\n==============================="));
  SerialMon.println(GF("===============================\n"));

  // get some data from a server via HTTP GET request with the WiFi connection
  SerialMon.print(GF("Connecting to "));
  SerialMon.println(server2);
  if (!client2.connect(server2, port2)) {
    SerialMon.println(GF(" fail"));
    delay(10000);
    return;
  }
  SerialMon.println(GF(" success"));

  // Make a HTTP GET request:
  SerialMon.println(GF("Performing HTTP GET request..."));
  client2.print(String("GET ") + resource2 + " HTTP/1.1\r\n");
  client2.print(String("Host: ") + server2 + "\r\n");
  client2.print("Connection: close\r\n\r\n");

  timeout = millis();
  while (client2.connected() && millis() - timeout < 50000L) {
    // Print available data
    while (client2.available()) {
      SerialMon.println(client2.readString());
      timeout = millis();
    }
  }
  SerialMon.println();

  client2.stop();
  SerialMon.println(GF("Server disconnected"));

  SerialMon.println(GF("\n==============================="));
  SerialMon.println(GF("===============================\n"));

  modem1.dataDisconnect();
  SerialMon.println(GF("Cellular data disconnected"));
  modem2.networkDisconnect();
  SerialMon.println(GF("WiFi disconnected"));

  // Do nothing forevermore
  while (true) { delay(1000); }
}
