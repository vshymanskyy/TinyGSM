/**
 * @file TinyGSM.h
 * @brief Main TinyGSM include, which includes the modem-specific client header
 * file.
 */

#ifndef TINYGSM_H
#define TINYGSM_H

#if defined(TINY_GSM_MODEM_SIM800) || defined(TINY_GSM_MODEM_SIM808) ||      \
    defined(TINY_GSM_MODEM_SIM868) || defined(TINY_GSM_MODEM_SIM900) ||      \
    defined(TINY_GSM_MODEM_SIM7000) || defined(TINY_GSM_MODEM_SIM7000SSL) || \
    defined(TINY_GSM_MODEM_SIM7070) || defined(TINY_GSM_MODEM_SIM7080) ||    \
    defined(TINY_GSM_MODEM_SIM7090) || defined(TINY_GSM_MODEM_SIM5320) ||    \
    defined(TINY_GSM_MODEM_SIM5360) || defined(TINY_GSM_MODEM_SIM5300) ||    \
    defined(TINY_GSM_MODEM_SIM7100) || defined(TINY_GSM_MODEM_SIM7600) ||    \
    defined(TINY_GSM_MODEM_SIM7800) || defined(TINY_GSM_MODEM_SIM7500) ||    \
    defined(TINY_GSM_MODEM_UBLOX) || defined(TINY_GSM_MODEM_SARAR4) ||       \
    defined(TINY_GSM_MODEM_SARAR5) || defined(TINY_GSM_MODEM_M95) ||         \
    defined(TINY_GSM_MODEM_BG96) || defined(TINY_GSM_MODEM_BG95) ||          \
    defined(TINY_GSM_MODEM_BG95SSL) || defined(TINY_GSM_MODEM_A6) ||         \
    defined(TINY_GSM_MODEM_A7) || defined(TINY_GSM_MODEM_M590) ||            \
    defined(TINY_GSM_MODEM_MC60) || defined(TINY_GSM_MODEM_MC60E) ||         \
    defined(TINY_GSM_MODEM_ESP32) || defined(TINY_GSM_MODEM_ESP8266) ||      \
    defined(TINY_GSM_MODEM_ESP8266_NONOS) || defined(TINY_GSM_MODEM_XBEE) || \
    defined(TINY_GSM_MODEM_SEQUANS_MONARCH) || defined(TINY_GSM_MODEM_A7672X)
// if **ANY** of the supported modems is defined, include the main TinyGSM
// client header
#include "TinyGsmClient.h"
#endif

// If none of the supported modems is defined, do **nothing**.  This allows
// users to include this file as a shim for the Arduino IDE to find the library
// and add it to the search path so users can directly include the individual
// modem headers that are in the "modems" directory.  The Arduino IDE's
// dependency scanner will *not* detect TinyGSM as a dependency and add it to
// the include/scan path if only one or more of individual modem headers are
// included.  For the Arduino IDE to detect TinyGSM as a dependency, one or more
// of the public header files from the top level /src directory must be
// included.

#endif
