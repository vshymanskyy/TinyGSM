/**************************************************************
 *
 *  DO NOT USE THIS - this is just a compilation test!
 *
 **************************************************************/
#define TINY_GSM_MODEM_SIM800

#include <TinyGsmClient.h>

TinyGsm modem(Serial);
TinyGsmClient client(modem);

#if defined(TINY_GSM_MODEM_HAS_SSL)
  TinyGsmClientSecure client_secure(modem);
#endif

char server[] = "somewhere";
char resource[] = "something";

void setup() {
  Serial.begin(115200);
  delay(3000);
}

void loop() {

  // Test the basic functions
  // modem.init();
  modem.begin();
  modem.setBaud(115200);
  modem.testAT();
  modem.factoryDefault();
  modem.getModemInfo();
  modem.getModemName();
  modem.maintain();
  modem.hasSSL();
  modem.hasWifi();
  modem.hasGPRS();

  // Test Power functions
  modem.restart();
  // modem.sleepEnable();
  modem.radioOff();
  modem.poweroff();

  // Test the SIM card functions
  #if defined(TINY_GSM_MODEM_HAS_GPRS)
  modem.getSimCCID();
  modem.getIMEI();
  modem.getSimStatus();
  modem.getOperator();
  #endif

  // Test the Networking functions
  modem.getRegistrationStatus();
  modem.getSignalQuality();
  modem.localIP();

  #if defined(TINY_GSM_MODEM_HAS_GPRS)
    modem.waitForNetwork();
    modem.gprsConnect("YourAPN", "", "");
  #endif
  #if defined(TINY_GSM_MODEM_HAS_WIFI)
    modem.networkConnect("YourSSID", "YourWiFiPass");
    modem.waitForNetwork();
  #endif

  client.connect(server, 80);

  // Make a HTTP GET request:
  client.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print("Connection: close\r\n\r\n");

  uint32_t timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    while (client.available()) {
      client.read();
      timeout = millis();
    }
  }

  client.stop();

  #if defined(TINY_GSM_MODEM_HAS_GPRS)
    modem.gprsDisconnect();
  #endif
  #if defined(TINY_GSM_MODEM_HAS_WIFI)
    modem.networkDisconnect();
  #endif

  // Test battery and temperature functions
  // modem.getBattVoltage();
  // modem.getBattPercent();
  // modem.getTemperature();
}
