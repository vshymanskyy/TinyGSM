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

#ifndef AT_NL
#define AT_NL "\r\n"
#endif

#ifndef AT_OK
#define AT_OK "OK"
#endif

#ifndef AT_ERROR
#define AT_ERROR "ERROR"
#endif

#if defined TINY_GSM_DEBUG
#ifndef AT_VERBOSE
#define AT_VERBOSE "+CME ERROR:"
#endif

#ifndef AT_VERBOSE_2
#define AT_VERBOSE_2 "+CMS ERROR:"
#endif
#endif

#ifndef MODEM_MANUFACTURER
#define MODEM_MANUFACTURER "unknown"
#endif

#ifndef MODEM_MODEL
#define MODEM_MODEL "unknown"
#endif

static const char GSM_OK[] TINY_GSM_PROGMEM    = AT_OK AT_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = AT_ERROR AT_NL;

#if defined       TINY_GSM_DEBUG
static const char GSM_VERBOSE[] TINY_GSM_PROGMEM   = AT_VERBOSE;
static const char GSM_VERBOSE_2[] TINY_GSM_PROGMEM = AT_VERBOSE_2;
#endif

template <class modemType>
class TinyGsmModem {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor basic_functions
   * @name Basic functions
   */
  /**@{*/

  /**
   * @brief Sets up the GSM module
   *
   * @param pin A pin code to unlock the SIM, if necessary
   *
   * @return *true* The module was set up as expected
   * @return *false* Something failed in module set up
   */
  bool begin(const char* pin = nullptr) {
    return thisModem().initImpl(pin);
  }
  /**
   * @copydoc TinyGsmModem::begin()
   */
  bool init(const char* pin = nullptr) {
    return thisModem().initImpl(pin);
  }

  /**
   * @brief Recursive variadic template to send AT commands
   *
   * @tparam Args
   * @param cmd The commands to send
   */
  template <typename... Args>
  inline void sendAT(Args... cmd) {
    thisModem().streamWrite("AT", cmd..., AT_NL);
    thisModem().stream.flush();
    TINY_GSM_YIELD(); /* DBG("### AT:", cmd...); */
  }

  /**
   * @brief Set the module baud rate
   *
   * @param baud The baud rate the use
   *
   * @note After setting and applying the new baud rate, you will have to end()
   * and begin() the serial object.
   */
  bool setBaud(uint32_t baud) {
    return thisModem().setBaudImpl(baud);
  }

  /**
   * @brief Test response to AT commands
   *
   * @param timeout_ms The the amount of time to test for; optional with a
   * default value of 10s.
   * @return *true*  The module responeded to AT commands
   * @return *false*  The module failed to respond
   */
  bool testAT(uint32_t timeout_ms = 10000L) {
    return thisModem().testATImpl(timeout_ms);
  }

  /**
   * @brief Listen for responses to commands and handle URCs
   *
   * @param timeout_ms The time to wait for a response
   * @param data A string of data to fill in with response results
   * @param r1 The first output to test against, optional with a default value
   * of "OK"
   * @param r2 The second output to test against, optional with a default value
   * of "ERROR"
   * @param r3 The third output to test against, optional with a default value
   * of NULL
   * @param r4 The fourth output to test against, optional with a default value
   * of NULL
   * @param r5 The fifth output to test against, optional with a default value
   * of NULL
   * @param r6 The sixth output to test against, optional with a default value
   * of NULL
   * @param r7 The seventh output to test against, optional with a default value
   * of NULL
   * @return *int8_t* the index of the response input
   */
  int8_t waitResponse(uint32_t timeout_ms, String& data,
                      GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR), GsmConstStr r3 = nullptr,
                      GsmConstStr r4 = nullptr, GsmConstStr r5 = nullptr,
                      GsmConstStr r6 = nullptr, GsmConstStr r7 = nullptr) {
    return thisModem().waitResponseImpl(timeout_ms, data, r1, r2, r3, r4, r5,
                                        r6, r7);
  }

  /**
   * @brief Listen for responses to commands and handle URCs
   *
   * @param timeout_ms The time to wait for a response
   * @param r1 The first output to test against, optional with a default value
   * of "OK"
   * @param r2 The second output to test against, optional with a default value
   * of "ERROR"
   * @param r3 The third output to test against, optional with a default value
   * of NULL
   * @param r4 The fourth output to test against, optional with a default value
   * of NULL
   * @param r5 The fifth output to test against, optional with a default value
   * of NULL
   * @param r6 The sixth output to test against, optional with a default value
   * of NULL
   * @param r7 The seventh output to test against, optional with a default value
   * of NULL
   * @return *int8_t* the index of the response input
   */
  int8_t waitResponse(uint32_t timeout_ms, GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR), GsmConstStr r3 = nullptr,
                      GsmConstStr r4 = nullptr, GsmConstStr r5 = nullptr,
                      GsmConstStr r6 = nullptr, GsmConstStr r7 = nullptr) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5, r6, r7);
  }

  /**
   * @brief Listen for responses to commands and handle URCs; listening for 1
   * second.
   *
   * @param r1 The first output to test against, optional with a default value
   * of "OK"
   * @param r2 The second output to test against, optional with a default value
   * of "ERROR"
   * @param r3 The third output to test against, optional with a default value
   * of NULL
   * @param r4 The fourth output to test against, optional with a default value
   * of NULL
   * @param r5 The fifth output to test against, optional with a default value
   * of NULL
   * @param r6 The sixth output to test against, optional with a default value
   * of NULL
   * @param r7 The seventh output to test against, optional with a default value
   * of NULL
   * @return *int8_t* the index of the response input
   */
  int8_t waitResponse(GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR), GsmConstStr r3 = nullptr,
                      GsmConstStr r4 = nullptr, GsmConstStr r5 = nullptr,
                      GsmConstStr r6 = nullptr, GsmConstStr r7 = nullptr) {
    return waitResponse(1000L, r1, r2, r3, r4, r5, r6, r7);
  }

  /**
   * @brief Asks for modem information via the 3GPP TS 27.007 standard ATI
   * command
   *
   * @note  The actual value and style of the response is quite varied
   * @return *String* Some info about the GSM module.
   */
  String getModemInfo() {
    return thisModem().getModemInfoImpl();
  }

  /**
   * @brief Get the modem name - a combination of the manufacturer and model, as
   * the modem calls itself
   *
   * @return *String*  The modem name
   */
  String getModemName() {
    return thisModem().getModemNameImpl();
  }

  /**
   * @brief Get the modem manufacturer
   *
   * @return *String* The modem manufacturer
   */
  String getModemManufacturer() {
    return thisModem().getModemManufacturerImpl();
  }

  /**
   * @brief Get the modem model
   *
   * @return *String* The modem model, as it calls itself
   */
  String getModemModel() {
    return thisModem().getModemModelImpl();
  }

  /**
   * @brief Get the modem revision information.
   *
   * What is returned as the revision may be either a hardware or a firmware
   * version or some combination of both.
   *
   * @return *String* The modem revision information
   */
  String getModemRevision() {
    return thisModem().getModemRevisionImpl();
  }

  /**
   * @brief Get the modem serial number
   *
   * @return *String* The modem serial number
   */
  String getModemSerialNumber() {
    return thisModem().getModemSerialNumberImpl();
  }

  /**
   * @brief Reset the module to factory defaults.
   *
   * This generally restarts the module as well.
   *
   * @return *true* The module successfully reset to default.
   * @return *false* The module failed to reset to default.
   */
  bool factoryDefault() {
    return thisModem().factoryDefaultImpl();
  }
  /**@}*/

  /**
   * @anchor power_functions
   * @name Power functions
   */
  /**@{*/

  /**
   * @brief Restart the module
   *
   * @param pin A pin code to unlock the SIM, if necessary
   *
   * @return *true* The module was successfully restarted.
   * @return *false* There was an error in restarting the module.
   */
  bool restart(const char* pin = nullptr) {
    return thisModem().restartImpl(pin);
  }
  /**
   * @brief Power off the module
   *
   * @return *true* The module was successfully powered down.
   * @return *false* There was an error in powering down module.
   */
  bool poweroff() {
    return thisModem().powerOffImpl();
  }
  /**
   * @brief Turn off the module radio
   *
   * @return *true* The module radio was successfully turned off.
   * @return *false* There was an error in turning off the radio.
   */
  bool radioOff() {
    return thisModem().radioOffImpl();
  }

  /**
   * @brief Enable sleep on the module.
   *
   * For some modules this immediately puts
   * the module to sleep, for others this sets them to be able to sleep based on
   * pin levels.
   *
   * @param enable True to enable sleep, false to disable
   * @return *true* Sleep was successfully enabled or disabled
   * @return *false* There was a problem setting sleep
   */
  bool sleepEnable(bool enable = true) {
    return thisModem().sleepEnableImpl(enable);
  }

  /**
   * @brief Set the phone functionality
   *
   * @param fun The phone functionality setting. The value and meaning of this
   * varies by module; check your documentation.
   * @param reset True to reset the module before changing the functionality.
   * @return *true* The phone functionalilty was successfully changed.
   * @return *false* There was a problem changing the functionality.
   */
  bool setPhoneFunctionality(uint8_t fun, bool reset = false) {
    return thisModem().setPhoneFunctionalityImpl(fun, reset);
  }
  /**@}*/

  /**
   * @anchor network_functions
   * @name Generic Network Functions
   */
  /**@{*/

  // RegStatus getRegistrationStatus() {}

  /**
   * @brief Confirm whether the module is currently connected to the
   * GSM/GPRS/LTE network.
   *
   * @return *true* The module is connected to the network
   * @return *false* The module is not connected to the network
   */
  bool isNetworkConnected() {
    return thisModem().isNetworkConnectedImpl();
  }

  /**
   * @brief Wait until the module has connected to the network
   *
   * @param timeout_ms The time to wait for attachment in milliseconds. Optional
   * with a default value of 1 minute.
   * @param check_signal True to alternate between checking for connection and
   * checking the signal strength.
   * @return *true* The module is now connected to the network.
   * @return *false* The module did not connect to the network even after
   * waiting.
   */
  bool waitForNetwork(uint32_t timeout_ms = 60000L, bool check_signal = false) {
    return thisModem().waitForNetworkImpl(timeout_ms, check_signal);
  }

  /**
   * @brief Get the signal quality report
   *
   * This is often a "CSQ" value ranging from 0 to 32, but may be an RSSI or a
   * percent.
   *
   * @return *int16_t* The signal quality
   */
  int16_t getSignalQuality() {
    return thisModem().getSignalQualityImpl();
  }

  /**
   * @brief Get the Local IP address assigned to the module by the network as a
   * String
   *
   * @return *String* The local IP address
   */
  String getLocalIP() {
    return thisModem().getLocalIPImpl();
  }

  /**
   * @brief Get the Local IP address assigned to the module by the network as an
   * IPAddress object.
   *
   * @return *IPAddress* The local IP address
   */
  IPAddress localIP() {
    return thisModem().TinyGsmIpFromString(thisModem().getLocalIP());
  }
  /**@}*/

  /**
   * @anchor crtp_helper
   * @name CRTP Helper
   */
  /**@{*/
 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }
  /**@}*/
  ~TinyGsmModem() {}


  /**
   * @anchor modem_utilities
   * @name Utilities
   */
  /**@{*/
 public:
  // Utility templates for writing/skipping characters on a stream
  template <typename T>
  inline void streamWrite(T last) {
    thisModem().stream.print(last);
  }

  template <typename T, typename... Args>
  inline void streamWrite(T head, Args... tail) {
    thisModem().stream.print(head);
    thisModem().streamWrite(tail...);
  }

  inline void streamClear() {
    while (thisModem().stream.available()) {
      thisModem().waitResponse(50, nullptr, nullptr);
    }
  }

 protected:
  inline bool streamGetLength(char* buf, int8_t numChars,
                              const uint32_t timeout_ms = 1000L) {
    if (!buf) { return false; }

    int8_t   numCharsReady = -1;
    uint32_t startMillis   = millis();
    while (millis() - startMillis < timeout_ms &&
           (numCharsReady = thisModem().stream.available()) < numChars) {
      TINY_GSM_YIELD();
    }

    if (numCharsReady >= numChars) {
      thisModem().stream.readBytes(buf, numChars);
      return true;
    }

    return false;
  }

  inline int16_t streamGetIntLength(int8_t         numChars,
                                    const uint32_t timeout_ms = 1000L) {
    char buf[numChars + 1];
    if (streamGetLength(buf, numChars, timeout_ms)) {
      buf[numChars] = '\0';
      return atoi(buf);
    }

    return -9999;
  }

  inline int16_t streamGetIntBefore(char lastChar) {
    char   buf[7];
    size_t bytesRead = thisModem().stream.readBytesUntil(
        lastChar, buf, static_cast<size_t>(7));
    // if we read 7 or more bytes, it's an overflow
    if (bytesRead && bytesRead < 7) {
      buf[bytesRead] = '\0';
      int16_t res    = atoi(buf);
      return res;
    }

    return -9999;
  }

  inline float streamGetFloatLength(int8_t         numChars,
                                    const uint32_t timeout_ms = 1000L) {
    char buf[numChars + 1];
    if (streamGetLength(buf, numChars, timeout_ms)) {
      buf[numChars] = '\0';
      return atof(buf);
    }

    return -9999.0F;
  }

  inline float streamGetFloatBefore(char lastChar) {
    char   buf[16];
    size_t bytesRead = thisModem().stream.readBytesUntil(
        lastChar, buf, static_cast<size_t>(16));
    // if we read 16 or more bytes, it's an overflow
    if (bytesRead && bytesRead < 16) {
      buf[bytesRead] = '\0';
      float res      = atof(buf);
      return res;
    }

    return -9999.0F;
  }

  inline bool streamSkipUntil(const char c, const uint32_t timeout_ms = 1000L) {
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

  inline void cleanResponseString(String& res) {
    // Do the replaces twice so we cover both \r and \r\n type endings
    res.replace("\r\nOK\r\n", "");
    res.replace("\rOK\r", "");
    res.replace("\r\n", " ");
    res.replace("\r", " ");
    res.trim();
  }

  static inline IPAddress TinyGsmIpFromString(const String& strIP) {
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
  /**@}*/

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Basic functions
   */
 protected:
  bool initImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool setBaudImpl(uint32_t baud) {
    thisModem().sendAT(GF("+IPR="), baud);
    return thisModem().waitResponse() == 1;
  }

  bool testATImpl(uint32_t timeout_ms = 10000L) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      thisModem().sendAT(GF(""));
      if (thisModem().waitResponse(200) == 1) { return true; }
      delay(100);
    }
    return false;
  }

  // TODO(vshymanskyy): Optimize this!
  int8_t waitResponseImpl(uint32_t timeout_ms, String& data,
                          GsmConstStr r1 = GFP(GSM_OK),
                          GsmConstStr r2 = GFP(GSM_ERROR),
                          GsmConstStr r3 = nullptr, GsmConstStr r4 = nullptr,
                          GsmConstStr r5 = nullptr, GsmConstStr r6 = nullptr,
                          GsmConstStr r7 = nullptr) {
    data.reserve(64);

#ifdef TINY_GSM_DEBUG_DEEP
    DBG(GF("r1 <"), r1 ? r1 : GF("NULL"), GF("> r2 <"), r2 ? r2 : GF("NULL"),
        GF("> r3 <"), r3 ? r3 : GF("NULL"), GF("> r4 <"), r4 ? r4 : GF("NULL"),
        GF("> r5 <"), r5 ? r5 : GF("NULL"), GF("> r6 <"), r6 ? r6 : GF("NULL"),
        GF("> r7 <"), r7 ? r7 : GF("NULL"), '>');
#endif
    uint8_t  index       = 0;
    uint32_t startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (thisModem().stream.available() > 0) {
        TINY_GSM_YIELD();
        int8_t a = thisModem().stream.read();
        if (a <= 0) continue;  // Skip 0x00 bytes, just in case
        data += static_cast<char>(a);
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
          index = 2;
          goto finish;
        } else if (r3 && data.endsWith(r3)) {
          index = 3;
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (r6 && data.endsWith(r6)) {
          index = 6;
          goto finish;
        } else if (r7 && data.endsWith(r7)) {
          index = 7;
          goto finish;
        }
#if defined TINY_GSM_DEBUG
        else if (data.endsWith(GFP(GSM_VERBOSE)) ||
                 data.endsWith(GFP(GSM_VERBOSE_2))) {
          // check how long the new line is
          // should be either 1 ('\r' or '\n') or 2 ("\r\n"))
          int len_atnl = strnlen(AT_NL, 3);
          // Read out the verbose message, until the last character of the new
          // line
          data += thisModem().stream.readStringUntil(AT_NL[len_atnl]);
#ifdef TINY_GSM_DEBUG_DEEP
          data.trim();
          DBG(GF("Verbose details <<<"), data, GF(">>>"));
#endif
          data = "";
          goto finish;
        }
#endif
        else if (thisModem().handleURCs(data)) {
          data = "";
        }
      }
    } while (millis() - startMillis < timeout_ms);
  finish:
#ifdef TINY_GSM_DEBUG_DEEP
    data.replace("\r", "←");
    data.replace("\n", "↓");
#endif
    if (!index) {
      data.trim();
      if (data.length()) { DBG("### Unhandled:", data); }
      data = "";
    } else {
#ifdef TINY_GSM_DEBUG_DEEP
      DBG('<', index, '>', data);
#endif
    }
    return index;
  }


  String getModemInfoImpl() {
    thisModem().sendAT(GF("I"));  // 3GPP TS 27.007
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return ""; }
    thisModem().cleanResponseString(res);
    return res;
  }

  String getModemNameImpl() {
    String manufacturer = getModemManufacturer();
    String model        = getModemModel();
    String name         = manufacturer + String(" ") + model;
    DBG("### Modem:", name);
    return name;
  }

  // Gets the modem manufacturer
  String getModemManufacturerImpl() {
    String manufacturer = MODEM_MANUFACTURER;
    thisModem().sendAT(GF("+CGMI"));  // 3GPP TS 27.007 standard
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return manufacturer; }
    thisModem().cleanResponseString(res);
    return res;
  }

  // Gets the modem hardware version
  String getModemModelImpl() {
    String model = MODEM_MODEL;
    thisModem().sendAT(GF("+CGMM"));  // 3GPP TS 27.007 standard
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return model; }
    thisModem().cleanResponseString(res);
    return res;
  }

  // Gets the modem firmware version
  String getModemRevisionImpl() {
    thisModem().sendAT(GF("+CGMR"));  // 3GPP TS 27.007 standard
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return "unknown"; }
    thisModem().cleanResponseString(res);
    return res;
  }

  // Gets the modem serial number
  String getModemSerialNumberImpl() {
    thisModem().sendAT(GF("+CGSN"));  // 3GPP TS 27.007 standard
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return "unknown"; }
    thisModem().cleanResponseString(res);
    return res;
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
    if (!thisModem().setPhoneFunctionality(0)) { return false; }
    delay(3000);
    return true;
  }

  bool sleepEnableImpl(bool enable = true) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false)
      TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 protected:
  // Gets the modem's registration status via CREG/CGREG/CEREG
  // CREG = Generic network registration
  // CGREG = GPRS service registration
  // CEREG = EPS registration for LTE modules
  int8_t getRegistrationStatusXREG(const char* regCommand) {
    thisModem().sendAT('+', regCommand, '?');
    // check for any of the three for simplicity
    int8_t resp = thisModem().waitResponse(GF("+CREG:"), GF("+CGREG:"),
                                           GF("+CEREG:"));
    if (resp != 1 && resp != 2 && resp != 3) { return -1; }
    thisModem().streamSkipUntil(','); /* Skip format (0) */
    int status = thisModem().stream.parseInt();
    thisModem().waitResponse();
    return status;
  }

  bool waitForNetworkImpl(uint32_t timeout_ms   = 60000L,
                          bool     check_signal = false) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      if (check_signal) { thisModem().getSignalQuality(); }
      if (thisModem().isNetworkConnected()) { return true; }
      delay(250);
    }
    return false;
  }

  // Gets signal quality report according to 3GPP TS command AT+CSQ
  int8_t getSignalQualityImpl() {
    thisModem().sendAT(GF("+CSQ"));
    if (thisModem().waitResponse(GF("+CSQ:")) != 1) { return 99; }
    int8_t res = thisModem().streamGetIntBefore(',');
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
};

#endif  // SRC_TINYGSMMODEM_H_
