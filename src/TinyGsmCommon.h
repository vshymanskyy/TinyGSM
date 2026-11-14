/**
 * @file       TinyGsmCommon.h
 * @brief      Common TinyGSM macros, utilities, and shared constants.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCOMMON_H_
#define SRC_TINYGSMCOMMON_H_

#include <stdint.h>
#include <string.h>

#if (defined(__AVR__) || defined(ARDUINO_ARCH_AVR)) && \
    !defined(__AVR_ATmega4809__)
#include <avr/pgmspace.h>
#endif

/// The current library version number
#define TINYGSM_VERSION "0.13.0-rc.1"

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

/**
 * @def TINY_GSM_RX_BUFFER
 * @brief The size of the receive buffer for the modem.
 *
 * This is used in the TinyGsm class to store incoming data from the modem. This
 * buffer is used to store incoming data from the modem before it is read by the
 * user.  If this buffer is too small, data may be lost if the user does not
 * read it quickly enough - especially for modems that do not internally buffer
 * data.  If this buffer is too large, it may use more memory than necessary.
 *
 * @note This is *not* the size of the modem's internal buffer!
 *
 * @important This is a library-wide setting.  It applies to all modems and all
 * clients.  If you need different buffer sizes for different modems, you will
 * need to modify the library.
 */
#if !defined(TINY_GSM_RX_BUFFER)
// Fallback log buffer size based on processor type
#if defined(__SAMD51__)
#define TINY_GSM_RX_BUFFER 1024
#elif defined(ARDUINO_ARCH_SAMD)
#define TINY_GSM_RX_BUFFER 256
#else
#define TINY_GSM_RX_BUFFER 64
#endif
#endif

#ifndef TINY_GSM_INTERCHAR_TIMEOUT_MS
/**
 * @def TINY_GSM_INTERCHAR_TIMEOUT_MS
 * @brief Maximum silence between characters before timing out.
 */
#define TINY_GSM_INTERCHAR_TIMEOUT_MS 10L
#endif

#ifndef TINY_GSM_RESPONSE_MAX_GRACE_MS
/**
 * @def TINY_GSM_RESPONSE_MAX_GRACE_MS
 * @brief Maximum total response-completion time after the overall timeout.
 *
 * This bounds the inter-character grace period so that continuous input cannot
 * cause waitResponse() to wait indefinitely.
 */
#define TINY_GSM_RESPONSE_MAX_GRACE_MS 1000L
#endif

/**
 * @def TINY_GSM_UNREAD_CHECK_MS
 * @brief The time in milliseconds to wait before checking for unread data from
 * the modem.
 *
 * This is used to ask the modem if there is any unread data available because
 * they don't always issue a URC to notify the host that data is available.
 *
 * @important This is a library-wide setting.  It applies to all modems and all
 * clients.  If you need a different check interval for different modems, you
 * will need to modify the library.
 */
#if !defined(TINY_GSM_UNREAD_CHECK_MS)
#define TINY_GSM_UNREAD_CHECK_MS 500
#endif

/**
 * @def TINY_GSM_ATTR_NOT_AVAILABLE
 * @brief Helper macro to mark a function as not available on this modem type
 * @def TINY_GSM_ATTR_NOT_IMPLEMENTED
 * @brief Helper macro to mark a function as not implemented on this modem type
 */
#define TINY_GSM_ATTR_NOT_AVAILABLE \
  __attribute__((error("Not available on this modem type")))
#define TINY_GSM_ATTR_NOT_IMPLEMENTED __attribute__((error("Not implemented")))

/**
 * @def TINY_GSM_PROGMEM
 * @brief Helper macro for memory storage location
 * @typedef GsmConstStr
 * @brief Helper typedef for a constant string stored in program memory
 * (PROGMEM) on AVR platforms and in standard memory on other systems.
 * @def GFP
 * @brief Helper macro to cast data as a constant string in program memory
 * (PROGMEM) on AVR platforms and in standard memory on other systems.
 * @def GF
 * @brief Helper macro to store data in program memory (PROGMEM) on AVR
 * platforms and in standard memory on other systems.
 * @def TINY_GSM_F_READ(value)
 * @brief Read a 32-bit constant stored with TINY_GSM_PROGMEM.
 * @param value Pointer to the stored constant.
 * @return The stored value.
 * @def TINY_GSM_F_STRNCMP(str, constant, length)
 * @brief Compare a RAM string with a constant string wrapped in GFP().
 * @param str The string in RAM.
 * @param constant The constant string wrapped in GFP().
 * @param length The maximum number of characters to compare.
 * @return A negative, zero, or positive value as for strncmp().
 * @def TINY_GSM_F_STRCPY(dest, constant)
 * @brief Copy a constant string wrapped in GFP() into a RAM buffer.
 * @param dest The destination buffer, large enough for the string and
 * terminator.
 * @param constant The constant string wrapped in GFP().
 * @return The destination buffer.
 */
#if defined(PROGMEM) && (defined(__AVR__) || defined(ARDUINO_ARCH_AVR)) && \
    !defined(__AVR_ATmega4809__) && !defined(GFP) && !defined(GF)
/// Defined when program memory requires special access functions.
#define TINY_GSM_HAS_PROGMEM
#define TINY_GSM_PROGMEM PROGMEM
typedef const __FlashStringHelper* GsmConstStr;
#define GFP(x) (reinterpret_cast<GsmConstStr>(x))
#define GF(x) F(x)
#define TINY_GSM_F_READ(value) pgm_read_dword((value))
#define TINY_GSM_F_STRNCMP(str, constant, length) \
  strncmp_P((str), reinterpret_cast<const char*>(constant), (length))
#define TINY_GSM_F_STRCPY(dest, constant) \
  strcpy_P((dest), reinterpret_cast<const char*>(constant))
#elif !defined(TINY_GSM_PROGMEM) && !defined(GFP) && !defined(GF)
#define TINY_GSM_PROGMEM
typedef const char* GsmConstStr;
#define GFP(x) x
#define GF(x) x
#define TINY_GSM_F_READ(value) (*(value))
#define TINY_GSM_F_STRNCMP(str, constant, length) \
  strncmp((str), (constant), (length))
#define TINY_GSM_F_STRCPY(dest, constant) strcpy((dest), (constant))
#endif

#ifdef TINY_GSM_DEBUG
namespace {
/**
 * @brief Debug print function for a single argument
 *
 * @tparam T The type of the argument
 * @param last The argument to print
 */
template <typename T>
static void DBG_PLAIN(T last) {
  TINY_GSM_DEBUG.println(last);
}

/**
 * @brief Debug print function for multiple arguments
 *
 * @tparam T The type of the first argument
 * @tparam Args The types of the remaining arguments
 * @param head The first argument to print
 * @param tail The remaining arguments to print
 */
template <typename T, typename... Args>
static void DBG_PLAIN(T head, Args... tail) {
  TINY_GSM_DEBUG.print(head);
  TINY_GSM_DEBUG.print(' ');
  DBG_PLAIN(tail...);
}

/**
 * @brief Debug print function for multiple arguments with timestamp
 *
 * @tparam Args The types of the arguments
 * @param args The arguments to print
 */
template <typename... Args>
static void DBG(Args... args) {
  TINY_GSM_DEBUG.print('[');
  TINY_GSM_DEBUG.print(millis());
  TINY_GSM_DEBUG.print(GF("] "));
  DBG_PLAIN(args...);
}
}  // namespace
#else
#define DBG_PLAIN(...)
#define DBG(...)
#endif

/*
 * Min/Max Helpers
 */
template <class T>
const T& TinyGsmMin(const T& a, const T& b) {
  return (b < a) ? b : a;
}
template <class T>
const T& TinyGsmMax(const T& a, const T& b) {
  return (b < a) ? a : b;
}

/*
 * String/IP Address Helpers
 */
/**
 * @brief Convert an IPAddress to a String for use in connect()
 * @param ip The IPAddress to convert
 * @return A String representation of the IPAddress
 */
static inline String TinyGsmStringFromIp(IPAddress ip) {
  String host;
  host.reserve(16);
  host += ip[0];
  host += '.';
  host += ip[1];
  host += '.';
  host += ip[2];
  host += '.';
  host += ip[3];
  return host;
}

/**
 * @brief Standard baud rates to attempt during auto-baud detection and modem
 * initialization.
 *
 * This array is shared between TinyGsmAutoBaud() and forceModemBaud() to
 * ensure consistent baud rate selection across the library.
 */
static const uint32_t TINY_GSM_AUTOBAUD_RATES[] TINY_GSM_PROGMEM = {
    115200, 57600, 9600,  921600, 38400, 19200, 460800,
    230400, 74400, 74880, 2400,   4800,  14400, 28800};

/**
 * @brief Attempts to automatically find the baud rate for the modem.
 * @note This DOES NOT work with the XBee module
 *
 * @param at_serial The serial port connected to the modem
 * @param minimum The minimum baud rate to try (default: 9600)
 * @param maximum The maximum baud rate to try (default: 921600)
 * @return The baud rate that the modem responded to, or 0 if no response was
 * received
 */
template <class T>
uint32_t TinyGsmAutoBaud(T& at_serial, uint32_t minimum = 9600,
                         uint32_t maximum = 921600) {
  static const uint32_t* rates = TINY_GSM_AUTOBAUD_RATES;

  for (uint8_t i = 0; i < 14;
       i++) {  // sizeof(TINY_GSM_AUTOBAUD_RATES)/sizeof(uint32_t)
    uint32_t rate = TINY_GSM_F_READ(&rates[i]);
    if (rate < minimum || rate > maximum) continue;

    DBG("Trying baud rate", rate, "...");
    at_serial.end();
#if !defined(ARDUINO_ARCH_PIC32) && !defined(ARDUINO_ARCH_ARC32) && \
    !defined(ARDUINO_NRF52840_FEATHER)
    unsigned long origTimeout = at_serial.getTimeout();
#endif
    at_serial.setTimeout(100);  // avoid 1s default blocking wait
    at_serial.begin(rate);
    delay(10);
    for (int j = 0; j < 10; j++) {
      at_serial.print("AT\r\n");
      String input = at_serial.readString();
      if (input.indexOf("OK") >= 0) {
        DBG("Modem responded at rate", rate);
#if !defined(ARDUINO_ARCH_PIC32) && !defined(ARDUINO_ARCH_ARC32) && \
    !defined(ARDUINO_NRF52840_FEATHER)
        at_serial.setTimeout(origTimeout);  // reset timeout
#else
        at_serial.setTimeout(1000L);  // reset timeout, assuming 1s default
#endif
        return rate;
      }
    }
#if !defined(ARDUINO_ARCH_PIC32) && !defined(ARDUINO_ARCH_ARC32) && \
    !defined(ARDUINO_NRF52840_FEATHER)
    at_serial.setTimeout(origTimeout);  // reset timeout
#else
    at_serial.setTimeout(1000L);  // reset timeout, assuming 1s default
#endif
  }
  at_serial.begin(minimum);
  return 0;
}

#endif  // SRC_TINYGSMCOMMON_H_
