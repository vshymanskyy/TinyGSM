/**************************************************************
 *
 *  DO NOT USE THIS - this is just a compilation test!
 *  This is NOT an example for use of this library!
 *
 **************************************************************/
#include <TinyGsmClient.h>

TinyGsm modem(Serial);

void setup() {
  Serial.begin(115200);
  delay(6000);
}

void loop() {
  // Test the basic functions
  modem.begin();
  modem.begin("1234");
  modem.init();
  modem.init("1234");
  modem.setBaud(115200);
  modem.testAT();

  modem.getModemInfo();
  modem.getModemName();
  modem.factoryDefault();

  // Test Power functions
  modem.restart();
  // modem.sleepEnable();  // Not available for all modems
  // modem.radioOff();  // Not available for all modems
  modem.poweroff();

  // Test generic network functions
  modem.getRegistrationStatus();
  modem.isNetworkConnected();
  modem.waitForNetwork();
  modem.waitForNetwork(15000L);
  modem.waitForNetwork(15000L, true);
  modem.getSignalQuality();
  modem.getLocalIP();
  modem.localIP();

// Test the GPRS and SIM card functions
#if defined(TINY_GSM_MODEM_HAS_GPRS)
  modem.simUnlock("1234");
  modem.getSimCCID();
  modem.getIMEI();
  modem.getIMSI();
  modem.getSimStatus();

  modem.gprsConnect("myAPN");
  modem.gprsConnect("myAPN", "myUser");
  modem.gprsConnect("myAPN", "myAPNUser", "myAPNPass");
  modem.gprsDisconnect();
  modem.getOperator();
#endif

// Test WiFi Functions
#if defined(TINY_GSM_MODEM_HAS_WIFI)
  modem.networkConnect("mySSID", "mySSIDPassword");
  modem.networkDisconnect();
#endif

  // Test TCP functions
  modem.maintain();
  TinyGsmClient client;
  TinyGsmClient client2(modem);
  TinyGsmClient client3(modem, 1);
  client.init(&modem);
  client.init(&modem, 1);

  char server[]   = "somewhere";
  char resource[] = "something";

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
  // modem.addCertificate();  // not yet impemented
  // modem.deleteCertificate();  // not yet impemented
  TinyGsmClientSecure client_secure(modem);
  TinyGsmClientSecure client_secure2(modem);
  TinyGsmClientSecure client_secure3(modem, 1);
  client_secure.init(&modem);
  client_secure.init(&modem, 1);

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

// Test the calling functions
#if defined(TINY_GSM_MODEM_HAS_CALLING) && not defined(__AVR_ATmega32U4__)
  modem.callNumber(String("+380000000000"));
  modem.callHangup();

#if not defined(TINY_GSM_MODEM_SEQUANS_MONARCH)
  modem.callAnswer();
  modem.dtmfSend('A', 1000);
#endif

#endif

// Test the SMS functions
#if defined(TINY_GSM_MODEM_HAS_SMS) && not defined(__AVR_ATmega32U4__)
  modem.sendSMS(String("+380000000000"), String("Hello from "));

#if not defined(TINY_GSM_MODEM_XBEE) && not defined(TINY_GSM_MODEM_SARAR4)
  modem.sendUSSD("*111#");
#endif

#if not defined(TINY_GSM_MODEM_XBEE) && not defined(TINY_GSM_MODEM_M590) && \
    not defined(TINY_GSM_MODEM_SARAR4)
  modem.sendSMS_UTF16("+380000000000", "Hello", 5);
#endif

#endif

// Test the GSM location functions
#if defined(TINY_GSM_MODEM_HAS_GSM_LOCATION) && not defined(__AVR_ATmega32U4__)
  modem.getGsmLocationRaw();
  modem.getGsmLocation();
  float glatitude  = -9999;
  float glongitude = -9999;
  float gacc       = 0;
  int   gyear      = 0;
  int   gmonth     = 0;
  int   gday       = 0;
  int   ghour      = 0;
  int   gmin       = 0;
  int   gsec       = 0;
  modem.getGsmLocation(&glatitude, &glongitude);
  modem.getGsmLocation(&glatitude, &glongitude, &gacc, &gyear, &gmonth, &gday,
                       &ghour, &gmin, &gsec);
  modem.getGsmLocationTime(&gyear, &gmonth, &gday, &ghour, &gmin, &gsec);
#endif

// Test the GPS functions
#if defined(TINY_GSM_MODEM_HAS_GPS) && not defined(__AVR_ATmega32U4__)
  modem.enableGPS();
  modem.getGPSraw();
  float latitude  = -9999;
  float longitude = -9999;
  float speed     = 0;
  float alt       = 0;
  int   vsat      = 0;
  int   usat      = 0;
  float acc       = 0;
  int   year      = 0;
  int   month     = 0;
  int   day       = 0;
  int   hour      = 0;
  int   minute    = 0;
  int   second    = 0;
  modem.getGPS(&latitude, &longitude);
  modem.getGPS(&latitude, &longitude, &speed, &alt, &vsat, &usat, &acc, &year,
               &month, &day, &hour, &minute, &second);
  modem.disableGPS();
#endif

// Test the Network time function
#if defined(TINY_GSM_MODEM_HAS_NTP) && not defined(__AVR_ATmega32U4__)
  modem.NTPServerSync("pool.ntp.org", 3);
#endif

// Test the Network time function
#if defined(TINY_GSM_MODEM_HAS_TIME) && not defined(__AVR_ATmega32U4__)
  modem.getGSMDateTime(DATE_FULL);
  int   year3    = 0;
  int   month3   = 0;
  int   day3     = 0;
  int   hour3    = 0;
  int   min3     = 0;
  int   sec3     = 0;
  float timezone = 0;
  modem.getNetworkTime(&year3, &month3, &day3, &hour3, &min3, &sec3, &timezone);
#endif

// Test Battery functions
#if defined(TINY_GSM_MODEM_HAS_BATTERY)
  uint8_t  chargeState   = 0;
  int8_t   chargePercent = 0;
  uint16_t milliVolts    = 0;
  modem.getBattStats(chargeState, chargePercent, milliVolts);
#endif

// Test the temperature function
#if defined(TINY_GSM_MODEM_HAS_TEMPERATURE)
  modem.getTemperature();
#endif
}
