/**
 * @file       TinyGsmBattery.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMBATTERY_H_
#define SRC_TINYGSMBATTERY_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_BATTERY

template <class modemType>
class TinyGsmBattery {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * Battery functions
   */

  /**
   * @brief Get the current battery voltage.
   *
   * @note Unless you have a battery directly connected to your modem module,
   * this will be the input voltage going to the module from your main processor
   * board, not the battery voltage of your main processor.
   *
   * @return *int16_t*  The battery voltage measured by the modem module.
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
   * @return *int8_t*  The current battery percent.
   */
  int8_t getBattPercent() {
    return thisModem().getBattPercentImpl();
  }

  /**
   * @brief Get the battery charging state.
   *
   * @return *int8_t* The battery charge state.
   */
  int8_t getBattChargeState() {
    return thisModem().getBattChargeStateImpl();
  }

  /**
   * @brief Get the all battery state
   *
   * @param chargeState A reference to an int to set to the battery charge state
   * @param percent A reference to an int to set to the battery percent
   * @param milliVolts A reference to an int to set to the battery voltage
   * @return *true* The battery stats were updated by the module.
   * @return *false* There was a failure in updating the battery stats from the
   * module.
   */
  bool getBattStats(int8_t& chargeState, int8_t& percent, int16_t& milliVolts) {
    return thisModem().getBattStatsImpl(chargeState, percent, milliVolts);
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
  ~TinyGsmBattery() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Battery functions
   */
 protected:
  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  int16_t getBattVoltageImpl() {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return 0; }
    thisModem().streamSkipUntil(',');  // Skip battery charge status
    thisModem().streamSkipUntil(',');  // Skip battery charge level
    // return voltage in mV
    uint16_t res = thisModem().streamGetIntBefore('\n');
    // Wait for final OK
    thisModem().waitResponse();
    return res;
  }

  int8_t getBattPercentImpl() {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return false; }
    thisModem().streamSkipUntil(',');  // Skip battery charge status
    // Read battery charge level
    int8_t res = thisModem().streamGetIntBefore(',');
    // Wait for final OK
    thisModem().waitResponse();
    return res;
  }

  int8_t getBattChargeStateImpl() {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return false; }
    // Read battery charge status
    int8_t res = thisModem().streamGetIntBefore(',');
    // Wait for final OK
    thisModem().waitResponse();
    return res;
  }

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return false; }
    chargeState = thisModem().streamGetIntBefore(',');
    percent     = thisModem().streamGetIntBefore(',');
    milliVolts  = thisModem().streamGetIntBefore('\n');
    // Wait for final OK
    thisModem().waitResponse();
    return true;
  }
};

#endif  // SRC_TINYGSMBATTERY_H_
