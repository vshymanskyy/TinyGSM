/**
 * @file       TinyGsmGSMLocation.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMGSMLOCATION_H_
#define SRC_TINYGSMGSMLOCATION_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_GSM_LOCATION

template <class modemType>
class TinyGsmGSMLocation {
 public:
  /*
   * Location functions
   */
  String getGsmLocation() {
    return thisModem().getGsmLocationImpl();
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
   * Location functions
   */
 protected:
  String getGsmLocationImpl() {
    thisModem().sendAT(GF("+CIPGSMLOC=1,1"));
    if (thisModem().waitResponse(10000L, GF("+CIPGSMLOC:")) != 1) { return ""; }
    String res = thisModem().stream.readStringUntil('\n');
    thisModem().waitResponse();
    res.trim();
    return res;
  }
};

#endif  // SRC_TINYGSMGSMLOCATION_H_
