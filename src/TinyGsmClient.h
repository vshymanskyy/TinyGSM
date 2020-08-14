/**
 * @file       TinyGsmClient.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENT_H_
#define SRC_TINYGSMCLIENT_H_

#if defined(TINY_GSM_MODEM_SIM800)
#include "TinyGsmClientSIM800.h"
typedef TinyGsmSim800                        TinyGsm;
typedef TinyGsmSim800::GsmClientSim800       TinyGsmClient;
typedef TinyGsmSim800::GsmClientSecureSim800 TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_SIM808) || defined(TINY_GSM_MODEM_SIM868)
#include "TinyGsmClientSIM808.h"
typedef TinyGsmSim808                        TinyGsm;
typedef TinyGsmSim808::GsmClientSim800       TinyGsmClient;
typedef TinyGsmSim808::GsmClientSecureSim800 TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_SIM900)
#include "TinyGsmClientSIM800.h"
typedef TinyGsmSim800                  TinyGsm;
typedef TinyGsmSim800::GsmClientSim800 TinyGsmClient;

#elif defined(TINY_GSM_MODEM_SIM7000)
#include "TinyGsmClientSIM7000.h"
typedef TinyGsmSim7000                   TinyGsm;
typedef TinyGsmSim7000::GsmClientSim7000 TinyGsmClient;
// typedef TinyGsmSim7000::GsmClientSecureSim7000 TinyGsmClientSecure; TODO!

#elif defined(TINY_GSM_MODEM_SIM5320) || defined(TINY_GSM_MODEM_SIM5360) || \
    defined(TINY_GSM_MODEM_SIM5300) || defined(TINY_GSM_MODEM_SIM7100)
#include "TinyGsmClientSIM5360.h"
typedef TinyGsmSim5360                   TinyGsm;
typedef TinyGsmSim5360::GsmClientSim5360 TinyGsmClient;

#elif defined(TINY_GSM_MODEM_SIM7600) || defined(TINY_GSM_MODEM_SIM7800) || \
    defined(TINY_GSM_MODEM_SIM7500)
#include "TinyGsmClientSIM7600.h"
typedef TinyGsmSim7600                   TinyGsm;
typedef TinyGsmSim7600::GsmClientSim7600 TinyGsmClient;

#elif defined(TINY_GSM_MODEM_UBLOX)
#include "TinyGsmClientUBLOX.h"
typedef TinyGsmUBLOX                       TinyGsm;
typedef TinyGsmUBLOX::GsmClientUBLOX       TinyGsmClient;
typedef TinyGsmUBLOX::GsmClientSecureUBLOX TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_SARAR4)
#include "TinyGsmClientSaraR4.h"
typedef TinyGsmSaraR4                    TinyGsm;
typedef TinyGsmSaraR4::GsmClientSaraR4   TinyGsmClient;
typedef TinyGsmSaraR4::GsmClientSecureR4 TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_M95)
#include "TinyGsmClientM95.h"
typedef TinyGsmM95               TinyGsm;
typedef TinyGsmM95::GsmClientM95 TinyGsmClient;

#elif defined(TINY_GSM_MODEM_BG96)
#include "TinyGsmClientBG96.h"
typedef TinyGsmBG96                TinyGsm;
typedef TinyGsmBG96::GsmClientBG96 TinyGsmClient;

#elif defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_A7)
#include "TinyGsmClientA6.h"
typedef TinyGsmA6              TinyGsm;
typedef TinyGsmA6::GsmClientA6 TinyGsmClient;

#elif defined(TINY_GSM_MODEM_M590)
#include "TinyGsmClientM590.h"
typedef TinyGsmM590                TinyGsm;
typedef TinyGsmM590::GsmClientM590 TinyGsmClient;

#elif defined(TINY_GSM_MODEM_MC60) || defined(TINY_GSM_MODEM_MC60E)
#include "TinyGsmClientMC60.h"
typedef TinyGsmMC60                TinyGsm;
typedef TinyGsmMC60::GsmClientMC60 TinyGsmClient;

#elif defined(TINY_GSM_MODEM_ESP8266)
#define TINY_GSM_MODEM_HAS_WIFI
#include "TinyGsmClientESP8266.h"
typedef TinyGsmESP8266                         TinyGsm;
typedef TinyGsmESP8266::GsmClientESP8266       TinyGsmClient;
typedef TinyGsmESP8266::GsmClientSecureESP8266 TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_XBEE)
#define TINY_GSM_MODEM_HAS_WIFI
#include "TinyGsmClientXBee.h"
typedef TinyGsmXBee                      TinyGsm;
typedef TinyGsmXBee::GsmClientXBee       TinyGsmClient;
typedef TinyGsmXBee::GsmClientSecureXBee TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_SEQUANS_MONARCH)
#include "TinyGsmClientSequansMonarch.h"
typedef TinyGsmSequansMonarch                          TinyGsm;
typedef TinyGsmSequansMonarch::GsmClientSequansMonarch TinyGsmClient;
typedef TinyGsmSequansMonarch::GsmClientSecureSequansMonarch
    TinyGsmClientSecure;

#else
#error "Please define GSM modem model"
#endif

#endif  // SRC_TINYGSMCLIENT_H_
