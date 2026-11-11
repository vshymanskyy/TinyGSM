/**
 * @file       TinyGsmBluetooth.tpp
 * @brief      Bluetooth control and status mixin for modem implementations.
 * @author     Adrian Cervera Andes
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2021 Adrian Cervera Andes
 * @date       Jan 2021
 */

#ifndef SRC_TINYGSMBLUETOOTH_TPP_
#define SRC_TINYGSMBLUETOOTH_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_BLUETOOTH
/// flag to indicate that the modem has Bluetooth functions
#define TINY_GSM_MODEM_HAS_BLUETOOTH
#endif


/**
 * @class TinyGsmBluetooth
 * @brief The CRTP parent class for Bluetooth functions.
 * @tparam modemType The derived modem class
 */
template <class modemType>
class TinyGsmBluetooth {
 public:
  /// Compile-time capability flag indicating Bluetooth support
  static constexpr bool hasBluetooth = true;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor bluetooth_functions
   * @name Bluetooth functions
   */
  /**@{*/

  /**
   * @brief Enable module Bluetooth
   * @return True if the Bluetooth was successfully enabled, false
   * otherwise.
   */
  bool enableBluetooth() {
    return thisModem().enableBluetoothImpl();
  }

  /**
   * @brief Disable module Bluetooth
   * @return True if the Bluetooth was successfully disabled, false otherwise.
   */
  bool disableBluetooth() {
    return thisModem().disableBluetoothImpl();
  }

  /**
   * @brief Set the Bluetooth visibility
   *
   * @param visible True to make the modem visible over Bluetooth, false to make
   * it invisible.
   * @return True if the Bluetooth visibility was successfully changed, false
   * otherwise.
   */
  bool setBluetoothVisibility(bool visible) {
    return thisModem().setBluetoothVisibilityImpl(visible);
  }

  /**
   * @brief Set the Bluetooth host name
   *
   * @param name The name visible to other Bluetooth objects
   * @return True if the Bluetooth host name was successfully changed, false
   * otherwise.
   */
  bool setBluetoothHostName(const char* name) {
    return thisModem().setBluetoothHostNameImpl(name);
  }
  /**@}*/


 protected:
  // destructor (protected!)
  ~TinyGsmBluetooth() {}

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
   * Bluetooth functions
   */

  bool enableBluetoothImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool disableBluetoothImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool setBluetoothVisibilityImpl(bool visible) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool setBluetoothHostNameImpl(const char* name) TINY_GSM_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYGSMBLUETOOTH_TPP_
