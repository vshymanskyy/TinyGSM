/**************************************************************
 *
 *  DO NOT USE THIS - this is just a compilation test!
 *
 **************************************************************/

#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>

TinyGsm modem(Serial);
TinyGsmClient client(modem);

char server[] = "somewhere";
char resource[] = "something";

void setup() {
  Serial.begin(115200);
  delay(3000);
  modem.restart();
}

void loop() {
  modem.restart();

  modem.begin();

  modem.getOperator();

  modem.getSimCCID();
  
  modem.getSimStatus();

  modem.waitForNetwork();

  if (!modem.gprsConnect("YourAPN", "", "")) {
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

  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    // Print available data
    while (client.available()) {
      char c = client.read();
      timeout = millis();
    }
  }

  client.stop();

  modem.gprsDisconnect();

  // Do nothing forevermore
  while (true) {
    delay(1000);
  }
}

