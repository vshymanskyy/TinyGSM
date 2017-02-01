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
#elif defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_A7)
  #include <TinyGsmClientA6.h>
#elif defined(TINY_GSM_MODEM_M590)
  #include <TinyGsmClientM590.h>
#elif defined(TINY_GSM_MODEM_ESP8266)
  #include <TinyWiFiClientESP8266.h>
#else
  #error "Please define GSM modem model"
#endif

#endif
