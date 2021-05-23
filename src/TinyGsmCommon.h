/**
 * @file       TinyGsmCommon.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCOMMON_H_
#define SRC_TINYGSMCOMMON_H_

// The current library version number
#define TINYGSM_VERSION "0.10.9"

#if defined(SPARK) || defined(PARTICLE)
#include "Particle.h"
#elif defined(ARDUINO)
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#endif

#if defined(ARDUINO_DASH)
#include <ArduinoCompat/Client.h>
#else
#include <Client.h>
#endif

#ifndef TINY_GSM_YIELD_MS
#define TINY_GSM_YIELD_MS 0
#endif

#ifndef TINY_GSM_PHONEBOOK_RESULTS
#define TINY_GSM_PHONEBOOK_RESULTS 5
#endif

#ifndef TINY_GSM_YIELD
#define TINY_GSM_YIELD() \
  { delay(TINY_GSM_YIELD_MS); }
#endif

#define TINY_GSM_ATTR_NOT_AVAILABLE \
  __attribute__((error("Not available on this modem type")))
#define TINY_GSM_ATTR_NOT_IMPLEMENTED __attribute__((error("Not implemented")))

#if defined(__AVR__) && !defined(__AVR_ATmega4809__)
#define TINY_GSM_PROGMEM PROGMEM
typedef const __FlashStringHelper* GsmConstStr;
#define GFP(x) (reinterpret_cast<GsmConstStr>(x))
#define GF(x) F(x)
#else
#define TINY_GSM_PROGMEM
typedef const char* GsmConstStr;
#define GFP(x) x
#define GF(x) x
#endif

#ifdef TINY_GSM_DEBUG
namespace {
template <typename T>
static void DBG_PLAIN(T last) {
  TINY_GSM_DEBUG.println(last);
}

template <typename T, typename... Args>
static void DBG_PLAIN(T head, Args... tail) {
  TINY_GSM_DEBUG.print(head);
  TINY_GSM_DEBUG.print(' ');
  DBG_PLAIN(tail...);
}

template <typename... Args>
static void DBG(Args... args) {
  TINY_GSM_DEBUG.print(GF("["));
  TINY_GSM_DEBUG.print(millis());
  TINY_GSM_DEBUG.print(GF("] "));
  DBG_PLAIN(args...);
}
}  // namespace
#else
#define DBG_PLAIN(...)
#define DBG(...)
#endif

template <class T>
const T& TinyGsmMin(const T& a, const T& b) {
  return (b < a) ? b : a;
}

enum class SmsStatus : uint8_t {
  REC_UNREAD  = 0,
  REC_READ    = 1,
  STO_UNSENT  = 2,
  STO_SENT    = 3,
  ALL         = 4
};

enum class SmsAlphabet : uint8_t {
  GSM_7bit   = B00,
  Data_8bit  = B01,
  UCS2       = B10,
  Reserved   = B11
};

struct Sms {
  SmsStatus status;              // <stat>
  SmsAlphabet alphabet;          // alphabet part of TP-DCS
  String originatingAddress;     // <oa>
  String phoneBookEntry;         // <alpha>
  String serviceCentreTimeStamp; // <scts>, format: yy/MM/dd,hh:mm:ss±zz; zz: time zone, quarter of an hour
  String message;                // <data>
};

enum class MessageStorageType : uint8_t {
  SIM,                // SM
  Phone,              // ME
  SIMPreferred,       // SM_P
  PhonePreferred,     // ME_P
  Either_SIMPreferred // MT (use both)
};

struct MessageStorage {
  /*
   * [0]: Messages to be read and deleted from this memory storage
   * [1]: Messages will be written and sent to this memory storage
   * [2]: Received messages will be placed in this memory storage
   */
  MessageStorageType type[3];
  uint8_t used[3]  = {0};
  uint8_t total[3] = {0};
};

enum class DeleteAllSmsMethod : uint8_t {
  Read     = 1,
  Unread   = 2,
  Sent     = 3,
  Unsent   = 4,
  Received = 5,
  All      = 6
};

enum class PhonebookStorageType : uint8_t {
  SIM,    // Typical size: 250
  Phone,  // Typical size: 100
  Invalid
};

struct PhonebookStorage {
  PhonebookStorageType type = PhonebookStorageType::Invalid;
  uint8_t used  = {0};
  uint8_t total = {0};
};

struct PhonebookEntry {
  String number;
  String text;
};

struct PhonebookMatches {
  uint8_t index[TINY_GSM_PHONEBOOK_RESULTS] = {0};
};



template <class T>
const T& TinyGsmMax(const T& a, const T& b) {
  return (b < a) ? a : b;
}

template <class T>
uint32_t TinyGsmAutoBaud(T& SerialAT, uint32_t minimum = 9600,
                         uint32_t maximum = 115200) {
  static uint32_t rates[] = {115200, 57600,  38400, 19200, 9600,  74400, 74880,
                             230400, 460800, 2400,  4800,  14400, 28800};

  for (uint8_t i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
    uint32_t rate = rates[i];
    if (rate < minimum || rate > maximum) continue;

    DBG("Trying baud rate", rate, "...");
    SerialAT.begin(rate);
    delay(10);
    for (int j = 0; j < 10; j++) {
      SerialAT.print("AT\r\n");
      String input = SerialAT.readString();
      if (input.indexOf("OK") >= 0) {
        DBG("Modem responded at rate", rate);
        return rate;
      }
    }
  }
  SerialAT.begin(minimum);
  return 0;
}

#endif  // SRC_TINYGSMCOMMON_H_
