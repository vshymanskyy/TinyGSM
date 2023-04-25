/* TODO:

 - MUX isn't working
 - MUX isn't implemented correcty
 - Quectel modem specific implementation needs to move into modem class

*/

/**
 * @file       TinyGsmMQTT.tpp
 * @author     Alex J Lennon
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2023 Alex J Lennon
 * @date       Apr 2023
 */

#ifndef SRC_TINYGSMMQTT_H_
#define SRC_TINYGSMMQTT_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_MQTT

#define MQTT_DEFAULT_MUX 0
#define MQTT_MUX 5
#define MQTT_DEFAULT_PORT 1883

template <class modemType>
class TinyGsmMQTT {
 public:
  /*
   * Messaging functions
   */

  bool mqttConnected()
  {
    return mqttConnectedImpl(MQTT_DEFAULT_MUX);
  }

  bool mqttConnected(int mux)
  {
    return mqttConnectedImpl(mux);
  }

  int mqttOpen(const char* hostName, int port)
  {
    return thisModem().mqttOpenImpl(MQTT_DEFAULT_MUX, hostName, port);
  }

  int mqttOpen(int mux, const char* hostName, int port)
  {
    return thisModem().mqttOpenImpl(mux, hostName, port);
  }

  int mqttClose()
  {
    return thisModem().mqttCloseImpl(MQTT_DEFAULT_MUX);
  }

  int mqttClose(int mux)
  {
    return thisModem().mqttCloseImpl(mux);
  }

  int mqttConnect(const char* clientId, const char* userName, const char* password)
  {
    return thisModem().mqttConnectImpl(MQTT_DEFAULT_MUX, clientId, userName, password);
  }

  int mqttConnect(int mux, const char* clientId, const char* userName, const char* password)
  {
    return thisModem().mqttConnectImpl(mux, clientId, userName, password);
  }

  int mqttDisconnect()
  {
    return thisModem().mqttDisconnectImpl(MQTT_DEFAULT_MUX);
  }

  int mqttDisconnect(int mux)
  {
    return thisModem().mqttDisconnectImpl(mux);
  }

  int mqttSubscribe(int msgId, const char* topic, int qos)
  {
    return thisModem().mqttSubscribeImpl(MQTT_DEFAULT_MUX, msgId, topic, qos);
  }

  int mqttSubscribe(int mux, int msgId, const char* topic, int qos)
  {
    return thisModem().mqttSubscribeImpl(mux, msgId, topic, qos);
  }

  int mqttUnsubscribe(int msgId, const char *topic)
  {
    return thisModem().mqttUnsubscribeImpl(MQTT_DEFAULT_MUX, msgId, topic);
  }

  int mqttUnsubscribe(int mux, int msgId, const char *topic)
  {
    return thisModem().mqttUnsubscribeImpl(mux, msgId, topic);
  }

  int mqttPublish(int msgId, int qos, bool retain,  const char* topic, const char* payload, int payloadLen)
  {
    return thisModem().mqttPublishImpl(MQTT_DEFAULT_MUX, msgId, qos, retain, topic, payload, payloadLen);
  }

  int mqttPublish(int mux, int msgId, int qos, bool retain,  const char* topic, const char* payload, int payloadLen)
  {
    return thisModem().mqttPublishImpl(mux, msgId, qos, retain, topic, payload, payloadLen);
  }

  int mqttLoop(int mux)
  {
    return thisModem().mqttLoopImpl(mux);
  }

  int mqttLoop()
  {
    return thisModem().mqttLoopImpl(MQTT_DEFAULT_MUX);
  }

  int mqttSetKeepAlive(int keep_alive_secs)
  {
    return thisModem().mqttSetKeepAliveImpl(MQTT_DEFAULT_MUX, keep_alive_secs);
  }

  int mqttSetKeepAlive(int mux, int keep_alive_secs)
  {
    return thisModem().mqttSetKeepAliveImpl(mux, keep_alive_secs);
  }
  
  void mqttSetReceiveCB(void (*cb)(int mux, int msgId, const char *topic, const char *payload, int len) )
  {
    _mqttReceiveCB[MQTT_DEFAULT_MUX] = cb;
  }

  void mqttSetReceiveCB(int mux, void (*cb)(int mux, int msgId, const char *topic, const char *payload, int len) )
  {
    _mqttReceiveCB[mux] = cb;
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

 protected:

  void (*(_mqttReceiveCB[MQTT_MUX]))(int mux, int msgId, const char *topic, const char *payload, int len);
  bool       mqtt_connected[MQTT_MUX];
  bool       mqtt_got_data[MQTT_MUX];

  bool mqttConnectedImpl(int mux)
  {
    return mqtt_connected[mux];
  }
};

#endif  // SRC_TINYGSMSMS_H_
