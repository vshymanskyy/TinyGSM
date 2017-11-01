/**
 * @file       TinyGsmClient.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClient_h
#define TinyGsmClient_h

#if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM868) || defined(TINY_GSM_MODEM_U201) || defined(TINY_GSM_MODEM_ESP8266)
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

#elif defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_A7)
  #define TINY_GSM_MODEM_HAS_GPRS
  #include <TinyGsmClientA6.h>
  typedef TinyGsm::GsmClient TinyGsmClient;

#elif defined(TINY_GSM_MODEM_M590)
  #define TINY_GSM_MODEM_HAS_GPRS
  #include <TinyGsmClientM590.h>
  typedef TinyGsm::GsmClient TinyGsmClient;

#elif defined(TINY_GSM_MODEM_U201)
  #define TINY_GSM_MODEM_HAS_GPRS
  #include <TinyGsmClientU201.h>
  typedef TinyGsmU201 TinyGsm;
  typedef TinyGsmU201::GsmClient TinyGsmClient;
  typedef TinyGsmU201::GsmClientSecure TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_ESP8266)
  #define TINY_GSM_MODEM_HAS_WIFI
  #include <TinyGsmClientESP8266.h>
  typedef TinyGsm::GsmClient TinyGsmClient;
  typedef TinyGsm::GsmClientSecure TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_XBEE)
  #define TINY_GSM_MODEM_HAS_GPRS
  #define TINY_GSM_MODEM_HAS_WIFI
  #include <TinyGsmClientXBee.h>
  typedef TinyGsm::GsmClient TinyGsmClient;

#else
  #error "Please define GSM modem model"
#endif

#endif
