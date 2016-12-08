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

char apn[]  = "YourAPN";
char user[] = "";
char pass[] = "";

// Set serial for debug console (to the Serial Monitor, speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#define SerialAT Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);

#include <TinyGsmClient.h>
TinyGsm modem(debugger);
TinyGsmClient client(modem);

char server[] = "cdn.rawgit.com";
char resource[] = "/vshymanskyy/tinygsm/master/extras/test_simple.txt";

void setup() {
  // Set console baud rate
  SerialMon.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(3000);

  // Restart takes quite some time
  // You can skip it in many cases
  SerialMon.println("Restarting modem...");
  modem.restart();

  SerialMon.println("Waiting for network... ");
  if (modem.waitForNetwork()) {
    SerialMon.println("OK");
  } else {
    SerialMon.println("fail");
  }
}

void loop() {
  if (!modem.gprsConnect(apn, user, pass)) {
    delay(10000);
    return;
  }

  if (!client.connect(server, 80)) {
    delay(10000);
    return;
  }

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

  modem.gprsDisconnect();

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

