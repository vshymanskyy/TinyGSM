/**
 * @file       TinyGsmClient.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClient_h
#define TinyGsmClient_h

#if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM868) || defined(TINY_GSM_MODEM_UBLOX) || defined(TINY_GSM_MODEM_ESP8266)
  #define TINY_GSM_MODEM_HAS_SSL
#endif

#if defined(TINY_GSM_MODEM_SIM808) || defined(TINY_GSM_MODEM_SIM868) || defined(TINY_GSM_MODEM_A7)
  #define TINY_GSM_MODEM_HAS_GPS
#endif

#if   defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM900)
  #define TINY_GSM_MODEM_HAS_GPRS
  #include <TinyGsmClientSIM800.h>
  typedef TinyGsmSim800 TinyGsm;
  typedef TinyGsmSim800::GsmClient TinyGsmClient;
  typedef TinyGsmSim800::GsmClientSecure TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_SIM808) || defined(TINY_GSM_MODEM_SIM868)
  #define TINY_GSM_MODEM_HAS_GPRS
  #include <TinyGsmClientSIM808.h>
  typedef TinyGsmSim808 TinyGsm;
  typedef TinyGsmSim808::GsmClient TinyGsmClient;
  typedef TinyGsmSim808::GsmClientSecure TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_UBLOX)
  #define TINY_GSM_MODEM_HAS_GPRS
  #include <TinyGsmClientUBLOX.h>
  typedef TinyGsmUBLOX TinyGsm;
  typedef TinyGsmUBLOX::GsmClient TinyGsmClient;
  typedef TinyGsmUBLOX::GsmClientSecure TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_BG96)
  #define TINY_GSM_MODEM_HAS_GPRS
  #include <TinyGsmClientBG96.h>
  typedef TinyGsmBG96 TinyGsm;
  typedef TinyGsmBG96::GsmClient TinyGsmClient;

#elif defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_A7)
  #define TINY_GSM_MODEM_HAS_GPRS
  #include <TinyGsmClientA6.h>
  typedef TinyGsmA6 TinyGsm;
  typedef TinyGsmA6::GsmClient TinyGsmClient;

#elif defined(TINY_GSM_MODEM_M590)
  #define TINY_GSM_MODEM_HAS_GPRS
  #include <TinyGsmClientM590.h>
  typedef TinyGsmM590 TinyGsm;
  typedef TinyGsmM590::GsmClient TinyGsmClient;

#elif defined(TINY_GSM_MODEM_ESP8266)
  #define TINY_GSM_MODEM_HAS_WIFI
  #include <TinyGsmClientESP8266.h>
  typedef TinyGsmESP8266 TinyGsm;
  typedef TinyGsmESP8266::GsmClient TinyGsmClient;
  typedef TinyGsmESP8266::GsmClientSecure TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_XBEE)
  #define TINY_GSM_MODEM_HAS_GPRS
  #define TINY_GSM_MODEM_HAS_WIFI
  #include <TinyGsmClientXBee.h>
  typedef TinyGsmXBee TinyGsm;
  typedef TinyGsmXBee::GsmClient TinyGsmClient;
  typedef TinyGsmXBee::GsmClientSecure TinyGsmClientSecure;

#else
  #error "Please define GSM modem model"
#endif

#endif
