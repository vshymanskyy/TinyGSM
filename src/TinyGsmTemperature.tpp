/**
 * @file       TinyGsmTemperature.tpp
 * @brief      Modem temperature query helper mixin.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMTEMPERATURE_TPP_
#define SRC_TINYGSMTEMPERATURE_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_TEMPERATURE
/// flag to indicate that the modem has temperature functions
#define TINY_GSM_MODEM_HAS_TEMPERATURE
#endif

/**
 * @brief Provides temperature-related functions for the modem.
 *
 * This class allows you to access the modem's temperature sensor and retrieve
 * the current temperature in degrees Celsius.
 *
 * @tparam modemType The derived modem class
 */
template <class modemType>
class TinyGsmTemperature {
 public:
  /// Compile-time capability flag indicating temperature sensor support
  static constexpr bool hasTemperature = true;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor temperature_functions
   * @name Temperature functions
   */
  /**@{*/

  /**
   * @brief Get the modem chip temperature in degrees celsius.
   * @return *float* The modem chip temperature in degrees celsius, or
   * -9999.0F if the modem does not respond or returns an invalid value.
   */
  float getTemperature() {
    return thisModem().getTemperatureImpl();
  }
  /**@}*/


 protected:
  // destructor (protected!)
  ~TinyGsmTemperature() {}

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
   * Temperature functions
   */

  float getTemperatureImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYGSMTEMPERATURE_TPP_
