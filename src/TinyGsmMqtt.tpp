/**
 * @file       TinyGsmMqtt.tpp
 * @author     Matteo Murtas
 * @license    TODO
 * @copyright  Copyright (c) 2024 Matteo Murtas
 * @date       Jul 2024
 */

#ifndef SRC_TINYGSMMQTT_H_
#define SRC_TINYGSMMQTT_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_MQTT


#if defined(ESP8266) || defined(ESP32)
#include <functional>
#define MQTT_CALLBACK(f) std::function<void(char*, uint8_t*, unsigned int)> f
#else
#define MQTT_CALLBACK(f) void (*f)(char*, uint8_t*, unsigned int)
#endif

template <class modemType>
class TinyGsmMqtt {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * MQTT functions
   */


  bool mqttSetBroker(String host, uint16_t port) {
    mqtt_host = host;
    mqtt_port = port;
    return thisModem().mqttSetBrokerImpl(host, port);
  }

  bool mqttSetClientId(String id) {
    mqtt_client_id = id;
    return thisModem().mqttSetClientIdImpl(id);
  }

  bool mqttSetKeepAlive(uint16_t timeout) {
    return thisModem().mqttSetKeepAliveImpl(timeout);
  }

  bool mqttConnect() {
    return thisModem().mqttConnectImpl();
  }

  void mqttDisconnect() {
    thisModem().mqttDisconnectImpl();
  }

  bool mqttIsConnected() {
    return thisModem().mqttIsConnectedImpl();
  }

  bool mqttPublish(const char* topic, const uint8_t * payload, unsigned int plength) {
    return thisModem().mqttPublishImpl(topic, payload, plength);
  }
  bool mqttSubscribe(const char* topic, uint8_t qos) {
    return thisModem().mqttSubscribeImpl(topic, qos);
  }
  bool mqttUnsubscribe(const char* topic) {
    return thisModem().mqttUnsubscribeImpl(topic);
  }

  void mqttLoop()  {
    thisModem().mqttLoopImpl();
  }
  
  void mqttSetCallback(MQTT_CALLBACK(cb)) {
    mqtt_callback = cb;
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
  ~TinyGsmMqtt() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */
  String   mqtt_host;
  uint16_t mqtt_port;
  String   mqtt_client_id;
  MQTT_CALLBACK(mqtt_callback);
};

#endif  // SRC_TINYGSMTEMPERATURE_H_
