/**
 * @file       TinyGsmTime.tpp
 * @brief      Clock and date-time helper mixin for modem implementations.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMTIME_TPP_
#define SRC_TINYGSMTIME_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_TIME
/// flag to indicate that the modem has time printing and retrieval functions
#define TINY_GSM_MODEM_HAS_TIME
#endif

/// Enum for different date/time formats.
enum class TinyGSMDateTimeFormat : int8_t {
  DATE_FULL = 0,  ///< The full date and time
  DATE_TIME = 1,  ///< Only the time portion of the date/time
  DATE_DATE = 2   ///< Only the date portion of the date/time
};

/// Enum for the epoch start value.
enum class TinyGSM_EpochStart : int8_t {
  /// Use a Unix epoch, starting 1/1/1970 (946684800 seconds before the Y2K
  /// epoch, 315964800 seconds before the GPS epoch)
  UNIX = 0,
  /// Use an epoch starting 1/1/2000, as some RTCs and Arduinos do (946684800
  /// seconds after the UNIX epoch, 630720000 seconds after the GPS epoch)
  Y2K = 1,
  /// Use the GPS epoch starting Jan 6, 1980 (315964800 seconds after the UNIX
  /// epoch, 630720000 seconds before the Y2K epoch)
  GPS = 2
};

/**
 * @brief The CRTP parent class for time printing and retrieval functions
 * @tparam modemType The derived modem class
 */
template <class modemType>
class TinyGsmTime {
 public:
  /// Compile-time capability flag indicating time/clock support
  static constexpr bool hasTime = true;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor time_functions
   * @name Time functions
   */
  /**@{*/

  /**
   * @brief Get the Date Time as a String
   *
   * @param format The date or time part to get:
   * TinyGSMDateTimeFormat::DATE_FULL, TinyGSMDateTimeFormat::DATE_TIME, or
   * TinyGSMDateTimeFormat::DATE_DATE
   * @return *String*  The date and/or time from the module
   */
  String getGSMDateTime(TinyGSMDateTimeFormat format) {
    return thisModem().getGSMDateTimeImpl(format);
  }

  /**
   * @brief Get the date and time as parts
   *
   * @param year Reference to an int for the year
   * @param month Reference to an int for the month
   * @param day Reference to an int for the day
   * @param hour Reference to an int for the hour
   * @param minute Reference to an int for the minute
   * @param second Reference to an int for the second
   * @param timezone Reference to a float for the timezone
   * @return True if the references have been filled with valid values from
   * the GSM module, false otherwise.
   */
  bool getNetworkTime(int* year, int* month, int* day, int* hour, int* minute,
                      int* second, float* timezone) {
    return thisModem().getNetworkTimeImpl(year, month, day, hour, minute,
                                          second, timezone);
  }

  /**
   * @brief Get the date and time as parts in UTC
   *
   * @param year Reference to an int for the year
   * @param month Reference to an int for the month
   * @param day Reference to an int for the day
   * @param hour Reference to an int for the hour
   * @param minute Reference to an int for the minute
   * @param second Reference to an int for the second
   * @param timezone Reference to a float for the timezone
   * @return True if the references have been filled with valid values from
   * the GSM module, false otherwise.
   */
  bool getNetworkUTCTime(int* year, int* month, int* day, int* hour,
                         int* minute, int* second, float* timezone) {
    return thisModem().getNetworkUTCTimeImpl(year, month, day, hour, minute,
                                             second, timezone);
  }

  /**
   * @brief Get the Date/Time as an epoch value
   *
   * @param epoch The epoch start to use.
   * @return *uint32_t* The offset from the start of the epoch or 0 if the epoch
   * could not be retrieved.
   */
  uint32_t
  getNetworkEpoch(TinyGSM_EpochStart epoch = TinyGSM_EpochStart::UNIX) {
    return thisModem().getNetworkEpochImpl(epoch);
  }
  /**@}*/


 protected:
  // destructor (protected!)
  ~TinyGsmTime() {}

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
   * Time functions
   */

  // Basic validator
  static bool isValidDateTimeValues(int16_t iyear, int16_t imonth, int16_t iday,
                                    int16_t ihour, int16_t imin, int16_t isec,
                                    int16_t itimezone) {
    return (iyear >= 2000 && imonth >= 1 && imonth <= 12 && iday >= 1 &&
            iday <= 31 && ihour >= 0 && ihour <= 23 && imin >= 0 &&
            imin <= 59 && isec >= 0 && isec <= 59 && itimezone >= -48 &&
            itimezone <= 56);
  }

  String getGSMDateTimeImpl(TinyGSMDateTimeFormat format) {
    thisModem().sendAT(GF("+CCLK?"));
    if (thisModem().waitResponse(2000L, GF("+CCLK: \"")) != 1) { return ""; }

    String res;

    switch (format) {
      case TinyGSMDateTimeFormat::DATE_FULL:
        res = thisModem().stream.readStringUntil('"');
        break;
      case TinyGSMDateTimeFormat::DATE_TIME:
        thisModem().streamSkipUntil(',');
        res = thisModem().stream.readStringUntil('"');
        break;
      case TinyGSMDateTimeFormat::DATE_DATE:
        res = thisModem().stream.readStringUntil(',');
        break;
    }
    thisModem().waitResponse();  // Ends with OK
    return res;
  }

  bool getNetworkTimeImpl(int* year, int* month, int* day, int* hour,
                          int* minute, int* second, float* timezone) {
    thisModem().sendAT(GF("+CCLK?"));
    if (thisModem().waitResponse(2000L, GF("+CCLK: \"")) != 1) { return false; }

    int16_t iyear     = 0;
    int16_t imonth    = 0;
    int16_t iday      = 0;
    int16_t ihour     = 0;
    int16_t imin      = 0;
    int16_t isec      = 0;
    int16_t itimezone = 0;

    // Date & Time
    iyear     = thisModem().streamGetIntBefore('/');
    imonth    = thisModem().streamGetIntBefore('/');
    iday      = thisModem().streamGetIntBefore(',');
    ihour     = thisModem().streamGetIntBefore(':');
    imin      = thisModem().streamGetIntBefore(':');
    isec      = thisModem().streamGetIntLength(2);
    itimezone = thisModem().streamGetIntBefore('\n');

    // Set pointers
    if (iyear < 2000) iyear += 2000;
    if (year != nullptr) *year = iyear;
    if (month != nullptr) *month = imonth;
    if (day != nullptr) *day = iday;
    if (hour != nullptr) *hour = ihour;
    if (minute != nullptr) *minute = imin;
    if (second != nullptr) *second = isec;
    if (timezone != nullptr) *timezone = static_cast<float>(itimezone) / 4.0;

    // Final OK
    thisModem().waitResponse();

#if 0
    DBG(GF("Year:"), iyear, GF("	Month:"), imonth, GF("	Day:"), iday);
    DBG(GF("Hour:"), ihour, GF("	Minute:"), imin, GF("	Second:"), isec);
    DBG(GF("Timezone:"), static_cast<float>(itimezone) / 4.0);
#endif

    // Validate parsed values
    // NOTE: We fill in the pointers before validating so that the user can see
    // what was returned even if it was invalid.
    if (!isValidDateTimeValues(iyear, imonth, iday, ihour, imin, isec,
                               itimezone)) {
      return false;
    }
    return true;
  }

  bool getNetworkUTCTimeImpl(int* year, int* month, int* day, int* hour,
                             int* minute, int* second,
                             float* timezone) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  uint32_t
  getNetworkEpochImpl(TinyGSM_EpochStart epoch) TINY_GSM_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYGSMTIME_TPP_

// cSpell:words ihour
