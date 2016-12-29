/**
 * @file       TinyGsmClient.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClient_h
#define TinyGsmClient_h

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

#if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM900)
  #include <TinyGsmClientSIM800.h>
#elif defined(TINY_GSM_MODEM_M590)
  #include <TinyGsmClientM590.h>
#elif defined(TINY_GSM_MODEM_ESP8266)
  #include <TinyWiFiClientESP8266.h>
#else
  #error "Please define GSM modem model"
#endif

#endif
