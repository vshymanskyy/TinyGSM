/**
 * @file       TinyGsmClient.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClient_h
#define TinyGsmClient_h

#if   defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM900)
  #include <TinyGsmClientSIM800.h>
  typedef TinyGsmSim800 TinyGsm;
  typedef TinyGsmSim800::GsmClient TinyGsmClient;
  typedef TinyGsmSim800::GsmClientSecure TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_SIM808) || defined(TINY_GSM_MODEM_SIM868)
  #include <TinyGsmClientSIM808.h>
  typedef TinyGsmSim808 TinyGsm;
  typedef TinyGsmSim808::GsmClient TinyGsmClient;
  typedef TinyGsmSim808::GsmClientSecure TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_A7)
  #include <TinyGsmClientA6.h>
  typedef TinyGsm::GsmClient TinyGsmClient;

#elif defined(TINY_GSM_MODEM_M590)
  #include <TinyGsmClientM590.h>
  typedef TinyGsm::GsmClient TinyGsmClient;

#elif defined(TINY_GSM_MODEM_ESP8266)
  #include <TinyGsmClientESP8266.h>
  typedef TinyGsm::GsmClient TinyGsmClient;

#else
  #error "Please define GSM modem model"
#endif

#endif
