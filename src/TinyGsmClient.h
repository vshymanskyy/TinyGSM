/**
 * @file       TinyGsmClient.h
 * @brief      Primary TinyGSM include that selects modem-specific client
 * aliases.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 *
 * For simple single-modem sketches, include this file to get the global
 * `TinyGsm`, `TinyGsmClient`, `TinyGsmClientSecure`, and
 * `TinyGsmSecureContext` aliases supported by the selected modem. Libraries and
 * multi-modem sketches should include modem-specific headers and use scoped
 * aliases such as `TinyGsmSaraR4::TinyGsmClient`.
 *
 * @note The RegStatus type is defined in the global namespace as a typedef to
 * the modem-specific registration status type (e.g., Sim800RegStatus,
 * Sim70xxRegStatus). If a naming conflict occurs, use the modem-scoped
 * alternative directly (e.g., TinyGsmSim800::Sim800RegStatus).
 */

#ifndef SRC_TINYGSMCLIENT_H_
#define SRC_TINYGSMCLIENT_H_

#if defined(TINY_GSM_MODEM_SIM800)
#include "TinyGsmClientSIM800.h"
typedef TinyGsmSim800                      TinyGsm;
typedef TinyGsmSim800::TinyGsmClient       TinyGsmClient;
typedef TinyGsmSim800::TinyGsmClientSecure TinyGsmClientSecure;
// typedef TinyGsmSim800::GsmSecureContextSim800 TinyGsmSecureContext;
typedef Sim800RegStatus RegStatus;

#elif defined(TINY_GSM_MODEM_SIM808) || defined(TINY_GSM_MODEM_SIM868)
#include "TinyGsmClientSIM808.h"
typedef TinyGsmSim808                      TinyGsm;
typedef TinyGsmSim808::TinyGsmClient       TinyGsmClient;
typedef TinyGsmSim808::TinyGsmClientSecure TinyGsmClientSecure;
// typedef TinyGsmSim808::GsmSecureContextSim808 TinyGsmSecureContext;
typedef Sim800RegStatus RegStatus;

#elif defined(TINY_GSM_MODEM_SIM900)
#include "TinyGsmClientSIM800.h"
typedef TinyGsmSim800                TinyGsm;
typedef TinyGsmSim800::TinyGsmClient TinyGsmClient;
typedef Sim800RegStatus              RegStatus;

#elif defined(TINY_GSM_MODEM_SIM7000)
#include "TinyGsmClientSIM7000.h"
typedef TinyGsmSim7000                TinyGsm;
typedef TinyGsmSim7000::TinyGsmClient TinyGsmClient;
typedef Sim70xxRegStatus              RegStatus;

#elif defined(TINY_GSM_MODEM_SIM7000SSL)
#include "TinyGsmClientSIM7000SSL.h"
typedef TinyGsmSim7000SSL                       TinyGsm;
typedef TinyGsmSim7000SSL::TinyGsmClient        TinyGsmClient;
typedef TinyGsmSim7000SSL::TinyGsmClientSecure  TinyGsmClientSecure;
typedef TinyGsmSim7000SSL::TinyGsmSecureContext TinyGsmSecureContext;
typedef Sim70xxRegStatus                        RegStatus;

#elif defined(TINY_GSM_MODEM_SIM7070) || defined(TINY_GSM_MODEM_SIM7080) || \
    defined(TINY_GSM_MODEM_SIM7090)
#include "TinyGsmClientSIM7080.h"
typedef TinyGsmSim7080                       TinyGsm;
typedef TinyGsmSim7080::TinyGsmClient        TinyGsmClient;
typedef TinyGsmSim7080::TinyGsmClientSecure  TinyGsmClientSecure;
typedef TinyGsmSim7080::TinyGsmSecureContext TinyGsmSecureContext;
typedef Sim70xxRegStatus                     RegStatus;

#elif defined(TINY_GSM_MODEM_SIM5320) || defined(TINY_GSM_MODEM_SIM5360) || \
    defined(TINY_GSM_MODEM_SIM5300) || defined(TINY_GSM_MODEM_SIM7100)
#include "TinyGsmClientSIM5360.h"
typedef TinyGsmSim5360                TinyGsm;
typedef TinyGsmSim5360::TinyGsmClient TinyGsmClient;
typedef Sim5360RegStatus              RegStatus;

#elif defined(TINY_GSM_MODEM_SIM7600) || defined(TINY_GSM_MODEM_SIM7800) || \
    defined(TINY_GSM_MODEM_SIM7500)
#include "TinyGsmClientSIM7600.h"
typedef TinyGsmSim7600                       TinyGsm;
typedef TinyGsmSim7600::TinyGsmClient        TinyGsmClient;
typedef TinyGsmSim7600::TinyGsmClientSecure  TinyGsmClientSecure;
typedef TinyGsmSim7600::TinyGsmSecureContext TinyGsmSecureContext;
typedef Sim7600RegStatus                     RegStatus;

#elif defined(TINY_GSM_MODEM_UBLOX)
#include "TinyGsmClientUBLOX.h"
typedef TinyGsmUBLOX                      TinyGsm;
typedef TinyGsmUBLOX::TinyGsmClient       TinyGsmClient;
typedef TinyGsmUBLOX::TinyGsmClientSecure TinyGsmClientSecure;
// typedef TinyGsmUBLOX::GsmSecureContextUBLOX TinyGsmSecureContext;
typedef UBLOXRegStatus RegStatus;

#elif defined(TINY_GSM_MODEM_SARAR4)
#include "TinyGsmClientSaraR4.h"
typedef TinyGsmSaraR4                      TinyGsm;
typedef TinyGsmSaraR4::TinyGsmClient       TinyGsmClient;
typedef TinyGsmSaraR4::TinyGsmClientSecure TinyGsmClientSecure;
// typedef TinyGsmSaraR4::GsmSecureContextSaraR4 TinyGsmSecureContext;
typedef SaraR4RegStatus RegStatus;

#elif defined(TINY_GSM_MODEM_SARAR5)
#include "TinyGsmClientSaraR5.h"
typedef TinyGsmSaraR5                      TinyGsm;
typedef TinyGsmSaraR5::TinyGsmClient       TinyGsmClient;
typedef TinyGsmSaraR5::TinyGsmClientSecure TinyGsmClientSecure;
// typedef TinyGsmSaraR5::GsmSecureContextSaraR5 TinyGsmSecureContext;
typedef SaraR5RegStatus RegStatus;

#elif defined(TINY_GSM_MODEM_M95)
#include "TinyGsmClientM95.h"
typedef TinyGsmM95                TinyGsm;
typedef TinyGsmM95::TinyGsmClient TinyGsmClient;
typedef M95RegStatus              RegStatus;

#elif defined(TINY_GSM_MODEM_BG96) || defined(TINY_GSM_MODEM_BG95) || \
    defined(TINY_GSM_MODEM_BG95SSL)
#include "TinyGsmClientBG96.h"
typedef TinyGsmBG96                       TinyGsm;
typedef TinyGsmBG96::TinyGsmClient        TinyGsmClient;
typedef TinyGsmBG96::TinyGsmClientSecure  TinyGsmClientSecure;
typedef TinyGsmBG96::TinyGsmSecureContext TinyGsmSecureContext;
typedef BG96RegStatus                     RegStatus;

#elif defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_A7)
#include "TinyGsmClientA6.h"
typedef TinyGsmA6                TinyGsm;
typedef TinyGsmA6::TinyGsmClient TinyGsmClient;
typedef A6RegStatus              RegStatus;

#elif defined(TINY_GSM_MODEM_M590)
#include "TinyGsmClientM590.h"
typedef TinyGsmM590                TinyGsm;
typedef TinyGsmM590::TinyGsmClient TinyGsmClient;
typedef M590RegStatus              RegStatus;

#elif defined(TINY_GSM_MODEM_MC60) || defined(TINY_GSM_MODEM_MC60E)
#include "TinyGsmClientMC60.h"
typedef TinyGsmMC60                TinyGsm;
typedef TinyGsmMC60::TinyGsmClient TinyGsmClient;
typedef MC60RegStatus              RegStatus;

#elif defined(TINY_GSM_MODEM_ESP32)
#include "TinyGsmClientESP32.h"
typedef TinyGsmESP32                       TinyGsm;
typedef TinyGsmESP32::TinyGsmClient        TinyGsmClient;
typedef TinyGsmESP32::TinyGsmClientSecure  TinyGsmClientSecure;
typedef TinyGsmESP32::TinyGsmSecureContext TinyGsmSecureContext;
typedef ESP32RegStatus                     RegStatus;

#elif defined(TINY_GSM_MODEM_ESP8266)
#include "TinyGsmClientESP8266.h"
typedef TinyGsmESP8266                       TinyGsm;
typedef TinyGsmESP8266::TinyGsmClient        TinyGsmClient;
typedef TinyGsmESP8266::TinyGsmClientSecure  TinyGsmClientSecure;
typedef TinyGsmESP8266::TinyGsmSecureContext TinyGsmSecureContext;
typedef ESP8266RegStatus                     RegStatus;

#elif defined(TINY_GSM_MODEM_ESP8266_NONOS)
#include "TinyGsmClientESP8266NonOS.h"
typedef TinyGsmESP8266NonOS                      TinyGsm;
typedef TinyGsmESP8266NonOS::TinyGsmClient       TinyGsmClient;
typedef TinyGsmESP8266NonOS::TinyGsmClientSecure TinyGsmClientSecure;
// typedef TinyGsmESP8266NonOS::GsmSecureContextESP8266NonOS
// TinyGsmSecureContext;
typedef ESP8266NonOSRegStatus RegStatus;

#elif defined(TINY_GSM_MODEM_XBEE)
#include "TinyGsmClientXBee.h"
typedef TinyGsmXBee                       TinyGsm;
typedef TinyGsmXBee::TinyGsmClient        TinyGsmClient;
typedef TinyGsmXBee::TinyGsmClientSecure  TinyGsmClientSecure;
typedef TinyGsmXBee::TinyGsmSecureContext TinyGsmSecureContext;
typedef XBeeRegStatus                     RegStatus;

#elif defined(TINY_GSM_MODEM_SEQUANS_MONARCH)
#include "TinyGsmClientSequansMonarch.h"
typedef TinyGsmSequansMonarch                      TinyGsm;
typedef TinyGsmSequansMonarch::TinyGsmClient       TinyGsmClient;
typedef TinyGsmSequansMonarch::TinyGsmClientSecure TinyGsmClientSecure;
// typedef TinyGsmSequansMonarch::GsmSecureContextSequansMonarch
//                          TinyGsmSecureContext;
typedef MonarchRegStatus RegStatus;

#elif defined(TINY_GSM_MODEM_A7672X)
#include "TinyGsmClientA7672x.h"
typedef TinyGsmA7672X                       TinyGsm;
typedef TinyGsmA7672X::TinyGsmClient        TinyGsmClient;
typedef TinyGsmA7672X::TinyGsmClientSecure  TinyGsmClientSecure;
typedef TinyGsmA7672X::TinyGsmSecureContext TinyGsmSecureContext;
typedef A7672XRegStatus                     RegStatus;

#else
#error "Please define GSM modem model"
#endif

#endif  // SRC_TINYGSMCLIENT_H_
