/**
 * @file       TinyGsmWifi.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMWIFI_H_
#define SRC_TINYGSMWIFI_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_WIFI

template <class modemType>
class TinyGsmWifi {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * WiFi functions
   */
  bool networkConnect(const char* ssid, const char* pwd) {
    return thisModem().networkConnectImpl(ssid, pwd);
  }
  bool networkDisconnect() {
    return thisModem().networkDisconnectImpl();
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
  ~TinyGsmWifi() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * WiFi functions
   */

  bool networkConnectImpl(const char* ssid,
                          const char* pwd) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool networkDisconnectImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYGSMWIFI_H_
