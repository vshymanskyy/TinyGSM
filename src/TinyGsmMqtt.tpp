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

// TAKEN FROM PUB-SUB-CLIENT
#include <functional>
#define MQTT_CALLBACK_SIGNATURE_T std::function<void(char*, uint8_t*, unsigned int)>

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
  bool disconnectMqtt(uint8 connectId) {
    return thisModem().disconnectMqttImpl(connectId);
  }
  bool closeMqtt(uint8 connectId) {
    return thisModem().closeMqttImpl(connectId);
  }
  bool publishMqtt(uint8 connectId, uint16 msgId, String topic, String msg, uint8 qos = 0, uint8 retain = 0) {
    return thisModem().publishMqttImpl(connectId, msgId, topic, msg, qos, retain);
  }
  bool subscribeMqtt(uint8 connectId, uint16 msgId, String topic, uint8 qos = 0) {
    return thisModem().subscribeMqttImpl(connectId, msgId, topic, qos);
  }
  bool unsubscribeMqtt(uint8 connectId, uint16 msgId, String topic) {
    return thisModem().unsubscribeMqttImpl(connectId, msgId, topic);
  }
  void setRecvCallbackMqtt(MQTT_CALLBACK_SIGNATURE_T callback) {
    thisModem().setRecvCallbackMqttImpl(callback);
  }
  void loopMqtt() {
    thisModem().loopMqttImpl();
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
  bool disconnectMqttImpl(uint8 connectId) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool closeMqttImpl(uint8 connectId) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool subscribeMqttImpl(uint8 connectId, uint16 msgId, String topic, uint8 qos) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool unsubscribeMqttImpl(uint8 connectId, uint16 msgId, String topic) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool publishMqttImpl(uint8 connectId, uint16 msgId, String topic, String msg, uint8 qos, uint8 retain) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  void setRecvCallbackMqttImpl(MQTT_CALLBACK_SIGNATURE_T callback) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  void loopMqttImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
};

#endif  // SRC_TINYGSMSSL_H_
