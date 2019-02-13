/**************************************************************
 *
 * TinyGSM Getting Started guide:
 *   https://tiny.cc/tinygsm-readme
 *
 * NOTE:
 * Some of the functions may be unavailable for your modem.
 * Just comment them out.
 *
 **************************************************************/

// Select your modem:
//#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM900
#define TINY_GSM_MODEM_SIM7000
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590

// Set serial for debug console (to the Serial Monitor, speed 115200)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro, ESP32
#include "HardwareSerial.h"
#define SerialAT Serial2

// or Software Serial on Uno, Nano
//#include <SoftwareSerial.h>
//SoftwareSerial SerialAT(2, 3); // RX, TX

#define MODEM_PWRKEY 18
//#define DUMP_AT_COMMANDS
#define TINY_GSM_DEBUG SerialMon

// Set phone numbers, if you want to test SMS and Calls
//#define SMS_TARGET  "+4366066033403"
//#define CALL_TARGET "+43xxxxxx"

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "drei.at";
const char user[] = "";
const char pass[] = "";

#include <TinyGsmClient.h>

#ifdef DUMP_AT_COMMANDS
  #include <StreamDebugger.h>
  StreamDebugger debugger(SerialAT, SerialMon);
  TinyGsm modem(debugger);
#else
  TinyGsm modem(SerialAT);
  TinyGsmClient client(modem);
#endif

void setup() {
  // Set pwr pin 18 (ESP32) --> shield's PWRKEY
  pinMode(MODEM_PWRKEY, OUTPUT);
  powerOn(); //function for powering on SIM7000

  // Set console baud rate
  SerialMon.begin(115200);
  delay(1000);

  SerialAT.begin(9600);
  delay(1000);
  // Set GSM module baud rate
  //TinyGsmAutoBaud(SerialAT);
}

void loop() {

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  DBG("Initializing modem...");
  if (!modem.restart()) {
    delay(15000);
    return;
  }

  //String modemInfo = modem.getModemInfo();
  //DBG("Modem:", modemInfo);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");


  // Network modes for SIM7000 (2-Automatic),(13-GSM Only),(38-LTE Only),(51-GSM And LTE Only)
  String NetworkModes = modem.getNetworkModes();
  DBG("Network Modes:", NetworkModes);

  String NetworkMode = modem.setNetworkMode(0);
  DBG("Changed Network Mode:", NetworkMode);

  // Preferred LTE mode selection (1-Cat-M),(2-NB-IoT),(3-Cat-M And NB-IoT)
  String PreferredModes = modem.getPreferredModes();
  DBG("Preferred Modes:", PreferredModes);

  String PreferredMode = modem.setPreferredMode(3);
  DBG("Changed Preferred Mode:", PreferredMode);


/*  DBG("Waiting for network...");
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }

  if (modem.isNetworkConnected()) {
    DBG("Network connected");
  }*/

/*
  DBG("Connecting to", apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    delay(10000);
    return;
  }

  bool res = modem.isGprsConnected();
  DBG("GPRS status:", res ? "connected" : "not connected");

  String ccid = modem.getSimCCID();
  DBG("CCID:", ccid);

  String imei = modem.getIMEI();
  DBG("IMEI:", imei);

  String cop = modem.getOperator();
  DBG("Operator:", cop);

  IPAddress local = modem.localIP();
  DBG("Local IP:", local);

  int csq = modem.getSignalQuality();
  DBG("Signal quality:", csq);

  // This is NOT supported on M590
  int battLevel = modem.getBattPercent();
  DBG("Battery lavel:", battLevel);

  // This is only supported on SIMxxx series
  float battVoltage = modem.getBattVoltage() / 1000.0F;
  DBG("Battery voltage:", battVoltage);

  // This is only supported on SIMxxx series
  String gsmLoc = modem.getGsmLocation();
  DBG("GSM location:", gsmLoc);

  // This is only supported on SIMxxx series
  String gsmTime = modem.getGSMDateTime(DATE_TIME);
  DBG("GSM Time:", gsmTime);
  String gsmDate = modem.getGSMDateTime(DATE_DATE);
  DBG("GSM Date:", gsmDate);


  modem.enableGPS();
  String gps_raw = modem.getGPSraw();
  modem.disableGPS();
  DBG("GPS raw data:", gps_raw);


#if defined(SMS_TARGET)
  res = modem.sendSMS(SMS_TARGET, String("Hello from ") + imei);
  DBG("SMS:", res ? "OK" : "fail");

  // This is only supported on SIMxxx series
  res = modem.sendSMS_UTF16(SMS_TARGET, u"Привіііт!", 9);
  DBG("UTF16 SMS:", res ? "OK" : "fail");
#endif

  modem.gprsDisconnect();
  if (!modem.isGprsConnected()) {
    DBG("GPRS disconnected");
  } else {
    DBG("GPRS disconnect: Failed.");
  }
*/

  // Try to power-off (modem may decide to restart automatically)
  // To turn off modem completely, please use Reset/Enable pins
  modem.poweroff();
  DBG("Poweroff.");

  // Do nothing forevermore
  while (true) {
    modem.maintain();
  }
}

void powerOn() {
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(100);
  digitalWrite(MODEM_PWRKEY, HIGH);
}
