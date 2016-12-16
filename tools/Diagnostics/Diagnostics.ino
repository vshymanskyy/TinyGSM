/**************************************************************
 *
 * To run this tool you need StreamDebugger library:
 *   https://github.com/vshymanskyy/StreamDebugger
 *   or from http://librarymanager/all#StreamDebugger
 *
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 *
 **************************************************************/

// Increase buffer fo see less commands
#define GSM_RX_BUFFER 256

#include <TinyGsmClient.h>
#include <StreamDebugger.h>

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "YourAPN";
const char user[] = "";
const char pass[] = "";

// Set serial for debug console (to the Serial Monitor, speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX


StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm modem(debugger);
TinyGsmClient client(modem);

const char server[] = "cdn.rawgit.com";
const char resource[] = "/vshymanskyy/tinygsm/master/extras/test_simple.txt";

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  modem.restart();

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

void loop() {
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    SerialMon.println("************************");
    SerialMon.println(" Is your sim card locked?");
    SerialMon.println(" Do you have a good signal?");
    SerialMon.println(" Is antenna attached?");
    SerialMon.println(" Does the SIM card work with your phone?");
    SerialMon.println("************************");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print("Connecting to ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    SerialMon.println("************************");
    SerialMon.println(" Is GPRS enabled by network provider?");
    SerialMon.println(" Try checking your card balance.");
    SerialMon.println("************************");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  SerialMon.print("Connecting to ");
  SerialMon.print(server);
  if (!client.connect(server, 80)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" OK");

  // Make a HTTP GET request:
  client.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");

  // Skip all headers
  client.find("\r\n\r\n");

  // Read data
  unsigned long timeout = millis();
  unsigned long bytesReceived = 0;
  while (client.connected() && millis() - timeout < 10000L) {
    while (client.available()) {
      char c = client.read();
      //SerialMon.print(c);
      bytesReceived += 1;
      timeout = millis();
    }
  }

  client.stop();
  SerialMon.println("Server disconnected");

  modem.gprsDisconnect();
  SerialMon.println("GPRS disconnected");

  SerialMon.println();
  SerialMon.println("************************");
  SerialMon.print  (" Received: ");
  SerialMon.print(bytesReceived);
  SerialMon.println("bytes");
  SerialMon.print  (" Test:     ");
  SerialMon.println((bytesReceived == 1000) ? "PASSED" : "FAIL");
  SerialMon.println("************************");

  // Do nothing forevermore
  while (true) {
    delay(1000);
  }
}

