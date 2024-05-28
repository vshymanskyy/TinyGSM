/**
 * @file       TinyGsmGPS.tpp
 * @author     Adrian Cervera Andes
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2021 Adrian Cervera Andes
 * @date       Jan 2021
 */

#ifndef SRC_TINYGSMBLUETOOTH_H_
#define SRC_TINYGSMBLUETOOTH_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_BLUETOOTH

template <class modemType>
class TinyGsmBluetooth {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * Bluetooth functions
   */

  /**
   * @brief Enable module Bluetooth
   *
   * @return *true* Bluetooth was succcessfully enabled
   * @return *false* Bluetooth failed to enable
   */
  bool enableBluetooth() {
    return thisModem().enableBluetoothImpl();
  }

  /**
   * @brief Disable module Bluetooth
   *
   * @return *true* Bluetooth was succcessfully disabled
   * @return *false* Bluetooth failed to disable
   */
  bool disableBluetooth() {
    return thisModem().disableBluetoothImpl();
  }

  /**
   * @brief Set the Bluetooth visibility
   *
   * @param visible True to make the modem visible over Bluetooth, false to make
   * it invisible.
   * @return *true* Bluetooth visibility was successfully changed.
   * @return *false* Bluetooth visibility failed to change
   */
  bool setBluetoothVisibility(bool visible) {
    return thisModem().setBluetoothVisibilityImpl(visible);
  }

  /**
   * @brief Set the Bluetooth host name
   *
   * @param name The name visible to other Bluetooth objects
   * @return *true* Bluetooth host name was successfully changed.
   * @return *false* Bluetooth host name failed to change
   */
  bool setBluetoothHostName(const char* name) {
    return thisModem().setBluetoothHostNameImpl(name);
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
  ~TinyGsmBluetooth() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Bluetooth functions
   */

  bool enableBluetoothImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool disableBluetoothImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool setBluetoothVisibilityImpl(bool visible) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool setBluetoothHostNameImpl(const char* name) TINY_GSM_ATTR_NOT_IMPLEMENTED;
};


#endif  // SRC_TINYGSMBLUETOOTH_H_
