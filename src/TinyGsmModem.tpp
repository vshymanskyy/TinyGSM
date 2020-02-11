/**
 * @file       TinyGsmModem.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMMODEM_H_
#define SRC_TINYGSMMODEM_H_

#include "TinyGsmCommon.h"

template <class modemType>
class TinyGsmModem {
 public:
  /*
   * Basic functions
   */
  bool begin(const char* pin = NULL) {
    return thisModem().initImpl(pin);
  }
  bool init(const char* pin = NULL) {
    return thisModem().initImpl(pin);
  }
  template <typename... Args>
  void sendAT(Args... cmd) {
    thisModem().streamWrite("AT", cmd..., thisModem().gsmNL);
    thisModem().stream.flush();
    TINY_GSM_YIELD(); /* DBG("### AT:", cmd...); */
  }
  void setBaud(uint32_t baud) {
    thisModem().setBaudImpl(baud);
  }
  // Test response to AT commands
  bool testAT(uint32_t timeout_ms = 10000L) {
    return thisModem().testATImpl(timeout_ms);
  }

  // Asks for modem information via the V.25TER standard ATI command
  // NOTE:  The actual value and style of the response is quite varied
  String getModemInfo() {
    return thisModem().getModemInfoImpl();
  }
  // Gets the modem name (as it calls itself)
  String getModemName() {
    return thisModem().getModemNameImpl();
  }
  bool factoryDefault() {
    return thisModem().factoryDefaultImpl();
  }

  /*
   * Power functions
   */
  bool restart() {
    return thisModem().restartImpl();
  }
  bool poweroff() {
    return thisModem().powerOffImpl();
  }
  bool radioOff() {
    return thisModem().radioOffImpl();
  }
  bool sleepEnable(bool enable = true) {
    return thisModem().sleepEnableImpl(enable);
  }

  /*
   * Generic network functions
   */
  // RegStatus getRegistrationStatus() {}
  bool isNetworkConnected() {
    return thisModem().isNetworkConnectedImpl();
  }
  // Waits for network attachment
  bool waitForNetwork(uint32_t timeout_ms = 60000L) {
    return thisModem().waitForNetworkImpl(timeout_ms);
  }
  // Gets signal quality report
  int16_t getSignalQuality() {
    return thisModem().getSignalQualityImpl();
  }
  String getLocalIP() {
    return thisModem().getLocalIPImpl();
  }
  IPAddress localIP() {
    return thisModem().TinyGsmIpFromString(thisModem().getLocalIP());
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
   * Basic functions
   */
 protected:
  void setBaudImpl(uint32_t baud) {
    thisModem().sendAT(GF("+IPR="), baud);
    thisModem().waitResponse();
  }

  bool testATImpl(uint32_t timeout_ms = 10000L) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      thisModem().sendAT(GF(""));
      if (thisModem().waitResponse(200) == 1) { return true; }
      delay(100);
    }
    return false;
  }

  String getModemInfoImpl() {
    thisModem().sendAT(GF("I"));
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return ""; }
    // Do the replaces twice so we cover both \r and \r\n type endings
    res.replace("\r\nOK\r\n", "");
    res.replace("\rOK\r", "");
    res.replace("\r\n", " ");
    res.replace("\r", " ");
    res.trim();
    return res;
  }

  String getModemNameImpl() {
    thisModem().sendAT(GF("+CGMI"));
    String res1;
    if (thisModem().waitResponse(1000L, res1) != 1) { return "unknown"; }
    res1.replace("\r\nOK\r\n", "");
    res1.replace("\rOK\r", "");
    res1.trim();

    thisModem().sendAT(GF("+GMM"));
    String res2;
    if (thisModem().waitResponse(1000L, res2) != 1) { return "unknown"; }
    res1.replace("\r\nOK\r\n", "");
    res1.replace("\rOK\r", "");
    res2.trim();

    String name = res1 + String(' ') + res2;
    DBG("### Modem:", name);
    return name;
  }

  bool factoryDefaultImpl() {
    thisModem().sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    thisModem().waitResponse();
    thisModem().sendAT(GF("+IPR=0"));  // Auto-baud
    thisModem().waitResponse();
    thisModem().sendAT(GF("&W"));  // Write configuration
    return thisModem().waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  bool radioOffImpl() {
    thisModem().sendAT(GF("+CFUN=0"));
    if (thisModem().waitResponse(10000L) != 1) { return false; }
    delay(3000);
    return true;
  }

  bool sleepEnableImpl(bool enable = true) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 protected:
  // Gets the modem's registration status via CREG/CGREG/CEREG
  // CREG = Generic network registration
  // CGREG = GPRS service registration
  // CEREG = EPS registration for LTE modules
  int getRegistrationStatusXREG(const char* regCommand) {
    thisModem().sendAT('+', regCommand, '?');
    // check for any of the three for simplicity
    int resp = thisModem().waitResponse(GF("+CREG:"), GF("+CGREG:"),
                                        GF("+CEREG:"));
    if (resp != 1 && resp != 2 && resp != 3) { return -1; }
    thisModem().streamSkipUntil(','); /* Skip format (0) */
    int status = thisModem().streamGetInt('\n');
    thisModem().waitResponse();
    return status;
  }

  bool waitForNetworkImpl(uint32_t timeout_ms = 60000L) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      if (thisModem().isNetworkConnected()) { return true; }
      delay(250);
    }
    return false;
  }

  // Gets signal quality report according to 3GPP TS command AT+CSQ
  int16_t getSignalQualityImpl() {
    thisModem().sendAT(GF("+CSQ"));
    if (thisModem().waitResponse(GF("+CSQ:")) != 1) { return 99; }
    int res = thisModem().streamGetInt(',');
    thisModem().waitResponse();
    return res;
  }

  String getLocalIPImpl() {
    thisModem().sendAT(GF("+CGPADDR=1"));
    if (thisModem().waitResponse(GF("+CGPADDR:")) != 1) { return ""; }
    thisModem().streamSkipUntil(',');  // Skip context id
    String res = thisModem().stream.readStringUntil('\r');
    if (thisModem().waitResponse() != 1) { return ""; }
    return res;
  }

  static IPAddress TinyGsmIpFromString(const String& strIP) {
    int Parts[4] = {
        0,
    };
    int Part = 0;
    for (uint8_t i = 0; i < strIP.length(); i++) {
      char c = strIP[i];
      if (c == '.') {
        Part++;
        if (Part > 3) { return IPAddress(0, 0, 0, 0); }
        continue;
      } else if (c >= '0' && c <= '9') {
        Parts[Part] *= 10;
        Parts[Part] += c - '0';
      } else {
        if (Part == 3) break;
      }
    }
    return IPAddress(Parts[0], Parts[1], Parts[2], Parts[3]);
  }

  /*
   Utilities
   */
 protected:
  // Utility templates for writing/skipping characters on a stream
  template <typename T>
  void inline streamWrite(T last) {
    thisModem().stream.print(last);
  }

  template <typename T, typename... Args>
  void inline streamWrite(T head, Args... tail) {
    thisModem().stream.print(head);
    thisModem().streamWrite(tail...);
  }

  int16_t inline streamGetInt(char lastChar) {
    char   buf[6];
    size_t bytesRead = thisModem().stream.readBytesUntil(
        lastChar, buf, static_cast<size_t>(6));
    if (bytesRead) {
      int16_t res = atoi(buf);
      return res;
    } else {
      return -9999;
    }
  }

  float inline streamGetFloat(char lastChar) {
    char   buf[12];
    size_t bytesRead = thisModem().stream.readBytesUntil(
        lastChar, buf, static_cast<size_t>(12));
    if (bytesRead) {
      float res = atof(buf);
      return res;
    } else {
      return static_cast<float>(-9999);
    }
  }

  bool inline streamSkipUntil(const char c, const uint32_t timeout_ms = 1000L) {
    uint32_t startMillis = millis();
    while (millis() - startMillis < timeout_ms) {
      while (millis() - startMillis < timeout_ms &&
             !thisModem().stream.available()) {
        TINY_GSM_YIELD();
      }
      if (thisModem().stream.read() == c) { return true; }
    }
    return false;
  }

  void inline streamClear() {
    while (thisModem().stream.available()) {
      thisModem().waitResponse(50, NULL, NULL);
    }
  }
};

#endif  // SRC_TINYGSMMODEM_H_
