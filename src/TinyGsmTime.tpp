/**
 * @file       TinyGsmTime.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMTIME_H_
#define SRC_TINYGSMTIME_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_TIME

enum TinyGSMDateTimeFormat { DATE_FULL = 0, DATE_TIME = 1, DATE_DATE = 2 };

template <class modemType>
class TinyGsmTime {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * Time functions
   */

  /**
   * @brief Get the Date Time as a String
   *
   * @param format The date or time part to get: DATE_FULL,
   * DATE_TIME, or DATE_DATE
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
   * @return *true*  The references have been filled with valid values from the
   * GSM module.
   * @return *false*  There was a problem getting the time from the module.
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
   * @return *true*  The references have been filled with valid values from the
   * GSM module.
   * @return *false*  There was a problem getting the time from the module.
   */
  bool getNetworkUTCTime(int* year, int* month, int* day, int* hour,
                         int* minute, int* second, float* timezone) {
    return thisModem().getNetworkUTCTimeImpl(year, month, day, hour, minute,
                                             second, timezone);
  }

  /*
   * CRTP Helper
   */
 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }
  ~TinyGsmTime() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Time functions
   */
 protected:
  String getGSMDateTimeImpl(TinyGSMDateTimeFormat format) {
    thisModem().sendAT(GF("+CCLK?"));
    if (thisModem().waitResponse(2000L, GF("+CCLK: \"")) != 1) { return ""; }

    String res;

    switch (format) {
      case DATE_FULL: res = thisModem().stream.readStringUntil('"'); break;
      case DATE_TIME:
        thisModem().streamSkipUntil(',');
        res = thisModem().stream.readStringUntil('"');
        break;
      case DATE_DATE: res = thisModem().stream.readStringUntil(','); break;
    }
    thisModem().waitResponse();  // Ends with OK
    return res;
  }

  bool getNetworkTimeImpl(int* year, int* month, int* day, int* hour,
                          int* minute, int* second, float* timezone) {
    thisModem().sendAT(GF("+CCLK?"));
    if (thisModem().waitResponse(2000L, GF("+CCLK: \"")) != 1) { return false; }

    int iyear     = 0;
    int imonth    = 0;
    int iday      = 0;
    int ihour     = 0;
    int imin      = 0;
    int isec      = 0;
    int itimezone = 0;

    // Date & Time
    iyear       = thisModem().streamGetIntBefore('/');
    imonth      = thisModem().streamGetIntBefore('/');
    iday        = thisModem().streamGetIntBefore(',');
    ihour       = thisModem().streamGetIntBefore(':');
    imin        = thisModem().streamGetIntBefore(':');
    isec        = thisModem().streamGetIntLength(2);
    char tzSign = thisModem().stream.read();
    itimezone   = thisModem().streamGetIntBefore('\n');
    if (tzSign == '-') { itimezone = itimezone * -1; }

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
    return true;
  }

  bool getNetworkUTCTimeImpl(int* year, int* month, int* day, int* hour,
                             int* minute, int* second,
                             float* timezone) TINY_GSM_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYGSMTIME_H_
