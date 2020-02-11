/**
 * @file       TinyGsmClientSIM800.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSIM800_H_
#define SRC_TINYGSMCLIENTSIM800_H_
// #pragma message("TinyGSM:  TinyGsmClientSIM800")

// #define TINY_GSM_DEBUG Serial
// #define TINY_GSM_USE_HEX

#define TINY_GSM_MUX_COUNT 5

#include "TinyGsmBattery.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmModem.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmTCP.tpp"
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
class TinyGsmSim800
    : public TinyGsmModem<TinyGsmSim800>,
      public TinyGsmGPRS<TinyGsmSim800>,
      public TinyGsmTCP<TinyGsmSim800, READ_AND_CHECK_SIZE, TINY_GSM_MUX_COUNT>,
      public TinyGsmSSL<TinyGsmSim800>,
      public TinyGsmCalling<TinyGsmSim800>,
      public TinyGsmSMS<TinyGsmSim800>,
      public TinyGsmGSMLocation<TinyGsmSim800>,
      public TinyGsmTime<TinyGsmSim800>,
      public TinyGsmBattery<TinyGsmSim800> {
  friend class TinyGsmModem<TinyGsmSim800>;
  friend class TinyGsmGPRS<TinyGsmSim800>;
  friend class TinyGsmTCP<TinyGsmSim800, READ_AND_CHECK_SIZE,
                          TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmSim800>;
  friend class TinyGsmCalling<TinyGsmSim800>;
  friend class TinyGsmSMS<TinyGsmSim800>;
  friend class TinyGsmGSMLocation<TinyGsmSim800>;
  friend class TinyGsmTime<TinyGsmSim800>;
  friend class TinyGsmBattery<TinyGsmSim800>;

  /*
   * Inner Client
   */
 public:
  class GsmClientSim800 : public GsmClient {
    friend class TinyGsmSim800;

   public:
    GsmClientSim800() {}

    explicit GsmClientSim800(TinyGsmSim800& modem, uint8_t mux = 1) {
      init(&modem, mux);
    }

    bool init(TinyGsmSim800* modem, uint8_t mux = 1) {
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
      sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    virtual void stop(uint32_t maxWaitMs) {
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+CIPCLOSE="), mux, GF(",1"));  // Quick close
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
  class GsmClientSecureSim800 : public GsmClientSim800 {
   public:
    GsmClientSecureSim800() {}

    explicit GsmClientSecureSim800(TinyGsmSim800& modem, uint8_t mux = 1)
        : GsmClientSim800(modem, mux) {}

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

  /*
   * Constructor
   */
 public:
  explicit TinyGsmSim800(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);

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

    // Enable Local Time Stamp for getting network time
    sendAT(GF("+CLTS=1"));
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

  String getModemNameImpl() {
    String name = "";
#if defined(TINY_GSM_MODEM_SIM800)
    name = "SIMCom SIM800";
#elif defined(TINY_GSM_MODEM_SIM808)
    name = "SIMCom SIM808";
#elif defined(TINY_GSM_MODEM_SIM868)
    name = "SIMCom SIM868";
#elif defined(TINY_GSM_MODEM_SIM900)
    name = "SIMCom SIM900";
#endif

    sendAT(GF("+GMM"));
    String res2;
    if (waitResponse(1000L, res2) != 1) { return name; }
    res2.replace(GSM_NL "OK" GSM_NL, "");
    res2.replace("_", " ");
    res2.trim();

    name = res2;
    DBG("### Modem:", name);
    return name;
  }

  bool factoryDefaultImpl() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("+IPR=0"));  // Auto-baud
    waitResponse();
    sendAT(GF("+IFC=0,0"));  // No Flow Control
    waitResponse();
    sendAT(GF("+ICF=3,3"));  // 8 data 0 parity 1 stop
    waitResponse();
    sendAT(GF("+CSCLK=0"));  // Disable Slow Clock
    waitResponse();
    sendAT(GF("&W"));  // Write configuration
    return waitResponse() == 1;
  }

  /*
    bool thisHasSSL() {
  #if defined(TINY_GSM_MODEM_SIM900)
      return false;
  #else
      sendAT(GF("+CIPSSL=?"));
      if (waitResponse(GF(GSM_NL "+CIPSSL:")) != 1) { return false; }
      return waitResponse() == 1;
  #endif
    }
    */

  /*
   * Power functions
   */
 protected:
  bool restartImpl() {
    if (!testAT()) { return false; }
    sendAT(GF("&W"));
    waitResponse();
    sendAT(GF("+CFUN=0"));
    if (waitResponse(10000L) != 1) { return false; }
    sendAT(GF("+CFUN=1,1"));
    if (waitResponse(10000L) != 1) { return false; }
    delay(3000);
    return init();
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOWD=1"));
    return waitResponse(10000L, GF("NORMAL POWER DOWN")) == 1;
  }

  // During sleep, the SIM800 module has its serial communication disabled. In
  // order to reestablish communication pull the DRT-pin of the SIM800 module
  // LOW for at least 50ms. Then use this function to disable sleep mode. The
  // DTR-pin can then be released again.
  bool sleepEnableImpl(bool enable = true) {
    sendAT(GF("+CSCLK="), enable);
    return waitResponse() == 1;
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
    sendAT(GF("+CIFSR;E0"));
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

    // Set the Bearer for the IP
    sendAT(GF(
        "+SAPBR=3,1,\"Contype\",\"GPRS\""));  // Set the connection type to GPRS
    waitResponse();

    sendAT(GF("+SAPBR=3,1,\"APN\",\""), apn, '"');  // Set the APN
    waitResponse();

    if (user && strlen(user) > 0) {
      sendAT(GF("+SAPBR=3,1,\"USER\",\""), user, '"');  // Set the user name
      waitResponse();
    }
    if (pwd && strlen(pwd) > 0) {
      sendAT(GF("+SAPBR=3,1,\"PWD\",\""), pwd, '"');  // Set the password
      waitResponse();
    }

    // Define the PDP context
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Activate the PDP context
    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    // Open the definied GPRS bearer context
    sendAT(GF("+SAPBR=1,1"));
    waitResponse(85000L);
    // Query the GPRS bearer context status
    sendAT(GF("+SAPBR=2,1"));
    if (waitResponse(30000L) != 1) { return false; }

    // Attach to GPRS
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // TODO(?): wait AT+CGATT?

    // Set to multi-IP
    sendAT(GF("+CIPMUX=1"));
    if (waitResponse() != 1) { return false; }

    // Put in "quick send" mode (thus no extra "Send OK")
    sendAT(GF("+CIPQSEND=1"));
    if (waitResponse() != 1) { return false; }

    // Set to get data manually
    sendAT(GF("+CIPRXGET=1"));
    if (waitResponse() != 1) { return false; }

    // Start Task and Set APN, USER NAME, PASSWORD
    sendAT(GF("+CSTT=\""), apn, GF("\",\""), user, GF("\",\""), pwd, GF("\""));
    if (waitResponse(60000L) != 1) { return false; }

    // Bring Up Wireless Connection with GPRS or CSD
    sendAT(GF("+CIICR"));
    if (waitResponse(60000L) != 1) { return false; }

    // Get Local IP Address, only assigned after connection
    sendAT(GF("+CIFSR;E0"));
    if (waitResponse(10000L) != 1) { return false; }

    // Configure Domain Name Server (DNS)
    sendAT(GF("+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\""));
    if (waitResponse() != 1) { return false; }

    return true;
  }

  bool gprsDisconnectImpl() {
    // Shut the TCP/IP connection
    // CIPSHUT will close *all* open connections
    sendAT(GF("+CIPSHUT"));
    if (waitResponse(60000L) != 1) { return false; }

    sendAT(GF("+CGATT=0"));  // Deactivate the bearer context
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  /*
   * SIM card functions
   */
 protected:
  // Able to follow all SIM card functions as inherited from the template

  /*
   * Phone Call functions
   */
 public:
  bool setGsmBusy(bool busy = true) {
    sendAT(GF("+GSMBUSY="), busy ? 1 : 0);
    return waitResponse() == 1;
  }

  /*
   * Messaging functions
   */
 protected:
  // Follows all messaging functions per template

  /*
   * Location functions
   */
 protected:
  // Can return a location from CIPGSMLOC as per the template


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
  // Follows all battery functions per template

  /*
   * NTP server functions
   */
 public:
  boolean isValidNumber(String str) {
    if (!(str.charAt(0) == '+' || str.charAt(0) == '-' ||
          isDigit(str.charAt(0))))
      return false;

    for (byte i = 1; i < str.length(); i++) {
      if (!(isDigit(str.charAt(i)) || str.charAt(i) == '.')) { return false; }
    }
    return true;
  }

  String ShowNTPError(byte error) {
    switch (error) {
      case 1: return "Network time synchronization is successful";
      case 61: return "Network error";
      case 62: return "DNS resolution error";
      case 63: return "Connection error";
      case 64: return "Service response error";
      case 65: return "Service response timeout";
      default: return "Unknown error: " + String(error);
    }
  }

  byte NTPServerSync(String server = "pool.ntp.org", byte TimeZone = 3) {
    sendAT(GF("+CNTPCID=1"));
    if (waitResponse(10000L) != 1) { return -1; }

    sendAT(GF("+CNTP="), server, ',', String(TimeZone));
    if (waitResponse(10000L) != 1) { return -1; }

    sendAT(GF("+CNTP"));
    if (waitResponse(10000L, GF(GSM_NL "+CNTP:"))) {
      String result = stream.readStringUntil('\n');
      result.trim();
      if (isValidNumber(result)) { return result.toInt(); }
    } else {
      return -1;
    }
    return -1;
  }

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
    int      rsp;
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
#if !defined(TINY_GSM_MODEM_SIM900)
    sendAT(GF("+CIPSSL="), ssl);
    rsp = waitResponse();
    if (ssl && rsp != 1) { return false; }
#endif
    sendAT(GF("+CIPSTART="), mux, ',', GF("\"TCP"), GF("\",\""), host,
           GF("\","), port);
    rsp = waitResponse(
        timeout_ms, GF("CONNECT OK" GSM_NL), GF("CONNECT FAIL" GSM_NL),
        GF("ALREADY CONNECT" GSM_NL), GF("ERROR" GSM_NL),
        GF("CLOSE OK" GSM_NL));  // Happens when HTTPS handshake fails
    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(GF(GSM_NL "DATA ACCEPT:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux
    return streamGetInt('\n');
  }

  size_t modemRead(size_t size, uint8_t mux) {
#ifdef TINY_GSM_USE_HEX
    sendAT(GF("+CIPRXGET=3,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#else
    sendAT(GF("+CIPRXGET=2,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#endif
    streamSkipUntil(',');  // Skip Rx mode 2/normal or 3/HEX
    streamSkipUntil(',');  // Skip mux
    int len_requested = streamGetInt(',');
    //  ^^ Requested number of data bytes (1-1460 bytes)to be read
    int len_confirmed = streamGetInt('\n');
    // ^^ Confirmed number of data bytes to be read, which may be less than
    // requested. 0 indicates that no data can be read. This is actually be the
    // number of bytes that will be remaining after the read
    for (int i = 0; i < len_requested; i++) {
      uint32_t startMillis = millis();
#ifdef TINY_GSM_USE_HEX
      while (stream.available() < 2 &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_GSM_YIELD();
      }
      char buf[4] = {
          0,
      };
      buf[0] = stream.read();
      buf[1] = stream.read();
      char c = strtol(buf, NULL, 16);
#else
      while (!stream.available() &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_GSM_YIELD();
      }
      char c = stream.read();
#endif
      sockets[mux]->rx.put(c);
    }
    DBG("### READ:", len_requested, "from", mux);
    // sockets[mux]->sock_available = modemGetAvailable(mux);
    sockets[mux]->sock_available = len_confirmed;
    waitResponse();
    return len_requested;
  }

  size_t modemGetAvailable(uint8_t mux) {
    sendAT(GF("+CIPRXGET=4,"), mux);
    size_t result = 0;
    if (waitResponse(GF("+CIPRXGET:")) == 1) {
      streamSkipUntil(',');  // Skip mode 4
      streamSkipUntil(',');  // Skip mux
      result = streamGetInt('\n');
      waitResponse();
    }
    DBG("### Available:", result, "on", mux);
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+CIPSTATUS="), mux);
    waitResponse(GF("+CIPSTATUS"));
    int res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""),
                           GF(",\"CLOSING\""), GF(",\"REMOTE CLOSING\""),
                           GF(",\"INITIAL\""));
    waitResponse();
    return 1 == res;
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
        } else if (data.endsWith(GF(GSM_NL "+CIPRXGET:"))) {
          int mode = streamGetInt(',');
          if (mode == 1) {
            int mux = streamGetInt('\n');
            if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
              sockets[mux]->got_data = true;
            }
            data = "";
            DBG("### Got Data:", mux);
          } else {
            data += mode;
          }
        } else if (data.endsWith(GF(GSM_NL "+RECEIVE:"))) {
          int mux = streamGetInt(',');
          int len = streamGetInt('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data       = true;
            sockets[mux]->sock_available = len;
          }
          data = "";
          DBG("### Got Data:", len, "on", mux);
        } else if (data.endsWith(GF("CLOSED" GSM_NL))) {
          int nl   = data.lastIndexOf(GSM_NL, data.length() - 8);
          int coma = data.indexOf(',', nl + 2);
          int mux  = data.substring(nl + 2, coma).toInt();
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
  GsmClientSim800* sockets[TINY_GSM_MUX_COUNT];
  const char*      gsmNL = GSM_NL;
};

#endif  // SRC_TINYGSMCLIENTSIM800_H_
