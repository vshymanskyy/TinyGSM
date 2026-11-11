/** ============================================================================
 * @example{lineno} FileDownload.ino
 *
 * @brief This sketch downloads a file from a server and checks its CRC32
 * checksum.
 *
 * For this example, you need to install CRC32 library:
 *   https://github.com/bakercp/CRC32
 *   or from http://librarymanager/all#CRC32+checksum
 *
 * @attention Downloading big files requires knowledge of the TinyGSM internals
 * and some modem specifics, so this is for more experienced developers.
 *
 * This example downloads a file from a server and checks its CRC32 checksum.
 * It is a good example of how to download a file in chunks and check its
 * integrity.
 *
 * NOTE: The file being downloaded is from a site which requires SSL/TLS so this
 * example will only work on those boards that support it.
 * ========================================================================== */

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM868
// #define TINY_GSM_MODEM_SIM900 // Doesn't support SSL
// #define TINY_GSM_MODEM_SIM7000 // Doesn't support SSL
// #define TINY_GSM_MODEM_SIM7000SSL
// #define TINY_GSM_MODEM_SIM7080
// #define TINY_GSM_MODEM_SIM5360 // Doesn't support SSL
// #define TINY_GSM_MODEM_SIM7600
// #define TINY_GSM_MODEM_A7672X
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_SARAR4
// #define TINY_GSM_MODEM_SARAR5
// #define TINY_GSM_MODEM_M95 // Doesn't support SSL
// #define TINY_GSM_MODEM_BG95 // Doesn't support SSL
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6 // Doesn't support SSL
// #define TINY_GSM_MODEM_A7 // Doesn't support SSL
// #define TINY_GSM_MODEM_M590 // Doesn't support SSL
// #define TINY_GSM_MODEM_MC60 // Doesn't support SSL
// #define TINY_GSM_MODEM_MC60E // Doesn't support SSL
// #define TINY_GSM_MODEM_ESP32
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_ESP8266_NONOS
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
#define TINY_GSM_RX_BUFFER 1024
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

// Server details
const char server[]      = "vsh.pp.ua";
const int  port          = 443;
const char resource[]    = "/TinyGSM/test_1k.bin";
uint32_t   knownCRC32    = 0x6f50d767;
uint32_t   knownFileSize = 1024;  // In case server does not send it

#include <TinyGsmClient.h>
#include <CRC32.h>

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

#if defined(TINY_GSM_MODEM_HAS_SSL)
TinyGsmClientSecure client(modem);
#else
// WARNING: This will not work with the example URL!
TinyGsmClient client(modem);
#endif

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  SerialMon.println(F("Wait..."));
  delay(500L);

  // Set GSM module baud rate
  TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
  delay(500L);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing modem..."));
  modem.restart();
  // modem.init();

  String modemInfo = modem.getModemInfo();
  SerialMon.print(F("Modem Info: "));
  SerialMon.println(modemInfo);

#if TINY_GSM_USE_CELLULAR_DATA
  // Unlock your SIM card with a PIN if needed
  if (SIM_PIN && modem.getSimStatus() != SIM_READY) {
    // simUnlock will do nothing if the pin is empty
    modem.simUnlock(SIM_PIN);
  }
#endif
}

void printPercent(uint32_t readLength, uint32_t contentLength) {
  // If we know the total length
  if (contentLength != (uint32_t)-1) {
    SerialMon.print(F("\r "));
    SerialMon.print((100.0 * readLength) / contentLength);
    SerialMon.print('%');
  } else {
    SerialMon.println(readLength);
  }
}

void loop() {
#if TINY_GSM_USE_WIFI
  // Wifi connection parameters must be set before waiting for the network
  SerialMon.print(F("Setting SSID/password..."));
  if (!modem.networkConnect(wifiSSID, wifiPass)) {
    SerialMon.println(F(" fail"));
    delay(10000);
    return;
  }
  SerialMon.println(F(" success"));
#endif

#if TINY_GSM_USE_CELLULAR_DATA && defined TINY_GSM_MODEM_XBEE
  // The XBee must run the dataConnect function BEFORE waiting for network!
  modem.dataConnect(apn, simUser, simPass);
#endif

  SerialMon.print(F("Waiting for network..."));
  if (!modem.waitForNetwork()) {
    SerialMon.println(F(" fail"));
    delay(10000);
    return;
  }
  SerialMon.println(F(" success"));

  if (modem.isNetworkConnected()) { SerialMon.println(F("Network connected")); }

#if TINY_GSM_USE_CELLULAR_DATA
  // Cellular data connection parameters are usually set after network
  // registration
  SerialMon.print(F("Connecting to "));
  SerialMon.print(apn);
  if (!modem.dataConnect(apn, simUser, simPass)) {
    SerialMon.println(F(" fail"));
    delay(10000);
    return;
  }
  SerialMon.println(F(" success"));

  if (modem.isDataConnected()) {
    SerialMon.println(F("Cellular data connected"));
  }
#endif

  SerialMon.print(F("Connecting to "));
  SerialMon.print(server);
  if (!client.connect(server, port)) {
    SerialMon.println(F(" fail"));
    delay(10000);
    return;
  }
  SerialMon.println(F(" success"));

  // Make a HTTP GET request:
  client.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print(F("Connection: close\r\n\r\n"));

  // Let's see what the entire elapsed time is, from after we send the request.
  uint32_t timeElapsed = millis();

  SerialMon.println(F("Waiting for response header"));

  // While we are still looking for the end of the header (i.e. empty line
  // FOLLOWED by a newline), continue to read data into the buffer, parsing each
  // line (data FOLLOWED by a newline). If it takes too long to get data from
  // the client, we need to exit.

  const uint32_t clientReadTimeout   = 5000;
  uint32_t       clientReadStartTime = millis();
  String         headerBuffer;
  bool           finishedHeader = false;
  uint32_t       contentLength  = 0;

  while (!finishedHeader) {
    int nlPos;

    if (client.available()) {
      clientReadStartTime = millis();
      while (client.available()) {
        char c = client.read();
        headerBuffer += c;

        // Uncomment the lines below to see the data coming into the buffer
        // if (c < 16)
        //   SerialMon.print('0');
        // SerialMon.print(c, HEX);
        // SerialMon.print(' ');
        // if (isprint(c))
        //   SerialMon.print(reinterpret_cast<char> c);
        // else
        //   SerialMon.print('*');
        // SerialMon.print(' ');

        // Let's exit and process if we find a new line
        if (headerBuffer.indexOf(F("\r\n")) >= 0) break;
      }
    } else {
      if (millis() - clientReadStartTime > clientReadTimeout) {
        // Time-out waiting for data from client
        SerialMon.println(F(">>> Client Timeout !"));
        break;
      }
    }

    // See if we have a new line.
    nlPos = headerBuffer.indexOf(F("\r\n"));

    if (nlPos > 0) {
      headerBuffer.toLowerCase();
      // Check if line contains content-length
      if (headerBuffer.startsWith(F("content-length:"))) {
        contentLength =
            headerBuffer.substring(headerBuffer.indexOf(':') + 1).toInt();
        // SerialMon.print(F("Got Content Length: "));  // uncomment for
        // SerialMon.println(contentLength);            // confirmation
      }

      headerBuffer.remove(0, nlPos + 2);  // remove the line
    } else if (nlPos == 0) {
      // if the new line is empty (i.e. "\r\n" is at the beginning of the line),
      // we are done with the header.
      finishedHeader = true;
    }
  }

  // The two cases which are not managed properly are as follows:
  // 1. The client doesn't provide data quickly enough to keep up with this
  // loop.
  // 2. If the client data is segmented in the middle of the 'Content-Length: '
  // header,
  //    then that header may be missed/damaged.
  //

  uint32_t readLength = 0;
  CRC32    crc;

  if (finishedHeader && contentLength == knownFileSize) {
    SerialMon.println(F("Reading response data"));
    clientReadStartTime = millis();

    printPercent(readLength, contentLength);
    while (readLength < contentLength && client.connected() &&
           millis() - clientReadStartTime < clientReadTimeout) {
      while (client.available()) {
        uint8_t c = client.read();
        // SerialMon.print(reinterpret_cast<char>c);  // Uncomment this to show
        // data
        crc.update(c);
        readLength++;
        if (readLength % (contentLength / 13) == 0) {
          printPercent(readLength, contentLength);
        }
        clientReadStartTime = millis();
      }
    }
    printPercent(readLength, contentLength);
  }

  timeElapsed = millis() - timeElapsed;
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

  float duration = float(timeElapsed) / 1000;

  SerialMon.println();
  SerialMon.print(F("Content-Length: "));
  SerialMon.println(contentLength);
  SerialMon.print(F("Actually read:  "));
  SerialMon.println(readLength);
  SerialMon.print(F("Calc. CRC32:    0x"));
  SerialMon.println(crc.finalize(), HEX);
  SerialMon.print(F("Known CRC32:    0x"));
  SerialMon.println(knownCRC32, HEX);
  SerialMon.print(F("Duration:       "));
  SerialMon.print(duration);
  SerialMon.println('s');

  // Do nothing forevermore
  while (true) { delay(1000); }
}
