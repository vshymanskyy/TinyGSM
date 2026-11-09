/** ============================================================================
 * @example{lineno} AllFunctions.ino
 *
 * @brief This sketch tests all the functions of the TinyGSM library.
 *
 * @note Some of the functions may be unavailable for your modem.
 * Just comment them out.
 * ========================================================================== */

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM868
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_SIM7000
// #define TINY_GSM_MODEM_SIM7000SSL
// #define TINY_GSM_MODEM_SIM7080
// #define TINY_GSM_MODEM_SIM5360
// #define TINY_GSM_MODEM_SIM7600
// #define TINY_GSM_MODEM_A7672X
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_SARAR4
// #define TINY_GSM_MODEM_SARAR5
// #define TINY_GSM_MODEM_M95
// #define TINY_GSM_MODEM_BG95
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_MC60
// #define TINY_GSM_MODEM_MC60E
// #define TINY_GSM_MODEM_ESP32
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_ESP8266_NONOS
// #define TINY_GSM_MODEM_XBEE
// #define TINY_GSM_MODEM_SEQUANS_MONARCH

// Set serial for debug console (to the Serial Monitor)
#define SerialMon Serial

// If DBG isn't enabled, this sketch won't print anything to the console. Unless
// you want this to run silently, you should enable DBG.
#ifdef TINY_GSM_DEBUG
#undef TINY_GSM_DEBUG
#endif
#define TINY_GSM_DEBUG SerialMon

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#if !defined(__AVR_ATmega328P__) && !defined(SerialAT)
#define SerialAT Serial1

// or Software Serial on Uno, Nano
#elif !defined(SerialAT)
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(2, 3);  // RX, TX
#endif

// See all AT commands, if wanted
// WARNING: At high baud rates, incoming data may be lost when dumping AT
// commands
// #define DUMP_AT_COMMANDS

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 921600
#ifndef TARGET_BAUD
#define TARGET_BAUD 115200
#endif

/*
 * Tests enabled
 */
#define TINY_GSM_TEST_CELLULAR_DATA true
#define TINY_GSM_TEST_WIFI false
#define TINY_GSM_TEST_TCP true
#define TEST_BUILD_ADD_CERTS true
#define TINY_GSM_TEST_SSL true
#define TINY_GSM_TEST_CALL true
#define TINY_GSM_TEST_SMS true
#define TINY_GSM_TEST_USSD true
#define TINY_GSM_TEST_BATTERY true
#define TINY_GSM_TEST_TEMPERATURE true
#define TINY_GSM_TEST_GSM_LOCATION true
#define TINY_GSM_TEST_GPS true
#define TINY_GSM_TEST_NTP true
#define TINY_GSM_TEST_TIME true
// disconnect and power down modem after tests
#define TINY_GSM_POWERDOWN true

// set SIM card pin, if any
#define SIM_PIN ""

// Set phone numbers, if you want to test SMS and Calls
// #define SMS_TARGET  "+380xxxxxxxxx"
// #define CALL_TARGET "+380xxxxxxxxx"

// Your SIM card credentials, if any
const char apn[]     = "YourAPN";
const char simUser[] = "";
const char simPass[] = "";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

// Server details to test TCP without SSL
const char server[]   = "time.sodaq.net";
const char resource[] = "/";
const int  port       = 80;
// Server details to test TCP over SSL
// const char server_ssl[]   = "vsh.pp.ua";
// const char resource_ssl[] = "/TinyGSM/logo.txt";
const char server_ssl[] = "raw.githubusercontent.com";
const char resource_ssl[] =
    "/vshymanskyy/TinyGSM/refs/heads/master/extras/logo.txt";
const int port_ssl = 443;

#include <TinyGsmClient.h>
#include <TinyGsmCapabilities.h>

#if (defined(ARDUINO_NRF52840_FEATHER)) && !defined(ADAFRUIT_TINYUSB_H_)
#include <Adafruit_TinyUSB.h>  // for Serial
#endif

#if TINY_GSM_TEST_CELLULAR_DATA && not defined TINY_GSM_MODEM_HAS_CELLULAR_DATA
#undef TINY_GSM_TEST_CELLULAR_DATA
#undef TINY_GSM_TEST_WIFI
#define TINY_GSM_TEST_CELLULAR_DATA false
#define TINY_GSM_TEST_WIFI true
#endif
#if TINY_GSM_TEST_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_TEST_CELLULAR_DATA
#undef TINY_GSM_TEST_WIFI
#define TINY_GSM_TEST_CELLULAR_DATA true
#define TINY_GSM_TEST_WIFI false
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

void setup() {
  // Set console baud rate
#if defined(__AVR__)
  SerialMon.begin(115200);
#else
  SerialMon.begin(921600);
#endif
  while (!SerialMon && millis() < 10000L) {}
  delay(10);

  SerialMon.println("Wait...");

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  DBG("Wait...");
  delay(500L);

  SerialMon.println(F("All functions example for TinyGSM..."));
  SerialMon.println(F("The current version of TinyGSM is " TINYGSM_VERSION));
  SerialMon.print(F("The configured modem is "));
  SerialMon.println(modem.getConfiguredModem());
  SerialMon.println("=====================================");

  DBG("Looking for modem at", TARGET_BAUD, "baud and setting baud rate to",
      TARGET_BAUD, "if it is not already the baud rate of the modem.");
  uint32_t maximum = 921600;
#if defined(F_CPU)
  if (F_CPU <= 8000000L) {
    maximum = 57600;
  } else if (F_CPU <= 16000000L) {
    maximum = 115200;
  }
#endif
  uint32_t targetBaud = TARGET_BAUD;
  if (targetBaud > maximum) {
    DBG("Target baud rate", targetBaud,
        "is too high for this processor.  Maximum is", maximum);
    targetBaud = maximum;
  }

#if !defined(TINY_GSM_MODEM_XBEE) && !defined(TINY_GSM_SKIP_BAUDING)
  // Set GSM module baud rate
  uint32_t found_baud = TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN,
                                        GSM_AUTOBAUD_MAX);
  if (found_baud != 0 && found_baud != targetBaud) {
    DBG("Changing baud rate from", found_baud, "to", targetBaud);
#if defined(TINY_GSM_MODEM_ESP32) || defined(TINY_GSM_MODEM_ESP8266)
    modem.setDefaultBaud(targetBaud);
#else
    modem.setBaud(targetBaud);
#endif
    SerialAT.end();
    SerialAT.begin(targetBaud);
  } else if (found_baud == 0) {
    DBG("Attempting to force baud rate to", targetBaud);
    modem.forceModemBaud(SerialAT, targetBaud);
  }
#else
  SerialAT.begin(targetBaud);
#endif
}

void loop() {
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBG("Initializing modem...");
  if (!modem.restart()) {
    // if (!modem.init()) {
    DBG("Failed to restart modem, delaying 10s and retrying");
    // restart autobaud in case GSM just rebooted
    // TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
    return;
  }

  String modemInfo = modem.getModemInfo();
  DBG("Modem Info:", modemInfo);
  (void)modemInfo;

  String name = modem.getModemName();
  DBG("Modem Name:", name);
  (void)name;

  String manufacturer = modem.getModemManufacturer();
  DBG("Modem Manufacturer:", manufacturer);
  (void)manufacturer;

  String hw_ver = modem.getModemModel();
  DBG("Modem Hardware Version:", hw_ver);
  (void)hw_ver;

  String fv_ver = modem.getModemRevision();
  DBG("Modem Firmware Version:", fv_ver);
  (void)fv_ver;

#if !defined(TINY_GSM_MODEM_ESP32) && !defined(TINY_GSM_MODEM_ESP8266) && \
    !defined(TINY_GSM_MODEM_ESP8266_NONOS)
  String mod_sn = modem.getModemSerialNumber();
  DBG("Modem Serial Number (may be SIM CCID):", mod_sn);
  (void)mod_sn;
#endif

  // Display modem capabilities using compile-time detection
  DBG("Modem Capabilities:");
  DBG("  Cellular Data:",
      TinyGsmCapabilities::has_cellular_data<TinyGsm>::value ? "YES" : "NO");
  DBG("  WiFi:", TinyGsmCapabilities::has_wifi<TinyGsm>::value ? "YES" : "NO");
  DBG("  SSL:", TinyGsmCapabilities::has_ssl<TinyGsm>::value ? "YES" : "NO");
  DBG("  GPS:", TinyGsmCapabilities::has_gps<TinyGsm>::value ? "YES" : "NO");
  DBG("  SMS:", TinyGsmCapabilities::has_sms<TinyGsm>::value ? "YES" : "NO");
  DBG("  Calling:",
      TinyGsmCapabilities::has_calling<TinyGsm>::value ? "YES" : "NO");
  DBG("  Battery:",
      TinyGsmCapabilities::has_battery<TinyGsm>::value ? "YES" : "NO");
  DBG("  Temperature:",
      TinyGsmCapabilities::has_temperature<TinyGsm>::value ? "YES" : "NO");
  DBG("  GSM Location:",
      TinyGsmCapabilities::has_gsm_location<TinyGsm>::value ? "YES" : "NO");
  DBG("  NTP:", TinyGsmCapabilities::has_ntp<TinyGsm>::value ? "YES" : "NO");
  DBG("  Time:", TinyGsmCapabilities::has_time<TinyGsm>::value ? "YES" : "NO");

#if TINY_GSM_TEST_CELLULAR_DATA
  // Unlock your SIM card with a PIN if needed
  if (SIM_PIN && modem.getSimStatus() != SIM_READY) {
    // simUnlock will do nothing if the pin is empty
    modem.simUnlock(SIM_PIN);
  }
#endif

#if TINY_GSM_TEST_WIFI && defined(TINY_GSM_MODEM_HAS_WIFI)
  DBG("Setting SSID/password...");
  if (!modem.networkConnect(wifiSSID, wifiPass)) {
    DBG(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");
#endif

#if TINY_GSM_TEST_CELLULAR_DATA && defined(TINY_GSM_MODEM_XBEE)
  // The XBee must run the dataConnect function BEFORE waiting for network!
  modem.dataConnect(apn, simUser, simPass);
#endif

  DBG("Waiting for network...");
  if (!modem.waitForNetwork(600000L, true)) {
    delay(10000);
    return;
  }

  if (modem.isNetworkConnected()) { DBG("Network connected"); }

#if TINY_GSM_TEST_CELLULAR_DATA
  DBG("Connecting to", apn);
  if (!modem.dataConnect(apn, simUser, simPass)) {
    delay(10000);
    return;
  }

  bool res = modem.isDataConnected();
  DBG("Cellular Data status:", res ? "connected" : "not connected");
  (void)res;

  String ccid = modem.getSimCCID();
  DBG("CCID:", ccid);
  (void)ccid;

  String imei = modem.getIMEI();
  DBG("IMEI:", imei);

  String imsi = modem.getIMSI();
  DBG("IMSI:", imsi);
  (void)imsi;

  String cop = modem.getOperator();
  DBG("Operator:", cop);
  (void)cop;

  // String prov = modem.getProvider();
  // DBG("Provider:", prov);

  IPAddress local = modem.localIP();
  DBG("Local IP:", local);
  (void)local;

  int csq = modem.getSignalQuality();
  DBG("Signal quality:", csq);
  (void)csq;
#endif

#if TINY_GSM_TEST_USSD && defined TINY_GSM_MODEM_HAS_SMS && \
    !defined(TINY_GSM_MODEM_SARAR4) && !defined(TINY_GSM_MODEM_XBEE)
  String ussd_balance = modem.sendUSSD("*111#");
  DBG("Balance (USSD):", ussd_balance);
  (void)ussd_balance;

  String ussd_phone_num = modem.sendUSSD("*161#");
  DBG("Phone number (USSD):", ussd_phone_num);
  (void)ussd_phone_num;
#endif

#if TINY_GSM_TEST_TCP && defined TINY_GSM_MODEM_HAS_TCP
  TinyGsmClient client(modem, 0);
  DBG("Connecting to", server);
  if (!client.connect(server, port)) {
    DBG("... failed");
  } else {
    DBG(GF("... connected"));

    DBG(GF("Sending a raw HTTP GET request as three print commands."));
    // Make a HTTP GET request:
    client.print(String("GET ") + resource + " HTTP/1.1\r\n");
    client.print(String("Host: ") + server + "\r\n");
    client.print("Connection: close\r\n\r\n");
    client.flush();

    // Wait for data to arrive
    uint32_t start = millis();
    while (client.connected() && !client.available() &&
           millis() - start < 30000L) {
      delay(100);
    };

    // Read data
    start               = millis();
    char time_page[250] = {
        '\0',
    };
    int read_chars = 0;
    while (client.connected() && millis() - start < 10000L) {
      while (client.available() &&
             read_chars < static_cast<int>(sizeof(time_page)) - 1) {
        time_page[read_chars] = client.read();
        read_chars++;
        time_page[read_chars] = '\0';
        start                 = millis();
      }
    }
    SerialMon.println("\n----------------------------------");
    SerialMon.println(time_page);
    SerialMon.println("----------------------------------\n");
    DBG("#####  RECEIVED:", strlen(time_page), "CHARACTERS");
    client.stop();
  }

  DBG("Connecting a second time to", server);
  if (!client.connect(server, port)) {
    DBG("... failed");
  } else {
    DBG(GF("... connected"));
    DBG(GF("Sending a raw HTTP GET request as a single write command."));

    // make a buffer to contain the whole request
    char request[sizeof(resource) + sizeof(server) + 45] = {
        '\0',
    };
    // concatenate the request
    strcat(request, "GET ");
    strcat(request, resource);
    strcat(request, " HTTP/1.1\r\n");
    strcat(request, "Host: ");
    strcat(request, server);
    strcat(request, "\r\n");
    strcat(request, "Connection: close\r\n\r\n");
    // Write the request out
    client.write((uint8_t*)request, strlen(request));
    client.flush();

    // Wait for data to arrive
    uint32_t start = millis();
    while (client.connected() && !client.available() &&
           millis() - start < 30000L) {
      delay(100);
    };

    // Read data
    start               = millis();
    char time_page[250] = {
        '\0',
    };
    int read_chars = 0;
    while (client.connected() && millis() - start < 10000L) {
      size_t space = sizeof(time_page) - 1 - read_chars;
      size_t avail = client.available();
      if (avail > space) { avail = space; }
      if (avail) {
        read_chars += client.read(
            reinterpret_cast<uint8_t*>(time_page) + read_chars, avail);
        time_page[read_chars] = '\0';
        start                 = millis();
      }
    }
    SerialMon.println("\n----------------------------------");
    SerialMon.println(time_page);
    SerialMon.println("----------------------------------\n");
    DBG("#####  RECEIVED:", strlen(time_page), "CHARACTERS");
    client.stop();
  }
#endif

#if TINY_GSM_TEST_SSL && \
    (defined TINY_GSM_MODEM_ESP32 || defined TINY_GSM_MODEM_ESP8266)
  DBG(GF("Setting timezone..."));
  modem.setTimeZone(-5);
  DBG(GF("Waiting for time sync..."));
  modem.waitForTimeSync();
#endif

#if TINY_GSM_TEST_SSL && defined TINY_GSM_MODEM_HAS_SSL
  TinyGsmClientSecure secureClient(modem, (uint8_t)0);

#if defined(TINY_GSM_MODEM_CAN_SPECIFY_CERTS)
  TinyGsmSecureContext secureContext(modem, (uint8_t)0);
  DBG(GF("Setting SSL authentication mode..."));
  secureContext.setSSLAuthMode(SSLAuthMode::CA_VALIDATION);

// For Espressif modules, only two certificate sets are supported and the
// certificates must be named "client_ca.{0|1}", "client_cert.{0|1}", or
// "client_key.{0|1}"
#ifdef TINY_GSM_MODEM_ESP32
  const char* root_ca_name = "client_ca.0";
  // const char* client_cert_name = "client_cert.0";
  // const char* client_key_name  = "client_key.0";
#else
  // For most modules the actual filename doesn't matter much but it CANNOT
  // HAVE SPACES.
  // Some modules will not accept filenames with special characters so avoid
  // those, too.
  // This library restricts file names to 32 characters.
  const char* root_ca_name = "isrgrootx1.pem";
  // const char* client_cert_name = THING_NAME "-certificate.pem.crt";
  // const char* client_key_name  = THING_NAME "-private-key.pem.key";
#endif

#if TEST_BUILD_ADD_CERTS && defined(TINY_GSM_MODEM_CAN_LOAD_CERTS)

  static const char isrgrootx1_certificate[] TINY_GSM_PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

  DBG(GF("\nLoading root CA certificate..."));
  modem.loadCertificate(root_ca_name, GFP(isrgrootx1_certificate),
                        sizeof(isrgrootx1_certificate) - 1);
  DBG(GF("\n\nConverting root CA certificate..."));
  modem.convertCACertificate(root_ca_name);
#if 0
  DBG(GF("Loading client certificate..."));
  modem.loadCertificate(client_cert_name, client_cert,
                        strlen(client_cert));
  DBG(GF("Loading client key..."));
  modem.loadCertificate(client_key_name, client_key,
                        strlen(client_key));
  DBG(GF("Converting client certificate..."));
  modem.convertClientCertificates(client_cert_name, client_key_name);
  DBG(GF("Converting PSK and ID..."));
  modem.convertPSKandID(psk_name, psk_hint_name);
#endif

#if !defined(TINY_GSM_MODEM_ESP32) && !defined(TINY_GSM_MODEM_BG96) &&     \
    !defined(TINY_GSM_MODEM_A7672X) && !defined(TINY_GSM_MODEM_SIM7600) && \
    !defined(TINY_GSM_MODEM_XBEE)
  DBG(GF("Deleting root CA certificate..."));
  modem.deleteCertificate(root_ca_name);
#endif
#endif

  DBG(GF("Setting the CA certificate name in the secure context..."));
  secureContext.setCACertName(root_ca_name);
#if 0
  DBG(GF("Setting the client credentials in the secure context..."));
  secureContext.setClientCredentials(client_cert_name, client_key_name);
  DBG(GF("Setting the pre-shared key in the secure context..."));
  secureContext.setPreSharedKey(pre_shared_key_hint_text, pre_shared_key_text);
#endif

#endif

#if defined(TINY_GSM_MODEM_CAN_SPECIFY_CERTS) && \
    !defined(TINY_GSM_MODEM_CAN_LOAD_CERTS)
  // if we didn't actually load a certificate, don't use it
  DBG(GF("Setting SSL authentication mode to not validate because certificates "
         "weren't loaded..."));
  secureContext.setSSLAuthMode(SSLAuthMode::NO_VALIDATION);
#endif

  DBG("Connecting securely to", server_ssl);
  if (!secureClient.connect(server_ssl, port_ssl)) {
    DBG("... failed");
  } else {
    // make a buffer to contain the whole request
    char request[sizeof(resource_ssl) + sizeof(server_ssl) + 45] = {
        '\0',
    };
    // concatenate the request
    strcat(request, "GET ");
    strcat(request, resource_ssl);
    strcat(request, " HTTP/1.1\r\n");
    strcat(request, "Host: ");
    strcat(request, server_ssl);
    strcat(request, "\r\n");
    strcat(request, "Connection: close\r\n\r\n");
    // Write the request out
    secureClient.write((uint8_t*)request, strlen(request));
    secureClient.flush();

    // Wait for data to arrive
    uint32_t startS = millis();
    while (secureClient.connected() && !secureClient.available() &&
           millis() - startS < 30000L) {
      delay(100);
    };

    // Read data
    startS         = millis();
    char logo[640] = {
        '\0',
    };
    int read_charsS = 0;
    while (secureClient.connected() && millis() - startS < 10000L) {
      while (secureClient.available() &&
             read_charsS < static_cast<int>(sizeof(logo)) - 1) {
        logo[read_charsS] = secureClient.read();
        read_charsS++;
        logo[read_charsS] = '\0';
        startS            = millis();
      }
    }
    SerialMon.println("\n----------------------------------");
    SerialMon.println(logo);
    SerialMon.println("----------------------------------\n");
    DBG("#####  RECEIVED:", strlen(logo), "CHARACTERS");
    secureClient.stop();
  }
#endif

#if TINY_GSM_TEST_CALL && defined(TINY_GSM_MODEM_HAS_CALLING) && \
    defined(CALL_TARGET)
  DBG("Calling:", CALL_TARGET);

  // This is NOT supported on M590
  res = modem.callNumber(CALL_TARGET);
  DBG("Call:", res ? "OK" : "fail");

  if (res) {
    delay(1000L);

    // Play DTMF A, duration 1000ms
    modem.dtmfSend('A', 1000);

    // Play DTMF 0..4, default duration (100ms)
    for (char tone = '0'; tone <= '4'; tone++) { modem.dtmfSend(tone); }

    delay(5000);

    res = modem.callHangup();
    DBG("Hang up:", res ? "OK" : "fail");
  }
#endif

// Test the SMS functions
#if TINY_GSM_TEST_SMS && defined TINY_GSM_MODEM_HAS_SMS && defined SMS_TARGET
  res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
  DBG("SMS:", res ? "OK" : "fail");

  // This is only supported on SIMxxx series
  res = modem.sendSMS_UTF8_begin(SMS_TARGET);
  if (res) {
    auto stream = modem.sendSMS_UTF8_stream();
    stream.print(F("Привіііт! Print number: "));
    stream.print(595);
    res = modem.sendSMS_UTF8_end();
  }
  DBG("UTF8 SMS:", res ? "OK" : "fail");

#endif

// Test the GSM location functions
#if TINY_GSM_TEST_GSM_LOCATION && defined TINY_GSM_MODEM_HAS_GSM_LOCATION
  float gsm_latitude  = 0;
  float gsm_longitude = 0;
  float gsm_accuracy  = 0;
  int   gsm_year      = 0;
  int   gsm_month     = 0;
  int   gsm_day       = 0;
  int   gsm_hour      = 0;
  int   gsm_minute    = 0;
  int   gsm_second    = 0;
  for (int8_t i = 15; i; i--) {
    DBG("Requesting current GSM location");
    if (modem.getGsmLocation(&gsm_latitude, &gsm_longitude, &gsm_accuracy,
                             &gsm_year, &gsm_month, &gsm_day, &gsm_hour,
                             &gsm_minute, &gsm_second)) {
      break;
    } else if (i > 1) {
      DBG("Couldn't get GSM location, retrying in 15s.");
      delay(15000L);
    }
  }
  // Print the coordinates, date, and time, even if the overall query failed, to
  // show which portions were filled in
  DBG("Latitude:", String(gsm_latitude, 8),
      "\tLongitude:", String(gsm_longitude, 8));
  DBG("Accuracy:", gsm_accuracy);
  DBG("Year:", gsm_year, "\tMonth:", gsm_month, "\tDay:", gsm_day);
  DBG("Hour:", gsm_hour, "\tMinute:", gsm_minute, "\tSecond:", gsm_second);
  DBG("Retrieving GSM location again as a string");
  String location = modem.getGsmLocation();
  DBG("GSM Based Location String:", location);
#endif

// Test the GPS functions
#if TINY_GSM_TEST_GPS && defined TINY_GSM_MODEM_HAS_GPS
  DBG("Enabling GPS/GNSS/GLONASS and waiting 15s for warm-up");

  modem.enableGPS();

  delay(15000L);
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
  for (int8_t i = 15; i; i--) {
    DBG("Requesting current GPS/GNSS/GLONASS location");
    if (modem.getGPS(&gps_latitude, &gps_longitude, &gps_speed, &gps_altitude,
                     &gps_vsat, &gps_usat, &gps_accuracy, &gps_year, &gps_month,
                     &gps_day, &gps_hour, &gps_minute, &gps_second)) {
      break;
    } else if (i > 1) {
      DBG("Couldn't get GPS/GNSS/GLONASS location, retrying in 15s.");
      delay(15000L);
    }
  }
  // Print the NMEA information, even if the overall query failed, to show which
  // portions were filled in
  DBG("Latitude:", String(gps_latitude, 8),
      "\tLongitude:", String(gps_longitude, 8));
  DBG("Speed:", gps_speed, "\tAltitude:", gps_altitude);
  DBG("Visible Satellites:", gps_vsat, "\tUsed Satellites:", gps_usat);
  DBG("Accuracy:", gps_accuracy);
  DBG("Year:", gps_year, "\tMonth:", gps_month, "\tDay:", gps_day);
  DBG("Hour:", gps_hour, "\tMinute:", gps_minute, "\tSecond:", gps_second);
  DBG("Retrieving GPS/GNSS/GLONASS location again as a string");
  String gps_raw = modem.getGPSraw();
  (void)gps_raw;
  DBG("GPS/GNSS Based Location String:", gps_raw);

  DBG("Disabling GPS");
  modem.disableGPS();
#endif

// Test the Network time functions
#if TINY_GSM_TEST_NTP && defined TINY_GSM_MODEM_HAS_NTP
  DBG("Asking modem to sync with NTP");
  modem.NTPServerSync("pool.ntp.org", -20);
#endif

#if TINY_GSM_TEST_TIME && defined TINY_GSM_MODEM_HAS_TIME
  int   ntp_year     = 0;
  int   ntp_month    = 0;
  int   ntp_day      = 0;
  int   ntp_hour     = 0;
  int   ntp_min      = 0;
  int   ntp_sec      = 0;
  float ntp_timezone = 0;
  for (int8_t i = 5; i; i--) {
    DBG("Requesting current network time");
    if (modem.getNetworkTime(&ntp_year, &ntp_month, &ntp_day, &ntp_hour,
                             &ntp_min, &ntp_sec, &ntp_timezone)) {
      break;
    } else if (i > 1) {
      DBG("Couldn't get network time, retrying in 15s.");
      delay(15000L);
    }
  }
  // Print the date and time, even if the overall query failed, to show which
  // portions were filled in
  DBG("Year:", ntp_year, "\tMonth:", ntp_month, "\tDay:", ntp_day);
  DBG("Hour:", ntp_hour, "\tMinute:", ntp_min, "\tSecond:", ntp_sec);
  DBG("Timezone:", ntp_timezone);
  DBG("Retrieving time again as a string");
  String time = modem.getGSMDateTime(TinyGSMDateTimeFormat::DATE_FULL);
  DBG("Current Network Time:", time);
#endif

// Test Battery functions
#if TINY_GSM_TEST_BATTERY && defined TINY_GSM_MODEM_HAS_BATTERY
  int8_t  chargeState   = -99;
  int8_t  chargePercent = -99;
  int16_t milliVolts    = -9999;
  modem.getBattStats(chargeState, chargePercent, milliVolts);
  DBG("Battery charge state:", chargeState);
  DBG("Battery charge 'percent':", chargePercent);
  DBG("Battery voltage:", milliVolts / 1000.0F);
#endif

// Test temperature functions
#if TINY_GSM_TEST_TEMPERATURE && defined TINY_GSM_MODEM_HAS_TEMPERATURE
  float temp = modem.getTemperature();
  DBG("Chip temperature:", temp);
  (void)temp;
#endif

#if TINY_GSM_POWERDOWN

#if TINY_GSM_TEST_CELLULAR_DATA
  modem.dataDisconnect();
  delay(5000L);
  if (!modem.isDataConnected()) {
    DBG("Cellular data disconnected");
  } else {
    DBG("Cellular Data disconnect: Failed.");
  }
#endif

#if TINY_GSM_TEST_WIFI
  modem.networkDisconnect();
  DBG("WiFi disconnected");
#endif

#if !defined(TINY_GSM_MODEM_ESP32)
  // Try to power-off (modem may decide to restart automatically)
  // To turn off modem completely, please use Reset/Enable pins
  modem.powerOff();
  DBG("Poweroff.");
#endif

#endif

  DBG("End of tests.");

  // Do nothing forevermore
  while (true) { modem.maintain(); }
}

// cSpell:ignore isrgrootx1
