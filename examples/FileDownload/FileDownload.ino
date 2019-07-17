/**************************************************************
 *
 * For this example, you need to install CRC32 library:
 *   https://github.com/bakercp/CRC32
 *   or from http://librarymanager/all#CRC32+checksum
 *
 * TinyGSM Getting Started guide:
 *   https://tiny.cc/tinygsm-readme
 *
 * ATTENTION! Downloading big files requires of knowledge of
 * the TinyGSM internals and some modem specifics,
 * so this is for more experienced developers.
 *
 **************************************************************/

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM868
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_SIM7000
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_SARAR4
// #define TINY_GSM_MODEM_M95
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_MC60
// #define TINY_GSM_MODEM_MC60E
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_XBEE
// #define TINY_GSM_MODEM_SEQUANS_MONARCH

// Increase RX buffer if needed
#define TINY_GSM_RX_BUFFER 1024

#include <TinyGsmClient.h>
#include <CRC32.h>

// Uncomment this if you want to see all AT commands
//#define DUMP_AT_COMMANDS

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX


// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "YourAPN";
const char user[] = "";
const char pass[] = "";

// Server details
const char server[] = "vsh.pp.ua";
const int  port = 80;

const char resource[]  = "/TinyGSM/test_1k.bin";
uint32_t knownCRC32    = 0x6f50d767;
uint32_t knownFileSize = 1024;   // In case server does not send it

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
#endif

TinyGsmClient client(modem);

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println(F("Initializing modem..."));
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print(F("Modem: "));
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

void printPercent(uint32_t readLength, uint32_t contentLength) {
  // If we know the total length
  if (contentLength != -1) {
    SerialMon.print("\r ");
    SerialMon.print((100.0 * readLength) / contentLength);
    SerialMon.print('%');
  } else {
    SerialMon.println(readLength);
  }
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

  // This timeout check is unneeded since there is a timeout handler in the data retrieval below
  // long timeout = millis();
  // while (client.available() == 0) {
  //   if (millis() - timeout > 5000L) {
  //     SerialMon.println(F(">>> Client Timeout !"));
  //     client.stop();
  //     delay(10000L);
  //     return;
  //   }
  // }

  // Let's see what the entire elapsed time is, from after we send the request.
  unsigned long timeElapsed = millis();

  SerialMon.println(F("Waiting for response header"));

  // While we are still looking for the end of the header (i.e. empty line FOLLOWED by a newline),
  // continue to read data into the buffer, parsing each line (data FOLLOWED by a newline).
  // If it takes too long to get data from the client, we need to exit.

  const uint32_t clientReadTimeout = 5000;
  uint32_t clientReadStartTime = millis();
  String headerBuffer;
  bool finishedHeader = false;
  uint32_t contentLength = 0;

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
        //   SerialMon.print((char) c);
        // else
        //   SerialMon.print('*');
        // SerialMon.print(' ');

        // Let's exit and process if we find a new line
        if (headerBuffer.indexOf(F("\r\n")) >= 0)
          break;
      }
    }
    else {
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
        contentLength = headerBuffer.substring(headerBuffer.indexOf(':') + 1).toInt();
        // SerialMon.print(F("Got Content Length: "));  // uncomment for
        // SerialMon.println(contentLength);            // confirmation
      }

      headerBuffer.remove(0, nlPos + 2);  // remove the line
    }
    else if (nlPos == 0) {
      // if the new line is empty (i.e. "\r\n" is at the beginning of the line), we are done with the header.
      finishedHeader = true;
    }
  }

  // vv Broken vv
  // while (client.available()) {  // ** race condition -- if the client doesn't have enough data, we will exit.
  //   String line = client.readStringUntil('\n');  // ** Depending on what we get from the client, this could be a partial line.
  //   line.trim();
  //   //SerialMon.println(line);    // Uncomment this to show response header
  //   line.toLowerCase();
  //   if (line.startsWith("content-length:")) {
  //     contentLength = line.substring(line.lastIndexOf(':') + 1).toInt();
  //   } else if (line.length() == 0) {
  //     break;
  //   }
  // }
  // ^^ Broken ^^
  //
  // The two cases which are not managed properly are as follows:
  // 1. The client doesn't provide data quickly enough to keep up with this loop.
  // 2. If the client data is segmented in the middle of the 'Content-Length: ' header,
  //    then that header may be missed/damaged.
  //

  uint32_t readLength = 0;
  CRC32 crc;

  if (finishedHeader && contentLength == knownFileSize) {
    SerialMon.println(F("Reading response data"));
    clientReadStartTime = millis();

    printPercent(readLength, contentLength);
    while (readLength < contentLength && client.connected() && millis() - clientReadStartTime < clientReadTimeout) {
      while (client.available()) {
        uint8_t c = client.read();
        //SerialMon.print((char)c);       // Uncomment this to show data
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

  modem.gprsDisconnect();
  SerialMon.println(F("GPRS disconnected"));

  float duration = float(timeElapsed) / 1000;

  SerialMon.println();
  SerialMon.print("Content-Length: ");   SerialMon.println(contentLength);
  SerialMon.print("Actually read:  ");   SerialMon.println(readLength);
  SerialMon.print("Calc. CRC32:    0x"); SerialMon.println(crc.finalize(), HEX);
  SerialMon.print("Known CRC32:    0x"); SerialMon.println(knownCRC32, HEX);
  SerialMon.print("Duration:       ");   SerialMon.print(duration); SerialMon.println("s");

  // Do nothing forevermore
  while (true) {
    delay(1000);
  }
}
