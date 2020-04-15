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
 public:
  /*
   * Battery functions
   */
  uint16_t getBattVoltage() {
    return thisModem().getBattVoltageImpl();
  }
  int8_t getBattPercent() {
    return thisModem().getBattPercentImpl();
  }
  uint8_t getBattChargeState() {
    return thisModem().getBattChargeStateImpl();
  }
  bool getBattStats(uint8_t& chargeState, int8_t& percent,
                    uint16_t& milliVolts) {
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

  /*
   * Battery functions
   */
 protected:
  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  uint16_t getBattVoltageImpl() {
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

  uint8_t getBattChargeStateImpl() {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return false; }
    // Read battery charge status
    int8_t res = thisModem().streamGetIntBefore(',');
    // Wait for final OK
    thisModem().waitResponse();
    return res;
  }

  bool getBattStatsImpl(uint8_t& chargeState, int8_t& percent,
                        uint16_t& milliVolts) {
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
