/** ============================================================================
 * @example{lineno} EspressifUpdateFirmware.ino
 *
 * @brief This sketch connects to WiFi and attempts to update the firmware for
 * an Espressif module using an over-the-air update.
 * ========================================================================== */

// Select your modem:
// #define TINY_GSM_MODEM_ESP32
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_ESP8266_NONOS

#if (defined(ARDUINO_NRF52840_FEATHER)) && !defined(ADAFRUIT_TINYUSB_H_)
#include <Adafruit_TinyUSB.h>  // for Serial
#endif

// Set serial for debug console (to the Serial Monitor)
#define SerialMon Serial
// Set console baud rate
#if !defined(SerialBaud) && defined(__AVR__)
#define SerialBaud 115200
#elif !defined(SerialBaud)
#define SerialBaud 921600
#endif

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

// Your WiFi connection credentials, if applicable
const char wifiSSID[] = "YourSSID";
const char wifiPass[] = "YourWiFiPass";

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, SerialMon);
TinyGsm        modem(debugger);
#else
TinyGsm modem(SerialAT);
#endif

bool setupSuccess = false;


void printModemInfo() {
  String modemInfo = modem.getModemInfo();
  SerialMon.print(GF("Modem Info: "));
  SerialMon.println(modemInfo);

  String name = modem.getModemName();
  SerialMon.print(GF("Modem Name: "));
  SerialMon.println(name);

  String manufacturer = modem.getModemManufacturer();
  SerialMon.print(GF("Modem Manufacturer: "));
  SerialMon.println(manufacturer);

  String hw_ver = modem.getModemModel();
  SerialMon.print(GF("Modem Hardware Version: "));
  SerialMon.println(hw_ver);

  String fv_ver = modem.getModemRevision();
  SerialMon.print(GF("Modem Firmware Version: "));
  SerialMon.println(fv_ver);
}

bool connectNetwork() {
  SerialMon.println(GF("Setting SSID/password..."));
  if (!modem.networkConnect(wifiSSID, wifiPass)) {
    SerialMon.println(GF(" ...failed"));
    delay(10000UL);
    return false;
  }
  SerialMon.println(GF(" ...success"));

  SerialMon.println(GF("Waiting for network..."));
  if (!modem.waitForNetwork(600000UL, true)) {
    delay(10000UL);
    return false;
  }

  if (modem.isNetworkConnected()) {
    SerialMon.println(GF(" ...network connected"));
    return true;
  }
  return false;
}

void checkNetworkTime() {
  modem.NTPServerSync("pool.ntp.org", -20);
  modem.waitForTimeSync();
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
}


bool updateFirmware(uint32_t update_timeout = 300000UL) {
  SerialMon.println(GF("Attempting to update firmware..."));
  bool     success         = true;
  bool     update_complete = false;
  uint32_t update_start    = millis();
  // Update using default firmware server and settings in blocking mode
  modem.sendAT(GF("+CIUPDATE"));
  // in blocking mode, we wait for the update to complete all the way to the
  // final OK
  if (success) { success &= modem.waitResponse(update_timeout) == 1; }
#if 0
  // If we used non-blocking mode, we could use this
  // Non-blocking mode requires manually specifying the update server
  // check for successful beginning of the update
  success &= modem.waitResponse(update_timeout, GF("+CIPUPDATE:")) == 1;
  // wait for the OK after the command to start updating the firmware
  if (success) { success &= modem.waitResponse() == 1; }
  // poll for the update status
  if (success) {
    SerialMon.println(GF("Firmware update started"));
    while (!update_complete && (millis() - update_start < update_timeout)) {
      modem.sendAT(GF("+CIUPDATE?"));
      success = modem.waitResponse(GF("+CIPUPDATE:")) == 1;
      int16_t res;
      if (success) {
        res = SerialAT.parseInt();
        success &= modem.waitResponse() == 1;
      } else {
        res = -1;
      }
      if (res == 4) {
        SerialMon.println(GF("Firmware update completed successfully"));
        update_complete = true;
      } else if (res == -1) {
        SerialMon.println(GF("Firmware update failed"));
        update_complete = true;
      } else {
        SerialMon.println(GF("Firmware update ongoing"));
        delay(1000L);
      }
    }
  }
#endif
  if (success) {
    delay(2500L);  // let things settle
    SerialMon.println("Restoring initial configuration...");
    modem.sendAT(GF("+RESTORE"));
    success &= modem.waitResponse(5000L) == 1;
    delay(2500L);  // let things settle

    SerialAT.end();
    // After a restore, the baud rate will revert to the default 115200
    SerialAT.begin(115200);

    success &= modem.init();
  }
  return success;
}


void setup() {
  // Set console baud rate
  SerialMon.begin(SerialBaud);
  while (!SerialMon && millis() < 10000L) {}
  delay(10);

  SerialMon.println("Wait...");

  // !!!!!!!!!!!
  // Set your reset, enable, power pins here
  // !!!!!!!!!!!

  SerialMon.println(GF("Wait..."));
  delay(500L);

  SerialMon.println(F("Espressif firmware update tool for TinyGSM..."));
  SerialMon.println(F("The current version of TinyGSM is " TINYGSM_VERSION));
  SerialMon.print(F("The configured modem is "));
  SerialMon.println(modem.getConfiguredModem());
  SerialMon.println("=====================================");

  SerialMon.print(GF("Looking for modem at "));
  SerialMon.print(TARGET_BAUD);
  SerialMon.print(GF(" baud and setting baud rate to "));
  SerialMon.print(TARGET_BAUD);
  SerialMon.println(GF(" if it is not already the baud rate of the modem."));

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
    SerialMon.print(GF("Target baud rate "));
    SerialMon.print(targetBaud);
    SerialMon.print(GF(" is too high for this processor.  Maximum is "));
    SerialMon.println(maximum);
    targetBaud = maximum;
  }
  // Set GSM module baud rate
  uint32_t found_baud = TinyGsmAutoBaud(SerialAT, GSM_AUTOBAUD_MIN,
                                        GSM_AUTOBAUD_MAX);
  if (found_baud != 0 && found_baud != targetBaud) {
    SerialMon.print(GF("Changing baud rate from "));
    SerialMon.print(found_baud);
    SerialMon.print(GF(" to "));
    SerialMon.println(targetBaud);
    modem.setDefaultBaud(targetBaud);
    SerialAT.end();
    SerialAT.begin(targetBaud);
  } else if (found_baud == 0) {
    SerialMon.print(GF("Attempting to force baud rate to "));
    SerialMon.println(targetBaud);
    modem.forceModemBaud(SerialAT, targetBaud);
  }

  SerialMon.println(GF("Initializing modem..."));
  if (!modem.init()) {
    SerialMon.println(
        GF("Failed to initialize modem; reset the board to retry"));
    delay(10000L);
    return;
  }

  printModemInfo();

  if (!connectNetwork()) {
    SerialMon.println(
        GF("Failed to connect to network; aborting firmware update"));
    return;
  }
  checkNetworkTime();

  bool firmware_update_success = updateFirmware();
  if (firmware_update_success) {
    SerialMon.println(GF("Firmware update succeeded"));
  } else {
    SerialMon.println(GF("Firmware update failed"));
  }

  printModemInfo();

  // reconnect to the network after firmware update to verify we still can
  connectNetwork();
  checkNetworkTime();

  modem.networkDisconnect();
  SerialMon.println(GF("WiFi disconnected"));
}

void loop() {
  // Do nothing forevermore
  while (true) { modem.maintain(); }
}
