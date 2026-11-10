/**
 * @file       TinyGsmBattery.tpp
 * @brief      Battery status and power reporting mixin for modem
 * implementations.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMBATTERY_TPP_
#define SRC_TINYGSMBATTERY_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_BATTERY
/// flag to indicate that the modem has battery functions
#define TINY_GSM_MODEM_HAS_BATTERY
#endif

/**
 * @class TinyGsmBattery
 * @brief The CRTP parent class for battery functions.
 * @tparam modemType The derived modem class
 */
template <class modemType>
class TinyGsmBattery {
 public:
  /// Compile-time capability flag indicating battery status support
  static constexpr bool hasBattery = true;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor battery_functions
   * @name Battery functions
   */
  /**@{*/

  /**
   * @brief Get the current battery voltage.
   *
   * @note Unless you have a battery directly connected to your modem module,
   * this will be the input voltage going to the module from your main processor
   * board, not the battery voltage of your main processor.
   *
   * @return *int16_t*  The battery voltage measured by the modem module, in
   * millivolts, or -9999 if the modem didn't respond or returned an invalid
   * value.
   */
  int16_t getBattVoltage() {
    return thisModem().getBattVoltageImpl();
  }

  /**
   * @brief Get the current battery percent.
   *
   * @note Unless you have a battery directly connected to your modem module,
   * this will be the percent from the input voltage going to the module from
   * your main processor board, not the battery percent of your main processor.
   *
   * @return *int8_t* The current battery percent, or -99 if the modem does not
   * respond or returns an invalid value.
   */
  int8_t getBattPercent() {
    return thisModem().getBattPercentImpl();
  }

  /**
   * @brief Get the battery charging state.
   * @return *int8_t* The battery charge state, or -99 if the modem does not
   * respond or returns an invalid value.
   */
  int8_t getBattChargeState() {
    return thisModem().getBattChargeStateImpl();
  }

  /**
   * @brief Get the all battery state
   *
   * @param chargeState A reference to an int to set to the battery charge
   * state, or -99 if unavailable, not valid, or not received.
   * @param percent A reference to an int to set to the battery percent, or -99
   * if unavailable, not valid, or not received.
   * @param milliVolts A reference to an int to set to the battery voltage in
   * millivolts, or -9999 if unavailable, not valid, or not received.
   *
   * @return True only if all battery stats were received and are valid, false
   * if any were not returned or not valid.
   */
  bool getBattStats(int8_t& chargeState, int8_t& percent, int16_t& milliVolts) {
    return thisModem().getBattStatsImpl(chargeState, percent, milliVolts);
  }
  /**@}*/


 protected:
  // destructor (protected!)
  ~TinyGsmBattery() {}

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
   * Battery functions
   */

  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  int16_t getBattVoltageImpl() {
    thisModem().sendAT(GF("+CBC"));
    // Return -9999 for no response
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return -9999; }
    thisModem().streamSkipUntil(',');  // Skip battery charge status
    thisModem().streamSkipUntil(',');  // Skip battery charge level
    // get voltage in milliVolts
    int16_t milliVolts = thisModem().streamGetIntBefore('\n');
    // Wait for final OK
    bool validResponse = thisModem().waitResponse() == 1;
    // Return -9999 for an invalid value
    if (!validResponse || milliVolts <= 0) { return -9999; }
    // Return millivolts
    return milliVolts;
  }

  int8_t getBattPercentImpl() {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return -99; }
    thisModem().streamSkipUntil(',');  // Skip battery charge status
    // Read battery charge level
    int16_t res = thisModem().streamGetIntBefore(',');
    // Wait for final OK
    bool validResponse = thisModem().waitResponse() == 1;
    if (!validResponse || res < 0 || res > 100) { return -99; }
    return static_cast<int8_t>(res);
  }

  int8_t getBattChargeStateImpl() {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return -99; }
    // Read battery charge status
    int16_t res = thisModem().streamGetIntBefore(',');
    // Wait for final OK
    bool validResponse = thisModem().waitResponse() == 1;
    if (!validResponse || res < 0 || res > 2) { return -99; }
    return static_cast<int8_t>(res);
  }

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    // set outputs to default invalid values
    chargeState = -99;
    percent     = -99;
    milliVolts  = -9999;

    thisModem().sendAT(GF("+CBC"));
    // if there's no response to the request, return false immediately
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return false; }
    // try to parse the responses
    int16_t parsedChargeState = thisModem().streamGetIntBefore(',');
    int16_t parsedPercent     = thisModem().streamGetIntBefore(',');
    int16_t parsedMilliVolts  = thisModem().streamGetIntBefore('\n');
    // Wait for final OK
    if (thisModem().waitResponse() != 1) { return false; }

    bool results_valid = true;
    // the charge state should be between 0 and 2
    if (parsedChargeState >= 0 && parsedChargeState <= 2) {
      chargeState = static_cast<int8_t>(parsedChargeState);
    } else {
      results_valid = false;
    }
    // the percent should be between 0 and 100
    if (parsedPercent >= 0 && parsedPercent <= 100) {
      percent = static_cast<int8_t>(parsedPercent);
    } else {
      results_valid = false;
    }
    // assign the parsed milliVolts to the output if it is valid
    if (parsedMilliVolts > 0) {
      milliVolts = parsedMilliVolts;
    } else {
      results_valid = false;
    }

    // return true if everything is valid and within the expected ranges
    return results_valid;
  }
};

#endif  // SRC_TINYGSMBATTERY_TPP_
