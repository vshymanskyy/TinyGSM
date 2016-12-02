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

char apn[]  = "YourAPN";
char user[] = "";
char pass[] = "";

// Use Hardware Serial on Mega, Leonardo, Micro
#define GsmSerial Serial1

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial GsmSerial(2, 3); // RX, TX

StreamDebugger DebugSerial(GsmSerial, Serial);
TinyGsmClient client(DebugSerial);

char server[] = "cdn.rawgit.com";
char resource[] = "/vshymanskyy/tinygsm/master/extras/test_simple.txt";

void setup() {
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  // Set GSM module baud rate
  GsmSerial.begin(115200);
  delay(3000);

  // Restart takes quite some time
  // You can skip it in many cases
  Serial.println("Restarting modem...");
  client.restart();
}

void loop() {
  if (!client.networkConnect(apn, user, pass)) {
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
      //Serial.print(c);
      bytesReceived += 1;
      timeout = millis();
    }
  }

  client.stop();

  client.networkDisconnect();

  Serial.println();
  Serial.println("************************");
  Serial.print  (" Test:  ");   Serial.println((bytesReceived == 1000) ? "PASSED" : "FAIL");
  Serial.println("************************");

  // Do nothing forevermore
  while (true) {
    delay(1000);
  }
}

