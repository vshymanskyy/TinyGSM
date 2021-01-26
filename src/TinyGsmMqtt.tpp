/**
 * @file       TinyGsmMqtt.tpp
 * @author     Clemens Arth
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2021 Clemens Arth
 * @date       Jan 2021
 */

#ifndef SRC_TINYGSMMQTT_H_
#define SRC_TINYGSMMQTT_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_MQTT


template <class modemType>
class TinyGsmMqtt {
 public:
  /*
   * MQTT functions
   */
  bool configureMqtt(String cmd, uint8 arg1, uint8 arg2, uint8 arg3 = 0, uint8 arg4 = 0) {
    return thisModem().configureMqttImpl(cmd, arg1, arg2, arg3, arg4);
  }
  bool openMqtt(uint8 connectId, String servername, uint16 serverport) {
    return thisModem().openMqttImpl(connectId, servername, serverport);
  }
  bool connectMqtt(uint8 connectId, String clientname, String username = "", String password = "") {
    return thisModem().connectMqttImpl(connectId, clientname, username, password);
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
   * MQTT functions
   */
 protected:
  bool configureMqttImpl(String cmd, uint8 arg1, uint8 arg2, uint8 arg3, uint8 arg4) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool openMqttImpl(uint8 connectId, String servername, uint16 serverport) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool connectMqttImpl(uint8 connectId, String clientname, String username, String password) TINY_GSM_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYGSMSSL_H_
