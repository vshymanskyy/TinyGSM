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

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 256
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

  TinyGsmModem(Stream& stream)
    : stream(stream)
  {}

  /*
   * Basic functions
   */

  // Prepare the modem for further functionality
  virtual bool init(const char* pin = NULL) = 0;
  // Begin is redundant with init
  virtual bool begin(const char* pin = NULL) {
    return init(pin);
  }
  // Returns a string with the chip name
  virtual String getModemName() = 0;
  // Sets the serial communication baud rate
  virtual void setBaud(unsigned long baud) = 0;
  // Checks that the modem is responding to standard AT commands
  virtual bool testAT(unsigned long timeout = 10000L) = 0;
  // Holds open communication with the modem waiting for data to come in
  virtual void maintain() = 0;
  // Resets all modem chip settings to factor defaults
  virtual bool factoryDefault() = 0;
  // Returns the response to a get info request.  The format varies by modem.
  virtual String getModemInfo() = 0;
  // Answers whether types of communication are available on this modem
  virtual bool hasSSL() = 0;
  virtual bool hasWifi() = 0;
  virtual bool hasGPRS() = 0;

  /*
   * Power functions
   */

  virtual bool restart() = 0;
  virtual bool poweroff() = 0;

  /*
   * SIM card functions - only apply to cellular modems
   */

  virtual bool simUnlock(const char *pin) { return false; }
  virtual String getSimCCID() { return ""; }
  virtual String getIMEI() { return ""; }
  virtual String getOperator() { return ""; }

 /*
  * Generic network functions
  */

  virtual int16_t getSignalQuality() = 0;
  // NOTE:  this returns whether the modem is registered on the cellular or WiFi
  // network NOT whether GPRS or other internet connections are available
  virtual bool isNetworkConnected() = 0;
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
   * WiFi functions - only apply to WiFi modems
   */

  virtual bool networkConnect(const char* ssid, const char* pwd) { return false; }
  virtual bool networkDisconnect() { return false; }

  /*
   * GPRS functions - only apply to cellular modems
   */

  virtual bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {
    return false;
  }
  virtual bool gprsDisconnect() { return false; }
  virtual bool isGprsConnected() { return false; }

  /*
   * IP Address functions
   */

  virtual String getLocalIP() = 0;
  virtual IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
  }

    /*
     Utilities
     */

    template<typename T>
    void streamWrite(T last) {
      stream.print(last);
    }

    template<typename T, typename... Args>
    void streamWrite(T head, Args... tail) {
      stream.print(head);
      streamWrite(tail...);
    }

    bool streamSkipUntil(const char c, const unsigned long timeout = 1000L) {
      unsigned long startMillis = millis();
      while (millis() - startMillis < timeout) {
        while (millis() - startMillis < timeout && !stream.available()) {
          TINY_GSM_YIELD();
        }
        if (stream.read() == c)
          return true;
      }
      return false;
    }

public:
  Stream&       stream;
};




#endif
