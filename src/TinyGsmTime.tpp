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
 public:
  /*
   * Time functions
   */
  String getGSMDateTime(TinyGSMDateTimeFormat format) {
    return thisModem().getGSMDateTimeImpl(format);
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
    return res;
  }
};

#endif  // SRC_TINYGSMTIME_H_
