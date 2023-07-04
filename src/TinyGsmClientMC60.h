/**
 * @file       TinyGsmClientMC60.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 *
 * @MC60 support added by Tamas Dajka 2017.10.15 - with fixes by Sara Damiano
 *
 */

#ifndef SRC_TINYGSMCLIENTMC60_H_
#define SRC_TINYGSMCLIENTMC60_H_
// #pragma message("TinyGSM:  TinyGsmClientMC60")

// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MUX_COUNT 6
#define TINY_GSM_BUFFER_READ_NO_CHECK

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

class TinyGsmMC60 : public TinyGsmModem<TinyGsmMC60>,
                    public TinyGsmGPRS<TinyGsmMC60>,
                    public TinyGsmTCP<TinyGsmMC60, TINY_GSM_MUX_COUNT>,
                    public TinyGsmCalling<TinyGsmMC60>,
                    public TinyGsmSMS<TinyGsmMC60>,
                    public TinyGsmTime<TinyGsmMC60>,
                    public TinyGsmBattery<TinyGsmMC60> {
  friend class TinyGsmModem<TinyGsmMC60>;
  friend class TinyGsmGPRS<TinyGsmMC60>;
  friend class TinyGsmTCP<TinyGsmMC60, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmCalling<TinyGsmMC60>;
  friend class TinyGsmSMS<TinyGsmMC60>;
  friend class TinyGsmTime<TinyGsmMC60>;
  friend class TinyGsmBattery<TinyGsmMC60>;

  /*
   * Inner Client
   */
 public:
  class GsmClientMC60 : public GsmClient {
    friend class TinyGsmMC60;

   public:
    GsmClientMC60() {}

    explicit GsmClientMC60(TinyGsmMC60& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmMC60* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_available = 0;
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
      uint32_t startMillis = millis();
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+QICLOSE="), mux);
      sock_connected = false;
      at->waitResponse((maxWaitMs - (millis() - startMillis)), GF("CLOSED"),
                       GF("CLOSE OK"), GF("ERROR"));
    }
    void stop() override {
      stop(75000L);
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  };

  /*
   * Inner Secure Client
   */

  /*
    class GsmClientSecureMC60 : public GsmClientMC60
    {
    public:
      GsmClientSecure() {}

      GsmClientSecure(TinyGsmMC60& modem, uint8_t mux = 0)
       : GsmClient(modem, mux)
      {}


    public:
      int connect(const char* host, uint16_t port, int timeout_s) override {
        stop();
        TINY_GSM_YIELD();
        rx.clear();
        sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
        return sock_connected;
      }
      TINY_GSM_CLIENT_CONNECT_OVERRIDES
    };
 */

  /*
   * Constructor
   */
 public:
  explicit TinyGsmMC60(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientMC60"));

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

    DBG(GF("### Modem:"), getModemName());

    // Enable network time synchronization
    sendAT(GF("+QNITZ=1"));
    if (waitResponse(10000L) != 1) { return false; }

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

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = NULL) {
    if (!testAT()) { return false; }
    if (!setPhoneFunctionality(0)) { return false; }
    if (!setPhoneFunctionality(1, true)) { return false; }
    delay(3000);
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+QPOWD=1"));
    return waitResponse(GF("NORMAL POWER DOWN")) == 1;
  }

  // When entering into sleep mode is enabled, DTR is pulled up, and WAKEUP_IN
  // is pulled up, the module can directly enter into sleep mode.If entering
  // into sleep mode is enabled, DTR is pulled down, and WAKEUP_IN is pulled
  // down, there is a need to pull the DTR pin and the WAKEUP_IN pin up first,
  // and then the module can enter into sleep mode.
  bool sleepEnableImpl(bool enable = true) {
    sendAT(GF("+QSCLK="), enable);
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
    sendAT(GF("+QILOCIP"));
    streamSkipUntil('\n');
    String res = stream.readStringUntil('\n');
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

    // select foreground context 0 = VIRTUAL_UART_1
    sendAT(GF("+QIFGCNT=0"));
    if (waitResponse() != 1) { return false; }

    // Select GPRS (=1) as the Bearer
    sendAT(GF("+QICSGP=1,\""), apn, GF("\",\""), user, GF("\",\""), pwd,
           GF("\""));
    if (waitResponse() != 1) { return false; }

    // Define PDP context - is this necessary?
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Activate PDP context - is this necessary?
    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    // Select TCP/IP transfer mode - NOT transparent mode
    sendAT(GF("+QIMODE=0"));
    if (waitResponse() != 1) { return false; }

    // Enable multiple TCP/IP connections
    sendAT(GF("+QIMUX=1"));
    if (waitResponse() != 1) { return false; }

    // Modem is used as a client
    sendAT(GF("+QISRVC=1"));
    if (waitResponse() != 1) { return false; }

    // Start TCPIP Task and Set APN, User Name and Password
    sendAT("+QIREGAPP=\"", apn, "\",\"", user, "\",\"", pwd, "\"");
    if (waitResponse() != 1) { return false; }

    // Activate GPRS/CSD Context
    sendAT(GF("+QIACT"));
    if (waitResponse(60000L) != 1) { return false; }

    // Check that we have a local IP address
    if (localIP() == IPAddress(0, 0, 0, 0)) { return false; }

    // Set Method to Handle Received TCP/IP Data
    // Mode=2 - Output a notification statement:
    // +QIRDI: <id>,<sc>,<sid>,<num>,<len>,< tlen>
    sendAT(GF("+QINDI=2"));
    if (waitResponse() != 1) { return false; }

    return true;
  }

  bool gprsDisconnectImpl() {
    sendAT(GF("+QIDEACT"));  // Deactivate the bearer context
    return waitResponse(60000L, GF("DEACT OK"), GF("ERROR")) == 1;
  }

  /*
   * SIM card functions
   */
 protected:
  SimStatus getSimStatusImpl(uint32_t timeout_ms = 10000L) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      sendAT(GF("+CPIN?"));
      if (waitResponse(GF(GSM_NL "+CPIN:")) != 1) {
        delay(1000);
        continue;
      }
      int8_t status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"),
                                   GF("NOT INSERTED"), GF("PH_SIM PIN"),
                                   GF("PH_SIM PUK"));
      waitResponse();
      switch (status) {
        case 2:
        case 3: return SIM_LOCKED;
        case 5:
        case 6: return SIM_ANTITHEFT_LOCKED;
        case 1: return SIM_READY;
        default: return SIM_ERROR;
      }
    }
    return SIM_ERROR;
  }

  /*
   * Phone Call functions
   */
 protected:
  // Can follow all of the phone call functions from the template

  /*
   * Messaging functions
   */
 protected:
  // Can follow all template functions

 public:
  /** Delete all SMS */
  bool deleteAllSMS() {
    sendAT(GF("+QMGDA=6"));
    if (waitResponse(waitResponse(60000L, GF("OK"), GF("ERROR")) == 1)) {
      return true;
    }
    return false;
  }

  /*
   * Time functions
   */
 protected:
  // Can follow the standard CCLK function in the template

  /*
   * Battery functions
   */
  // Can follow battery functions as in the template

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
    if (ssl) { DBG("SSL not yet supported on this module!"); }

    // By default, MC60 expects IP address as 'host' parameter.
    // If it is a domain name, "AT+QIDNSIP=1" should be executed.
    // "AT+QIDNSIP=0" is for dotted decimal IP address.
    IPAddress addr;
    sendAT(GF("+QIDNSIP="),
           (TinyGsmIpFromString(host) == IPAddress(0, 0, 0, 0) ? 0 : 1));
    if (waitResponse() != 1) { return false; }

    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    sendAT(GF("+QIOPEN="), mux, GF(",\""), GF("TCP"), GF("\",\""), host,
           GF("\","), port);
    int8_t rsp = waitResponse(timeout_ms, GF("CONNECT OK" GSM_NL),
                              GF("CONNECT FAIL" GSM_NL),
                              GF("ALREADY CONNECT" GSM_NL));
    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+QISEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(GF(GSM_NL "SEND OK")) != 1) { return 0; }

    bool allAcknowledged = false;
    // bool failed = false;
    while (!allAcknowledged) {
      sendAT(GF("+QISACK="), mux);  // If 'mux' is not specified, MC60 returns
                                    // 'ERRROR' (for QIMUX == 1)
      if (waitResponse(5000L, GF(GSM_NL "+QISACK:")) != 1) {
        return -1;
      } else {
        streamSkipUntil(','); /** Skip total */
        streamSkipUntil(','); /** Skip acknowledged data size */
        if (streamGetIntBefore('\n') == 0) { allAcknowledged = true; }
      }
    }
    waitResponse(5000L);

    // streamSkipUntil(','); // Skip mux
    // return streamGetIntBefore('\n');

    return len;  // TODO(?): verify len/ack
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) return 0;
    // TODO(?):  Does this even work????
    // AT+QIRD=<id>,<sc>,<sid>,<len>
    // id = GPRS context number = 0, set in GPRS connect
    // sc = role in connection = 1, client of connection
    // sid = index of connection = mux
    // len = maximum length of data to retrieve
    sendAT(GF("+QIRD=0,1,"), mux, ',', (uint16_t)size);
    // If it replies only OK for the write command, it means there is no
    // received data in the buffer of the connection.
    int8_t res = waitResponse(GF("+QIRD:"), GFP(GSM_OK), GFP(GSM_ERROR));
    if (res == 1) {
      streamSkipUntil(':');  // skip IP address
      streamSkipUntil(',');  // skip port
      streamSkipUntil(',');  // skip connection type (TCP/UDP)
      // read the real length of the retrieved data
      uint16_t len = streamGetIntBefore('\n');
      // It's possible that the real length available is less than expected
      // This is quite likely if the buffer is broken into packets - which may
      // be different sizes.
      // If so, make sure we make sure we re-set the amount of data available.
      if (len < size) { sockets[mux]->sock_available = len; }
      for (uint16_t i = 0; i < len; i++) {
        moveCharFromStreamToFifo(mux);
        sockets[mux]->sock_available--;
        // ^^ One less character available after moving from modem's FIFO to our
        // FIFO
      }
      waitResponse();  // ends with an OK
      // DBG("### READ:", len, "from", mux);
      return len;
    } else {
      sockets[mux]->sock_available = 0;
      return 0;
    }
  }

  // Not possible to check the number of characters remaining in buffer
  size_t modemGetAvailable(uint8_t) {
    return 0;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+QISTATE=1,"), mux);
    // +QISTATE: 0,"TCP","151.139.237.11",80,5087,4,1,0,0,"uart1"

    if (waitResponse(GF("+QISTATE:")) != 1) { return false; }

    streamSkipUntil(',');                  // Skip mux
    streamSkipUntil(',');                  // Skip socket type
    streamSkipUntil(',');                  // Skip remote ip
    streamSkipUntil(',');                  // Skip remote port
    streamSkipUntil(',');                  // Skip local port
    int8_t res = streamGetIntBefore(',');  // socket state

    waitResponse();

    // 0 Initial, 1 Opening, 2 Connected, 3 Listening, 4 Closing
    return 2 == res;
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
                      GsmConstStr r5 = NULL, GsmConstStr r6 = NULL) {
    /*String r1s(r1); r1s.trim();
    String r2s(r2); r2s.trim();
    String r3s(r3); r3s.trim();
    String r4s(r4); r4s.trim();
    String r5s(r5); r5s.trim();
    String r6s(r6); r6s.trim();
    DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s, ",", r6s);*/
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
        } else if (r6 && data.endsWith(r6)) {
          index = 6;
          goto finish;
        } else if (data.endsWith(
                       GF(GSM_NL "+QIRDI:"))) {  // TODO(?):  QIRD? or QIRDI?
          // +QIRDI: <id>,<sc>,<sid>,<num>,<len>,< tlen>
          streamSkipUntil(',');  // Skip the context
          streamSkipUntil(',');  // Skip the role
          // read the connection id
          int8_t mux = streamGetIntBefore(',');
          // read the number of packets in the buffer
          int8_t num_packets = streamGetIntBefore(',');
          // read the length of the current packet
          streamSkipUntil(
              ',');  // Skip the length of the current package in the buffer
          int16_t len_total =
              streamGetIntBefore('\n');  // Total length of all packages
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux] &&
              num_packets >= 0 && len_total >= 0) {
            sockets[mux]->sock_available = len_total;
          }
          data = "";
          // DBG("### Got Data:", len_total, "on", mux);
        } else if (data.endsWith(GF("CLOSED" GSM_NL))) {
          int8_t nl   = data.lastIndexOf(GSM_NL, data.length() - 8);
          int8_t coma = data.indexOf(',', nl + 2);
          int8_t mux  = data.substring(nl + 2, coma).toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### Closed: ", mux);
        } else if (data.endsWith(GF("+QNITZ:"))) {
          streamSkipUntil('\n');  // URC for time sync
          DBG("### Network time updated.");
          data = "";
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
                      GsmConstStr r5 = NULL, GsmConstStr r6 = NULL) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5, r6);
  }

  int8_t waitResponse(GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL, GsmConstStr r6 = NULL) {
    return waitResponse(1000, r1, r2, r3, r4, r5, r6);
  }

 public:
  Stream& stream;

 protected:
  GsmClientMC60* sockets[TINY_GSM_MUX_COUNT];
  const char*    gsmNL = GSM_NL;
};

#endif  // SRC_TINYGSMCLIENTMC60_H_
