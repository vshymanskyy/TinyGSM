/**
 * @file       TinyGsmClientSequansMonarch.h
 * @author     Michael Krumpus
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2019 Michael Krumpus
 * @date       Jan 2019
 */

#ifndef TinyGsmClientSequansMonarch_h
#define TinyGsmClientSequansMonarch_h

//#define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 64
#endif

#define TINY_GSM_MUX_COUNT 6

#include <TinyGsmCommon.h>

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;

enum SimStatus {
  SIM_ERROR = 0,
  SIM_READY = 1,
  SIM_LOCKED = 2,
};

enum RegStatus {
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
{

public:

class GsmClient : public Client
{
  friend class TinyGsmSequansMonarch;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmSequansMonarch& modem, uint8_t mux = 1) {
    init(&modem, mux);
  }

  virtual ~GsmClient(){}

  bool init(TinyGsmSequansMonarch* modem, uint8_t mux = 1) {
    this->at = modem;
    this->mux = mux;
    sock_available = 0;
    prev_check = 0;
    sock_connected = false;
    got_data = false;

    // adjust for zero indexed socket array vs Sequans' 1 indexed mux numbers
    // using modulus will force 6 back to 0
    at->sockets[mux % TINY_GSM_MUX_COUNT] = this;

    return true;
  }

public:
  virtual int connect(const char *host, uint16_t port, int timeout_s) {
    if (sock_connected) stop();
    TINY_GSM_YIELD();
    rx.clear();
    sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
    return sock_connected;
  }

TINY_GSM_CLIENT_CONNECT_OVERLOADS()

  virtual void stop(uint32_t maxWaitMs) {
    TINY_GSM_CLIENT_DUMP_MODEM_BUFFER()
    at->sendAT(GF("+SQNSH="), mux);
    sock_connected = false;
    at->waitResponse();
  }

  virtual void stop() { stop(15000L); }

TINY_GSM_CLIENT_WRITE()

TINY_GSM_CLIENT_AVAILABLE_WITH_BUFFER_CHECK()

TINY_GSM_CLIENT_READ_WITH_BUFFER_CHECK()

TINY_GSM_CLIENT_PEEK_FLUSH_CONNECTED()

  /*
   * Extended API
   */

  String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

private:
  TinyGsmSequansMonarch* at;
  uint8_t         mux;
  uint16_t        sock_available;
  uint32_t        prev_check;
  bool            sock_connected;
  bool            got_data;
  RxFifo          rx;
};


class GsmClientSecure : public GsmClient
{
public:
  GsmClientSecure() {}

  GsmClientSecure(TinyGsmSequansMonarch& modem, uint8_t mux = 1)
    : GsmClient(modem, mux)
  {}

  virtual ~GsmClientSecure(){}

protected:
  bool          strictSSL = false;

public:
  virtual int connect(const char *host, uint16_t port, int timeout_s) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();

    // configure security profile 1 with parameters:
    if (strictSSL) {
      // require minimum of TLS 1.2 (3)
      // only support cipher suite 0x3D: TLS_RSA_WITH_AES_256_CBC_SHA256
      // verify server certificate against imported CA certs 0 and enforce validity period (3)
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

  void setStrictSSL(bool strict) {
    strictSSL = strict;
  }

};

public:

  TinyGsmSequansMonarch(Stream& stream)
    : stream(stream)
  {
    memset(sockets, 0, sizeof(sockets));
  }

  virtual ~TinyGsmSequansMonarch() {}

  /*
   * Basic functions
   */

  bool begin(const char* pin = NULL) {
    return init(pin);
  }

  bool init(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);

    if (!testAT()) {
      return false;
    }

    sendAT(GF("E0"));   // Echo Off
    if (waitResponse() != 1) {
      return false;
    }

    DBG(GF("### Modem:"), getModemName());

    int ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != NULL && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    }
    // if the sim is ready, or it's locked but no pin has been provided, return
    // true
    else {
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  String getModemName() {
    return "Sequans Monarch";
  }

TINY_GSM_MODEM_SET_BAUD_IPR()

TINY_GSM_MODEM_TEST_AT()

  void maintain() {
    for (int mux = 1; mux <= TINY_GSM_MUX_COUNT; mux++) {
      GsmClient* sock = sockets[mux % TINY_GSM_MUX_COUNT];
      if (sock && sock->got_data) {
        sock->got_data = false;
        sock->sock_available = modemGetAvailable(mux);
        // modemGetConnected() always checks the state of ALL socks
        modemGetConnected();
      }
    }
    while (stream.available()) {
      waitResponse(15, NULL, NULL);
  }
  }

  bool factoryDefault() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("+IPR=0"));   // Auto-baud
    waitResponse();
    sendAT(GF("+IFC=0,0")); // No Flow Control
    waitResponse();
    sendAT(GF("+ICF=3,3")); // 8 data 0 parity 1 stop
    waitResponse();
    sendAT(GF("+CSCLK=0")); // Disable Slow Clock
    waitResponse();
    sendAT(GF("&W"));       // Write configuration
    return waitResponse() == 1;
  }

TINY_GSM_MODEM_GET_INFO_ATI()

  bool hasSSL() {
    return true;
  }

  /*
   * Power functions
   */

  bool restart() {
    if (!testAT()) {
      return false;
    }

    sendAT(GF("+CFUN=0"));
    int res = waitResponse(20000L, GFP(GSM_OK), GFP(GSM_ERROR), GF("+SYSSTART")) ;
    if (res != 1 && res != 3) {
      return false;
    }

    sendAT(GF("+CFUN=1,1"));
    res = waitResponse(20000L, GF("+SYSSTART"), GFP(GSM_ERROR)) ;
    if (res != 1 && res != 3) {
      return false;
    }
    delay(1000);
    return init();
  }

  bool poweroff() {
    sendAT(GF("+SQNSSHDN"));
    return waitResponse();
  }

  bool radioOff() {
    sendAT(GF("+CFUN=0"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);
    return true;
  }

  /*
   When power saving is enabled, UART0 interface is activated with sleep mode support.
   Module power state is controlled by RTS0 line.  When no activity on UART, CTS line
   will be set to OFF state (driven high level) <timeout> milliseconds (100ms to 10s,
   default 5s) after the last sent character, then module will go to sleep mode as soon
   as DTE set RTS line to OFF state (driver high level).
  */
  bool sleepEnable(bool enable = true) {
    sendAT(GF("+SQNIPSCFG="), enable);
    return waitResponse() == 1;
  }

  /*
   * SIM card functions
   */

TINY_GSM_MODEM_SIM_UNLOCK_CPIN()

  String getSimCCID() {
    sendAT(GF("+SQNCCID"));
    if (waitResponse(GF(GSM_NL "+SQNCCID:")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

TINY_GSM_MODEM_GET_IMEI_GSN()

  SimStatus getSimStatus(unsigned long timeout_ms = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
      sendAT(GF("+CPIN?"));
      if (waitResponse(GF(GSM_NL "+CPIN:")) != 1) {
        delay(1000);
        continue;
      }
      int status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"), GF("NOT INSERTED"));
      waitResponse();
      switch (status) {
      case 2:
      case 3:  return SIM_LOCKED;
      case 1:  return SIM_READY;
      default: return SIM_ERROR;
      }
    }
    return SIM_ERROR;
  }

TINY_GSM_MODEM_GET_REGISTRATION_XREG(CEREG)

TINY_GSM_MODEM_GET_OPERATOR_COPS()

  /*
   * Generic network functions
   */

TINY_GSM_MODEM_GET_CSQ()

  bool isNetworkConnected() {
    RegStatus s = getRegistrationStatus();
    if (s == REG_OK_HOME || s == REG_OK_ROAMING) {
      // DBG(F("connected with status:"), s);
      return true;
    } else {
      return false;
    }
  }

TINY_GSM_MODEM_WAIT_FOR_NETWORK()

  /*
   * GPRS functions
   */

  bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {
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
    if (waitResponse(60000L) != 1)
      return false;

    return true;
  }

  bool gprsDisconnect() {
    sendAT(GF("+CGATT=0"));
    if (waitResponse(60000L) != 1)
      return false;

    return true;
  }

  bool isGprsConnected() {
    sendAT(GF("+CGATT?"));
    if (waitResponse(GF(GSM_NL "+CGATT:")) != 1) {
      return false;
    }
    int res = stream.readStringUntil('\n').toInt();
    waitResponse();
    if (res != 1)
      return false;

    return true;
  }


  /*
   * IP Address functions
   */

  String getLocalIP() {
    sendAT(GF("+CGPADDR=3"));
    if (waitResponse(10000L, GF("+CGPADDR: 3,\"")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\"');
    waitResponse();
    return res;
  }

  IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
  }


  /*
   * Phone Call functions
   */

  bool setGsmBusy(bool busy = true) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool callAnswer() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool callNumber(const String& number) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool callHangup() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  String sendUSSD(const String& code) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sendSMS(const String& number, const String& text) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    //Set GSM 7 bit default alphabet (3GPP TS 23.038)
    sendAT(GF("+CSCS=\"GSM\""));
    waitResponse();
    sendAT(GF("+CMGS=\""), number, GF("\""));
    if (waitResponse(GF(">")) != 1) {
      return false;
    }
    stream.print(text);
    stream.write((char)0x1A);
    stream.flush();
    return waitResponse(60000L) == 1;
  }

  bool sendSMS_UTF16(const String& number, const void* text, size_t len) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Location functions
   */

  String getGsmLocation() TINY_GSM_ATTR_NOT_AVAILABLE;


  /*
   * Battery & temperature functions
   */

  uint16_t getBattVoltage() TINY_GSM_ATTR_NOT_AVAILABLE;
  int8_t getBattPercent() TINY_GSM_ATTR_NOT_AVAILABLE;
  uint8_t getBattChargeState() TINY_GSM_ATTR_NOT_AVAILABLE;
  bool getBattStats(uint8_t &chargeState, int8_t &percent, uint16_t &milliVolts) TINY_GSM_ATTR_NOT_AVAILABLE;

  float getTemperature() {
    sendAT(GF("+SMDTH"));
    if (waitResponse(10000L, GF("+SMDTH: ")) != 1) {
      return (float)-9999;
    }
    String res;
    if (waitResponse(1000L, res) != 1) {
      return (float)-9999;
    }
    if (res.indexOf("ERROR") >=0) {
      return (float)-9999;
    }
    return res.toFloat();
  }

protected:

  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75)
 {
    int rsp;
    unsigned long startMillis = millis();
    uint32_t timeout_ms = ((uint32_t)timeout_s)*1000;

    if (ssl) {
      // enable SSl and use security profile 1
      //AT+SQNSSCFG=<connId>,<enable>,<spId>
      sendAT(GF("+SQNSSCFG="), mux, GF(",1,1"));
      if (waitResponse() != 1) {
        DBG("### WARNING: failed to configure secure socket");
        return false;
      }
    }

    // Socket configuration
    //AT+SQNSCFG:<connId1>, <cid1>, <pktSz1>, <maxTo1>, <connTo1>, <txTo1>
    // <connId1> = Connection ID = mux
    // <cid1> = PDP context ID = 3 - this is number set up above in the GprsConnect function
    // <pktSz1> = Packet Size, used for online data mode only = 300 (default)
    // <maxTo1> = Max timeout in seconds = 90 (default)
    // <connTo1> = Connection timeout in hundreds of milliseconds = 600 (default)
    // <txTo1> = Data sending timeout in hundreds of milliseconds, used for online data mode only = 50 (default)
    sendAT(GF("+SQNSCFG="), mux, GF(",3,300,90,600,50"));
    waitResponse(5000L);

    // Socket configuration extended
    //AT+SQNSCFGEXT:<connId1>, <srMode1>, <recvDataMode1>, <keepalive1>, <listenAutoRsp1>, <sendDataMode1>
    // <connId1> = Connection ID = mux
    // <srMode1> = Send/Receive URC model = 1 - data amount mode
    // <recvDataMode1> = Receive data mode = 0  - data as text (1 would be as hex)
    // <keepalive1> = unused = 0
    // <listenAutoRsp1> = Listen auto-response mode = 0 - deactivated
    // <sendDataMode1> = Send data mode = 0  - data as text (1 would be as hex)
    sendAT(GF("+SQNSCFGEXT="), mux, GF(",1,0,0,0,0"));
    waitResponse(5000L);

    // Socket dial
    //AT+SQNSD=<connId>,<txProt>,<rPort>,<IPaddr>[,<closureType>[,<lPort>[,<connMode>[,acceptAnyRemote]]]]
    // <connId> = Connection ID = mux
    // <txProt> = Transmission protocol = 0 - TCP (1 for UDP)
    // <rPort> = Remote host port to contact
    // <IPaddr> = Any valid IP address in the format “xxx.xxx.xxx.xxx” or any host name solved with a DNS query
    // <closureType> = Socket closure behaviour for TCP, has no effect for UDP = 0 - local port closes when remote does (default)
    // <lPort> = UDP connection local port, has no effect for TCP connections.
    // <connMode> = Connection mode = 1 - command mode connection
    // <acceptAnyRemote> = Applies to UDP only
    sendAT(GF("+SQNSD="), mux, ",0,", port, ',', GF("\""), host, GF("\""), ",0,0,1");
    rsp = waitResponse((timeout_ms - (millis() - startMillis)),
                      GFP(GSM_OK),
                      GFP(GSM_ERROR),
                      GF("NO CARRIER" GSM_NL)
                      );

    // creation of socket failed immediately.
    if (rsp != 1) return false;

    // wait until we get a good status
    bool connected = false;
    while (!connected && ((millis() - startMillis) < timeout_ms)) {
      connected = modemGetConnected(mux);
      delay(100); // socket may be in opening state
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
    stream.write((uint8_t*)buff, len);
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
    //   stream.write((uint8_t*)buff, len);
    //   stream.write((char)0x1A);
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
    if (waitResponse(GF("+SQNSRECV: ")) != 1) {
      return 0;
    }
    streamSkipUntil(','); // Skip mux
    int len = stream.readStringUntil('\n').toInt();
    for (int i=0; i<len; i++) {
      uint32_t startMillis = millis(); \
      while (!stream.available() && ((millis() - startMillis) < sockets[mux % TINY_GSM_MUX_COUNT]->_timeout)) { TINY_GSM_YIELD(); } \
      char c = stream.read(); \
      sockets[mux % TINY_GSM_MUX_COUNT]->rx.put(c);
    }
    DBG("### Read:", len, "from", mux);
    waitResponse();
    sockets[mux % TINY_GSM_MUX_COUNT]->sock_available = modemGetAvailable(mux);
    return len;
  }

  size_t modemGetAvailable(uint8_t mux) {
    sendAT(GF("+SQNSI="), mux);
    size_t result = 0;
    if (waitResponse(GF("+SQNSI:")) == 1) {
      streamSkipUntil(','); // Skip mux
      streamSkipUntil(','); // Skip total sent
      streamSkipUntil(','); // Skip total received
      result = stream.readStringUntil(',').toInt();  // keep data not yet read
      waitResponse();
    }
    DBG("### Available:", result, "on", mux);
    return result;
  }

  bool modemGetConnected(uint8_t mux = 1) {
    // This single command always returns the connection status of all
    // six possible sockets.
    sendAT(GF("+SQNSS"));
    for (int muxNo = 1; muxNo <= TINY_GSM_MUX_COUNT; muxNo++) {
      if (waitResponse(GFP(GSM_OK), GF(GSM_NL "+SQNSS: ")) != 2) {
        break;
      };
      uint8_t status = 0;
      // if (stream.readStringUntil(',').toInt() != muxNo) { // check the mux no
      //   DBG("### Warning: misaligned mux numbers!");
      // }
      streamSkipUntil(',');  // skip mux [use muxNo]
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
      sockets[muxNo % TINY_GSM_MUX_COUNT]->sock_connected = \
        ((status != SOCK_CLOSED) && (status != SOCK_INCOMING) && (status != SOCK_OPENING));
    }
    waitResponse();  // Should be an OK at the end
    return sockets[mux % TINY_GSM_MUX_COUNT]->sock_connected;
  }

public:

  /*
   Utilities
   */

TINY_GSM_MODEM_STREAM_UTILITIES()

  // TODO: Optimize this!
  uint8_t waitResponse(uint32_t timeout_ms, String& data,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    /*String r1s(r1); r1s.trim();
    String r2s(r2); r2s.trim();
    String r3s(r3); r3s.trim();
    String r4s(r4); r4s.trim();
    String r5s(r5); r5s.trim();
    DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s);*/
    data.reserve(64);
    int index = 0;
    unsigned long startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (stream.available() > 0) {
        TINY_GSM_YIELD();
        int a = stream.read();
        if (a <= 0) continue; // Skip 0x00 bytes, just in case
        data += (char)a;
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
        } else if (data.endsWith(GF(GSM_NL "+SQNSRING:"))) {
          int mux = stream.readStringUntil(',').toInt();
          int len = stream.readStringUntil('\n').toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux % TINY_GSM_MUX_COUNT]) {
            sockets[mux % TINY_GSM_MUX_COUNT]->got_data = true;
            sockets[mux % TINY_GSM_MUX_COUNT]->sock_available = len;
          }
          data = "";
          DBG("### URC Data Received:", len, "on", mux);
        } else if (data.endsWith(GF("SQNSH: "))) {
          int mux = stream.readStringUntil('\n').toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux % TINY_GSM_MUX_COUNT]) {
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
      if (data.length()) {
        DBG("### Unhandled:", data);
      }
      data = "";
    }
    //data.replace(GSM_NL, "/");
    //DBG('<', index, '>', data);
    return index;
  }

  uint8_t waitResponse(uint32_t timeout_ms,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
  }

  uint8_t waitResponse(GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    return waitResponse(1000, r1, r2, r3, r4, r5);
  }

public:
  Stream&       stream;

protected:
  GsmClient*    sockets[TINY_GSM_MUX_COUNT];
};

#endif
