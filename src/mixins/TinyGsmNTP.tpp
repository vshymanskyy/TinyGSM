/**
 * @file       TinyGsmNTP.tpp
 * @brief      Network time synchronization helper mixin.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMNTP_TPP_
#define SRC_TINYGSMNTP_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_NTP
/// flag to indicate that the modem has network time protocol (NTP) functions
#define TINY_GSM_MODEM_HAS_NTP
#endif

/**
 * @brief The CRTP parent class for network time protocol (NTP) configuration
 * and synchronization functions.
 * @tparam modemType The derived modem class
 */
template <class modemType>
class TinyGsmNTP {
 public:
  /// Compile-time capability flag indicating NTP support
  static constexpr bool hasNTP = true;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor ntp_server_functions
   * @name NTP server functions
   */
  /**@{*/

 public:
  /**
   * @brief Synchronize the modem with an NTP server
   *
   * @remark In versions 0.12.0 and prior, this function returned a byte, the
   * meaning of which varied by modem.  In some cases the return value indicated
   * success with 0 and failure with other codes.  In the current version, it
   * returns a boolean indicating success or failure.
   *
   * @param server The NTP server to use
   * @param TimeZone The timezone offset from UTC, in quarter-hours (15-minute
   * increments)
   *
   * @return True if the synchronization was successful, false otherwise.
   */
  bool NTPServerSync(const char* server = "pool.ntp.org", int TimeZone = 0) {
    return thisModem().NTPServerSyncImpl(server, TimeZone);
  }

  /// @copydoc NTPServerSync(const char*, int)
  bool NTPServerSync(const String& server, int TimeZone = 0) {
    return NTPServerSync(server.c_str(), TimeZone);
  }

  /**
   * @brief Wait for the modem to synchronize with the NTP server
   * @param timeout_s The timeout in seconds
   * @return True if the modem synchronized successfully, false otherwise.
   */
  bool waitForTimeSync(uint16_t timeout_s = 120) {
    return thisModem().waitForTimeSyncImpl(timeout_s);
  }
  /**@}*/

  /**
   * @anchor ntp_utilities
   * @name NTP Utilities
   */
  /**@{*/
 protected:
  /**
   * @brief Check if a string is a valid number
   *
   * @param str The string to check
   *
   * @return True if the string is a valid number, false otherwise.
   */
  bool TinyGsmIsValidNumber(String str) {
    if (!(str.charAt(0) == '+' || str.charAt(0) == '-' ||
          isDigit(str.charAt(0))))
      return false;

    for (byte i = 1; i < str.length(); i++) {
      if (!(isDigit(str.charAt(i)) || str.charAt(i) == '.')) { return false; }
    }
    return true;
  }
  /**@}*/


 protected:
  // destructor (protected!)
  ~TinyGsmNTP() {}

  /*
   * CRTP Helper
   */
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */
 protected:
  /*
   * NTP server functions
   */

  bool NTPServerSyncImpl(const char* server, int TimeZone) {
    // Set bearer profile to associate with NTP sync
    // this is allowed to fail; it's not supported by all modules
    thisModem().sendAT(GF("+CNTPCID=1"));
    thisModem().waitResponse(10000L);

    // Set NTP server and timezone
    // AT+CNTP=<ntpserver>[,<timezone>]
    // <ntpserver> - NTP server’s url
    // <time zone> - Local time zone, the range is (-47 to 48), in fact, time
    // zone range (-12 to 12), but taking into account that some countries and
    // regions will use half time zone, or even fourth time zone, so the entire
    // extended four time zones X, so that when the time zone of the input
    // integers are used, without the need for decimal. Time zone in front of
    // the West if it is a negative number indicates the time zone.
    thisModem().sendAT(GF("+CNTP=\""), server, GF("\","), TimeZone);
    if (thisModem().waitResponse(10000L) != 1) { return false; }

    // TODO: Should we have a "quick fail" here? We don't want to wait for the
    // time sync to finish here, but if we don't wait up to the maximum possible
    // response time, will the "+CNTP:" end up mangled in the responses to a
    // later command?  The waitForTimeSync() function calls the exact command
    // repeatedly waiting for a response within 10s each time.  Are those going
    // to cause trouble?

    // Request network synchronization - execution command
    thisModem().sendAT(GF("+CNTP"));
    if (thisModem().waitResponse(10000L, GF("+CNTP:")) == 1) {
      // Return: +CNTP: <code>[,<time>]
      // <code> - Result code of the NTP synchronization
      //    - 1 Network time synchronization is successful
      //    - 61 Network Error
      //    - 62 DNS resolution error
      //    - 63 Connection Error
      //    - 64 Service response error
      //    - 65 Service Response Timeout
      int code = thisModem().stream.parseInt();
      // Throw away the rest of the line, since we don't care about it here
      thisModem().streamSkipUntil('\n');
      return code == 1;
    }
    return false;
  }

  bool waitForTimeSyncImpl(uint16_t timeout_s) {
    // if we're not connected, we'll never get the time
    if (!thisModem().isNetworkConnected()) { return false; }
    // if we're sure we should be able to get the time, wait for it
    uint32_t start_millis = millis();
    while (millis() - start_millis < static_cast<uint32_t>(timeout_s) * 1000) {
      // Request network synchronization
      thisModem().sendAT(GF("+CNTP"));
      if (thisModem().waitResponse(10000L, GF("+CNTP:")) == 1) {
        int code = thisModem().stream.parseInt();
        // Throw away the rest of the line, since we don't care about it here
        thisModem().streamSkipUntil('\n');
        if (code == 1) { return true; }
      }
      delay(250);
    }
    return false;
  }
};

#endif  // SRC_TINYGSMNTP_TPP_
