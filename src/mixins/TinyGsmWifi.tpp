/**
 * @file       TinyGsmWifi.tpp
 * @brief      Wi-Fi connection helper mixin for supported modems.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMWIFI_TPP_
#define SRC_TINYGSMWIFI_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_WIFI
/// flag to indicate that the modem has WiFi functions
#define TINY_GSM_MODEM_HAS_WIFI
#endif

/**
 * @brief The CRTP parent class for WiFi functions.
 * @tparam modemType The derived modem class
 */
template <class modemType>
class TinyGsmWifi {
 public:
  /// Compile-time capability flag indicating WiFi support
  static constexpr bool hasWifi = true;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor wifi_functions
   * @name WiFi functions
   */
  /**@{*/

  /**
   * @brief Connect to a WiFi network
   *
   * @param ssid The SSID of the WiFi network
   * @param pwd The password of the WiFi network
   * @return True if successfully connected to the WiFi network, false
   * otherwise.
   */
  bool networkConnect(const char* ssid, const char* pwd) {
    return thisModem().networkConnectImpl(ssid, pwd);
  }
  /**
   * @brief Disconnect from the WiFi network
   * @return True if successfully disconnected from the WiFi network, false
   * otherwise.
   */
  bool networkDisconnect() {
    return thisModem().networkDisconnectImpl();
  }
  /**@}*/


 protected:
  // destructor (protected!)
  ~TinyGsmWifi() {}

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
   * Wifi functions
   */

  bool networkConnectImpl(const char* ssid,
                          const char* pwd) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool networkDisconnectImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYGSMWIFI_TPP_
