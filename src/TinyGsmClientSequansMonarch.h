/**
 * @file       TinyGsmClientSequansMonarch.h
 * @author     Michael Krumpus
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2019 Michael Krumpus
 * @date       Jan 2019
 */

#ifndef SRC_TINYGSMCLIENTSEQUANSMONARCH_H_
#define SRC_TINYGSMCLIENTSEQUANSMONARCH_H_

// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MUX_COUNT 6
#define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE

#include "TinyGsmCalling.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmModem.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmTemperature.tpp"
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

enum SocketStatus {
  SOCK_CLOSED                 = 0,
  SOCK_ACTIVE_DATA            = 1,
  SOCK_SUSPENDED              = 2,
  SOCK_SUSPENDED_PENDING_DATA = 3,
  SOCK_LISTENING              = 4,
  SOCK_INCOMING               = 5,
  SOCK_OPENING                = 6,
};

class TinyGsmSequansMonarch
    : public TinyGsmModem<TinyGsmSequansMonarch>,
      public TinyGsmGPRS<TinyGsmSequansMonarch>,
      public TinyGsmTCP<TinyGsmSequansMonarch, TINY_GSM_MUX_COUNT>,
      public TinyGsmSSL<TinyGsmSequansMonarch>,
      public TinyGsmCalling<TinyGsmSequansMonarch>,
      public TinyGsmSMS<TinyGsmSequansMonarch>,
      public TinyGsmTime<TinyGsmSequansMonarch>,
      public TinyGsmTemperature<TinyGsmSequansMonarch> {
  friend class TinyGsmModem<TinyGsmSequansMonarch>;
  friend class TinyGsmGPRS<TinyGsmSequansMonarch>;
  friend class TinyGsmTCP<TinyGsmSequansMonarch, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmSequansMonarch>;
  friend class TinyGsmCalling<TinyGsmSequansMonarch>;
  friend class TinyGsmSMS<TinyGsmSequansMonarch>;
  friend class TinyGsmTime<TinyGsmSequansMonarch>;
  friend class TinyGsmTemperature<TinyGsmSequansMonarch>;

  /*
   * Inner Client
   */
 public:
  class GsmClientSequansMonarch : public GsmClient {
    friend class TinyGsmSequansMonarch;

   public:
    GsmClientSequansMonarch() {}

    explicit GsmClientSequansMonarch(TinyGsmSequansMonarch& modem,
                                     uint8_t                mux = 1) {
      init(&modem, mux);
    }

    bool init(TinyGsmSequansMonarch* modem, uint8_t mux = 1) {
      this->at       = modem;
      sock_available = 0;
      prev_check     = 0;
      sock_connected = false;
      got_data       = false;

      // adjust for zero indexed socket array vs Sequans' 1 indexed mux numbers
      // using modulus will force 6 back to 0
      if (mux >= 1 && mux <= TINY_GSM_MUX_COUNT) {
        this->mux = mux;
      } else {
        this->mux = (mux % TINY_GSM_MUX_COUNT) + 1;
      }
      at->sockets[this->mux % TINY_GSM_MUX_COUNT] = this;

      return true;
    }

   public:
    virtual int connect(const char* host, uint16_t port, int timeout_s) {
      if (sock_connected) stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void stop(uint32_t maxWaitMs) {
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+SQNSH="), mux);
      sock_connected = false;
      at->waitResponse();
    }
    void stop() override {
      stop(15000L);
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
  class GsmClientSecureSequansMonarch : public GsmClientSequansMonarch {
   public:
    GsmClientSecureSequansMonarch() {}

    explicit GsmClientSecureSequansMonarch(TinyGsmSequansMonarch& modem,
                                           uint8_t                mux = 1)
        : GsmClientSequansMonarch(modem, mux) {}

   protected:
    bool strictSSL = false;

   public:
    int connect(const char* host, uint16_t port, int timeout_s) override {
      stop();
      TINY_GSM_YIELD();
      rx.clear();

      // configure security profile 1 with parameters:
      if (strictSSL) {
        // require minimum of TLS 1.2 (3)
        // only support cipher suite 0x3D: TLS_RSA_WITH_AES_256_CBC_SHA256
        // verify server certificate against imported CA certs 0 and enforce
        // validity period (3)
        at->sendAT(GF("+SQNSPCFG=1,3,\"0x3D\",3,0,,,\"\",\"\""));
      } else {
        // use TLS 1.0 or higher (1)
        // support wider variety of cipher suites
        // do not verify server certificate (0)
        at->sendAT(GF("+SQNSPCFG=1,1,\"0x2F;0x35;0x3C;0x3D\",0,,,,\"\",\"\""));
      }
      if (at->waitResponse() != 1) {
        DBG("failed to configure security profile");
        return false;
      }

      sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void setStrictSSL(bool strict) {
      strictSSL = strict;
    }
  };

  /*
   * Constructor
   */
 public:
  explicit TinyGsmSequansMonarch(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSequansMonarch"));

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

    // Make sure the module is enabled. Unlike others, the VZN20Q powers on
    // with CFUN=0 not CFUN=1 (that is, at minimum functionality instead of full
    // functionality The module cannot even detect the sim card if the cellular
    // functionality is disabled so unless we explicitly enable the
    // functionality the init will fail.
    sendAT(GF("+CFUN=1"));
    waitResponse();

    // Disable time and time zone URC's
    sendAT(GF("+CTZR=0"));
    if (waitResponse(10000L) != 1) { return false; }

    // Enable automatic time zome update
    sendAT(GF("+CTZU=1"));
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

  String getModemNameImpl() {
    sendAT(GF("+CGMI"));
    String res1;
    if (waitResponse(1000L, res1) != 1) { return "unknown"; }
    res1.replace("\r\nOK\r\n", "");
    res1.replace("\rOK\r", "");
    res1.trim();

    sendAT(GF("+CGMM"));
    String res2;
    if (waitResponse(1000L, res2) != 1) { return "unknown"; }
    res2.replace("\r\nOK\r\n", "");
    res2.replace("\rOK\r", "");
    res2.trim();

    String name = res1 + String(' ') + res2;
    DBG("### Modem:", name);
    return name;
  }

  bool factoryDefaultImpl() {
    sendAT(GF("&F0"));  // Factory
    waitResponse();
    sendAT(GF("Z"));  // default configuration
    waitResponse();
    sendAT(GF("+IPR=0"));  // Auto-baud
    return waitResponse() == 1;
  }

  void maintainImpl() {
    for (int mux = 1; mux <= TINY_GSM_MUX_COUNT; mux++) {
      GsmClientSequansMonarch* sock = sockets[mux % TINY_GSM_MUX_COUNT];
      if (sock && sock->got_data) {
        sock->got_data       = false;
        sock->sock_available = modemGetAvailable(mux);
        // modemGetConnected() always checks the state of ALL socks
        modemGetConnected();
      }
    }
    while (stream.available()) { waitResponse(15, NULL, NULL); }
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = NULL) {
    if (!testAT()) { return false; }

    sendAT(GF("+CFUN=0"));
    int8_t res = waitResponse(20000L, GFP(GSM_OK), GFP(GSM_ERROR),
                              GF("+SYSSTART"));
    if (res != 1 && res != 3) { return false; }

    sendAT(GF("+CFUN=1,1"));
    res = waitResponse(20000L, GF("+SYSSTART"), GFP(GSM_ERROR));
    if (res != 1 && res != 3) { return false; }
    delay(1000);
    return init(pin);
  }

  bool powerOffImpl() {
    // NOTE:  The only way to turn the modem back on after this shutdown is with
    // a hard reset
    sendAT(GF("+SQNSSHDN"));
    return waitResponse();
  }

  //  When power saving is enabled, UART0 interface is activated with sleep mode
  //  support. Module power state is controlled by RTS0 line.  When no activity
  //  on UART, CTS line will be set to OFF state (driven high level) <timeout>
  //  milliseconds (100ms to 10s, default 5s) after the last sent character,
  //  then module will go to sleep mode as soon as DTE set RTS line to OFF state
  //  (driver high level).
  bool sleepEnableImpl(bool enable = true) {
    sendAT(GF("+SQNIPSCFG="), enable);
    return waitResponse() == 1;
  }

  bool setPhoneFunctionality(uint8_t fun,
                             bool reset = false) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 public:
  RegStatus getRegistrationStatus() {
    return (RegStatus)getRegistrationStatusXREG("CEREG");
  }

 protected:
  bool isNetworkConnectedImpl() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }
  String getLocalIPImpl() {
    sendAT(GF("+CGPADDR=3"));
    if (waitResponse(10000L, GF("+CGPADDR: 3,\"")) != 1) { return ""; }
    String res = stream.readStringUntil('\"');
    waitResponse();
    return res;
  }

  /*
   * GPRS functions
   */
 protected:
  bool gprsConnectImpl(const char* apn, const char* user = NULL,
                       const char* pwd = NULL) {
    gprsDisconnect();

    // Define the PDP context (This uses context #3!)
    sendAT(GF("+CGDCONT=3,\"IPV4V6\",\""), apn, '"');
    waitResponse();

    // Set authentication
    if (user && strlen(user) > 0) {
      sendAT(GF("+CGAUTH=3,1,\""), user, GF("\",\""), pwd, GF("\""));
      waitResponse();
    }

    // Activate the PDP context
    sendAT(GF("+CGACT=1,3"));
    waitResponse(60000L);

    // Attach to GPRS
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  bool gprsDisconnectImpl() {
    // Detach from PS network
    sendAT(GF("+CGATT=0"));
    if (waitResponse(60000L) != 1) { return false; }
    // Dectivate all PDP contexts
    sendAT(GF("+CGACT=0"));
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  /*
   * SIM card functions
   */
 protected:
  String getSimCCIDImpl() {
    sendAT(GF("+SQNCCID"));
    if (waitResponse(GF(GSM_NL "+SQNCCID:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone Call functions
   */
 protected:
  bool callAnswerImpl() TINY_GSM_ATTR_NOT_AVAILABLE;
  bool dtmfSendImpl(char cmd,
                    int  duration_ms = 100) TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Messaging functions
   */
 protected:
  // Follows all messaging functions per template

  /*
   * Time functions
   */
 protected:
  // Can follow the standard CCLK function in the template

  /*
   * Temperature functions
   */
 protected:
  float getTemperatureImpl() {
    sendAT(GF("+SMDTH"));
    if (waitResponse(10000L, GF("+SMDTH: ")) != 1) {
      return static_cast<float>(-9999);
    }
    String res;
    if (waitResponse(1000L, res) != 1) { return static_cast<float>(-9999); }
    if (res.indexOf("ERROR") >= 0) { return static_cast<float>(-9999); }
    return res.toFloat();
  }

 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
    int8_t   rsp;
    uint32_t startMillis = millis();
    uint32_t timeout_ms  = ((uint32_t)timeout_s) * 1000;

    if (ssl) {
      // enable SSl and use security profile 1
      // AT+SQNSSCFG=<connId>,<enable>,<spId>
      sendAT(GF("+SQNSSCFG="), mux, GF(",1,1"));
      if (waitResponse() != 1) {
        DBG("### WARNING: failed to configure secure socket");
        return false;
      }
    }

    // Socket configuration
    // AT+SQNSCFG:<connId1>, <cid1>, <pktSz1>, <maxTo1>, <connTo1>, <txTo1>
    // <connId1> = Connection ID = mux
    // <cid1> = PDP context ID = 3 - this is number set up above in the
    // GprsConnect function
    // <pktSz1> = Packet Size, used for online data mode only = 300 (default)
    // <maxTo1> = Max timeout in seconds = 90 (default)
    // <connTo1> = Connection timeout in hundreds of milliseconds
    //           = 600 (default)
    // <txTo1> = Data sending timeout in hundreds of milliseconds,
    // used for online data mode only = 50 (default)
    sendAT(GF("+SQNSCFG="), mux, GF(",3,300,90,600,50"));
    waitResponse(5000L);

    // Socket configuration extended
    // AT+SQNSCFGEXT:<connId1>, <srMode1>, <recvDataMode1>, <keepalive1>,
    // <listenAutoRsp1>, <sendDataMode1>
    // <connId1> = Connection ID = mux
    // <srMode1> = Send/Receive URC model = 1 - data amount mode
    // <recvDataMode1> = Receive data mode = 0  - data as text (1 for hex)
    // <keepalive1> = unused = 0
    // <listenAutoRsp1> = Listen auto-response mode = 0 - deactivated
    // <sendDataMode1> = Send data mode = 1  - data as hex (0 for text)
    sendAT(GF("+SQNSCFGEXT="), mux, GF(",1,0,0,0,1"));
    waitResponse(5000L);

    // Socket dial
    // AT+SQNSD=<connId>,<txProt>,<rPort>,<IPaddr>[,<closureType>[,<lPort>[,<connMode>[,acceptAnyRemote]]]]
    // <connId> = Connection ID = mux
    // <txProt> = Transmission protocol = 0 - TCP (1 for UDP)
    // <rPort> = Remote host port to contact
    // <IPaddr> = Any valid IP address in the format xxx.xxx.xxx.xxx or any
    // host name solved with a DNS query
    // <closureType> = Socket closure behaviour for TCP, has no effect for UDP
    //               = 0 - local port closes when remote does (default)
    // <lPort> = UDP connection local port, has no effect for TCP connections.
    // <connMode> = Connection mode = 1 - command mode connection
    // <acceptAnyRemote> = Applies to UDP only
    sendAT(GF("+SQNSD="), mux, ",0,", port, ',', GF("\""), host, GF("\""),
           ",0,0,1");
    rsp = waitResponse((timeout_ms - (millis() - startMillis)), GFP(GSM_OK),
                       GFP(GSM_ERROR), GF("NO CARRIER" GSM_NL));

    // creation of socket failed immediately.
    if (rsp != 1) { return false; }

    // wait until we get a good status
    bool connected = false;
    while (!connected && ((millis() - startMillis) < timeout_ms)) {
      connected = modemGetConnected(mux);
      delay(100);  // socket may be in opening state
    }
    return connected;
  }

  int modemSend(const void* buff, size_t len, uint8_t mux) {
    if (sockets[mux % TINY_GSM_MUX_COUNT]->sock_connected == false) {
      DBG("### Sock closed, cannot send data!");
      return 0;
    }

    sendAT(GF("+SQNSSENDEXT="), mux, ',', (uint16_t)len);
    waitResponse(10000L, GF(GSM_NL "> "));
    // Translate bytes into char to be able to send them as an hex string
    char char_command[2];
    for (int i=0; i<len; i++) {
      memset(&char_command, 0, sizeof(char_command));
      sprintf(&char_command[0], "%02X", reinterpret_cast<const uint8_t*>(buff)[i]);
      stream.write(char_command, sizeof(char_command));
    }
    stream.flush();
    if (waitResponse() != 1) {
      DBG("### no OK after send");
      return 0;
    }
    return len;

    // uint8_t nAttempts = 5;
    // bool gotPrompt = false;
    // while (nAttempts > 0 && !gotPrompt) {
    //   sendAT(GF("+SQNSSEND="), mux);
    //   if (waitResponse(5000, GF(GSM_NL "> ")) == 1) {
    //     gotPrompt = true;
    //   }
    //   nAttempts--;
    //   delay(50);
    // }
    // if (gotPrompt) {
    //   stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    //   stream.write(reinterpret_cast<char>0x1A);
    //   stream.flush();
    //   if (waitResponse() != 1) {
    //     DBG("### no OK after send");
    //     return 0;
    //   }
    //   return len;
    // }
    // return 0;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    sendAT(GF("+SQNSRECV="), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+SQNSRECV: ")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux
    int16_t len = streamGetIntBefore('\n');
    for (int i = 0; i < len; i++) {
      uint32_t startMillis = millis();
      while (!stream.available() &&
             ((millis() - startMillis) <
              sockets[mux % TINY_GSM_MUX_COUNT]->_timeout)) {
        TINY_GSM_YIELD();
      }
      char c = stream.read();
      sockets[mux % TINY_GSM_MUX_COUNT]->rx.put(c);
    }
    // DBG("### READ:", len, "from", mux);
    waitResponse();
    sockets[mux % TINY_GSM_MUX_COUNT]->sock_available = modemGetAvailable(mux);
    return len;
  }

  size_t modemGetAvailable(uint8_t mux) {
    sendAT(GF("+SQNSI="), mux);
    size_t result = 0;
    if (waitResponse(GF("+SQNSI:")) == 1) {
      streamSkipUntil(',');              // Skip mux
      streamSkipUntil(',');              // Skip total sent
      streamSkipUntil(',');              // Skip total received
      result = streamGetIntBefore(',');  // keep data not yet read
      waitResponse();
    }
    // DBG("### Available:", result, "on", mux);
    return result;
  }

  bool modemGetConnected(uint8_t mux = 1) {
    // This single command always returns the connection status of all
    // six possible sockets.
    sendAT(GF("+SQNSS"));
    for (int muxNo = 1; muxNo <= TINY_GSM_MUX_COUNT; muxNo++) {
      if (waitResponse(GFP(GSM_OK), GF(GSM_NL "+SQNSS: ")) != 2) { break; }
      uint8_t status = 0;
      // if (streamGetIntBefore(',') != muxNo) { // check the mux no
      //   DBG("### Warning: misaligned mux numbers!");
      // }
      streamSkipUntil(',');        // skip mux [use muxNo]
      status = stream.parseInt();  // Read the status
      // if mux is in use, will have comma then other info after the status
      // if not, there will be new line immediately after status
      // streamSkipUntil('\n'); // Skip port and IP info
      // SOCK_CLOSED                 = 0,
      // SOCK_ACTIVE_DATA            = 1,
      // SOCK_SUSPENDED              = 2,
      // SOCK_SUSPENDED_PENDING_DATA = 3,
      // SOCK_LISTENING              = 4,
      // SOCK_INCOMING               = 5,
      // SOCK_OPENING                = 6,
      GsmClientSequansMonarch* sock = sockets[muxNo % TINY_GSM_MUX_COUNT];
      if (sock) {
        sock->sock_connected = ((status != SOCK_CLOSED) &&
                                (status != SOCK_INCOMING) &&
                                (status != SOCK_OPENING));
      }
    }
    waitResponse();  // Should be an OK at the end
    return sockets[mux % TINY_GSM_MUX_COUNT]->sock_connected;
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
        } else if (data.endsWith(GF(GSM_NL "+SQNSRING:"))) {
          int8_t  mux = streamGetIntBefore(',');
          int16_t len = streamGetIntBefore('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT &&
              sockets[mux % TINY_GSM_MUX_COUNT]) {
            sockets[mux % TINY_GSM_MUX_COUNT]->got_data       = true;
            sockets[mux % TINY_GSM_MUX_COUNT]->sock_available = len;
          }
          data = "";
          DBG("### URC Data Received:", len, "on", mux);
        } else if (data.endsWith(GF("SQNSH: "))) {
          int8_t mux = streamGetIntBefore('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT &&
              sockets[mux % TINY_GSM_MUX_COUNT]) {
            sockets[mux % TINY_GSM_MUX_COUNT]->sock_connected = false;
          }
          data = "";
          DBG("### URC Sock Closed: ", mux);
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
  GsmClientSequansMonarch* sockets[TINY_GSM_MUX_COUNT];
  // GSM_NL (\r\n) is not accepted with SQNSSENDEXT in data mode so use \n
  const char*              gsmNL = "\n";
};

#endif  // SRC_TINYGSMCLIENTSEQUANSMONARCH_H_
