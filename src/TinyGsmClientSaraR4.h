/**
 * @file       TinyGsmClientSaraR4.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSARAR4_H_
#define SRC_TINYGSMCLIENTSARAR4_H_
// #pragma message("TinyGSM:  TinyGsmClientSaraR4")

// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MUX_COUNT 7

#include "TinyGsmBattery.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmModem.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmTemperature.tpp"
#include "TinyGsmTime.tpp"

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM        = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM     = "ERROR" GSM_NL;
static const char GSM_CME_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CME ERROR:";

enum RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};

class TinyGsmSaraR4
    : public TinyGsmModem<TinyGsmSaraR4>,
      public TinyGsmGPRS<TinyGsmSaraR4>,
      public TinyGsmTCP<TinyGsmSaraR4, READ_AND_CHECK_SIZE, TINY_GSM_MUX_COUNT>,
      public TinyGsmSSL<TinyGsmSaraR4>,
      public TinyGsmBattery<TinyGsmSaraR4>,
      public TinyGsmGSMLocation<TinyGsmSaraR4>,
      public TinyGsmSMS<TinyGsmSaraR4>,
      public TinyGsmTemperature<TinyGsmSaraR4>,
      public TinyGsmTime<TinyGsmSaraR4> {
  friend class TinyGsmModem<TinyGsmSaraR4>;
  friend class TinyGsmGPRS<TinyGsmSaraR4>;
  friend class TinyGsmTCP<TinyGsmSaraR4, READ_AND_CHECK_SIZE,
                          TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmSaraR4>;
  friend class TinyGsmBattery<TinyGsmSaraR4>;
  friend class TinyGsmGSMLocation<TinyGsmSaraR4>;
  friend class TinyGsmSMS<TinyGsmSaraR4>;
  friend class TinyGsmTemperature<TinyGsmSaraR4>;
  friend class TinyGsmTime<TinyGsmSaraR4>;

  /*
   * Inner Client
   */
 public:
  class GsmClientSaraR4 : public GsmClient {
    friend class TinyGsmSaraR4;

   public:
    GsmClientSaraR4() {}

    explicit GsmClientSaraR4(TinyGsmSaraR4& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmSaraR4* modem, uint8_t mux = 0) {
      this->at       = modem;
      this->mux      = mux;
      sock_available = 0;
      prev_check     = 0;
      sock_connected = false;
      got_data       = false;

      at->sockets[mux] = this;

      return true;
    }

   public:
    virtual int connect(const char* host, uint16_t port, int timeout_s) {
      stop();
      TINY_GSM_YIELD();
      rx.clear();

      uint8_t oldMux = mux;
      sock_connected = at->modemConnect(host, port, &mux, false, timeout_s);
      if (mux != oldMux) {
        DBG("WARNING:  Mux number changed from", oldMux, "to", mux);
        at->sockets[oldMux] = NULL;
      }
      at->sockets[mux] = this;
      at->maintain();

      return sock_connected;
    }
    virtual int connect(IPAddress ip, uint16_t port, int timeout_s) {
      return connect(TinyGsmStringFromIp(ip).c_str(), port, timeout_s);
    }
    int connect(const char* host, uint16_t port) override {
      return connect(host, port, 120);
    }
    int connect(IPAddress ip, uint16_t port) override {
      return connect(ip, port, 120);
    }

    virtual void stop(uint32_t maxWaitMs) {
      uint32_t startMillis = millis();
      dumpModemBuffer(maxWaitMs);
      // We want to use an async socket close because the syncrhonous close of
      // an open socket is INCREDIBLY SLOW and the modem can freeze up.  But we
      // only attempt the async close if we already KNOW the socket is open
      // because calling the async close on a closed socket and then attempting
      // opening a new socket causes the board to lock up for 2-3 minutes and
      // then finally return with a "new" socket that is immediately closed.
      // Attempting to close a socket that is already closed with a synchronous
      // close quickly returns an error.
      if (at->supportsAsyncSockets && sock_connected) {
        DBG("### Closing socket asynchronously!  Socket might remain open "
            "until arrival of +UUSOCL:",
            mux);
        // faster asynchronous close
        // NOT supported on SARA-R404M / SARA-R410M-01B
        at->sendAT(GF("+USOCL="), mux, GF(",1"));
        // NOTE:  can take up to 120s to get a response
        at->waitResponse((maxWaitMs - (millis() - startMillis)));
        // We set the sock as disconnected right away because it can no longer
        // be used
        sock_connected = false;
      } else {
        // synchronous close
        at->sendAT(GF("+USOCL="), mux);
        // NOTE:  can take up to 120s to get a response
        at->waitResponse((maxWaitMs - (millis() - startMillis)));
        sock_connected = false;
      }
    }
    void stop() override {
      stop(135000L);
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  };

  /*
   * Inner Secure Client
   */
 public:
  class GsmClientSecureR4 : public GsmClientSaraR4 {
   public:
    GsmClientSecureR4() {}

    explicit GsmClientSecureR4(TinyGsmSaraR4& modem, uint8_t mux = 1)
        : GsmClientSaraR4(modem, mux) {}

   public:
    int connect(const char* host, uint16_t port, int timeout_s) override {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      uint8_t oldMux = mux;
      sock_connected = at->modemConnect(host, port, &mux, true, timeout_s);
      if (mux != oldMux) {
        DBG("WARNING:  Mux number changed from", oldMux, "to", mux);
        at->sockets[oldMux] = NULL;
      }
      at->sockets[mux] = this;
      at->maintain();
      return sock_connected;
    }
    virtual int connect(IPAddress ip, uint16_t port, int timeout_s) {
      return connect(TinyGsmStringFromIp(ip).c_str(), port, timeout_s);
    }
    int connect(const char* host, uint16_t port) override {
      return connect(host, port, 120);
    }
    int connect(IPAddress ip, uint16_t port) override {
      return connect(ip, port, 120);
    }
  };

  /*
   * Constructor
   */
 public:
  explicit TinyGsmSaraR4(Stream& stream)
      : stream(stream),
        has2GFallback(false),
        supportsAsyncSockets(false) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    String modemName = getModemName();
    DBG(GF("### Modem:"), modemName);
    if (modemName.startsWith("u-blox SARA-R412")) {
      has2GFallback = true;
    } else {
      has2GFallback = false;
    }
    if (modemName.startsWith("u-blox SARA-R404M") ||
        modemName.startsWith("u-blox SARA-R410M-01B")) {
      supportsAsyncSockets = false;
    } else {
      supportsAsyncSockets = true;
    }

    // Enable automatic time zome update
    sendAT(GF("+CTZU=1"));
    if (waitResponse(10000L) != 1) { return false; }

    int ret = getSimStatus();
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

  // only difference in implementation is the warning on the wrong type
  String getModemNameImpl() {
    sendAT(GF("+CGMI"));
    String res1;
    if (waitResponse(1000L, res1) != 1) { return "u-blox Cellular Modem"; }
    res1.replace(GSM_NL "OK" GSM_NL, "");
    res1.trim();

    sendAT(GF("+GMM"));
    String res2;
    if (waitResponse(1000L, res2) != 1) { return "u-blox Cellular Modem"; }
    res2.replace(GSM_NL "OK" GSM_NL, "");
    res2.trim();

    String name = res1 + String(' ') + res2;
    DBG("### Modem:", name);
    if (!name.startsWith("u-blox SARA-R4") &&
        !name.startsWith("u-blox SARA-N4")) {
      DBG("### WARNING:  You are using the wrong TinyGSM modem!");
    }

    return name;
  }

  bool factoryDefaultImpl() {
    sendAT(GF("&F"));  // Resets the current profile, other NVM not affected
    return waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  // using +CFUN=15 instead of the more common CFUN=1,1
  bool restartImpl() {
    if (!testAT()) { return false; }
    sendAT(GF("+CFUN=15"));
    if (waitResponse(10000L) != 1) { return false; }
    delay(3000);  // TODO(?):  Verify delay timing here
    return init();
  }

  bool powerOffImpl() {
    sendAT(GF("+CPWROFF"));
    return waitResponse(40000L) == 1;
  }

  bool sleepEnableImpl(bool enable = true) TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Generic network functions
   */
 public:
  RegStatus getRegistrationStatus() {
    // Check first for EPS registration
    sendAT(GF("+CEREG?"));
    if (waitResponse(GF(GSM_NL "+CEREG:")) != 1) { return REG_UNKNOWN; }
    streamSkipUntil(','); /* Skip format (0) */
    int status = streamGetInt('\n');
    waitResponse();

    // If we're connected on EPS, great!
    if ((RegStatus)status == REG_OK_HOME ||
        (RegStatus)status == REG_OK_ROAMING) {
      return (RegStatus)status;
    } else {
      // Otherwise, check generic network status
      sendAT(GF("+CREG?"));
      if (waitResponse(GF(GSM_NL "+CREG:")) != 1) { return REG_UNKNOWN; }
      streamSkipUntil(','); /* Skip format (0) */
      status = streamGetInt('\n');
      waitResponse();
      return (RegStatus)status;
    }
  }

 protected:
  bool isNetworkConnectedImpl() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

 public:
  bool setURAT(uint8_t urat) {
    // AT+URAT=<SelectedAcT>[,<PreferredAct>[,<2ndPreferredAct>]]

    sendAT(GF("+COPS=2"));  // Deregister from network
    if (waitResponse() != 1) { return false; }
    sendAT(GF("+URAT="), urat);  // Radio Access Technology (RAT) selection
    if (waitResponse() != 1) { return false; }
    sendAT(GF("+COPS=0"));  // Auto-register to the network
    if (waitResponse() != 1) { return false; }
    return restart();
  }

  /*
   * GPRS functions
   */
 protected:
  bool gprsConnectImpl(const char* apn, const char* user = NULL,
                       const char* pwd = NULL) {
    // gprsDisconnect();

    sendAT(GF("+CGATT=1"));  // attach to GPRS
    if (waitResponse(360000L) != 1) { return false; }

    // Using CGDCONT sets up an "external" PCP context, i.e. a data connection
    // using the external IP stack (e.g. Windows dial up) and PPP link over the
    // serial interface.  This is the only command set supported by the LTE-M
    // and LTE NB-IoT modules (SARA-R4xx, SARA-N4xx)

    // Set the authentication
    if (user && strlen(user) > 0) {
      sendAT(GF("+CGAUTH=1,0,\""), user, GF("\",\""), pwd, '"');
      waitResponse();
    }

    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');  // Define PDP context 1
    waitResponse();

    sendAT(GF("+CGACT=1,1"));  // activate PDP profile/context 1
    if (waitResponse(150000L) != 1) { return false; }

    return true;
  }

  bool gprsDisconnectImpl() {
    // Mark all the sockets as closed
    // This ensures that asynchronously closed sockets are marked closed
    for (int mux = 0; mux < TINY_GSM_MUX_COUNT; mux++) {
      GsmClientSaraR4* sock = sockets[mux];
      if (sock && sock->sock_connected) { sock->sock_connected = false; }
    }

    // sendAT(GF("+CGACT=0,1"));  // Deactivate PDP context 1
    sendAT(GF("+CGACT=0"));  // Deactivate all contexts
    if (waitResponse(40000L) != 1) {
      // return false;
    }

    sendAT(GF("+CGATT=0"));  // detach from GPRS
    if (waitResponse(360000L) != 1) { return false; }

    return true;
  }

  /*
   * SIM card functions
   */
 protected:
  // This uses "CGSN" instead of "GSN"
  String getIMEIImpl() {
    sendAT(GF("+CGSN"));
    if (waitResponse(GF(GSM_NL)) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Messaging functions
   */
 protected:
  String sendUSSDImpl(const String& code) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool   sendSMS_UTF16Impl(const String& number, const void* text,
                           size_t len) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Location functions
   */
 protected:
  String getGsmLocationImpl() {
    sendAT(GF("+ULOC=2,3,0,120,1"));
    if (waitResponse(30000L, GF(GSM_NL "+UULOC:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * GPS location functions
   */
 protected:
  // No functions of this type supported

  /*
   * Time functions
   */
 protected:
  // Can follow the standard CCLK function in the template

  /*
   * Battery functions
   */
 protected:
  uint16_t getBattVoltageImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  int8_t getBattPercentImpl() {
    sendAT(GF("+CIND?"));
    if (waitResponse(GF(GSM_NL "+CIND:")) != 1) { return 0; }

    int    res     = streamGetInt(',');
    int8_t percent = res * 20;  // return is 0-5
    // Wait for final OK
    waitResponse();
    return percent;
  }

  uint8_t getBattChargeStateImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  bool getBattStatsImpl(uint8_t& chargeState, int8_t& percent,
                        uint16_t& milliVolts) {
    chargeState = 0;
    percent     = getBattPercent();
    milliVolts  = 0;
    return true;
  }

  /*
   * Temperature functions
   */

  float getTemperatureImpl() {
    // First make sure the temperature is set to be in celsius
    sendAT(GF("+UTEMP=0"));  // Would use 1 for Fahrenheit
    if (waitResponse() != 1) { return static_cast<float>(-9999); }
    sendAT(GF("+UTEMP?"));
    if (waitResponse(GF(GSM_NL "+UTEMP:")) != 1) {
      return static_cast<float>(-9999);
    }
    int16_t res  = streamGetInt('\n');
    float   temp = -9999;
    if (res != -1) { temp = (static_cast<float>(res)) / 10; }
    return temp;
  }

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t* mux,
                    bool ssl = false, int timeout_s = 120) {
    uint32_t timeout_ms  = ((uint32_t)timeout_s) * 1000;
    uint32_t startMillis = millis();

    // create a socket
    sendAT(GF("+USOCR=6"));
    // reply is +USOCR: ## of socket created
    if (waitResponse(GF(GSM_NL "+USOCR:")) != 1) { return false; }
    *mux = streamGetInt('\n');
    waitResponse();

    if (ssl) {
      sendAT(GF("+USOSEC="), *mux, ",1");
      waitResponse();
    }

    // Enable NODELAY
    // AT+USOSO=<socket>,<level>,<opt_name>,<opt_val>[,<opt_val2>]
    // <level> - 0 for IP, 6 for TCP, 65535 for socket level options
    // <opt_name> TCP/1 = no delay (do not delay send to coalesce packets)
    // NOTE:  Enabling this may increase data plan usage
    // sendAT(GF("+USOSO="), *mux, GF(",6,1,1"));
    // waitResponse();

    // Enable KEEPALIVE, 30 sec
    // sendAT(GF("+USOSO="), *mux, GF(",6,2,30000"));
    // waitResponse();

    // connect on the allocated socket

    // Use an asynchronous open to reduce the number of terminal freeze-ups
    // This is still blocking until the URC arrives
    // The SARA-R410M-02B with firmware revisions prior to L0.0.00.00.05.08
    // has a nasty habit of locking up when opening a socket, especially if
    // the cellular service is poor.
    // NOT supported on SARA-R404M / SARA-R410M-01B
    if (supportsAsyncSockets) {
      DBG("### Opening socket asynchronously!  Socket cannot be used until "
          "the URC '+UUSOCO' appears.");
      sendAT(GF("+USOCO="), *mux, ",\"", host, "\",", port, ",1");
      if (waitResponse(timeout_ms - (millis() - startMillis),
                       GF(GSM_NL "+UUSOCO:")) == 1) {
        streamGetInt(',');  // skip repeated mux
        int connection_status = streamGetInt('\n');
        DBG("### Waited", millis() - startMillis, "ms for socket to open");
        return (0 == connection_status);
      } else {
        DBG("### Waited", millis() - startMillis,
            "but never got socket open notice");
        return false;
      }
    } else {
      // use synchronous open
      sendAT(GF("+USOCO="), *mux, ",\"", host, "\",", port);
      int rsp = waitResponse(timeout_ms - (millis() - startMillis));
      return (1 == rsp);
    }
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+USOWR="), mux, ',', (uint16_t)len);
    if (waitResponse(GF("@")) != 1) { return 0; }
    // 50ms delay, see AT manual section 25.10.4
    delay(50);
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(GF(GSM_NL "+USOWR:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux
    int sent = streamGetInt('\n');
    waitResponse();  // sends back OK after the confirmation of number sent
    return sent;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    sendAT(GF("+USORD="), mux, ',', (uint16_t)size);
    if (waitResponse(GF(GSM_NL "+USORD:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux
    int len = streamGetInt(',');
    streamSkipUntil('\"');

    for (int i = 0; i < len; i++) { moveCharFromStreamToFifo(mux); }
    streamSkipUntil('\"');
    waitResponse();
    DBG("### READ:", len, "from", mux);
    sockets[mux]->sock_available = modemGetAvailable(mux);
    return len;
  }

  size_t modemGetAvailable(uint8_t mux) {
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
    sendAT(GF("+USORD="), mux, ",0");
    size_t  result = 0;
    uint8_t res    = waitResponse(GF(GSM_NL "+USORD:"));
    // Will give error "operation not allowed" when attempting to read a socket
    // that you have already told to close
    if (res == 1) {
      streamSkipUntil(',');  // Skip mux
      result = streamGetInt('\n');
      // if (result) DBG("### DATA AVAILABLE:", result, "on", mux);
      waitResponse();
    }
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    DBG("### AVAILABLE:", result, "on", mux);
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
    sendAT(GF("+USOCTL="), mux, ",10");
    uint8_t res = waitResponse(GF(GSM_NL "+USOCTL:"));
    if (res != 1) { return false; }

    streamSkipUntil(',');  // Skip mux
    streamSkipUntil(',');  // Skip type
    int result = streamGetInt('\n');
    // 0: the socket is in INACTIVE status (it corresponds to CLOSED status
    // defined in RFC793 "TCP Protocol Specification" [112])
    // 1: the socket is in LISTEN status
    // 2: the socket is in SYN_SENT status
    // 3: the socket is in SYN_RCVD status
    // 4: the socket is in ESTABILISHED status
    // 5: the socket is in FIN_WAIT_1 status
    // 6: the socket is in FIN_WAIT_2 status
    // 7: the sokcet is in CLOSE_WAIT status
    // 8: the socket is in CLOSING status
    // 9: the socket is in LAST_ACK status
    // 10: the socket is in TIME_WAIT status
    waitResponse();
    return (result != 0);
  }

  /*
   * Utilities
   */
 public:
  // TODO(vshymanskyy): Optimize this!
  uint8_t waitResponse(uint32_t timeout_ms, String& data,
                       GsmConstStr r1 = GFP(GSM_OK),
                       GsmConstStr r2 = GFP(GSM_ERROR),
                       GsmConstStr r3 = GFP(GSM_CME_ERROR),
                       GsmConstStr r4 = NULL, GsmConstStr r5 = NULL) {
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
        int a = stream.read();
        if (a <= 0) continue;  // Skip 0x00 bytes, just in case
        data += static_cast<char>(a);
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
          index = 2;
          goto finish;
        } else if (r3 && data.endsWith(r3)) {
          if (r3 == GFP(GSM_CME_ERROR)) {
            streamSkipUntil('\n');  // Read out the error
          }
          index = 3;
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (data.endsWith(GF("+UUSORD:"))) {
          int mux = streamGetInt(',');
          int len = streamGetInt('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data       = true;
            sockets[mux]->sock_available = len;
          }
          data = "";
          DBG("### URC Data Received:", len, "on", mux);
        } else if (data.endsWith(GF("+UUSOCL:"))) {
          int mux = streamGetInt('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### URC Sock Closed: ", mux);
        } else if (data.endsWith(GF("+UUSOCO:"))) {
          int mux          = streamGetInt('\n');
          int socket_error = streamGetInt('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux] &&
              socket_error == 0) {
            sockets[mux]->sock_connected = true;
          }
          data = "";
          DBG("### URC Sock Opened: ", mux);
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

  uint8_t waitResponse(uint32_t timeout_ms, GsmConstStr r1 = GFP(GSM_OK),
                       GsmConstStr r2 = GFP(GSM_ERROR),
                       GsmConstStr r3 = GFP(GSM_CME_ERROR),
                       GsmConstStr r4 = NULL, GsmConstStr r5 = NULL) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
  }

  uint8_t waitResponse(GsmConstStr r1 = GFP(GSM_OK),
                       GsmConstStr r2 = GFP(GSM_ERROR),
                       GsmConstStr r3 = GFP(GSM_CME_ERROR),
                       GsmConstStr r4 = NULL, GsmConstStr r5 = NULL) {
    return waitResponse(1000, r1, r2, r3, r4, r5);
  }

 protected:
  Stream&          stream;
  GsmClientSaraR4* sockets[TINY_GSM_MUX_COUNT];
  const char*      gsmNL = GSM_NL;
  bool             has2GFallback;
  bool             supportsAsyncSockets;
};

#endif  // SRC_TINYGSMCLIENTSARAR4_H_
