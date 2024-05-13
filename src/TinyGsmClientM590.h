/**
 * @file       TinyGsmClientM590.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTM590_H_
#define SRC_TINYGSMCLIENTM590_H_
// #pragma message("TinyGSM:  TinyGsmClientM590")

// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MUX_COUNT 2
#define TINY_GSM_NO_MODEM_BUFFER
#ifdef GSM_NL
#undef GSM_NL
#define GSM_NL "\r\n"  // NOTE:  define before including TinyGsmModem!
#endif

#include "TinyGsmGPRS.tpp"
#include "TinyGsmModem.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmTime.tpp"

enum RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 3,
  REG_DENIED       = 2,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};

class TinyGsmM590 : public TinyGsmModem<TinyGsmM590>,
                    public TinyGsmGPRS<TinyGsmM590>,
                    public TinyGsmTCP<TinyGsmM590, TINY_GSM_MUX_COUNT>,
                    public TinyGsmSMS<TinyGsmM590>,
                    public TinyGsmTime<TinyGsmM590> {
  friend class TinyGsmModem<TinyGsmM590>;
  friend class TinyGsmGPRS<TinyGsmM590>;
  friend class TinyGsmTCP<TinyGsmM590, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSMS<TinyGsmM590>;
  friend class TinyGsmTime<TinyGsmM590>;

  /*
   * Inner Client
   */
 public:
  class GsmClientM590 : public GsmClient {
    friend class TinyGsmM590;

   public:
    GsmClientM590() {}

    explicit GsmClientM590(TinyGsmM590& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmM590* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_connected = false;

      if (mux < TINY_GSM_MUX_COUNT) {
        this->mux = mux;
      } else {
        this->mux = (mux % TINY_GSM_MUX_COUNT);
      }
      at->sockets[this->mux] = this;

      return true;
    }

   public:
    virtual int connect(const char* host, uint16_t port, int timeout_s) {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void stop(uint32_t maxWaitMs) {
      TINY_GSM_YIELD();
      at->sendAT(GF("+TCPCLOSE="), mux);
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
   * Constructor
   */
 public:
  explicit TinyGsmM590(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientM590"));

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
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

  // Doesn't support CGMI
  String getModemNameImpl() {
    return "Neoway M590";
  }

  // Extra stuff here - pwr save, internal stack
  bool factoryDefaultImpl() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("+ICF=3,1"));  // 8 data 0 parity 1 stop
    waitResponse();
    sendAT(GF("+ENPWRSAVE=0"));  // Disable PWR save
    waitResponse();
    sendAT(GF("+XISP=0"));  // Use internal stack
    waitResponse();
    sendAT(GF("&W"));  // Write configuration
    return waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = NULL) {
    if (!testAT()) { return false; }
    if (!setPhoneFunctionality(15)) { return false; }
    // MODEM:STARTUP
    waitResponse(60000L, GF(GSM_NL "+PBREADY" GSM_NL));
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPWROFF"));
    return waitResponse(3000L) == 1;
  }

  bool sleepEnableImpl(bool enable = true) {
    sendAT(GF("+ENPWRSAVE="), enable);
    return waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

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
    sendAT(GF("+XIIC?"));
    if (waitResponse(GF(GSM_NL "+XIIC:")) != 1) { return ""; }
    streamSkipUntil(',');
    String res = stream.readStringUntil('\n');
    waitResponse();
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

    sendAT(GF("+XISP=0"));
    waitResponse();

    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    if (!user) user = "";
    if (!pwd) pwd = "";
    sendAT(GF("+XGAUTH=1,1,\""), user, GF("\",\""), pwd, GF("\""));
    waitResponse();

    sendAT(GF("+XIIC=1"));
    waitResponse();

    const uint32_t timeout_ms = 60000L;
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      if (isGprsConnected()) {
        // goto set_dns; // TODO
        return true;
      }
      delay(500);
    }
    return false;

    // set_dns:  // TODO
    //     sendAT(GF("+DNSSERVER=1,8.8.8.8"));
    //     waitResponse();
    //
    //     sendAT(GF("+DNSSERVER=2,8.8.4.4"));
    //     waitResponse();

    return true;
  }

  bool gprsDisconnectImpl() {
    // TODO(?): There is no command in AT command set
    // XIIC=0 does not work
    return true;
  }

  bool isGprsConnectedImpl() {
    sendAT(GF("+XIIC?"));
    if (waitResponse(GF(GSM_NL "+XIIC:")) != 1) { return false; }
    int8_t res = streamGetIntBefore(',');
    waitResponse();
    return res == 1;
  }

  /*
   * SIM card functions
   */
 protected:
  // Able to follow all SIM card functions as inherited from the template

  /*
   * Messaging functions
   */
 protected:
  bool sendSMS_UTF16Impl(const String& number, const void* text,
                         size_t len) TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Time functions
   */
 protected:
  // Can follow the standard CCLK function in the template

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux, bool,
                    int timeout_s = 75) {
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    for (int i = 0; i < 3; i++) {  // TODO(?): no need for loop?
      String ip = dnsIpQuery(host);

      sendAT(GF("+TCPSETUP="), mux, GF(","), ip, GF(","), port);
      int8_t rsp = waitResponse(timeout_ms, GF(",OK" GSM_NL),
                                GF(",FAIL" GSM_NL),
                                GF("+TCPSETUP:Error" GSM_NL));
      if (1 == rsp) {
        return true;
      } else if (3 == rsp) {
        sendAT(GF("+TCPCLOSE="), mux);
        waitResponse();
      }
      delay(1000);
    }
    return false;
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+TCPSEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.write(static_cast<char>(0x0D));
    stream.flush();
    if (waitResponse(30000L, GF(GSM_NL "+TCPSEND:")) != 1) { return 0; }
    streamSkipUntil('\n');
    return len;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+CIPSTATUS="), mux);
    int8_t res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""),
                              GF(",\"CLOSING\""), GF(",\"INITIAL\""));
    waitResponse();
    return 1 == res;
  }

  String dnsIpQuery(const char* host) {
    sendAT(GF("+DNS=\""), host, GF("\""));
    if (waitResponse(10000L, GF(GSM_NL "+DNS:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse(GF("+DNS:OK" GSM_NL));
    res.trim();
    return res;
  }

  /*
   * Utilities
   */
 public:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+TCPRECV:"))) {
      int8_t  mux      = streamGetIntBefore(',');
      int16_t len      = streamGetIntBefore(',');
      int16_t len_orig = len;
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        if (len > sockets[mux]->rx.free()) {
          DBG("### Buffer overflow: ", len, "->", sockets[mux]->rx.free());
          // reset the len to read to the amount free
          len = sockets[mux]->rx.free();
        }
        while (len--) { moveCharFromStreamToFifo(mux); }
        // TODO(?): Handle lost characters
        if (len_orig != sockets[mux]->available()) {
          DBG("### Different number of characters received than expected: ",
              sockets[mux]->available(), " vs ", len_orig);
        }
      }
      data = "";
      DBG("### Got Data: ", len_orig, "on", mux);
      return true;
    } else if (data.endsWith(GF("+TCPCLOSE:"))) {
      int8_t mux = streamGetIntBefore(',');
      streamSkipUntil('\n');
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        sockets[mux]->sock_connected = false;
      }
      data = "";
      DBG("### Closed: ", mux);
      return true;
    }
    return false;
  }

 public:
  Stream& stream;

 protected:
  GsmClientM590* sockets[TINY_GSM_MUX_COUNT];
};

#endif  // SRC_TINYGSMCLIENTM590_H_
