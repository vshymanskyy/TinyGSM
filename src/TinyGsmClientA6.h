/**
 * @file       TinyGsmClientA6.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTA6_H_
#define SRC_TINYGSMCLIENTA6_H_
// #pragma message("TinyGSM:  TinyGsmClientA6")

// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MUX_COUNT 8
#define TINY_GSM_NO_MODEM_BUFFER

#include "TinyGsmBattery.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmModem.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmTime.tpp"

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM    = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;
#if defined       TINY_GSM_DEBUG
static const char GSM_CME_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CME ERROR:";
static const char GSM_CMS_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CMS ERROR:";
#endif

enum RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};

class TinyGsmA6 : public TinyGsmModem<TinyGsmA6>,
                  public TinyGsmGPRS<TinyGsmA6>,
                  public TinyGsmTCP<TinyGsmA6, TINY_GSM_MUX_COUNT>,
                  public TinyGsmCalling<TinyGsmA6>,
                  public TinyGsmSMS<TinyGsmA6>,
                  public TinyGsmTime<TinyGsmA6>,
                  public TinyGsmBattery<TinyGsmA6> {
  friend class TinyGsmModem<TinyGsmA6>;
  friend class TinyGsmGPRS<TinyGsmA6>;
  friend class TinyGsmTCP<TinyGsmA6, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmCalling<TinyGsmA6>;
  friend class TinyGsmSMS<TinyGsmA6>;
  friend class TinyGsmTime<TinyGsmA6>;
  friend class TinyGsmBattery<TinyGsmA6>;

  /*
   * Inner Client
   */
 public:
  class GsmClientA6 : public GsmClient {
    friend class TinyGsmA6;

   public:
    GsmClientA6() {}

    explicit GsmClientA6(TinyGsmA6& modem, uint8_t = 0) {
      init(&modem, -1);
    }

    bool init(TinyGsmA6* modem, uint8_t = 0) {
      this->at       = modem;
      this->mux      = -1;
      sock_connected = false;

      return true;
    }

   public:
    int connect(const char* host, uint16_t port, int timeout_s) {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      uint8_t newMux = -1;
      sock_connected = at->modemConnect(host, port, &newMux, timeout_s);
      if (sock_connected) {
        mux              = newMux;
        at->sockets[mux] = this;
      }
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void stop(uint32_t maxWaitMs) {
      TINY_GSM_YIELD();
      at->sendAT(GF("+CIPCLOSE="), mux);
      sock_connected = false;
      at->waitResponse(maxWaitMs);
      rx.clear();
    }
    void stop() override {
      stop(1000L);
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  };

  /*
   * Inner Secure Client
   */

  // Doesn't support SSL

  /*
   * Constructor
   */
 public:
  explicit TinyGsmA6(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientA6"));

    if (!testAT()) { return false; }

    // sendAT(GF("&FZ"));  // Factory + Reset
    // waitResponse();

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();
    sendAT(
        GF("+CMER=3,0,0,2"));  // Set unsolicited result code output destination
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

    SimStatus ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != NULL && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    } else {
      // if the sim is ready, or it's locked but no pin has been provided,
      // return true
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  bool factoryDefaultImpl() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("&W"));  // Write configuration
    return waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl() {
    if (!testAT()) { return false; }
    sendAT(GF("+RST=1"));
    delay(3000);
    return init();
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOF"));
    // +CPOF: MS OFF OK
    return waitResponse() == 1;
  }

  bool sleepEnableImpl(bool enable = true) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false)
      TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 public:
  RegStatus getRegistrationStatus() {
    return (RegStatus)getRegistrationStatusXREG("CREG");
  }

 protected:
  bool isNetworkConnectedImpl() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  String getLocalIPImpl() {
    sendAT(GF("+CIFSR"));
    String res;
    if (waitResponse(10000L, res) != 1) { return ""; }
    res.replace(GSM_NL "OK" GSM_NL, "");
    res.replace(GSM_NL, "");
    res.trim();
    return res;
  }

  /*
   * GPRS functions
   */
 protected:
  bool gprsConnectImpl(const char* apn, const char* user = NULL,
                       const char* pwd = NULL) {
    gprsDisconnect();

    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // TODO(?): wait AT+CGATT?

    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    if (!user) user = "";
    if (!pwd) pwd = "";
    sendAT(GF("+CSTT=\""), apn, GF("\",\""), user, GF("\",\""), pwd, GF("\""));
    if (waitResponse(60000L) != 1) { return false; }

    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    sendAT(GF("+CIPMUX=1"));
    if (waitResponse() != 1) { return false; }

    return true;
  }

  bool gprsDisconnectImpl() {
    // Shut the TCP/IP connection
    sendAT(GF("+CIPSHUT"));
    if (waitResponse(60000L) != 1) { return false; }

    for (int i = 0; i < 3; i++) {
      sendAT(GF("+CGATT=0"));
      if (waitResponse(5000L) == 1) { return true; }
    }

    return false;
  }

  String getOperatorImpl() {
    sendAT(GF("+COPS=3,0"));  // Set format
    waitResponse();

    sendAT(GF("+COPS?"));
    if (waitResponse(GF(GSM_NL "+COPS:")) != 1) { return ""; }
    streamSkipUntil('"');  // Skip mode and format
    String res = stream.readStringUntil('"');
    waitResponse();
    return res;
  }

  /*
   * SIM card functions
   */
 protected:
  String getSimCCIDImpl() {
    sendAT(GF("+CCID"));
    if (waitResponse(GF(GSM_NL "+SCID: SIM Card ID:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone Call functions
   */
 protected:
  // Returns true on pick-up, false on error/busy
  bool callNumberImpl(const String& number) {
    if (number == GF("last")) {
      sendAT(GF("DLST"));
    } else {
      sendAT(GF("D\""), number, "\";");
    }

    if (waitResponse(5000L) != 1) { return false; }

    if (waitResponse(60000L, GF(GSM_NL "+CIEV: \"CALL\",1"),
                     GF(GSM_NL "+CIEV: \"CALL\",0"), GFP(GSM_ERROR)) != 1) {
      return false;
    }

    int8_t rsp = waitResponse(60000L, GF(GSM_NL "+CIEV: \"SOUNDER\",0"),
                              GF(GSM_NL "+CIEV: \"CALL\",0"));

    int8_t rsp2 = waitResponse(300L, GF(GSM_NL "BUSY" GSM_NL),
                               GF(GSM_NL "NO ANSWER" GSM_NL));

    return rsp == 1 && rsp2 == 0;
  }

  // 0-9,*,#,A,B,C,D
  bool dtmfSendImpl(char cmd, uint8_t duration_ms = 100) {
    duration_ms = constrain(duration_ms, 100, 1000);

    // The duration parameter is not working, so we simulate it using delay..
    // TODO(?): Maybe there's another way...

    // sendAT(GF("+VTD="), duration_ms / 100);
    // waitResponse();

    sendAT(GF("+VTS="), cmd);
    if (waitResponse(10000L) == 1) {
      delay(duration_ms);
      return true;
    }
    return false;
  }

  /*
   * Audio functions
   */
 public:
  bool audioSetHeadphones() {
    sendAT(GF("+SNFS=0"));
    return waitResponse() == 1;
  }

  bool audioSetSpeaker() {
    sendAT(GF("+SNFS=1"));
    return waitResponse() == 1;
  }

  bool audioMuteMic(bool mute) {
    sendAT(GF("+CMUT="), mute);
    return waitResponse() == 1;
  }

  /*
   * Messaging functions
   */
 protected:
  String sendUSSDImpl(const String& code) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    sendAT(GF("+CSCS=\"HEX\""));
    waitResponse();
    sendAT(GF("+CUSD=1,\""), code, GF("\",15"));
    if (waitResponse(10000L) != 1) { return ""; }
    if (waitResponse(GF(GSM_NL "+CUSD:")) != 1) { return ""; }
    streamSkipUntil('"');
    String hex = stream.readStringUntil('"');
    streamSkipUntil(',');
    int8_t dcs = streamGetIntBefore('\n');

    if (dcs == 15) {
      return TinyGsmDecodeHex7bit(hex);
    } else if (dcs == 72) {
      return TinyGsmDecodeHex16bit(hex);
    } else {
      return hex;
    }
  }

  /*
   * Time functions
   */
 protected:
  // Can follow the standard CCLK function in the template
  // Note - the clock probably has to be set manaually first

  /*
   * Battery functions
   */
 protected:
  uint16_t getBattVoltageImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  // Needs a '?' after CBC, unlike most
  int8_t getBattPercentImpl() {
    sendAT(GF("+CBC?"));
    if (waitResponse(GF(GSM_NL "+CBC:")) != 1) { return false; }
    streamSkipUntil(',');  // Skip battery charge status
    // Read battery charge level
    int8_t res = streamGetIntBefore('\n');
    // Wait for final OK
    waitResponse();
    return res;
  }

  // Needs a '?' after CBC, unlike most
  bool getBattStatsImpl(uint8_t& chargeState, int8_t& percent,
                        uint16_t& milliVolts) {
    sendAT(GF("+CBC?"));
    if (waitResponse(GF(GSM_NL "+CBC:")) != 1) { return false; }
    chargeState = streamGetIntBefore(',');
    percent     = streamGetIntBefore('\n');
    milliVolts  = 0;
    // Wait for final OK
    waitResponse();
    return true;
  }

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t* mux,
                    int timeout_s = 75) {
    uint32_t startMillis = millis();
    uint32_t timeout_ms  = ((uint32_t)timeout_s) * 1000;

    sendAT(GF("+CIPSTART="), GF("\"TCP"), GF("\",\""), host, GF("\","), port);
    if (waitResponse(timeout_ms, GF(GSM_NL "+CIPNUM:")) != 1) { return false; }
    int8_t newMux = streamGetIntBefore('\n');

    int8_t rsp = waitResponse(
        (timeout_ms - (millis() - startMillis)), GF("CONNECT OK" GSM_NL),
        GF("CONNECT FAIL" GSM_NL), GF("ALREADY CONNECT" GSM_NL));
    if (waitResponse() != 1) { return false; }
    *mux = newMux;

    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    if (waitResponse(2000L, GF(GSM_NL ">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(10000L, GFP(GSM_OK), GF(GSM_NL "FAIL")) != 1) { return 0; }
    return len;
  }

  bool modemGetConnected(uint8_t) {
    sendAT(GF("+CIPSTATUS"));  // TODO(?) mux?
    int8_t res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""),
                              GF(",\"CLOSING\""), GF(",\"INITIAL\""));
    waitResponse();
    return 1 == res;
  }

  /*
   * Utilities
   */
 public:
  // TODO(vshymanskyy): Optimize this!
  int8_t waitResponse(uint32_t timeout_ms, String& data,
                      GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    /*String r1s(r1); r1s.trim();
    String r2s(r2); r2s.trim();
    String r3s(r3); r3s.trim();
    String r4s(r4); r4s.trim();
    String r5s(r5); r5s.trim();
    DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s);*/
    data.reserve(64);
    uint8_t  index       = 0;
    uint32_t startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (stream.available() > 0) {
        TINY_GSM_YIELD();
        int8_t a = stream.read();
        if (a <= 0) continue;  // Skip 0x00 bytes, just in case
        data += static_cast<char>(a);
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
          index = 2;
          goto finish;
        } else if (r3 && data.endsWith(r3)) {
#if defined TINY_GSM_DEBUG
          if (r3 == GFP(GSM_CME_ERROR)) {
            streamSkipUntil('\n');  // Read out the error
          }
#endif
          index = 3;
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (data.endsWith(GF("+CIPRCV:"))) {
          int8_t  mux      = streamGetIntBefore(',');
          int16_t len      = streamGetIntBefore(',');
          int16_t len_orig = len;
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            if (len > sockets[mux]->rx.free()) {
              DBG("### Buffer overflow: ", len, "->", sockets[mux]->rx.free());
            } else {
              DBG("### Got: ", len, "->", sockets[mux]->rx.free());
            }
            while (len--) { moveCharFromStreamToFifo(mux); }
            // TODO(?) Deal with missing characters
            if (len_orig > sockets[mux]->available()) {
              DBG("### Fewer characters received than expected: ",
                  sockets[mux]->available(), " vs ", len_orig);
            }
          }
          data = "";
        } else if (data.endsWith(GF("+TCPCLOSED:"))) {
          int8_t mux = streamGetIntBefore('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### Closed: ", mux);
        }
      }
    } while (millis() - startMillis < timeout_ms);
  finish:
    if (!index) {
      data.trim();
      if (data.length()) { DBG("### Unhandled:", data); }
      data = "";
    }
    // data.replace(GSM_NL, "/");
    // DBG('<', index, '>', data);
    return index;
  }

  int8_t waitResponse(uint32_t timeout_ms, GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
  }

  int8_t waitResponse(GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    return waitResponse(1000, r1, r2, r3, r4, r5);
  }

 public:
  Stream& stream;

 protected:
  GsmClientA6* sockets[TINY_GSM_MUX_COUNT];
  const char*  gsmNL = GSM_NL;
};

#endif  // SRC_TINYGSMCLIENTA6_H_
