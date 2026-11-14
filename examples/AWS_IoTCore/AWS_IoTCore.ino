/** ============================================================================
 * @example{lineno} AWS_IoTCore.ino
 *
 * @brief This example connects to AWS IoT Core using MQTT over SSL.
 *
 * This program writes new certificates to the modem, connects to AWS IoT Core,
 * publishes an initial message, and then subscribes to a topic to toggle an
 * LED. After the initial connection, the board will check and try to reconnect
 * every 10 seconds and republish its status every 60 seconds.  If it receives
 * any messages on the subscribed topic, it will toggle the LED state.  The
 * content of any received messages is ignored.
 *
 * @important You should run this program once to load your certificates and
 * confirm that you can connect to AWS IoT Core over MQTT. Once you have
 * confirmed your certificates are loaded and working, there is no reason to
 * rerun this program unless you have a new modem, reset your modem, or your
 * certificates change. Most modules store the certificates in flash, which has
 * a limited number of read/write cycles. To avoid wearing out the flash
 * unnecessarily, only run this program when necessary; do not re-write the
 * certificates every time you want to connect to AWS IoT Core.
 *
 * For this example, you need to install PubSubClient library:
 *   https://github.com/knolleary/pubsubclient
 *   or from http://librarymanager/all#PubSubClient
 *
 * @note This example only works for modules that have support for writing
 * certificates. Modules that support SSL in this library, but not writing
 * certificates, cannot use this example!
 * ========================================================================== */

// Select your modem:
// #define TINY_GSM_MODEM_SIM7000SSL
// #define TINY_GSM_MODEM_SIM7080
// #define TINY_GSM_MODEM_A7672X
// #define TINY_GSM_MODEM_ESP32
// #define TINY_GSM_MODEM_BG96

#define TINY_GSM_TCP_KEEP_ALIVE 180

// Set serial for debug console (to the Serial Monitor)
#define SerialMon Serial

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

// Define the serial console for debug prints, if needed
// #define TINY_GSM_DEBUG SerialMon

// Range to attempt to autobaud
// NOTE:  DO NOT AUTOBAUD in production code.  Once you've established
// communication, set a fixed baud rate using modem.setBaud(#).
#define GSM_AUTOBAUD_MIN 9600
#define GSM_AUTOBAUD_MAX 921600

#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "aws_iot_config.h"

#if (defined(ARDUINO_NRF52840_FEATHER)) && !defined(ADAFRUIT_TINYUSB_H_)
#include <Adafruit_TinyUSB.h>  // for Serial
#endif

// Define how you're planning to connect to the internet.
// This is only needed for this example, not in other code.
#define TINY_GSM_USE_CELLULAR_DATA true
#define TINY_GSM_USE_WIFI false

// set SIM card pin, if any
#define SIM_PIN ""

// Your SIM card credentials, if any
const char apn[]     = "YourAPN";
const char simUser[] = "";
const char simPass[] = "";

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

// MQTT details
// get the broker host/endpoint from AWS IoT Core / Connect / Domain
// Configurations
const char* broker = AWS_IOT_ENDPOINT;
// the secure connection port for MQTT is always 8883
uint16_t port = 8883;
// the client ID should be the name of your "thing" in AWS IoT Core
const char* clientId = THING_NAME;

static const char topicInit[]      = THING_NAME "/init";
static const char topicLed[]       = THING_NAME "/led";
static const char topicLedStatus[] = THING_NAME "/ledStatus";

// whether to print certs after uploading
// not all modules support printing the content of certificates after uploading
// them
bool print_certs = false;

// NOTE: some modems (SIM70xx modules) suggest that you delete the
// certificate file from the file system after converting the certificate so
// that they cannot be read back.  On other modules (SIM7600, A7672, ESP32,
// BG96, XBee) the certificate must be in the file system to be used and cannot
// be deleted.
#if defined(TINY_GSM_MODEM_ESP32) || defined(TINY_GSM_MODEM_BG96)
// DON'T delete the certificates after loading them into the modem!
bool delete_certs = false;
#else
// Change this if you want to delete the certificates after loading them into
// the modem This testing program won't delete by default
bool delete_certs = false;
#endif


// The certificates should generally be formatted as ".pem", ".der", or (for
// some modules) ".p7b" files.

// For Espressif modules, only two certificate sets are supported and the
// certificates must be named "client_ca.{0|1}", "client_cert.{0|1}", or
// "client_key.{0|1}"
#ifdef TINY_GSM_MODEM_ESP32
const char* root_ca_name     = "client_ca.1";
const char* client_cert_name = "client_cert.1";
const char* client_key_name  = "client_key.1";
#else
// For most modules the actual filename doesn't matter much but it CANNOT
// HAVE SPACES and should be less than 64 characters.
// Some modules will not accept filenames with special characters so avoid
// those, too.
// NOTE: The certificate names as they are downloaded from AWS IoT Core are
// often too long for the modem to handle. Pick something shorter.
const char* root_ca_name     = "AmazonRootCA1.pem";
const char* client_cert_name = THING_NAME "-certificate.pem.crt";
const char* client_key_name  = THING_NAME "-private-key.pem.key";
#endif

// Just in case someone defined the wrong thing..
#if TINY_GSM_USE_CELLULAR_DATA && not defined TINY_GSM_MODEM_HAS_CELLULAR_DATA
#undef TINY_GSM_USE_CELLULAR_DATA
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_CELLULAR_DATA false
#define TINY_GSM_USE_WIFI true
#endif
#if TINY_GSM_USE_WIFI && not defined TINY_GSM_MODEM_HAS_WIFI
#undef TINY_GSM_USE_CELLULAR_DATA
#undef TINY_GSM_USE_WIFI
#define TINY_GSM_USE_CELLULAR_DATA true
#define TINY_GSM_USE_WIFI false
#endif

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

TinyGsmSecureContext secureContext(modem, (uint8_t)0);
TinyGsmClientSecure  secureClient(modem, (uint8_t)0);
PubSubClient         mqtt(secureClient);

#define LED_PIN 13
int ledStatus = LOW;

uint32_t lastReconnectAttempt = 0;
uint32_t lastStatusPublished  = 0;
bool     setupSuccess         = false;
bool     certificateSuccess   = false;
bool     networkSuccess       = false;

bool wakeModem() {
  // !!!!!!!!!!!
  // Put your function to wake and prepare the modem here
  // !!!!!!!!!!!
  return true;
}

bool setModemBaud(uint32_t baud) {
  SerialMon.print(F("Setting modem baud rate to "));
  SerialMon.println(baud);
  if (!modem.setBaud(baud)) {
    SerialMon.println(F("...failed!"));
    return false;
  }
  SerialMon.println(F("...success!"));
  // Set the serial port to the new baud rate
  SerialAT.begin(baud);
  delay(100);
  return modem.init();  // May need to re-init to turn off echo, etc
}

void printModemInfo() {
  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info: ");
  SerialMon.println(modemInfo);
  String modemManufacturer = modem.getModemManufacturer();
  SerialMon.print("Modem Manufacturer: ");
  SerialMon.println(modemManufacturer);
  String modemModel = modem.getModemModel();
  SerialMon.print("Modem Model: ");
  SerialMon.println(modemModel);
  String modemRevision = modem.getModemRevision();
  SerialMon.print("Modem Revision: ");
  SerialMon.println(modemRevision);
#if !defined(TINY_GSM_MODEM_ESP32) && !defined(TINY_GSM_MODEM_ESP8266) && \
    !defined(TINY_GSM_MODEM_ESP8266_NONOS)
  String modemSerial = modem.getModemSerialNumber();
  SerialMon.print("Modem Serial: ");
  SerialMon.println(modemSerial);
#endif
#if TINY_GSM_USE_CELLULAR_DATA
  String modemIMEI = modem.getIMEI();
  SerialMon.print("Modem IMEI: ");
  SerialMon.println(modemIMEI);
  String modemIMSI = modem.getIMSI();
  SerialMon.print("Modem IMSI: ");
  SerialMon.println(modemIMSI);
  String modemSimCCID = modem.getSimCCID();
  SerialMon.print("Modem SIM CCID: ");
  SerialMon.println(modemSimCCID);
#endif
}

bool setupCertificates() {
#ifdef TINY_GSM_MODEM_CAN_LOAD_CERTS
  // ======================== CA CERTIFICATE LOADING ========================
  bool ca_cert_success = true;
  // add the server's certificate authority certificate to the modem
  SerialMon.print("Loading Certificate Authority Certificate");
  ca_cert_success &= modem.loadCertificate(root_ca_name,
                                           GFP(AWS_SERVER_CERTIFICATE),
                                           sizeof(AWS_SERVER_CERTIFICATE) - 1);
  delay(250);
  if (!ca_cert_success) {
    SerialMon.println(" ...failed to load CA certificate!");
    return false;
  }
  SerialMon.println(" ...success");
#if !defined(TINY_GSM_MODEM_A7672X) && !defined(TINY_GSM_MODEM_SIM7600)
  if (print_certs) {
    // print out the certificate to make sure it matches
    SerialMon.println(
        "Printing Certificate Authority Certificate to confirm it matches");
    modem.printCertificate(root_ca_name, SerialMon);
    delay(1000);
  }
#endif
  // convert the certificate to the modem's format
  SerialMon.print("Converting Certificate Authority Certificate");
  ca_cert_success &= modem.convertCACertificate(root_ca_name);
  delay(250);
  if (!ca_cert_success) {
    SerialMon.println(" ...failed to convert CA certificate!");
    return false;
  }
  SerialMon.println(" ...success");

  if (delete_certs) {
    ca_cert_success &= modem.deleteCertificate(root_ca_name);
    delay(1000);
  }

  // ===================== CLIENT CERTIFICATE LOADING =====================
  bool client_cert_success = true;
  // add the client's certificate and private key to the modem
  SerialMon.print("Loading Client Certificate");
  client_cert_success &=
      modem.loadCertificate(client_cert_name, GFP(AWS_CLIENT_CERTIFICATE),
                            sizeof(AWS_CLIENT_CERTIFICATE) - 1);
  delay(250);
#if !defined(TINY_GSM_MODEM_A7672X) && !defined(TINY_GSM_MODEM_SIM7600)
  if (print_certs) {
    // print out the certificate to make sure it matches
    modem.printCertificate(client_cert_name, SerialMon);
    delay(1000);
  }
#endif
  SerialMon.print(" and Client Private Key ");
  client_cert_success &=
      modem.loadCertificate(client_key_name, GFP(AWS_CLIENT_PRIVATE_KEY),
                            sizeof(AWS_CLIENT_PRIVATE_KEY) - 1);
  delay(250);
#if !defined(TINY_GSM_MODEM_A7672X) && !defined(TINY_GSM_MODEM_SIM7600)
  if (print_certs) {
    // print out the certificate to make sure it matches
    modem.printCertificate(client_key_name, SerialMon);
    delay(1000);
  }
#endif
  if (!client_cert_success) {
    SerialMon.println(" ...failed to load client certificate or key!");
    return false;
  }
  SerialMon.println(" ...success");
  // convert the client certificate pair to the modem's format
  client_cert_success &= modem.convertClientCertificates(client_cert_name,
                                                         client_key_name);
  delay(250);
  if (!client_cert_success) {
    SerialMon.println(" ...failed to convert client certificate and key!");
    return false;
  }
  SerialMon.println(" ...success");

  if (delete_certs) {
    client_cert_success &= modem.deleteCertificate(client_cert_name);
    client_cert_success &= modem.deleteCertificate(client_key_name);
    delay(1000);
  }

  // ================= SET CERTIFICATES FOR THE CONNECTION =================
  // AWS IoT Core requires mutual authentication
  DBG("Requiring mutual authentication on socket");
  secureContext.setSSLAuthMode(SSLAuthMode::MUTUAL_AUTHENTICATION);
  DBG("Requesting TLS 1.3 on socket");
  secureContext.setSSLVersion(SSLVersion::TLS1_3);
  // attach the uploaded certificates to the secure context
  DBG("Assigning", root_ca_name, "as certificate authority on socket");
  secureContext.setCACertName(root_ca_name);
  DBG("Assigning", client_cert_name, "as client certificate on socket");
  DBG("Assigning", client_key_name, "as client key on socket");
  secureContext.setClientCredentials(client_cert_name, client_key_name);

  return ca_cert_success & client_cert_success;
#else
  return false;
#endif
}

String createStatusMessage() {
  String msgStatus = "{\"clientId\":\"" THING_NAME "\"";
  msgStatus += ",\"LED status\":\"" + String(ledStatus) + "\"";

#if TINY_GSM_USE_CELLULAR_DATA
  String modemIMEI = modem.getIMEI();
  msgStatus += ",\"modemIMEI\":\"" + modemIMEI + "\"";
  String modemSimCCID = modem.getSimCCID();
  msgStatus += ",\"modemSimCCID\":\"" + modemSimCCID + "\"";
#endif

  int16_t modemService = modem.getSignalQuality();
  msgStatus += ",\"modemSignalQuality\":\"" + String(modemService) + "\"";

#ifdef TINY_GSM_MODEM_HAS_NTP
  String time = modem.getGSMDateTime(TinyGSMDateTimeFormat::DATE_FULL);
  msgStatus += ",\"modemTime\":\"" + time + "\"";
#endif
  msgStatus += "}";
  return msgStatus;
}

String createInitMessage() {
  String msgInit = "{\"clientId\":\"" THING_NAME "\"";

  String modemInfo = modem.getModemInfo();
  msgInit += ",\"modemInfo\":\"" + modemInfo + "\"";
  String modemManufacturer = modem.getModemManufacturer();
  msgInit += ",\"modemManufacturer\":\"" + modemManufacturer + "\"";
  String modemModel = modem.getModemModel();
  msgInit += ",\"modemModel\":\"" + modemModel + "\"";
  String modemRevision = modem.getModemRevision();
  msgInit += ",\"modemRevision\":\"" + modemRevision + "\"";
#if !defined(TINY_GSM_MODEM_ESP32) && !defined(TINY_GSM_MODEM_ESP8266) && \
    !defined(TINY_GSM_MODEM_ESP8266_NONOS)
  String modemSerial = modem.getModemSerialNumber();
  msgInit += ",\"modemSerial\":\"" + modemSerial + "\"";
#endif
#if TINY_GSM_USE_CELLULAR_DATA
  String modemIMEI = modem.getIMEI();
  msgInit += ",\"modemIMEI\":\"" + modemIMEI + "\"";
  String modemIMSI = modem.getIMSI();
  msgInit += ",\"modemIMSI\":\"" + modemIMSI + "\"";
  String modemSimCCID = modem.getSimCCID();
  msgInit += ",\"modemSimCCID\":\"" + modemSimCCID + "\"";
#endif

  int16_t modemService = modem.getSignalQuality();
  msgInit += ",\"modemSignalQuality\":\"" + String(modemService) + "\"";

#ifdef TINY_GSM_MODEM_HAS_NTP
  String time = modem.getGSMDateTime(TinyGSMDateTimeFormat::DATE_FULL);
  msgInit += ",\"modemTime\":\"" + time + "\"";
#endif

  msgInit += "}";

  return msgInit;
}

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  SerialMon.print("Message arrived [");
  SerialMon.print(topic);
  SerialMon.print("]: ");
  SerialMon.write(payload, len);
  SerialMon.println();

  // Only proceed if incoming message's topic matches
  if (String(topic) == topicLed) {
    ledStatus = !ledStatus;
    digitalWrite(LED_PIN, ledStatus);

    // Create a status message to send to the broker
    String msgStatus = createStatusMessage();
    mqtt.publish(topicLedStatus, msgStatus.c_str());
  }
}

bool mqttConnect() {
  SerialMon.print("Connecting to ");
  SerialMon.print(broker);
  SerialMon.print(" with client ID ");
  SerialMon.println(clientId);

  // Connect to MQTT Broker
  bool status = mqtt.connect(clientId);

  if (status == false) {
    SerialMon.println(" ...failed to connect to AWS IoT MQTT broker!");
    return false;
  }
  SerialMon.println(" ...success");

  // Create a init message to send to the broker
  String msgInit = createInitMessage();

  // Make sure the MQTT buffer is large enough to hold the
  // initial message and the topic name.
  uint16_t neededBuffer = MQTT_MAX_HEADER_SIZE + 2 +
      strnlen(topicInit, mqtt.getBufferSize()) + msgInit.length() + 1;
  if (mqtt.getBufferSize() < neededBuffer) {
    SerialMon.print("Increasing MQTT buffer size from ");
    SerialMon.print(mqtt.getBufferSize());
    SerialMon.print(" to ");
    SerialMon.println(neededBuffer);
    mqtt.setBufferSize(neededBuffer);
  }

  SerialMon.print("Publishing a message to ");
  SerialMon.println(topicInit);
  SerialMon.print("Message content: ");
  SerialMon.println(msgInit);

  bool got_pub = mqtt.publish(topicInit, msgInit.c_str());
  SerialMon.println(got_pub ? "published" : "failed to publish");
  SerialMon.print("Subscribing to ");
  SerialMon.println(topicLed);
  bool got_sub = mqtt.subscribe(topicLed);
  SerialMon.println(got_sub ? "subscribed" : "failed to subscribe");

  return mqtt.connected();
}

bool mqttPublishStatus() {
  SerialMon.print("Publishing a message to ");
  SerialMon.println(topicLedStatus);

  // Create a status message to send to the broker
  String msgStatus = createStatusMessage();
  bool   got_pub   = mqtt.publish(topicLedStatus, msgStatus.c_str());

  SerialMon.println(got_pub ? "published" : "failed to publish");
  return got_pub;
}

bool setupModem() {
  bool success = true;

#ifndef TINY_GSM_MODEM_XBEE
  // Attempt to autobaud the modem
  TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN, GSM_AUTOBAUD_MAX);
#else
  SerialAT.begin(9600);
#endif

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.print("Initializing modem...");
  if (!modem.init()) {  // modem.restart();
    SerialMon.println(" ...failed to initialize modem!");
    delay(15000L);
    return false;
  }
  SerialMon.println(" ...success");

  // Max out the baud rate, if desired
  // NOTE: Do this **AFTER** the modem has been restarted - many modules
  // revert to default baud rates when reset or powered off.
  success &= setModemBaud(115200);

  printModemInfo();

#if TINY_GSM_USE_CELLULAR_DATA
  // Unlock your SIM card with a PIN if needed
  if (SIM_PIN && modem.getSimStatus() != SIM_READY) {
    // simUnlock will do nothing if the pin is empty
    success &= modem.simUnlock(SIM_PIN);
  }
#endif

  return success;
}

bool setupNetwork() {
  bool success = true;

#if TINY_GSM_USE_WIFI
  // Wifi connection parameters must be set before waiting for the network
  SerialMon.print(F("Setting SSID/password..."));
  success &= modem.networkConnect(wifiSSID, wifiPass);
  if (!success) {
    SerialMon.println(" ...failed to connect to WiFi!");
    return false;
  }
  SerialMon.println(" ...success");
#endif

#if TINY_GSM_USE_CELLULAR_DATA && defined TINY_GSM_MODEM_XBEE
  // The XBee must run the dataConnect function BEFORE waiting for network!
  // All other modules must wait for network first.
  success &= modem.dataConnect(apn, simUser, simPass);
#endif

  return success;
}

bool getInternetConnection() {
  // Make sure we're connected to or registered on the network
  // For Wi-Fi this is all we need to do
  if (!modem.isNetworkConnected()) {
    SerialMon.println("Network disconnected");
    SerialMon.println("Waiting up to 5 minutes for network connection...");
    if (!modem.waitForNetwork(300000L, true)) {
      SerialMon.println(" ...failed to reconnect to network!");
      delay(15000L);
      return false;
    }
    if (modem.isNetworkConnected()) { SerialMon.println("Network connected"); }
  }

#if TINY_GSM_USE_CELLULAR_DATA
  // Make sure cellular data is connected
  // For cellular data we need to connect to the cellular network in addition to
  // the base network connection.
  if (!modem.isDataConnected()) {
    SerialMon.println("Cellular data disconnected!");
    SerialMon.print(F("Connecting to "));
    SerialMon.println(apn);
    if (!modem.dataConnect(apn, simUser, simPass)) {
      SerialMon.println(" ...failed to connect to cellular data!");
      delay(15000L);
      return false;
    }
    if (modem.isDataConnected()) {
      SerialMon.println("Cellular data reconnected");
    }
  }
#endif

#ifdef TINY_GSM_MODEM_HAS_NTP
  // enable/force time sync with NTP server
  // This is **REQUIRED** for validated SSL connections
  static bool timeSyncSuccess = false;
  if (!timeSyncSuccess) {
    DBG("Enabling time sync with NTP server");
    timeSyncSuccess = modem.NTPServerSync("pool.ntp.org", -16);
    if (!timeSyncSuccess) { return false; }
  }
#endif

  // check and print the signal quality for debugging
  int16_t modemService = modem.getSignalQuality();
  SerialMon.print("Signal Quality: ");
  SerialMon.println(modemService);

#ifdef TINY_GSM_MODEM_HAS_NTP
  // check and print the current network time to ensure that the modem has
  // synchronized with the NTP server
  String time = modem.getGSMDateTime(TinyGSMDateTimeFormat::DATE_FULL);
  DBG("Current Network Time:", time);
#endif

  return true;
}


void setup() {
  // Set console baud rate
  SerialMon.begin(921600);
  delay(10);

  while (!SerialMon && millis() < 10000L) {}

  pinMode(LED_PIN, OUTPUT);

  // MQTT Broker setup
  // NOTE: This is only configuring the server and callback within the
  // PubSubClient object.
  // It does not take any action.
  mqtt.setServer(broker, port);
  mqtt.setCallback(mqttCallback);

  DBG("TINY_GSM_USE_WIFI:", TINY_GSM_USE_WIFI);
  DBG("TINY_GSM_USE_CELLULAR_DATA:", TINY_GSM_USE_CELLULAR_DATA);

  wakeModem();

  DBG("Wait...");
  delay(500L);

  SerialMon.println("Setting up modem...");
  setupSuccess = setupModem();
  if (!setupSuccess) {
    SerialMon.println(" ...failed to set up modem!");
    delay(15000L);
    return;
  }
  SerialMon.println(" ...success");

  SerialMon.println("Loading and configuring certificates...");
  certificateSuccess = setupCertificates();
  if (!certificateSuccess) {
    SerialMon.println(" ...failed to set up certificates!");
    delay(15000L);
    return;
  }
  SerialMon.println(" ...success");

  SerialMon.println("Setting up network...");
  networkSuccess = setupNetwork();
  if (!networkSuccess) {
    SerialMon.println(" ...failed to set up network!");
    delay(15000L);
    return;
  }
  SerialMon.println(" ...success");

  getInternetConnection();

  delay(500);
  DBG("Finished setup");
}

void loop() {
  if (!setupSuccess) {
    SerialMon.println("Modem setup failed, re-trying...");
    setupSuccess = setupModem();
    if (!setupSuccess) {
      SerialMon.println(" ...failed to set up modem!");
      delay(15000L);
      return;
    }
    SerialMon.println(" ...success");
  }

  if (!certificateSuccess) {
    SerialMon.println("Modem certificate configuration failed, re-trying...");
    certificateSuccess = setupCertificates();
    if (!certificateSuccess) {
      SerialMon.println(" ...failed to set up certificates!");
      delay(15000L);
      return;
    }
    SerialMon.println(" ...success");
  }

  if (!networkSuccess) {
    SerialMon.println("Network setup failed, re-trying...");
    networkSuccess = setupNetwork();
    if (!networkSuccess) {
      SerialMon.println(" ...failed to set up network!");
      delay(15000L);
      return;
    }
    SerialMon.println(" ...success");
  }

  // Re-test and reconnect if necessary every 10 seconds
  if (millis() - lastReconnectAttempt > 10000L) {
    lastReconnectAttempt = millis();
    if (!mqtt.connected()) {
      SerialMon.println("=== MQTT NOT CONNECTED ===");
      if (getInternetConnection()) { mqttConnect(); }
    }
  }

  // publish the current LED status every 60 seconds
  if (millis() - lastStatusPublished > 60000L) {
    lastStatusPublished = millis();
    if (mqtt.connected()) { mqttPublishStatus(); }
  }

  mqtt.loop();
}
