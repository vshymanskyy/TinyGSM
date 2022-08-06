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
 public:
  /*
   * Bluetooth functions
   */
  bool enableBluetooth() {
    return thisModem().enableBluetoothImpl();
  }
  bool disableBluetooth() {
    return thisModem().disableBluetoothImpl();
  }
  bool setBluetoothVisibility(bool visible) {
    return thisModem().setBluetoothVisibilityImpl(visible);
  }
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

  /*
   * Bluetooth functions
   */

  bool    enableBluetoothImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool    disableBluetoothImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool    setBluetoothVisibilityImpl(bool visible) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool    setBluetoothHostNameImpl(const char* name) TINY_GSM_ATTR_NOT_IMPLEMENTED;
};


#endif  // SRC_TINYGSMBLUETOOTH_H_
