/**************************************************************
 *
 *  DO NOT USE THIS - this is just a compilation test!
 *
 **************************************************************/
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

// Test the calling functions
#if defined(TINY_GSM_MODEM_HAS_CALLING)
  modem.callNumber(String("+380000000000"));
  modem.callAnswer();
  modem.callHangup();
#endif

// Test the SMS functions
#if defined(TINY_GSM_MODEM_HAS_SMS)
  modem.sendUSSD("*111#");
  modem.sendSMS(String("+380000000000"), String("Hello from "));
  modem.sendSMS_UTF16("+380000000000", "Hello", 5);
#endif

// Test the GSM location functions
#if defined(TINY_GSM_MODEM_HAS_GSM_LOCATION)
  modem.getGsmLocation();
#endif

// Test the Network time function
#if defined(TINY_GSM_MODEM_HAS_TIME)
  modem.getGSMDateTime(DATE_FULL);
#endif

// Test the Network time function
#if defined(TINY_GSM_MODEM_HAS_TIME)
  modem.getGSMDateTime(DATE_FULL);
#endif

// Test the GPS functions
#if defined(TINY_GSM_MODEM_HAS_GPS)
  modem.enableGPS();
  modem.getGPSraw();
  float latitude = -9999;
  float longitude = -9999;
  modem.getGPS(&latitude, &longitude);
#endif

// Test Battery functions
#if defined(TINY_GSM_MODEM_HAS_BATTERY)
  uint8_t chargeState = 0;
  int8_t chargePercent = 0;
  uint16_t milliVolts = 0;
  modem.getBattStats(chargeState, chargePercent, milliVolts);
#endif

// Test the temperature function
#if defined(TINY_GSM_MODEM_HAS_TEMPERATURE)
  modem.getTemperature();
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

#if defined(TINY_GSM_MODEM_HAS_SSL)
  client_secure.connect(server, 443);

  // Make a HTTP GET request:
  client_secure.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client_secure.print(String("Host: ") + server + "\r\n");
  client_secure.print("Connection: close\r\n\r\n");

  timeout = millis();
  while (client_secure.connected() && millis() - timeout < 10000L) {
    while (client_secure.available()) {
      client_secure.read();
      timeout = millis();
    }
  }

  client_secure.stop();
#endif

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
