/**
 * @file       TinyGsmCommon.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCOMMON_H_
#define SRC_TINYGSMCOMMON_H_

// The current library version number
#define TINYGSM_VERSION "0.10.0"

#if defined(SPARK) || defined(PARTICLE)
#include "Particle.h"
#elif defined(ARDUINO)
#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif
#endif

#if defined(ARDUINO_DASH)
#include <ArduinoCompat/Client.h>
#else
#include <Client.h>
#endif

#include "TinyGsmFifo.h"

#ifndef TINY_GSM_YIELD_MS
#define TINY_GSM_YIELD_MS 0
#endif

#ifndef TINY_GSM_YIELD
#define TINY_GSM_YIELD() \
  { delay(TINY_GSM_YIELD_MS); }
#endif

#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 64
#endif

#define TINY_GSM_ATTR_NOT_AVAILABLE \
  __attribute__((error("Not available on this modem type")))
#define TINY_GSM_ATTR_NOT_IMPLEMENTED __attribute__((error("Not implemented")))

#if defined(__AVR__)
#define TINY_GSM_PROGMEM PROGMEM
typedef const __FlashStringHelper* GsmConstStr;
#define GFP(x) (reinterpret_cast<GsmConstStr>(x))
#define GF(x) F(x)
#else
#define TINY_GSM_PROGMEM
typedef const char* GsmConstStr;
#define GFP(x) x
#define GF(x) x
#endif

#ifdef TINY_GSM_DEBUG
namespace {
template <typename T>
static void DBG_PLAIN(T last) {
  TINY_GSM_DEBUG.println(last);
}

template <typename T, typename... Args>
static void DBG_PLAIN(T head, Args... tail) {
  TINY_GSM_DEBUG.print(head);
  TINY_GSM_DEBUG.print(' ');
  DBG_PLAIN(tail...);
}

template <typename... Args>
static void DBG(Args... args) {
  TINY_GSM_DEBUG.print(GF("["));
  TINY_GSM_DEBUG.print(millis());
  TINY_GSM_DEBUG.print(GF("] "));
  DBG_PLAIN(args...);
}
}  // namespace
#else
#define DBG_PLAIN(...)
#define DBG(...)
#endif

template <class T>
const T& TinyGsmMin(const T& a, const T& b) {
  return (b < a) ? b : a;
}

template <class T>
const T& TinyGsmMax(const T& a, const T& b) {
  return (b < a) ? a : b;
}

template <class T>
uint32_t TinyGsmAutoBaud(T& SerialAT, uint32_t minimum = 9600,
                         uint32_t maximum = 115200) {
  static uint32_t rates[] = {115200, 57600,  38400, 19200, 9600,  74400, 74880,
                             230400, 460800, 2400,  4800,  14400, 28800};

  for (unsigned i = 0; i < sizeof(rates) / sizeof(rates[0]); i++) {
    uint32_t rate = rates[i];
    if (rate < minimum || rate > maximum) continue;

    DBG("Trying baud rate", rate, "...");
    SerialAT.begin(rate);
    delay(10);
    for (int j = 0; j < 10; j++) {
      SerialAT.print("AT\r\n");
      String input = SerialAT.readString();
      if (input.indexOf("OK") >= 0) {
        DBG("Modem responded at rate", rate);
        return rate;
      }
    }
  }
  SerialAT.begin(minimum);
  return 0;
}

enum modemInternalBuffferType {
  NO_MODEM_BUFFER =
      0,  // For modules that do not store incoming data in any sort of buffer
  READ_NO_CHECK = 1,  // Data is stored in a buffer, but we can only read from
                      // the buffer, not check how much data is stored in it
  READ_AND_CHECK_SIZE = 2,  // Data is stored in a buffer and we can both read
                            // and check the size of the buffer
};

enum SimStatus {
  SIM_ERROR            = 0,
  SIM_READY            = 1,
  SIM_LOCKED           = 2,
  SIM_ANTITHEFT_LOCKED = 3,
};

enum TinyGSMDateTimeFormat { DATE_FULL = 0, DATE_TIME = 1, DATE_DATE = 2 };

template <class modemType, modemInternalBuffferType bufType, uint8_t muxCount>
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
  void maintain() {
    return thisModem().maintainImpl();
  }
  bool factoryDefault() {
    return thisModem().factoryDefaultImpl();
  }
  bool hasSSL() {
    return thisModem().thisHasSSL();
  }
  bool hasWifi() {
    return thisModem().thisHasWifi();
  }
  bool hasGPRS() {
    return thisModem().thisHasGPRS();
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
   * SIM card functions
   */
  // Unlocks the SIM
  bool simUnlock(const char* pin) {
    return thisModem().simUnlockImpl(pin);
  }
  // Gets the CCID of a sim card via AT+CCID
  String getSimCCID() {
    return thisModem().getSimCCIDImpl();
  }
  // Asks for TA Serial Number Identification (IMEI)
  String getIMEI() {
    return thisModem().getIMEIImpl();
  }
  SimStatus getSimStatus(uint32_t timeout_ms = 10000L) {
    return thisModem().getSimStatusImpl(timeout_ms);
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

  /*
   * GPRS functions
   */
  bool gprsConnect(const char* apn, const char* user = NULL,
                   const char* pwd = NULL) {
    return thisModem().gprsConnectImpl(apn, user, pwd);
  }
  bool gprsDisconnect() {
    return thisModem().gprsDisconnectImpl();
  }
  // Checks if current attached to GPRS/EPS service
  bool isGprsConnected() {
    return thisModem().isGprsConnectedImpl();
  }
  // Gets the current network operator
  String getOperator() {
    return thisModem().getOperatorImpl();
  }

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
   * GPRS functions
   */
  String getLocalIP() {
    return thisModem().getLocalIPImpl();
  }
  IPAddress localIP() {
    return thisModem().TinyGsmIpFromString(thisModem().getLocalIP());
  }

  /*
   * Phone Call functions
   */
  bool callAnswer() {
    return thisModem().callAnswerImpl();
  }
  bool callNumber(const String& number) {
    return thisModem().callNumberImpl(number);
  }
  bool callHangup() {
    return thisModem().callHangupImpl();
  }
  bool dtmfSend(char cmd, int duration_ms = 100) {
    return thisModem().dtmfSendImpl(cmd, duration_ms);
  }

  /*
   * Messaging functions
   */
  String sendUSSD(const String& code) {
    return thisModem().sendUSSDImpl(code);
  }
  bool sendSMS(const String& number, const String& text) {
    return thisModem().sendSMSImpl(number, text);
  }
  bool sendSMS_UTF16(const char* const number, const void* text, size_t len) {
    return thisModem().sendSMS_UTF16Impl(number, text, len);
  }

  /*
   * Location functions
   */
  String getGsmLocation() {
    return thisModem().getGsmLocationImpl();
  }

  /*
   * GPS location functions
   */
  // No template interface or implementation of these functions

  /*
   * Time functions
   */
  String getGSMDateTime(TinyGSMDateTimeFormat format) {
    return thisModem().getGSMDateTimeImpl(format);
  }

  /*
   * Battery & temperature functions
   */
  uint16_t getBattVoltage() {
    return thisModem().getBattVoltageImpl();
  }
  int8_t getBattPercent() {
    return thisModem().getBattPercentImpl();
  }
  uint8_t getBattChargeState() {
    return thisModem().getBattChargeStateImpl();
  }
  bool getBattStats(uint8_t& chargeState, int8_t& percent,
                    uint16_t& milliVolts) {
    return thisModem().getBattStatsImpl(chargeState, percent, milliVolts);
  }
  float getTemperature() {
    return thisModem().getTemperatureImpl();
  }

  /*
   * CRTP Helper
   */
 protected:
  const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }

  /*
   * Inner Client
   */
 public:
  class GsmClient : public Client {
    // Make all classes created from the modem template friends
    friend class TinyGsmModem<modemType, bufType, muxCount>;
    typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

   public:
    // bool init(modemType* modem, uint8_t);
    // int connect(const char* host, uint16_t port, int timeout_s);

    // Connect to a IP address given as an IPAddress object by
    // converting said IP address to text
    // int connect(IPAddress ip, uint16_t port, int timeout_s) {
    //   return connect(TinyGsmStringFromIp(ip).c_str(), port,
    //   timeout_s);
    // }
    // int connect(const char* host, uint16_t port) override {
    //   return connect(host, port, 75);
    // }
    // int connect(IPAddress ip, uint16_t port) override {
    //   return connect(ip, port, 75);
    // }

    static String TinyGsmStringFromIp(IPAddress ip) {
      String host;
      host.reserve(16);
      host += ip[0];
      host += ".";
      host += ip[1];
      host += ".";
      host += ip[2];
      host += ".";
      host += ip[3];
      return host;
    }

    // void stop(uint32_t maxWaitMs);
    // void stop() override {
    //   stop(15000L);
    // }

    // Writes data out on the client using the modem send functionality
    size_t write(const uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      at->maintain();
      return at->modemSend(buf, size, mux);
    }

    size_t write(uint8_t c) override {
      return write(&c, 1);
    }

    size_t write(const char* str) {
      if (str == NULL) return 0;
      return write((const uint8_t*)str, strlen(str));
    }

    int available() override {
      TINY_GSM_YIELD();
      switch (bufType) {
        // Returns the number of characters available in the TinyGSM fifo
        case NO_MODEM_BUFFER:
          if (!rx.size() && sock_connected) { at->maintain(); }
          return rx.size();

        // Returns the combined number of characters available in the TinyGSM
        // fifo and the modem chips internal fifo.
        case READ_NO_CHECK:
          if (!rx.size()) { at->maintain(); }
          return rx.size() + sock_available;

        // Returns the combined number of characters available in the TinyGSM
        // fifo and the modem chips internal fifo, doing an extra check-in
        // with the modem to see if anything has arrived without a UURC.
        case READ_AND_CHECK_SIZE:
          if (!rx.size()) {
            if (millis() - prev_check > 500) {
              got_data   = true;
              prev_check = millis();
            }
            at->maintain();
          }
          return rx.size() + sock_available;
      }
    }

    int read(uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      size_t   cnt          = 0;
      uint32_t _startMillis = millis();

      switch (bufType) {
        // Reads characters out of the TinyGSM fifo, waiting for any URC's
        // from the modem for new data if there's nothing in the fifo.
        case NO_MODEM_BUFFER:
          while (cnt < size && millis() - _startMillis < _timeout) {
            size_t chunk = TinyGsmMin(size - cnt, rx.size());
            if (chunk > 0) {
              rx.get(buf, chunk);
              buf += chunk;
              cnt += chunk;
              continue;
            } /* TODO: Read directly into user buffer? */
            if (!rx.size() && sock_connected) { at->maintain(); }
          }
          return cnt;

        // Reads characters out of the TinyGSM fifo, and from the modem chip's
        // internal fifo if avaiable.
        case READ_NO_CHECK:
          at->maintain();
          while (cnt < size) {
            size_t chunk = TinyGsmMin(size - cnt, rx.size());
            if (chunk > 0) {
              rx.get(buf, chunk);
              buf += chunk;
              cnt += chunk;
              continue;
            } /* TODO: Read directly into user buffer? */
            at->maintain();
            if (sock_available > 0) {
              int n = at->modemRead(
                  TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
              if (n == 0) break;
            } else {
              break;
            }
          }
          return cnt;

        // Reads characters out of the TinyGSM fifo, and from the modem chips
        // internal fifo if avaiable, also double checking with the modem if
        // data has arrived without issuing a UURC.
        case READ_AND_CHECK_SIZE:
          at->maintain();
          while (cnt < size) {
            size_t chunk = TinyGsmMin(size - cnt, rx.size());
            if (chunk > 0) {
              rx.get(buf, chunk);
              buf += chunk;
              cnt += chunk;
              continue;
            }
            // Workaround: Some modules "forget" to notify about data arrival
            if (millis() - prev_check > 500) {
              got_data   = true;
              prev_check = millis();
            }
            // TODO(vshymanskyy): Read directly into user buffer?
            at->maintain();
            if (sock_available > 0) {
              int n = at->modemRead(
                  TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
              if (n == 0) break;
            } else {
              break;
            }
          }
          return cnt;
      }
    }

    int read() override {
      uint8_t c;
      if (read(&c, 1) == 1) { return c; }
      return -1;
    }

    // TODO(SRGDamia1): Implement peek
    int peek() override {
      return -1;
    }

    void flush() override {
      at->stream.flush();
    }

    uint8_t connected() override {
      if (available()) { return true; }
      return sock_connected;
    }
    operator bool() override {
      return connected();
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

   protected:
    // Read and dump anything remaining in the modem's internal buffer.
    // Using this in the client stop() function.
    // The socket will appear open in response to connected() even after it
    // closes until all data is read from the buffer.
    // Doing it this way allows the external mcu to find and get all of the
    // data that it wants from the socket even if it was closed externally.
    void dumpModemBuffer(uint32_t maxWaitMs) {
      TINY_GSM_YIELD();
      rx.clear();
      at->maintain();
      uint32_t startMillis = millis();
      while (sock_available > 0 && (millis() - startMillis < maxWaitMs)) {
        at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
        rx.clear();
        at->maintain();
      }
    }

    modemType* at;
    uint8_t    mux;
    uint16_t   sock_available;
    uint32_t   prev_check;
    bool       sock_connected;
    bool       got_data;
    RxFifo     rx;
  };

  /*
   * Inner Secure Client
   */

  /*
   * Constructor
   */

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

  void maintainImpl() {
    switch (bufType) {
      case READ_AND_CHECK_SIZE:
        // Keep listening for modem URC's and proactively iterate through
        // sockets asking if any data is avaiable
        for (int mux = 0; mux < muxCount; mux++) {
          GsmClient* sock = thisModem().sockets[mux];
          if (sock && sock->got_data) {
            sock->got_data       = false;
            sock->sock_available = thisModem().modemGetAvailable(mux);
          }
        }
        while (thisModem().stream.available()) {
          thisModem().waitResponse(15, NULL, NULL);
        }
        break;
      default:
        // Just listen for any URC's
        thisModem().waitResponse(100, NULL, NULL);
        break;
    }
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
   * SIM card functions
   */
 protected:
  // Unlocks a sim via the 3GPP TS command AT+CPIN
  bool simUnlockImpl(const char* pin) {
    if (pin && strlen(pin) > 0) {
      thisModem().sendAT(GF("+CPIN=\""), pin, GF("\""));
      return thisModem().waitResponse() == 1;
    }
    return true;
  }

  // Gets the CCID of a sim card via AT+CCID
  String getSimCCIDImpl() {
    thisModem().sendAT(GF("+CCID"));
    if (thisModem().waitResponse(GF("+CCID:")) != 1) { return ""; }
    String res = thisModem().stream.readStringUntil('\n');
    thisModem().waitResponse();
    res.trim();
    return res;
  }

  // Asks for TA Serial Number Identification (IMEI) via the V.25TER standard
  // AT+GSN command
  String getIMEIImpl() {
    thisModem().sendAT(GF("+GSN"));
    String res = thisModem().stream.readStringUntil('\n');
    thisModem().waitResponse();
    res.trim();
    return res;
  }

  SimStatus getSimStatusImpl(uint32_t timeout_ms = 10000L) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      thisModem().sendAT(GF("+CPIN?"));
      if (thisModem().waitResponse(GF("+CPIN:")) != 1) {
        delay(1000);
        continue;
      }
      int status = thisModem().waitResponse(GF("READY"), GF("SIM PIN"),
                                            GF("SIM PUK"), GF("NOT INSERTED"),
                                            GF("NOT READY"));
      thisModem().waitResponse();
      switch (status) {
        case 2:
        case 3: return SIM_LOCKED;
        case 1: return SIM_READY;
        default: return SIM_ERROR;
      }
    }
    return SIM_ERROR;
  }

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
    int status = thisModem().stream.readStringUntil('\n').toInt();
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
    int res = thisModem().stream.readStringUntil(',').toInt();
    thisModem().waitResponse();
    return res;
  }

  /*
   * GPRS functions
   */
 protected:
  // Checks if current attached to GPRS/EPS service
  bool isGprsConnectedImpl() {
    thisModem().sendAT(GF("+CGATT?"));
    if (thisModem().waitResponse(GF("+CGATT:")) != 1) { return false; }
    int res = thisModem().stream.readStringUntil('\n').toInt();
    thisModem().waitResponse();
    if (res != 1) { return false; }

    return thisModem().localIP() != IPAddress(0, 0, 0, 0);
  }

  // Gets the current network operator via the 3GPP TS command AT+COPS
  String getOperatorImpl() {
    thisModem().sendAT(GF("+COPS?"));
    if (thisModem().waitResponse(GF("+COPS:")) != 1) { return ""; }
    thisModem().streamSkipUntil('"'); /* Skip mode and format */
    String res = thisModem().stream.readStringUntil('"');
    thisModem().waitResponse();
    return res;
  }

  /*
   * WiFi functions
   */

  bool networkConnectImpl(const char* ssid, const char* pwd) {
    return false;
  }
  bool networkDisconnectImpl() {
    return thisModem().gprsConnectImpl();
  }

  /*
   * IP Address functions
   */
 protected:
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
   * Phone Call functions
   */
 protected:
  bool callAnswerImpl() {
    thisModem().sendAT(GF("A"));
    return thisModem().waitResponse() == 1;
  }

  // Returns true on pick-up, false on error/busy
  bool callNumberImpl(const String& number) {
    if (number == GF("last")) {
      thisModem().sendAT(GF("DL"));
    } else {
      thisModem().sendAT(GF("D"), number, ";");
    }
    int status = thisModem().waitResponse(60000L, GF("OK"), GF("BUSY"),
                                          GF("NO ANSWER"), GF("NO CARRIER"));
    switch (status) {
      case 1: return true;
      case 2:
      case 3: return false;
      default: return false;
    }
  }

  bool callHangupImpl() {
    thisModem().sendAT(GF("H"));
    return thisModem().waitResponse() == 1;
  }

  // 0-9,*,#,A,B,C,D
  bool dtmfSendImpl(char cmd, int duration_ms = 100) {
    duration_ms = constrain(duration_ms, 100, 1000);

    thisModem().sendAT(GF("+VTD="),
                       duration_ms / 100);  // VTD accepts in 1/10 of a second
    thisModem().waitResponse();

    thisModem().sendAT(GF("+VTS="), cmd);
    return thisModem().waitResponse(10000L) == 1;
  }

  /*
   * Messaging functions
   */
 protected:
  static inline String TinyGsmDecodeHex7bit(String& instr) {
    String result;
    byte   reminder = 0;
    int    bitstate = 7;
    for (unsigned i = 0; i < instr.length(); i += 2) {
      char buf[4] = {
          0,
      };
      buf[0] = instr[i];
      buf[1] = instr[i + 1];
      byte b = strtol(buf, NULL, 16);

      byte bb = b << (7 - bitstate);
      char c  = (bb + reminder) & 0x7F;
      result += c;
      reminder = b >> bitstate;
      bitstate--;
      if (bitstate == 0) {
        char cc = reminder;
        result += cc;
        reminder = 0;
        bitstate = 7;
      }
    }
    return result;
  }

  static inline String TinyGsmDecodeHex8bit(String& instr) {
    String result;
    for (unsigned i = 0; i < instr.length(); i += 2) {
      char buf[4] = {
          0,
      };
      buf[0] = instr[i];
      buf[1] = instr[i + 1];
      char b = strtol(buf, NULL, 16);
      result += b;
    }
    return result;
  }

  static inline String TinyGsmDecodeHex16bit(String& instr) {
    String result;
    for (unsigned i = 0; i < instr.length(); i += 4) {
      char buf[4] = {
          0,
      };
      buf[0] = instr[i];
      buf[1] = instr[i + 1];
      char b = strtol(buf, NULL, 16);
      if (b) {  // If high byte is non-zero, we can't handle it ;(
#if defined(TINY_GSM_UNICODE_TO_HEX)
        result += "\\x";
        result += instr.substring(i, i + 4);
#else
        result += "?";
#endif
      } else {
        buf[0] = instr[i + 2];
        buf[1] = instr[i + 3];
        b      = strtol(buf, NULL, 16);
        result += b;
      }
    }
    return result;
  }

  String sendUSSDImpl(const String& code) {
    // Set preferred message format to text mode
    thisModem().sendAT(GF("+CMGF=1"));
    thisModem().waitResponse();
    // Set 8-bit hexadecimal alphabet (3GPP TS 23.038)
    thisModem().sendAT(GF("+CSCS=\"HEX\""));
    thisModem().waitResponse();
    // Send the message
    thisModem().sendAT(GF("+CUSD=1,\""), code, GF("\""));
    if (thisModem().waitResponse() != 1) { return ""; }
    if (thisModem().waitResponse(10000L, GF("+CUSD:")) != 1) { return ""; }
    thisModem().stream.readStringUntil('"');
    String hex = thisModem().stream.readStringUntil('"');
    thisModem().stream.readStringUntil(',');
    int dcs = thisModem().stream.readStringUntil('\n').toInt();

    if (dcs == 15) {
      return TinyGsmDecodeHex8bit(hex);
    } else if (dcs == 72) {
      return TinyGsmDecodeHex16bit(hex);
    } else {
      return hex;
    }
  }

  bool sendSMSImpl(const String& number, const String& text) {
    // Set preferred message format to text mode
    thisModem().sendAT(GF("+CMGF=1"));
    thisModem().waitResponse();
    // Set GSM 7 bit default alphabet (3GPP TS 23.038)
    thisModem().sendAT(GF("+CSCS=\"GSM\""));
    thisModem().waitResponse();
    thisModem().sendAT(GF("+CMGS=\""), number, GF("\""));
    if (thisModem().waitResponse(GF(">")) != 1) { return false; }
    thisModem().stream.print(text);  // Actually send the message
    thisModem().stream.write(static_cast<char>(0x1A));  // Terminate the message
    thisModem().stream.flush();
    return thisModem().waitResponse(60000L) == 1;
  }

  // Common methods for UTF8/UTF16 SMS.
  // Supported by: BG96, M95, MC60, SIM5360, SIM7000, SIM7600, SIM800
  class UTF8Print : public Print {
   public:
    explicit UTF8Print(Print& p) : p(p) {}
    size_t write(const uint8_t c) override {
      if (prv < 0xC0) {
        if (c < 0xC0) printHex(c);
        prv = c;
      } else {
        uint16_t v = uint16_t(prv) << 8 | c;
        v -= (v >> 8 == 0xD0) ? 0xCC80 : 0xCD40;
        printHex(v);
        prv = 0;
      }
      return 1;
    }

   private:
    Print&  p;
    uint8_t prv = 0;
    void    printHex(const uint16_t v) {
      uint8_t c = v >> 8;
      if (c < 0x10) p.print('0');
      p.print(c, HEX);
      c = v & 0xFF;
      if (c < 0x10) p.print('0');
      p.print(c, HEX);
    }
  };

  bool sendSMS_UTF8_begin(const char* const number) {
    thisModem().sendAT(GF("+CMGF=1"));
    thisModem().waitResponse();
    thisModem().sendAT(GF("+CSCS=\"HEX\""));
    thisModem().waitResponse();
    thisModem().sendAT(GF("+CSMP=17,167,0,8"));
    thisModem().waitResponse();

    thisModem().sendAT(GF("+CMGS=\""), number, GF("\""));
    return thisModem().waitResponse(GF(">")) == 1;
  }
  bool sendSMS_UTF8_end() {
    thisModem().stream.write(static_cast<char>(0x1A));
    thisModem().stream.flush();
    return thisModem().waitResponse(60000L) == 1;
  }
  UTF8Print sendSMS_UTF8_stream() {
    return UTF8Print(thisModem().stream);
  }

  bool sendSMS_UTF16Impl(const char* const number, const void* text,
                         size_t len) {
    if (!sendSMS_UTF8_begin(number)) { return false; }

    uint16_t* t = reinterpret_cast<uint16_t*>(text);
    for (size_t i = 0; i < len; i++) {
      uint8_t c = t[i] >> 8;
      if (c < 0x10) { thisModem().stream.print('0'); }
      thisModem().stream.print(c, HEX);
      c = t[i] & 0xFF;
      if (c < 0x10) { thisModem().stream.print('0'); }
      thisModem().stream.print(c, HEX);
    }

    return sendSMS_UTF8_end();
  }

  /*
   * Location functions
   */
 protected:
  String getGsmLocationImpl() {
    thisModem().sendAT(GF("+CIPGSMLOC=1,1"));
    if (thisModem().waitResponse(10000L, GF("+CIPGSMLOC:")) != 1) { return ""; }
    String res = thisModem().stream.readStringUntil('\n');
    thisModem().waitResponse();
    res.trim();
    return res;
  }

  /*
   * GPS location functions
   */
 public:
  /*
   * Time functions
   */
 protected:
  String getGSMDateTimeImpl(TinyGSMDateTimeFormat format) {
    thisModem().sendAT(GF("+CCLK?"));
    if (thisModem().waitResponse(2000L, GF("+CCLK: \"")) != 1) { return ""; }

    String res;

    switch (format) {
      case DATE_FULL: res = thisModem().stream.readStringUntil('"'); break;
      case DATE_TIME:
        thisModem().streamSkipUntil(',');
        res = thisModem().stream.readStringUntil('"');
        break;
      case DATE_DATE: res = thisModem().stream.readStringUntil(','); break;
    }
    return res;
  }

  /*
   * Battery & temperature functions
   */
 protected:
  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  uint16_t getBattVoltageImpl() {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return 0; }
    thisModem().streamSkipUntil(',');  // Skip battery charge status
    thisModem().streamSkipUntil(',');  // Skip battery charge level
    // return voltage in mV
    uint16_t res = thisModem().stream.readStringUntil(',').toInt();
    // Wait for final OK
    thisModem().waitResponse();
    return res;
  }

  int8_t getBattPercentImpl() {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return false; }
    thisModem().streamSkipUntil(',');  // Skip battery charge status
    // Read battery charge level
    int res = thisModem().stream.readStringUntil(',').toInt();
    // Wait for final OK
    thisModem().waitResponse();
    return res;
  }

  uint8_t getBattChargeStateImpl() {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return false; }
    // Read battery charge status
    int res = thisModem().stream.readStringUntil(',').toInt();
    // Wait for final OK
    thisModem().waitResponse();
    return res;
  }

  bool getBattStatsImpl(uint8_t& chargeState, int8_t& percent,
                        uint16_t& milliVolts) {
    thisModem().sendAT(GF("+CBC"));
    if (thisModem().waitResponse(GF("+CBC:")) != 1) { return false; }
    chargeState = thisModem().stream.readStringUntil(',').toInt();
    percent     = thisModem().stream.readStringUntil(',').toInt();
    milliVolts  = thisModem().stream.readStringUntil('\n').toInt();
    // Wait for final OK
    thisModem().waitResponse();
    return true;
  }

  float getTemperatureImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Client related functions
   */
 protected:
  /*
   Utilities
   */

  // Utility templates for writing/skipping characters on a stream
  template <typename T>
  void streamWrite(T last) {
    thisModem().stream.print(last);
  }

  template <typename T, typename... Args>
  void streamWrite(T head, Args... tail) {
    thisModem().stream.print(head);
    thisModem().streamWrite(tail...);
  }

  // template <typename... Args> void sendAT(Args... cmd) {
  //   thisModem().streamWrite("AT", cmd..., thisModem().gsmNL);
  //   thisModem().stream.flush();
  //   TINY_GSM_YIELD(); /* DBG("### AT:", cmd...); */
  // }

  bool streamSkipUntil(const char c, const uint32_t timeout_ms = 1000L) {
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

  void streamClear() {
    while (thisModem().stream.available()) {
      thisModem().waitResponse(50, NULL, NULL);
    }
  }

  // Yields up to a time-out period and then reads a character from the stream
  // into the mux FIFO
  // TODO(SRGDamia1):  Do we need to wait two _timeout periods for no
  // character return?  Will wait once in the first "while
  // !stream.available()" and then will wait again in the stream.read()
  // function.
  void moveCharFromStreamToFifo(uint8_t mux) {
    uint32_t startMillis = millis();
    while (!thisModem().stream.available() &&
           (millis() - startMillis < thisModem().sockets[mux]->_timeout)) {
      TINY_GSM_YIELD();
    }
    char c = thisModem().stream.read();
    thisModem().sockets[mux]->rx.put(c);
  }
};

#endif  // SRC_TINYGSMCOMMON_H_
