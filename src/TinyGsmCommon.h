/**
 * @file       TinyGsmCommon.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmCommon_h
#define TinyGsmCommon_h

#if defined(SPARK) || defined(PARTICLE)
  #include "Particle.h"
#elif defined(ARDUINO)
  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif
#endif

#if defined(ARDUINO_DASH)
  #include <ArduinoCompat/Client.h>
#else
  #include <Client.h>
#endif

#include <TinyGsmFifo.h>

#ifndef TINY_GSM_YIELD
  #define TINY_GSM_YIELD() { delay(0); }
#endif

#define TINY_GSM_ATTR_NOT_AVAILABLE __attribute__((error("Not available on this modem type")))
#define TINY_GSM_ATTR_NOT_IMPLEMENTED __attribute__((error("Not implemented")))

#if defined(__AVR__)
  #define TINY_GSM_PROGMEM PROGMEM
  typedef const __FlashStringHelper* GsmConstStr;
  #define GFP(x) (reinterpret_cast<GsmConstStr>(x))
  #define GF(x)  F(x)
#else
  #define TINY_GSM_PROGMEM
  typedef const char* GsmConstStr;
  #define GFP(x) x
  #define GF(x)  x
#endif

#ifdef TINY_GSM_DEBUG
namespace {
  template<typename T>
  static void DBG_PLAIN(T last) {
    TINY_GSM_DEBUG.println(last);
  }

  template<typename T, typename... Args>
  static void DBG_PLAIN(T head, Args... tail) {
    TINY_GSM_DEBUG.print(head);
    TINY_GSM_DEBUG.print(' ');
    DBG_PLAIN(tail...);
  }

  template<typename... Args>
  static void DBG(Args... args) {
    TINY_GSM_DEBUG.print(GF("["));
    TINY_GSM_DEBUG.print(millis());
    TINY_GSM_DEBUG.print(GF("] "));
    DBG_PLAIN(args...);
  }
}
#else
  #define DBG_PLAIN(...)
  #define DBG(...)
#endif

template<class T>
const T& TinyGsmMin(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

template<class T>
const T& TinyGsmMax(const T& a, const T& b)
{
    return (b < a) ? a : b;
}

template<class T>
uint32_t TinyGsmAutoBaud(T& SerialAT, uint32_t minimum = 9600, uint32_t maximum = 115200)
{
  static uint32_t rates[] = { 115200, 57600, 38400, 19200, 9600, 74400, 74880, 230400, 460800, 2400, 4800, 14400, 28800 };

  for (unsigned i = 0; i < sizeof(rates)/sizeof(rates[0]); i++) {
    uint32_t rate = rates[i];
    if (rate < minimum || rate > maximum) continue;

    DBG("Trying baud rate", rate, "...");
    SerialAT.begin(rate);
    delay(10);
    for (int i=0; i<3; i++) {
      SerialAT.print("AT\r\n");
      String input = SerialAT.readString();
      if (input.indexOf("OK") >= 0) {
        DBG("Modem responded at rate", rate);
        return rate;
      }
    }
  }
  return 0;
}

static inline
IPAddress TinyGsmIpFromString(const String& strIP) {
  int Parts[4] = {0, };
  int Part = 0;
  for (uint8_t i=0; i<strIP.length(); i++) {
    char c = strIP[i];
    if (c == '.') {
      Part++;
      if (Part > 3) {
        return IPAddress(0,0,0,0);
      }
      continue;
    } else if (c >= '0' && c <= '9') {
      Parts[Part] *= 10;
      Parts[Part] += c - '0';
    } else {
      if (Part == 3) break;
    }
  }
  return IPAddress(Parts[0], Parts[1], Parts[2], Parts[3]);
}

static inline
String TinyGsmDecodeHex7bit(String &instr) {
  String result;
  byte reminder = 0;
  int bitstate = 7;
  for (unsigned i=0; i<instr.length(); i+=2) {
    char buf[4] = { 0, };
    buf[0] = instr[i];
    buf[1] = instr[i+1];
    byte b = strtol(buf, NULL, 16);

    byte bb = b << (7 - bitstate);
    char c = (bb + reminder) & 0x7F;
    result += c;
    reminder = b >> bitstate;
    bitstate--;
    if (bitstate == 0) {
      char c = reminder;
      result += c;
      reminder = 0;
      bitstate = 7;
    }
  }
  return result;
}

static inline
String TinyGsmDecodeHex8bit(String &instr) {
  String result;
  for (unsigned i=0; i<instr.length(); i+=2) {
    char buf[4] = { 0, };
    buf[0] = instr[i];
    buf[1] = instr[i+1];
    char b = strtol(buf, NULL, 16);
    result += b;
  }
  return result;
}

static inline
String TinyGsmDecodeHex16bit(String &instr) {
  String result;
  for (unsigned i=0; i<instr.length(); i+=4) {
    char buf[4] = { 0, };
    buf[0] = instr[i];
    buf[1] = instr[i+1];
    char b = strtol(buf, NULL, 16);
    if (b) { // If high byte is non-zero, we can't handle it ;(
#if defined(TINY_GSM_UNICODE_TO_HEX)
      result += "\\x";
      result += instr.substring(i, i+4);
#else
      result += "?";
#endif
    } else {
      buf[0] = instr[i+2];
      buf[1] = instr[i+3];
      b = strtol(buf, NULL, 16);
      result += b;
    }
  }
  return result;
}




class TinyGsmModem
{

public:

class GsmClient : public Client
{
  friend class TinyGsmModem;
};


public:

#ifdef GSM_DEFAULT_STREAM
  TinyGsmModem(Stream& stream = GSM_DEFAULT_STREAM)
#else
  TinyGsmModem(Stream& stream)
#endif
    : stream(stream)
  {}

  /*
   * Basic functions
   */

  // Prepare the modem for further functionality
  virtual bool init(const char* pin = NULL) {
    DBG_PLAIN("init function not implemented");
    return false;
  }
  // Begin is redundant with init
  virtual bool begin(const char* pin = NULL) {
    return init(pin);
  }
  // Returns a string with the chip name
  virtual String getModemName() {
    DBG_PLAIN("getModemName function not implemented");
    return "";
  }
  // Sets the serial communication baud rate
  virtual void setBaud(unsigned long baud) {
    DBG_PLAIN("setBaud function not implemented");
  }
  // Checks that the modem is responding to standard AT commands
  virtual bool testAT(unsigned long timeout = 10000L) {
    DBG_PLAIN("testAT function not implemented");
    return false;
  }
  // Holds open communication with the modem waiting for data to come in
  virtual void maintain() {
    DBG_PLAIN("maintain function not implemented");
  }
  // Resets all modem chip settings to factor defaults
  virtual bool factoryDefault() {
    DBG_PLAIN("factoryDefault function not implemented");
    return false;
  }
  // Returns the response to a get info request.  The format varies by modem.
  virtual String getModemInfo() {
    DBG_PLAIN("getModemInfo function not implemented");
    return "";
  }
  // Answers whether secure communication is available on this modem
  virtual bool hasSSL() {
    DBG_PLAIN("hasSSL function not implemented");
    return false;
  }
  virtual bool hasWifi() {
    DBG_PLAIN("hasWifi function not implemented");
    return false;
  }
  virtual bool hasGPRS() {
    DBG_PLAIN("hasGPRS function not implemented");
    return false;
  }

  /*
   * Power functions
   */

  virtual bool restart() {
    DBG_PLAIN("restart function not implemented");
    return false;
  }
  virtual bool sleepEnable(bool enable = true) {
    DBG_PLAIN("sleepEnable function not implemented");
    return false;
  }

  /*
   * SIM card functions
   */

  virtual bool simUnlock(const char *pin) {
    DBG_PLAIN("simUnlock function not implemented");
    return false;
  }
  virtual String getSimCCID() {
    DBG_PLAIN("getSimCCID function not implemented");
    return "";
  }
  virtual String getIMEI() {
    DBG_PLAIN("getIMEI function not implemented");
    return "";
  }
  virtual String getOperator() {
    DBG_PLAIN("getOperator function not implemented");
    return "";
  }

 /*
  * Generic network functions
  */

  virtual int getSignalQuality() {
    DBG_PLAIN("getSignalQuality function not implemented");
    return 0;
  }
  // NOTE:  this returns whether the modem is registered on the cellular or WiFi
  // network NOT whether GPRS or other internet connections are available
  virtual bool isNetworkConnected() {
    DBG_PLAIN("isNetworkConnected function not implemented");
    return false;
  }
  virtual bool waitForNetwork(unsigned long timeout = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      if (isNetworkConnected()) {
        return true;
      }
      delay(250);
    }
    return false;
  }

  /*
   * WiFi functions
   */

  virtual bool networkConnect(const char* ssid, const char* pwd) {
    DBG_PLAIN("networkConnect function not implemented");
    return false;
  }
  virtual bool networkDisconnect() {
    DBG_PLAIN("networkDisconnect function not implemented");
    return false;
  }

  /*
   * GPRS functions
   */

  virtual bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {
    DBG_PLAIN("gprsConnect function not implemented");
    return false;
  }
  virtual bool gprsDisconnect() {
    DBG_PLAIN("gprsDisconnect function not implemented");
    return false;
  }

  /*
   * IP Address functions
   */

  virtual String getLocalIP() {
    DBG_PLAIN("getLocalIP function not implemented");
    return "";
  }
  virtual IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
  }

public:
  Stream&       stream;
};




#endif
