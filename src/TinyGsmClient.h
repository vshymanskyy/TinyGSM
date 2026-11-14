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
 * multi-modem sketches should include modem-specific headers from the
 * `modems` subfolder and use scoped aliases such as
 * `TinyGsmSaraR4::TinyGsmClient`.
 *
 * @note The RegStatus type is defined in the global namespace as an alias for
 * the modem-specific registration status type (e.g., Sim800RegStatus,
 * Sim70xxRegStatus). If a naming conflict occurs, use the modem-scoped
 * alternative directly (e.g., Sim800RegStatus).
 */

#ifndef SRC_TINYGSMCLIENT_H_
#define SRC_TINYGSMCLIENT_H_

#if defined(TINY_GSM_MODEM_SIM800)
#include "modems/TinyGsmClientSIM800.h"
using TinyGsm             = TinyGsmSim800;
using TinyGsmClient       = TinyGsmSim800::TinyGsmClient;
using TinyGsmClientSecure = TinyGsmSim800::TinyGsmClientSecure;
// using TinyGsmSecureContext = TinyGsmSim800::GsmSecureContextSim800;
using RegStatus = Sim800RegStatus;

#elif defined(TINY_GSM_MODEM_SIM808) || defined(TINY_GSM_MODEM_SIM868)
#include "modems/TinyGsmClientSIM808.h"
using TinyGsm             = TinyGsmSim808;
using TinyGsmClient       = TinyGsmSim808::TinyGsmClient;
using TinyGsmClientSecure = TinyGsmSim808::TinyGsmClientSecure;
// using TinyGsmSecureContext = TinyGsmSim808::GsmSecureContextSim808;
using RegStatus = Sim800RegStatus;

#elif defined(TINY_GSM_MODEM_SIM900)
#include "modems/TinyGsmClientSIM800.h"
using TinyGsm       = TinyGsmSim800;
using TinyGsmClient = TinyGsmSim800::TinyGsmClient;
using RegStatus     = Sim800RegStatus;

#elif defined(TINY_GSM_MODEM_SIM7000)
#include "modems/TinyGsmClientSIM7000.h"
using TinyGsm       = TinyGsmSim7000;
using TinyGsmClient = TinyGsmSim7000::TinyGsmClient;
using RegStatus     = Sim70xxRegStatus;

#elif defined(TINY_GSM_MODEM_SIM7000SSL)
#include "modems/TinyGsmClientSIM7000SSL.h"
using TinyGsm              = TinyGsmSim7000SSL;
using TinyGsmClient        = TinyGsmSim7000SSL::TinyGsmClient;
using TinyGsmClientSecure  = TinyGsmSim7000SSL::TinyGsmClientSecure;
using TinyGsmSecureContext = TinyGsmSim7000SSL::TinyGsmSecureContext;
using RegStatus            = Sim70xxRegStatus;

#elif defined(TINY_GSM_MODEM_SIM7070) || defined(TINY_GSM_MODEM_SIM7080) || \
    defined(TINY_GSM_MODEM_SIM7090)
#include "modems/TinyGsmClientSIM7080.h"
using TinyGsm              = TinyGsmSim7080;
using TinyGsmClient        = TinyGsmSim7080::TinyGsmClient;
using TinyGsmClientSecure  = TinyGsmSim7080::TinyGsmClientSecure;
using TinyGsmSecureContext = TinyGsmSim7080::TinyGsmSecureContext;
using RegStatus            = Sim70xxRegStatus;

#elif defined(TINY_GSM_MODEM_SIM5320) || defined(TINY_GSM_MODEM_SIM5360) || \
    defined(TINY_GSM_MODEM_SIM5300) || defined(TINY_GSM_MODEM_SIM7100)
#include "modems/TinyGsmClientSIM5360.h"
using TinyGsm       = TinyGsmSim5360;
using TinyGsmClient = TinyGsmSim5360::TinyGsmClient;
using RegStatus     = Sim5360RegStatus;

#elif defined(TINY_GSM_MODEM_SIM7600) || defined(TINY_GSM_MODEM_SIM7800) || \
    defined(TINY_GSM_MODEM_SIM7500)
#include "modems/TinyGsmClientSIM7600.h"
using TinyGsm              = TinyGsmSim7600;
using TinyGsmClient        = TinyGsmSim7600::TinyGsmClient;
using TinyGsmClientSecure  = TinyGsmSim7600::TinyGsmClientSecure;
using TinyGsmSecureContext = TinyGsmSim7600::TinyGsmSecureContext;
using RegStatus            = Sim7600RegStatus;

#elif defined(TINY_GSM_MODEM_UBLOX)
#include "modems/TinyGsmClientUBLOX.h"
using TinyGsm             = TinyGsmUBLOX;
using TinyGsmClient       = TinyGsmUBLOX::TinyGsmClient;
using TinyGsmClientSecure = TinyGsmUBLOX::TinyGsmClientSecure;
// using TinyGsmSecureContext = TinyGsmUBLOX::GsmSecureContextUBLOX;
using RegStatus = UBLOXRegStatus;

#elif defined(TINY_GSM_MODEM_SARAR4)
#include "modems/TinyGsmClientSaraR4.h"
using TinyGsm             = TinyGsmSaraR4;
using TinyGsmClient       = TinyGsmSaraR4::TinyGsmClient;
using TinyGsmClientSecure = TinyGsmSaraR4::TinyGsmClientSecure;
// using TinyGsmSecureContext = TinyGsmSaraR4::GsmSecureContextSaraR4;
using RegStatus = SaraR4RegStatus;

#elif defined(TINY_GSM_MODEM_SARAR5)
#include "modems/TinyGsmClientSaraR5.h"
using TinyGsm             = TinyGsmSaraR5;
using TinyGsmClient       = TinyGsmSaraR5::TinyGsmClient;
using TinyGsmClientSecure = TinyGsmSaraR5::TinyGsmClientSecure;
// using TinyGsmSecureContext = TinyGsmSaraR5::GsmSecureContextSaraR5;
using RegStatus = SaraR5RegStatus;

#elif defined(TINY_GSM_MODEM_M95)
#include "modems/TinyGsmClientM95.h"
using TinyGsm       = TinyGsmM95;
using TinyGsmClient = TinyGsmM95::TinyGsmClient;
using RegStatus     = M95RegStatus;

#elif defined(TINY_GSM_MODEM_BG96) || defined(TINY_GSM_MODEM_BG95) || \
    defined(TINY_GSM_MODEM_BG95SSL)
#include "modems/TinyGsmClientBG96.h"
using TinyGsm              = TinyGsmBG96;
using TinyGsmClient        = TinyGsmBG96::TinyGsmClient;
using TinyGsmClientSecure  = TinyGsmBG96::TinyGsmClientSecure;
using TinyGsmSecureContext = TinyGsmBG96::TinyGsmSecureContext;
using RegStatus            = BG96RegStatus;

#elif defined(TINY_GSM_MODEM_BC660)
#include "TinyGsmClientBC660.h"
typedef TinyGsmBC660                        TinyGsm;
typedef TinyGsmBC660::GsmClientBC660        TinyGsmClient;
typedef TinyGsmBC660::GsmClientSecureBC660  TinyGsmClientSecure;

#elif defined(TINY_GSM_MODEM_A6) || defined(TINY_GSM_MODEM_A7)
#include "modems/TinyGsmClientA6.h"
using TinyGsm       = TinyGsmA6;
using TinyGsmClient = TinyGsmA6::TinyGsmClient;
using RegStatus     = A6RegStatus;

#elif defined(TINY_GSM_MODEM_M590)
#include "modems/TinyGsmClientM590.h"
using TinyGsm       = TinyGsmM590;
using TinyGsmClient = TinyGsmM590::TinyGsmClient;
using RegStatus     = M590RegStatus;

#elif defined(TINY_GSM_MODEM_MC60) || defined(TINY_GSM_MODEM_MC60E)
#include "modems/TinyGsmClientMC60.h"
using TinyGsm       = TinyGsmMC60;
using TinyGsmClient = TinyGsmMC60::TinyGsmClient;
using RegStatus     = MC60RegStatus;

#elif defined(TINY_GSM_MODEM_ESP32)
#include "modems/TinyGsmClientESP32.h"
using TinyGsm              = TinyGsmESP32;
using TinyGsmClient        = TinyGsmESP32::TinyGsmClient;
using TinyGsmClientSecure  = TinyGsmESP32::TinyGsmClientSecure;
using TinyGsmSecureContext = TinyGsmESP32::TinyGsmSecureContext;
using RegStatus            = ESP32RegStatus;

#elif defined(TINY_GSM_MODEM_ESP8266)
#include "modems/TinyGsmClientESP8266.h"
using TinyGsm              = TinyGsmESP8266;
using TinyGsmClient        = TinyGsmESP8266::TinyGsmClient;
using TinyGsmClientSecure  = TinyGsmESP8266::TinyGsmClientSecure;
using TinyGsmSecureContext = TinyGsmESP8266::TinyGsmSecureContext;
using RegStatus            = ESP8266RegStatus;

#elif defined(TINY_GSM_MODEM_ESP8266_NONOS)
#include "modems/TinyGsmClientESP8266NonOS.h"
using TinyGsm             = TinyGsmESP8266NonOS;
using TinyGsmClient       = TinyGsmESP8266NonOS::TinyGsmClient;
using TinyGsmClientSecure = TinyGsmESP8266NonOS::TinyGsmClientSecure;
// using TinyGsmSecureContext =
//     TinyGsmESP8266NonOS::GsmSecureContextESP8266NonOS;
using RegStatus = ESP8266NonOSRegStatus;

#elif defined(TINY_GSM_MODEM_XBEE)
#include "modems/TinyGsmClientXBee.h"
using TinyGsm              = TinyGsmXBee;
using TinyGsmClient        = TinyGsmXBee::TinyGsmClient;
using TinyGsmClientSecure  = TinyGsmXBee::TinyGsmClientSecure;
using TinyGsmSecureContext = TinyGsmXBee::TinyGsmSecureContext;
using RegStatus            = XBeeRegStatus;

#elif defined(TINY_GSM_MODEM_SEQUANS_MONARCH)
#include "modems/TinyGsmClientSequansMonarch.h"
using TinyGsm             = TinyGsmSequansMonarch;
using TinyGsmClient       = TinyGsmSequansMonarch::TinyGsmClient;
using TinyGsmClientSecure = TinyGsmSequansMonarch::TinyGsmClientSecure;
// using TinyGsmSecureContext =
//     TinyGsmSequansMonarch::GsmSecureContextSequansMonarch;
using RegStatus = MonarchRegStatus;

#elif defined(TINY_GSM_MODEM_A7672X)
#include "modems/TinyGsmClientA7672x.h"
using TinyGsm              = TinyGsmA7672X;
using TinyGsmClient        = TinyGsmA7672X::TinyGsmClient;
using TinyGsmClientSecure  = TinyGsmA7672X::TinyGsmClientSecure;
using TinyGsmSecureContext = TinyGsmA7672X::TinyGsmSecureContext;
using RegStatus            = A7672XRegStatus;

#else
#error "Please define GSM modem model"
#endif

#endif  // SRC_TINYGSMCLIENT_H_
