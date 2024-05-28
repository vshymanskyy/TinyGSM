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
  modem.getModemManufacturer();
  modem.getModemModel();
  modem.getModemRevision();
  modem.factoryDefault();

#if not defined(TINY_GSM_MODEM_ESP8266) && not defined(TINY_GSM_MODEM_ESP32)
  modem.getModemSerialNumber();
#endif

  // Test Power functions
  modem.restart();
  // modem.sleepEnable();  // Not available for all modems
  // modem.radioOff();     // Not available for all modems
  modem.poweroff();
  // modem.setPhoneFunctionality(1, 1);  // Not available for all modems

  // Test generic network functions
  modem.getRegistrationStatus();
  modem.isNetworkConnected();
  modem.waitForNetwork();
  modem.waitForNetwork(15000L);
  modem.waitForNetwork(15000L, true);
  modem.getSignalQuality();
  modem.getLocalIP();
  modem.localIP();

// Test WiFi Functions
#if defined(TINY_GSM_MODEM_HAS_WIFI)
  modem.networkConnect("mySSID", "mySSIDPassword");
  modem.networkDisconnect();
#endif

// Test the GPRS and SIM card functions
#if defined(TINY_GSM_MODEM_HAS_GPRS)
  modem.simUnlock("1234");
  modem.getSimStatus();

  modem.gprsConnect("myAPN");
  modem.gprsConnect("myAPN", "myUser");
  modem.gprsConnect("myAPN", "myAPNUser", "myAPNPass");
  modem.gprsDisconnect();

  modem.getSimCCID();
  modem.getIMEI();
  modem.getIMSI();
  modem.getOperator();
  // modem.getProvider();  // Not available for all modems
#endif

  char server[]   = "somewhere";
  char resource[] = "something";

  // Test TCP functions
  modem.maintain();
  TinyGsmClient client;
  TinyGsmClient client2(modem);
  TinyGsmClient client3(modem, 1);
  client.init(&modem);
  client.init(&modem, 1);

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
  // modem.addCertificate("certificateName");  // Not available for all modems
  // modem.addCertificate(
  //     String("certificateName"));  // Not available for all modems
  // modem.addCertificate("certificateName", "certificate_content",
  //                      20);  // Not available for all modems
  // modem.addCertificate(String("certificateName"),
  // String("certificate_content"),
  //                      20);                    // Not available for all
  //                      modems
  // modem.deleteCertificate("certificateName");  // Not available for all
  // modems
  modem.setCertificate("certificateName", 0);
  modem.setCertificate(String("certificateName"), 0);
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
  float gsm_latitude  = 0;
  float gsm_longitude = 0;
  float gsm_accuracy  = 0;
  int   gsm_year      = 0;
  int   gsm_month     = 0;
  int   gsm_day       = 0;
  int   gsm_hour      = 0;
  int   gsm_minute    = 0;
  int   gsm_second    = 0;
  modem.getGsmLocation(&gsm_latitude, &gsm_longitude);
  modem.getGsmLocation(&gsm_latitude, &gsm_longitude, &gsm_accuracy, &gsm_year,
                       &gsm_month, &gsm_day, &gsm_hour, &gsm_minute,
                       &gsm_second);
  modem.getGsmLocationTime(&gsm_year, &gsm_month, &gsm_day, &gsm_hour,
                           &gsm_minute, &gsm_second);
  modem.getGsmLocation();
#endif

// Test the GPS functions
#if defined(TINY_GSM_MODEM_HAS_GPS) && not defined(__AVR_ATmega32U4__)
  // modem.setGNSSMode(1, true);        // Not available for all modems
  // modem.getGNSSMode();               // Not available for all modems
#if !defined(TINY_GSM_MODEM_SARAR5)  // not available for this module
  modem.enableGPS();
#endif
  float gps_latitude  = 0;
  float gps_longitude = 0;
  float gps_speed     = 0;
  float gps_altitude  = 0;
  int   gps_vsat      = 0;
  int   gps_usat      = 0;
  float gps_accuracy  = 0;
  int   gps_year      = 0;
  int   gps_month     = 0;
  int   gps_day       = 0;
  int   gps_hour      = 0;
  int   gps_minute    = 0;
  int   gps_second    = 0;
  modem.getGPS(&gps_latitude, &gps_longitude);
  modem.getGPS(&gps_latitude, &gps_longitude, &gps_speed, &gps_altitude,
               &gps_vsat, &gps_usat, &gps_accuracy, &gps_year, &gps_month,
               &gps_day, &gps_hour, &gps_minute, &gps_second);
  modem.getGPSraw();
#if !defined(TINY_GSM_MODEM_SARAR5)  // not available for this module
  modem.disableGPS();
#endif
#endif

// Test the Network time functions
#if defined(TINY_GSM_MODEM_HAS_NTP) && not defined(__AVR_ATmega32U4__)
  modem.NTPServerSync("pool.ntp.org", 3);
  // modem.ShowNTPError(1);  // Not available for all modems
#endif

// Test the Network time function
#if defined(TINY_GSM_MODEM_HAS_TIME) && not defined(__AVR_ATmega32U4__)
  modem.getGSMDateTime(DATE_FULL);
  int   ntp_year     = 0;
  int   ntp_month    = 0;
  int   ntp_day      = 0;
  int   ntp_hour     = 0;
  int   ntp_min      = 0;
  int   ntp_sec      = 0;
  float ntp_timezone = 0;
  modem.getNetworkTime(&ntp_year, &ntp_month, &ntp_day, &ntp_hour, &ntp_min,
                       &ntp_sec, &ntp_timezone);
  // modem.getNetworkUTCTime(&ntp_year, &ntp_month, &ntp_day, &ntp_hour,
  // &ntp_min,
  //                         &ntp_sec,
  //                         &ntp_timezone);  // Not available for all modems
#endif

// Test bluetooth functions
#if defined(TINY_GSM_MODEM_HAS_BLUETOOTH)
  modem.enableBluetooth();
  modem.disableBluetooth();
  modem.setBluetoothVisibility(true);
  modem.setBluetoothHostName("bluetooth");
#endif

// Test Battery functions
#if defined(TINY_GSM_MODEM_HAS_BATTERY)
  // modem.getBattVoltage();      // Not available for all modems
  // modem.getBattPercent();      // Not available for all modems
  // modem.getBattChargeState();  // Not available for all modems
  int8_t  chargeState   = -99;
  int8_t  chargePercent = -99;
  int16_t milliVolts    = -9999;
  modem.getBattStats(chargeState, chargePercent, milliVolts);
#endif

// Test temperature functions
#if defined(TINY_GSM_MODEM_HAS_TEMPERATURE)
  modem.getTemperature();
#endif
}
