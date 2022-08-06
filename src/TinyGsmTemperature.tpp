/**
 * @file       TinyGsmTemperature.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMTEMPERATURE_H_
#define SRC_TINYGSMTEMPERATURE_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_TEMPERATURE

template <class modemType>
class TinyGsmTemperature {
 public:
  /*
   * Temperature functions
   */
  float getTemperature() {
    return thisModem().getTemperatureImpl();
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

  float getTemperatureImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYGSMTEMPERATURE_H_
