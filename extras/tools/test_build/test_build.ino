/** ============================================================================
 * @example{lineno} test_build.ino
 *
 * @warning DO NOT USE THIS - this is just a compilation test!
 * This is NOT an example for use of this library!
 * ========================================================================== */

#define TINY_GSM_DEBUG Serial
#define TINY_GSM_DEBUG_DEEP

#include <TinyGsmClient.h>
#include <TinyGsmCapabilities.h>

#if (defined(ARDUINO_NRF52840_FEATHER)) && !defined(ADAFRUIT_TINYUSB_H_)
#include <Adafruit_TinyUSB.h>  // for Serial
#endif

TinyGsm modem(Serial);

template <typename First, typename Second>
struct TinyGsmTestIsSame {
  static const bool value = false;
};

template <typename Type>
struct TinyGsmTestIsSame<Type, Type> {
  static const bool value = true;
};

static_assert(TinyGsmTestIsSame<TinyGsmClient, TinyGsm::TinyGsmClient>::value,
              "Global and modem-scoped client aliases must match");

#if defined(TINY_GSM_MODEM_HAS_SSL)
static_assert(
    TinyGsmTestIsSame<TinyGsmClientSecure, TinyGsm::TinyGsmClientSecure>::value,
    "Global and modem-scoped secure client aliases must match");
#endif

#if defined(TINY_GSM_MODEM_CAN_SPECIFY_CERTS)
static_assert(TinyGsmTestIsSame<TinyGsmSecureContext,
                                TinyGsm::TinyGsmSecureContext>::value,
              "Global and modem-scoped security-context aliases must match");
#endif

#if defined(TINY_GSM_MODEM_SIM808) || defined(TINY_GSM_MODEM_SIM868)
static_assert(
    TinyGsmTestIsSame<TinyGsmSim808::TinyGsmClient,
                      TinyGsmSim800::TinyGsmClient>::value,
    "The SIM808 client alias must use the inherited SIM800 client type");
static_assert(
    TinyGsmTestIsSame<TinyGsmSim808::TinyGsmClientSecure,
                      TinyGsmSim800::TinyGsmClientSecure>::value,
    "The SIM808 secure client alias must use the inherited SIM800 type");
#endif

void setup() {
  Serial.begin(115200);
  delay(6000);
}

void loop() {
  Serial.print(F("The configured modem is "));
  Serial.println(modem.getConfiguredModem());

  // Test run-time capability detection
  // These queries can be used to check modem capabilities at run time
  if (TinyGsmCapabilities::has_ssl<TinyGsm>::value) {
    Serial.println(F("SSL capability: available"));
  }
  if (TinyGsmCapabilities::has_gps<TinyGsm>::value) {
    Serial.println(F("GPS capability: available"));
  }
  if (TinyGsmCapabilities::has_wifi<TinyGsm>::value) {
    Serial.println(F("WiFi capability: available"));
  }
  if (TinyGsmCapabilities::has_cellular_data<TinyGsm>::value) {
    Serial.println(F("Cellular Data capability: available"));
  }
  if (TinyGsmCapabilities::has_sms<TinyGsm>::value) {
    Serial.println(F("SMS capability: available"));
  }
  if (TinyGsmCapabilities::has_calling<TinyGsm>::value) {
    Serial.println(F("Calling capability: available"));
  }
  if (TinyGsmCapabilities::has_battery<TinyGsm>::value) {
    Serial.println(F("Battery capability: available"));
  }
  if (TinyGsmCapabilities::has_temperature<TinyGsm>::value) {
    Serial.println(F("Temperature capability: available"));
  }
  if (TinyGsmCapabilities::has_ntp<TinyGsm>::value) {
    Serial.println(F("NTP capability: available"));
  }
  if (TinyGsmCapabilities::has_time<TinyGsm>::value) {
    Serial.println(F("Time capability: available"));
  }
  if (TinyGsmCapabilities::has_gsm_location<TinyGsm>::value) {
    Serial.println(F("GSM Location capability: available"));
  }
  if (TinyGsmCapabilities::has_bluetooth<TinyGsm>::value) {
    Serial.println(F("Bluetooth capability: available"));
  }
  if (TinyGsmCapabilities::has_tcp<TinyGsm>::value) {
    Serial.println(F("TCP capability: available"));
  }
  if (TinyGsmCapabilities::can_specify_certs<TinyGsm>::value) {
    Serial.println(F("Can specify certs capability: available"));
  }
  if (TinyGsmCapabilities::can_load_certs<TinyGsm>::value) {
    Serial.println(F("Can load certs capability: available"));
  }

  // ========================================================================
  // TinyGsm - Basic functions
  // ========================================================================

  modem.begin();
  modem.begin("1234");
  modem.init();
  modem.init("1234");
  modem.sendAT("+CGMI");
  modem.setBaud(115200);
#if defined(TINY_GSM_MODEM_ESP32) || defined(TINY_GSM_MODEM_ESP8266) || \
    defined(TINY_GSM_MODEM_ESP8266_NONOS)
  modem.setDefaultBaud(115200);
#endif
  modem.forceModemBaud(Serial, 115200);
  TinyGsmAutoBaud(Serial, 9600, 115200);
  modem.testAT();

  String waitData;
  modem.waitResponse(1000L, waitData);
  modem.waitResponse(1000L);
  modem.waitResponse();

  modem.getConfiguredModem();
  modem.getModemInfo();
  modem.getModemName();
  modem.getModemManufacturer();
  modem.getModemModel();
  modem.getModemRevision();
#if !defined(TINY_GSM_MODEM_ESP32) && !defined(TINY_GSM_MODEM_ESP8266) && \
    !defined(TINY_GSM_MODEM_ESP8266_NONOS)
  modem.getModemSerialNumber();
#endif
  modem.factoryDefault();

  // ========================================================================
  // TinyGsm - Power functions
  // ========================================================================

  modem.restart();
#if !defined(TINY_GSM_MODEM_ESP32)
  modem.powerOff();
#endif
#if !defined(TINY_GSM_MODEM_ESP8266_NONOS) && !defined(TINY_GSM_MODEM_A6) && \
    !defined(TINY_GSM_MODEM_UBLOX) && !defined(TINY_GSM_MODEM_SARAR4) &&     \
    !defined(TINY_GSM_MODEM_SARAR5) && !defined(TINY_GSM_MODEM_XBEE)
  modem.sleepEnable();
  modem.sleepEnable(false);
#endif
#if !defined(TINY_GSM_MODEM_ESP8266) &&                                      \
    !defined(TINY_GSM_MODEM_ESP8266_NONOS) && !defined(TINY_GSM_MODEM_A6) && \
    !defined(TINY_GSM_MODEM_M95) && !defined(TINY_GSM_MODEM_SEQUANS_MONARCH)
  modem.radioOff();
#endif
#if !defined(TINY_GSM_MODEM_ESP32) && !defined(TINY_GSM_MODEM_ESP8266) &&    \
    !defined(TINY_GSM_MODEM_ESP8266_NONOS) && !defined(TINY_GSM_MODEM_A6) && \
    !defined(TINY_GSM_MODEM_M95) &&                                          \
    !defined(TINY_GSM_MODEM_SEQUANS_MONARCH) && !defined(TINY_GSM_MODEM_XBEE)
  modem.setPhoneFunctionality(1, true);
#endif

  // ========================================================================
  // TinyGsm - Utilities
  // ========================================================================

  modem.streamPrint("AT", "\r\n");
  modem.streamClear();
  modem.streamDump(2);

  // ========================================================================
  // TinyGsm - Generic network functions
  // ========================================================================

  modem.getRegistrationStatus();
  modem.isNetworkConnected();
  modem.waitForNetwork();
  modem.waitForNetwork(15000L);
  modem.waitForNetwork(15000L, true);
  modem.getSignalQuality();
  modem.getLocalIP();
  modem.localIP();
#if defined(TINY_GSM_MODEM_A7672X)
  modem.getLocalIPSecure();
#endif

  // ========================================================================
  // TinyGsm - Network mode / type / technology functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_SARAR4)
  modem.setRadioAccessTechnology(7, 7);
#endif

#if defined(TINY_GSM_MODEM_SARAR5)
  modem.setRadioAccessTechnology(7, 7);
  modem.getCurrentRadioAccessTechnology();
#endif

#if defined(TINY_GSM_MODEM_UBLOX)
  modem.setRadioAccessTechnology(7, 7);
  modem.getCurrentRadioAccessTechnology();
#endif

#if defined(TINY_GSM_MODEM_SIM7000) || defined(TINY_GSM_MODEM_SIM7000SSL) || \
    defined(TINY_GSM_MODEM_SIM7080)
  modem.getNetworkModes();
  modem.getNetworkMode();
  modem.setNetworkMode(2);
  modem.getPreferredModes();
  modem.getPreferredMode();
  modem.setPreferredMode(2);
  bool    auto_reporting;
  int16_t stat;
  modem.getNetworkSystemMode(auto_reporting, stat);
  modem.setNetworkSystemMode(2);
#endif

#if defined(TINY_GSM_MODEM_SIM5360)
  modem.getNetworkModes();
  modem.getNetworkMode();
  modem.setNetworkMode(2);
#endif

#if defined(TINY_GSM_MODEM_SIM7600)
  modem.getNetworkModes();
  modem.getNetworkMode();
  modem.setNetworkMode(2);
  bool    auto_reporting;
  int16_t stat;
  modem.getNetworkSystemMode(auto_reporting, stat);
#endif

  // ========================================================================
  // TinyGsm - SIM card functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_CELLULAR_DATA)
  modem.simUnlock("1234");
  modem.getSimCCID();
  modem.getIMEI();
  modem.getIMSI();
  modem.getSimStatus();
#endif

  // ========================================================================
  // TinyGsm - Cellular data functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_CELLULAR_DATA)
  modem.dataConnect("myAPN");
  modem.dataConnect("myAPN", "myUser");
  modem.dataConnect("myAPN", "myAPNUser", "myAPNPass");
  modem.dataDisconnect();
  modem.isDataConnected();
  modem.getOperator();
#if defined(TINY_GSM_MODEM_A7672X) || defined(TINY_GSM_MODEM_BG96) ||     \
    defined(TINY_GSM_MODEM_M95) || defined(TINY_GSM_MODEM_MC60) ||        \
    defined(TINY_GSM_MODEM_SIM5360) || defined(TINY_GSM_MODEM_SIM7600) || \
    defined(TINY_GSM_MODEM_SIM800)
  modem.getProvider();
#endif
#endif

  // ========================================================================
  // TinyGsm - WiFi functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_WIFI)
  modem.networkConnect("mySSID", "mySSIDPassword");
  modem.networkDisconnect();
#endif

  // ========================================================================
  // TinyGsm - Socket listening functions
  // ========================================================================

  modem.maintain();
  modem.findFirstUnassignedMux();

  char server[]   = "somewhere";
  char resource[] = "something";

  // ========================================================================
  // TinyGsm - Secure socket layer (SSL) certificate management functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_SSL) && defined(TINY_GSM_MODEM_CAN_LOAD_CERTS)
  modem.findFirstUnassignedSSLContext();
  static const char flash_certificate_content[] TINY_GSM_PROGMEM =
      "certificate_content";
  modem.loadCertificate("certificateName", "certificate_content", 20);
  modem.loadCertificate("certificateName", GFP(flash_certificate_content),
                        sizeof(flash_certificate_content) - 1);
  modem.loadCertificate(String("certificateName"),
                        String("certificate_content"), 20);
#if !defined(TINY_GSM_MODEM_A7672X) && !defined(TINY_GSM_MODEM_SIM7600)
  modem.printCertificate("certificateName", Serial);
  modem.printCertificate(String("certificateName"), Serial);
#endif
  modem.deleteCertificate("certificateName");
  modem.deleteCertificate(String("certificateName"));

  modem.convertCertificate(CertificateType::CA_CERTIFICATE, "filename");
  modem.convertCertificate(CertificateType::CA_CERTIFICATE, String("filename"));
  modem.convertCACertificate("ca_cert_name");
  modem.convertCACertificate(String("ca_cert_name"));
  modem.convertClientCertificates("client_cert_name", "client_cert_key");
  modem.convertClientCertificates(String("client_cert_name"),
                                  String("client_cert_key"));
#if defined(TINY_GSM_MODEM_A7672X) || defined(TINY_GSM_MODEM_BG96) || \
    defined(TINY_GSM_MODEM_ESP32) || defined(TINY_GSM_MODEM_SIM7600)
  modem.convertPSKandID("psk", "pskIdent");
  modem.convertPSKandID(String("psk"), String("pskIdent"));
#endif
  modem.convertPSKTable("psk_table_name");
  modem.convertPSKTable(String("psk_table_name"));
#endif

#if defined(TINY_GSM_MODEM_ESP32)
  modem.loadCACert(0, "ca_certificate_data", 19);
  modem.loadClientCert(0, "client_certificate_data", 23);
  modem.loadPrivateKey(0, "private_key_data", 17);
  modem.loadCertificateByNumber(CertificateType::CA_CERTIFICATE, 0,
                                "ca_certificate_data", 19);
  modem.loadCertificateByNumber(CertificateType::CA_CERTIFICATE, 0,
                                GFP(flash_certificate_content),
                                sizeof(flash_certificate_content) - 1);
  modem.deleteCertificateByNumber(CertificateType::CA_CERTIFICATE, 0);
  modem.printCertificateByNumber(CertificateType::CA_CERTIFICATE, 0, Serial);
#endif

  // ========================================================================
  // TinyGsm - SSL context functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_BG96)
  modem.configureSSLContext(1, SSLAuthMode::CA_VALIDATION, SSLVersion::TLS1_2,
                            "ca_cert_name", "client_cert_name",
                            "client_key_name");
  modem.configureSSLContext(1);
#endif

#if defined(TINY_GSM_MODEM_SIM7000SSL) || defined(TINY_GSM_MODEM_SIM7080)
  modem.configureSSLContext(1, SSLAuthMode::CA_VALIDATION, SSLVersion::TLS1_2);
  modem.configureSSLContext(1);
  modem.configureSSLSNI(0, "sni_hostname");
  modem.applySSLCertificates(0, 1, SSLAuthMode::CA_VALIDATION, "ca_cert_name",
                             "client_cert_name", "client_key_name");
  modem.applySSLCertificates(0, 1);
  modem.applySSLPSK(0, 1, "psk_table_name");
  modem.applySSLPSK(0, 1);
  modem.linkSSLContext(0, 1);
#endif

#if defined(TINY_GSM_MODEM_A7672X) || defined(TINY_GSM_MODEM_SIM7600)
  modem.configureSSLContext(1, SSLAuthMode::CA_VALIDATION, SSLVersion::TLS1_2,
                            "ca_cert_name", "client_cert_name",
                            "client_key_name");
  modem.configureSSLContext(1);
  modem.linkSSLContext(0, 1);
#endif

#if defined(TINY_GSM_MODEM_XBEE)
  modem.configureSSLContext(0, SSLAuthMode::CA_VALIDATION, SSLVersion::TLS1_2,
                            "ca_cert_name", "client_cert_name",
                            "client_key_name");
  modem.configureSSLContext(0);
  modem.linkSSLContext(0, 0);
#endif

  // ========================================================================
  // TinyGsm - Phone call functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_CALLING)
  modem.callNumber(String("+380000000000"));
  modem.callHangup();
#if !defined(TINY_GSM_MODEM_SEQUANS_MONARCH)
  modem.callAnswer();
  modem.dtmfSend('A', 1000);
#endif
#if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM808)
  modem.setGsmBusy(true);
#endif
#if defined(TINY_GSM_MODEM_A7672X)
  modem.setGsmBusy(true);
#endif
#endif

  // ========================================================================
  // TinyGsm - Audio functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_A6)
  modem.audioSetHeadphones();
  modem.audioSetSpeaker();
  modem.audioMuteMic(true);
#endif

#if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM808)
  modem.setVolume(13);
  modem.getVolume();
  modem.setMicVolume(1, 13);
  modem.setAudioChannel(1);
  modem.playToolkitTone(1, 1000);
#endif

  // ========================================================================
  // TinyGsm - Text messaging (SMS) functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_SMS)
  modem.sendSMS(String("+380000000000"), String("Hello from TinyGSM!"));
#if !defined(TINY_GSM_MODEM_XBEE) && !defined(TINY_GSM_MODEM_SARAR4)
  modem.sendUSSD("*111#");
#endif
#if !defined(TINY_GSM_MODEM_XBEE) && !defined(TINY_GSM_MODEM_M590) && \
    !defined(TINY_GSM_MODEM_SARAR4)
  modem.sendSMS_UTF16("+380000000000", "Hello", 5);
#endif
#if defined(TINY_GSM_MODEM_M95) || defined(TINY_GSM_MODEM_MC60)
  modem.deleteAllSMS();
#endif
#endif

  // ========================================================================
  // TinyGsm - GSM location functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_GSM_LOCATION)
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
#endif

  // ========================================================================
  // TinyGsm - GPS (GNSS, GLONASS) functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_GPS)
#if defined(TINY_GSM_MODEM_SIM7600)
  modem.setGNSSMode(1, true);
  modem.getGNSSMode();
#endif

  modem.enableGPS();

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
  modem.getGPSTime(&gps_year, &gps_month, &gps_day, &gps_hour, &gps_minute,
                   &gps_second);
  modem.getGPSraw();

  modem.disableGPS();
#endif

  // ========================================================================
  // TinyGsm - NTP server functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_NTP)
  modem.NTPServerSync("pool.ntp.org", 12);
  modem.waitForTimeSync(1);
#endif

  // ========================================================================
  // TinyGsm - Time functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_TIME)
  modem.getGSMDateTime(TinyGSMDateTimeFormat::DATE_FULL);
  int   ntp_year     = 0;
  int   ntp_month    = 0;
  int   ntp_day      = 0;
  int   ntp_hour     = 0;
  int   ntp_min      = 0;
  int   ntp_sec      = 0;
  float ntp_timezone = 0;
  modem.getNetworkTime(&ntp_year, &ntp_month, &ntp_day, &ntp_hour, &ntp_min,
                       &ntp_sec, &ntp_timezone);
#if defined(TINY_GSM_MODEM_BG96) || defined(TINY_GSM_MODEM_BG95) || \
    defined(TINY_GSM_MODEM_BG95SSL)
  modem.getNetworkUTCTime(&ntp_year, &ntp_month, &ntp_day, &ntp_hour, &ntp_min,
                          &ntp_sec, &ntp_timezone);
#endif
#if defined(TINY_GSM_MODEM_ESP32) || defined(TINY_GSM_MODEM_ESP8266)
  modem.getNetworkEpoch();
  modem.getNetworkEpoch(TinyGSM_EpochStart::Y2K);
#endif
#if defined(TINY_GSM_MODEM_ESP8266)
  modem.setTimeZone(3.0);
#endif
#if defined(TINY_GSM_MODEM_ESP32)
  modem.setTimeZone(3.0);
  modem.setTimeSyncInterval(3600);
#endif
#endif

  // ========================================================================
  // TinyGsm - Battery functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_BATTERY)
#if !defined(TINY_GSM_MODEM_A6) && !defined(TINY_GSM_MODEM_SARAR4) && \
    !defined(TINY_GSM_MODEM_SARAR5) && !defined(TINY_GSM_MODEM_UBLOX)
  modem.getBattVoltage();
#endif
#if !defined(TINY_GSM_MODEM_SIM7600) && !defined(TINY_GSM_MODEM_XBEE)
  modem.getBattPercent();
#endif
#if !defined(TINY_GSM_MODEM_SARAR4) && !defined(TINY_GSM_MODEM_SARAR5) && \
    !defined(TINY_GSM_MODEM_UBLOX) && !defined(TINY_GSM_MODEM_SIM7600) && \
    !defined(TINY_GSM_MODEM_XBEE)
  modem.getBattChargeState();
#endif
  int8_t  chargeState   = -99;
  int8_t  chargePercent = -99;
  int16_t milliVolts    = -9999;
  modem.getBattStats(chargeState, chargePercent, milliVolts);
#endif

  // ========================================================================
  // TinyGsm - Temperature functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_TEMPERATURE)
  modem.getTemperature();
#endif

  // ========================================================================
  // TinyGsm - Bluetooth functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_HAS_BLUETOOTH)
  modem.enableBluetooth();
  modem.disableBluetooth();
  modem.setBluetoothVisibility(true);
  modem.setBluetoothHostName("bluetooth");
#endif

  // ========================================================================
  // TinyGsm - XBee identifier functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_XBEE)
  modem.getBeeType();
  modem.getBeeName();
  modem.getSeries();
#endif

  // ========================================================================
  // TinyGsm - XBee specific utilities
  // ========================================================================

#if defined(TINY_GSM_MODEM_XBEE)
  modem.commandMode();
  modem.writeChanges();
  modem.exitCommand();
  modem.exitAndFail();
  modem.readResponseString();
  modem.readResponseInt();
  modem.sendATGetString(GF("AT"));
  modem.changeSettingIfNeeded(GF("NI"), String("NewNodeID"));
  modem.changeSettingIfNeeded(GF("NI"), 0xFF);
#endif

  // ========================================================================
  // TinyGsmClient - Arduino Client interface
  // ========================================================================

  TinyGsmClient client;
  TinyGsmClient client2(modem);
  TinyGsmClient client3(modem, 1);
  client.init(&modem);
  client.init(&modem, 1);
  client.init(&modem, 1, false);
  client.init(&modem, 1, true);
  client.isAttached();

  client.connect(server, 80);

  // Make a HTTP GET request using client prints
  client.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client.print(String("Host: ") + server + "\r\n");
  client.print(F("Connection: close\r\n\r\n"));

  client.write("data");
  client.available();
  client.read();
  client.peek();
  client.flush();
  client.connected();

  uint32_t timeout = millis();
  while (client.connected() && millis() - timeout < 10000L) {
    while (client.available()) {
      client.read();
      timeout = millis();
    }
  }

  client.stop();
  client.detach();
  client.isAttached();
  client.init(&modem);

  // ========================================================================
  // TinyGsmClient - Extended Client API
  // ========================================================================

#if !defined(TINY_GSM_MODEM_SEQUANS_MONARCH)
  // Make the same HTTP GET request using beginWrite/write/endWrite
  client.beginWrite(62);
  client.write("GET ");
  client.write(resource);
  client.write(" HTTP/1.0\r\n");
  client.write("Host: ");
  client.write(server);
  client.write("\r\n");
  client.write("Connection: close\r\n\r\n");
  client.endWrite(62);
#endif

#if defined(TINY_GSM_MODEM_XBEE)
  client.remoteIP();
#endif

  client.getMux();
  client.getConnectionID();

  // ========================================================================
  // TinyGsmClientSecure - Arduino Client interface
  // ========================================================================

  // Note: You can now query modem capabilities at compile-time using
  // TinyGsmCapabilities traits instead of (or in addition to) preprocessor
  // defines. C++11 compatible - use regular if for runtime queries:
  //   if (TinyGsmCapabilities::has_ssl<TinyGsm>::value) { ... }
  // C++17 - use if constexpr for conditional compilation:
  //   if constexpr (TinyGsmCapabilities::has_ssl<TinyGsm>::value) { ... }
  // See examples/CapabilityQuery for a full demonstration.

#if defined(TINY_GSM_MODEM_HAS_SSL)
  TinyGsmClientSecure client_secure_default;
  TinyGsmClientSecure client_secure(modem);
  TinyGsmClientSecure client_secure2(modem, (uint8_t)0);

#if defined(TINY_GSM_MODEM_CAN_SPECIFY_CERTS)
  TinyGsmClientSecure client_secure3(modem, (uint8_t)0, (uint8_t)0);

  TinyGsmSecureContext context_secure_default;
  context_secure_default.isAttached();
  context_secure_default.getSSLContextIndex();
  context_secure_default.init(&modem, (uint8_t)1);
  context_secure_default.init(&modem, (uint8_t)1, true);
  context_secure_default.isAttached();
  context_secure_default.getSSLContextIndex();
  context_secure_default.detach();
  context_secure_default.init(&modem, (uint8_t)1);
  context_secure_default.init(&modem, (uint8_t)1, false);

  TinyGsmSecureContext context_secure(modem, 0);
  TinyGsmSecureContext(modem, SSLAuthMode::MUTUAL_AUTHENTICATION);
  TinyGsmSecureContext(modem, SSLAuthMode::MUTUAL_AUTHENTICATION,
                       SSLVersion::TLS1_2);
  TinyGsmSecureContext(modem, SSLAuthMode::MUTUAL_AUTHENTICATION,
                       SSLVersion::TLS1_2, "CAcertName");
  TinyGsmSecureContext(modem, SSLAuthMode::MUTUAL_AUTHENTICATION,
                       SSLVersion::TLS1_2, "CAcertName", "clientCertName");
  TinyGsmSecureContext(modem, 0, SSLAuthMode::MUTUAL_AUTHENTICATION,
                       SSLVersion::TLS1_2, "CAcertName", "clientCertName",
                       "clientKeyName");
  TinyGsmSecureContext(modem, "pskIdent", "psKey");
  TinyGsmSecureContext(modem, "pskIdent", "psKey", SSLVersion::TLS1_2);
  TinyGsmSecureContext(modem, 0, "pskIdent", "psKey", SSLVersion::TLS1_2);
  TinyGsmSecureContext(modem, "pskTableName", SSLVersion::TLS1_2);
  TinyGsmSecureContext(modem, "pskTableName");
  TinyGsmSecureContext(modem, 0, "pskTableName", SSLVersion::TLS1_2);
#endif

  client_secure.init(&modem);
  client_secure.init(&modem, 1);
  client_secure.init(&modem, 1, false);
  client_secure.init(&modem, 1, true);
  client_secure.isAttached();

  client_secure.connect(server, 443);

  // Make a HTTP GET request:
  client_secure.print(String("GET ") + resource + " HTTP/1.0\r\n");
  client_secure.print(String("Host: ") + server + "\r\n");
  client_secure.print(F("Connection: close\r\n\r\n"));

  client_secure.write("data");
  client_secure.available();
  client_secure.read();
  client_secure.peek();
  client_secure.flush();
  client_secure.connected();

  timeout = millis();
  while (client_secure.connected() && millis() - timeout < 10000L) {
    while (client_secure.available()) {
      client_secure.read();
      timeout = millis();
    }
  }

  client_secure.stop();
  client_secure.detach();
  client_secure.isAttached();
  client_secure.init(&modem);

  // ========================================================================
  // TinyGsmClientSecure - Extended Client API
  // ========================================================================

#if !defined(TINY_GSM_MODEM_SEQUANS_MONARCH)
  // Make the same HTTP GET request using beginWrite/write/endWrite
  client_secure.beginWrite(62);
  client_secure.write("GET ");
  client_secure.write(resource);
  client_secure.write(" HTTP/1.0\r\n");
  client_secure.write("Host: ");
  client_secure.write(server);
  client_secure.write("\r\n");
  client_secure.write("Connection: close\r\n\r\n");
  client_secure.endWrite(62);
#endif

#if defined(TINY_GSM_MODEM_XBEE)
  client_secure.remoteIP();
#endif

  client_secure.getMux();
  client_secure.getConnectionID();

  // ========================================================================
  // TinyGsmClientSecure - Secure client configuration
  // ========================================================================

#if defined(TINY_GSM_MODEM_CAN_SPECIFY_CERTS)
  client_secure.setSSLContextIndex((uint8_t)0);
  context_secure.setSSLAuthMode(SSLAuthMode::NO_VALIDATION);
  context_secure.setSSLAuthMode(SSLAuthMode::MUTUAL_AUTHENTICATION);
#if defined(TINY_GSM_MODEM_XBEE) || defined(TINY_GSM_MODEM_ESP32) ||      \
    defined(TINY_GSM_MODEM_ESP8266) || defined(TINY_GSM_MODEM_A7672X) ||  \
    defined(TINY_GSM_MODEM_BG96) || defined(TINY_GSM_MODEM_SIM7000SSL) || \
    defined(TINY_GSM_MODEM_SIM7080) || defined(TINY_GSM_MODEM_SIM7600)
  context_secure.setSSLVersion(SSLVersion::TLS1_2);
#endif
#if defined(TINY_GSM_MODEM_SEQUANS_MONARCH)
  context_secure.setStrictSSL(true);
#endif
#endif

  // ========================================================================
  // TinyGsmClientSecure - Client certificate assignment functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_CAN_SPECIFY_CERTS)
  context_secure.setCACertName("certificateName");
  context_secure.setCACertName(String("certificateName"));
  context_secure.setClientCredentials("certificateName", "keyName");
  context_secure.setClientCredentials(String("certificateName"),
                                      String("keyName"));
#endif

#if defined(TINY_GSM_MODEM_ESP8266)
  context_secure.setCACertificateNumber(1);
  context_secure.setClientCredentialNumber(0);
#endif

#if defined(TINY_GSM_MODEM_ESP32)
  context_secure.setCACertificateNumber(1);
  context_secure.setClientCredentialNumber(0);
#endif

  // ========================================================================
  // TinyGsmClientSecure - Client PSK assignment functions
  // ========================================================================

#if defined(TINY_GSM_MODEM_CAN_SPECIFY_CERTS)
  context_secure.setPreSharedKey("pskIdent", "psKey");
  context_secure.setPreSharedKey(String("pskIdent"), String("psKey"));
  context_secure.setPSKTableName("pskTableName");
  context_secure.setPSKTableName(String("pskTableName"));
#endif

#endif  // TINY_GSM_MODEM_HAS_SSL
}
