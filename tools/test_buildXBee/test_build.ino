/**************************************************************
 *
 *  DO NOT USE THIS - this is just a compilation test!
 *
 **************************************************************/

 // #define TINY_GSM_MODEM_SIM800
 // #define TINY_GSM_MODEM_A6
 // #define TINY_GSM_MODEM_M590
 // #define TINY_GSM_MODEM_ESP8266
 #define TINY_GSM_MODEM_XBEE

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

  // Test the start/restart functions
  modem.restart();
  modem.begin();
  modem.autoBaud();
  modem.factoryDefault();

  // Test the SIM card functions
  modem.getSimCCID();
  modem.getIMEI();
  modem.getSimStatus();
  modem.getRegistrationStatus();
  modem.getOperator();


  // Test the Networking functions
  modem.getSignalQuality();


  #if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_M590)
  modem.waitForNetwork();
  modem.gprsConnect("YourAPN", "", "");
  #else
    modem.networkConnect("YourSSID", "YourPWD");
    modem.waitForNetwork();
  #endif

  client.connect(server, 80);

  // Make a HTTP GET request:
  client.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    while (client.available()) {
      client.read();
      timeout = millis();
    }
  }

  client.stop();

  #if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_M590)
  modem.gprsDisconnect();
  #else
  networkDisconnect()
  #endif
}
