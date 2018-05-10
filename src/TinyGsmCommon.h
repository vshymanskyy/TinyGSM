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

#endif
