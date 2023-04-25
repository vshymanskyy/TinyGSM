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


  int mqttOpenImpl(int id, const char* hostName, int port)
  {
    thisModem().sendAT(GF("+QMTOPEN="),id, GF(",\""), hostName, GF("\","), port);

    // Wait for OK
    if (thisModem().waitResponse(300L) != 1) {
      return -1;
    }

    // Wait for +QMTOPEN
    if (thisModem().waitResponse(15000L, GF("+QMTOPEN:")) != 1) {
      return -1;
    }

    int clientIndex = thisModem().streamGetIntBefore(',');
    int result = thisModem().streamGetIntLength(1);

    DBG(GF("clientIndex: "), clientIndex);
    DBG(GF("result: "), result);

    thisModem().streamSkipUntil('\n');  // The error code of the operation. If it is not

    return result;
  }

  int mqttCloseImpl(int mux)
  {
    mqtt_connected[mux] = 0;

    thisModem().sendAT(GF("+QMTCLOSE="),mux);

    // Wait for OK
    if (thisModem().waitResponse(300L) != 1) {
      return -1;
    }

    // Wait for +QMTCLOSE
    if (thisModem().waitResponse(300L, GF("+QMTCLOSE:")) != 1) {
      return -1;
    }

    int clientIndex = thisModem().streamGetIntBefore(',');
    int state = thisModem().streamGetIntLength(1);

    DBG(GF("clientIndex: "), clientIndex);
    DBG(GF("state: "), state);

    thisModem().streamSkipUntil('\n');  // The error code of the operation. If it is not

    return state;
  }

  int mqttConnectImpl(int mux, const char* clientId, const char* userName, const char* password)
  {
    thisModem().sendAT(GF("+QMTCONN="),mux, ",\"", clientId, "\",\"", userName, "\",\"", password, "\"");
    if (thisModem().waitResponse(5000L, GF("+QMTCONN:")) != 1) {
      return -1;
    }

    int clientIndex = thisModem().streamGetIntBefore(',');
    int state = thisModem().streamGetIntLength(1);

    DBG(GF("clientIndex: "), clientIndex);
    DBG(GF("state: "), state);

    thisModem().streamSkipUntil('\n');  // The error code of the operation. If it is not

    mqtt_connected[mux] = (state == 0) ? true : false;

    return state;
  }

  int mqttDisconnectImpl(int mux)
  {
    mqtt_connected[mux] = false;

    thisModem().sendAT(GF("+QMTDISC="),mux);

    // Wait for OK
    if (thisModem().waitResponse(300L) != 1) {
      return -1;
    }

    // Wait for +QMTDISCs
    if (thisModem().waitResponse(300L, GF("+QMTDISC:")) != 1) {
      return -1;
    }

    int clientIndex = thisModem().streamGetIntBefore(',');
    int result = thisModem().streamGetIntLength(1);

    DBG(GF("clientIndex: "), clientIndex);
    DBG(GF("result: "), result);

    thisModem().streamSkipUntil('\n');  // The error code of the operation. If it is not

    return result;
  }

  int mqttSubscribeImpl(int id, int msgId, const char* topic, int qos)
  {
    thisModem().sendAT(GF("+QMTSUB="),id, ",", msgId, ",\"", topic, "\",", qos);

    // Wait for OK
    if (thisModem().waitResponse(300L) != 1) {
      return -1;
    }

    // Wait for +QMTSUB
    if (thisModem().waitResponse(15000L, GF("+QMTSUB:")) != 1) {
      return -1;
    }

    int clientIndex = thisModem().streamGetIntBefore(',');
    int msgIdRsp = thisModem().streamGetIntBefore(',');
    int result = thisModem().streamGetIntLength(1);
    // Ignore optional value

    DBG(GF("clientIndex: "), clientIndex);
    DBG(GF("msgId: "), msgIdRsp);
    DBG(GF("result: "), result);

    thisModem().streamSkipUntil('\n');  // The error code of the operation. If it is not

    return result;
  }

  int mqttUnsubscribeImpl(int id, int msgId, const char *topic)
  {
    thisModem().sendAT(GF("+QMTUNS="),id, msgId, topic);

    // Wait for OK
    if (thisModem().waitResponse(300L) != 1) {
      return -1;
    }

    // Wait for +QMTUNS
    if (thisModem().waitResponse(15000L, GF("+QMTUNS:")) != 1) {
      return -1;
    }

    int clientIndex = thisModem().streamGetIntBefore(',');
    int msgIdRsp = thisModem().streamGetIntBefore(',');
    int result = thisModem().streamGetIntLength(1);
    
    DBG(GF("clientIndex: "), clientIndex);
    DBG(GF("msgId: "), msgIdRsp);
    DBG(GF("result: "), result);

    thisModem().streamSkipUntil('\n');  // The error code of the operation. If it is not

    return result;
  }

  int mqttPublishImpl(int mux, int msgId, int qos, bool retain,  const char* topic, const char *payload, int payloadLen)
  {
    thisModem().sendAT(GF("+QMTPUB="),mux, ",", msgId, ",", qos, ",", retain, ",\"", topic, "\",", payloadLen);
    if (thisModem().waitResponse(GF(">")) != 1) { return -1; }

    // TODO: This isn't going to work for payloads with 0's in them ?

    thisModem().stream.print(payload);  // Actually send the message
    thisModem().stream.write(static_cast<char>(0x1A));  // Terminate the message
    thisModem().stream.flush();

    // Wait for OK
    if (thisModem().waitResponse(300L) != 1) {
      return -1;
    }

    // Wait for +QMTPUB
    if (thisModem().waitResponse(60000L, GF("+QMTPUB:")) != 1) {
      return -1;
    }

    int clientIndex = thisModem().streamGetIntBefore(',');
    int msgIdRsp = thisModem().streamGetIntBefore(',');
    int result = thisModem().streamGetIntLength(1);

    DBG(GF("clientIndex: "), clientIndex);
    DBG(GF("msgId: "), msgIdRsp);
    DBG(GF("result: "), result);

    thisModem().streamSkipUntil('\n');  // The error code of the operation. If it is not

    return result;
  }

  int mqttLoopImpl(int mux)
  {
    // Response is handled in modem implementation
    thisModem().sendAT(GF("+QMTRECV="), mux);

    // TODO: +QMTRECV always seems to return an ERROR not OK ?
    thisModem().waitResponse();

    return 0;  
  }
};

#endif  // SRC_TINYGSMSMS_H_
