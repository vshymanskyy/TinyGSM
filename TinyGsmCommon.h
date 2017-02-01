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

#include <Client.h>
#include <TinyGsmFifo.h>

#ifndef TINY_GSM_YIELD
  #define TINY_GSM_YIELD() { delay(0); }
#endif

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
  static void DBG(T last) {
    TINY_GSM_DEBUG.println(last);
  }

  template<typename T, typename... Args>
  static void DBG(T head, Args... tail) {
    TINY_GSM_DEBUG.print(head);
    TINY_GSM_DEBUG.print(' ');
    DBG(tail...);
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

#endif
