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
#define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE
#ifdef AT_NL
#undef AT_NL
#endif
#define AT_NL "\r\n"

#ifdef MODEM_MANUFACTURER
#undef MODEM_MANUFACTURER
#endif
#define MODEM_MANUFACTURER "unknown"

#ifdef MODEM_MODEL
#undef MODEM_MODEL
#endif
#if defined(TINY_GSM_MODEM_SIM808)
#define MODEM_MODEL "SIM808";
#elif defined(TINY_GSM_MODEM_SIM868)
#define MODEM_MODEL "SIM868";
#elif defined(TINY_GSM_MODEM_SIM900)
#define MODEM_MODEL "SIM900";
#else
#define MODEM_MODEL "SIM800";
#endif

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"

enum SIM800RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};
class TinyGsmSim800 : public TinyGsmModem<TinyGsmSim800>,
                      public TinyGsmGPRS<TinyGsmSim800>,
                      public TinyGsmTCP<TinyGsmSim800, TINY_GSM_MUX_COUNT>,
                      public TinyGsmSSL<TinyGsmSim800, TINY_GSM_MUX_COUNT>,
                      public TinyGsmCalling<TinyGsmSim800>,
                      public TinyGsmSMS<TinyGsmSim800>,
                      public TinyGsmGSMLocation<TinyGsmSim800>,
                      public TinyGsmTime<TinyGsmSim800>,
                      public TinyGsmNTP<TinyGsmSim800>,
                      public TinyGsmBattery<TinyGsmSim800> {
  friend class TinyGsmModem<TinyGsmSim800>;
  friend class TinyGsmGPRS<TinyGsmSim800>;
  friend class TinyGsmTCP<TinyGsmSim800, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmSim800, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmCalling<TinyGsmSim800>;
  friend class TinyGsmSMS<TinyGsmSim800>;
  friend class TinyGsmGSMLocation<TinyGsmSim800>;
  friend class TinyGsmTime<TinyGsmSim800>;
  friend class TinyGsmNTP<TinyGsmSim800>;
  friend class TinyGsmBattery<TinyGsmSim800>;

  /*
   * Inner Client
   */
 public:
  class GsmClientSim800 : public GsmClient {
    friend class TinyGsmSim800;

   public:
    GsmClientSim800() {}

    explicit GsmClientSim800(TinyGsmSim800& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmSim800* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_available = 0;
      prev_check     = 0;
      sock_connected = false;
      got_data       = false;

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

    explicit GsmClientSecureSim800(TinyGsmSim800& modem, uint8_t mux = 0)
        : GsmClientSim800(modem, mux) {}

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
  bool initImpl(const char* pin = nullptr) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM800"));

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

    // Enable battery checks
    sendAT(GF("+CBATCHK=1"));
    waitResponse();

    SimStatus ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != nullptr && strlen(pin) > 0) {
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
      if (waitResponse(GF(AT_NL "+CIPSSL:")) != 1) { return false; }
      return waitResponse() == 1;
  #endif
    }
    */

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = nullptr) {
    if (!testAT()) { return false; }
    sendAT(GF("&W"));
    waitResponse();
    if (!setPhoneFunctionality(0)) { return false; }
    if (!setPhoneFunctionality(1, true)) { return false; }
    delay(3000);
    return init(pin);
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

  // <fun> 0 Minimum functionality
  // <fun> 1 Full functionality (Default)
  // <fun> 4 Disable phone both transmit and receive RF circuits.
  // <rst> Reset the MT before setting it to <fun> power level.
  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 public:
  SIM800RegStatus getRegistrationStatus() {
    return (SIM800RegStatus)getRegistrationStatusXREG("CREG");
  }

 protected:
  bool isNetworkConnectedImpl() {
    SIM800RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  String getLocalIPImpl() {
    sendAT(GF("+CIFSR;E0"));
    String res;
    if (waitResponse(10000L, res) != 1) { return ""; }
    cleanResponseString(res);
    return res;
  }

  /*
   * Secure socket layer (SSL) functions
   */
  // Follows functions as inherited from TinyGsmSSL.tpp

  /*
   * WiFi functions
   */
  // No functions of this type supported

  /*
   * GPRS functions
   */
 protected:
  bool gprsConnectImpl(const char* apn, const char* user = nullptr,
                       const char* pwd = nullptr) {
    gprsDisconnect();

    // Bearer settings for applications based on IP
    // Set the connection type to GPRS
    sendAT(GF("+SAPBR=3,1,\"Contype\",\"GPRS\""));
    waitResponse();

    // Set the APN
    sendAT(GF("+SAPBR=3,1,\"APN\",\""), apn, '"');
    waitResponse();

    // Set the user name
    if (user && strlen(user) > 0) {
      sendAT(GF("+SAPBR=3,1,\"USER\",\""), user, '"');
      waitResponse();
    }
    // Set the password
    if (pwd && strlen(pwd) > 0) {
      sendAT(GF("+SAPBR=3,1,\"PWD\",\""), pwd, '"');
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

    sendAT(GF("+CGATT=0"));  // Detach from GPRS
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  String getProviderImpl() {
    sendAT(GF("+CSPN?"));
    if (waitResponse(GF("+CSPN:")) != 1) { return ""; }
    streamSkipUntil('"'); /* Skip mode and format */
    String res = stream.readStringUntil('"');
    waitResponse();
    return res;
  }

  /*
   * SIM card functions
   */
 protected:
  // May not return the "+CCID" before the number
  String getSimCCIDImpl() {
    sendAT(GF("+CCID"));
    if (waitResponse(GF(AT_NL)) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    // Trim out the CCID header in case it is there
    res.replace("CCID:", "");
    res.trim();
    return res;
  }
  bool isIPconnected() {
    sendAT(GF("+CIPSTATUS"));
    String h = stream.readString();
    if (h.indexOf("CONNECT") > 0) return 1;
    return 0;
  }

  /*
   * Phone Call functions
   */
 public:
  bool setGsmBusy(bool busy = true) {
    sendAT(GF("+GSMBUSY="), busy ? 1 : 0);
    return waitResponse() == 1;
  }

  /*
   * Audio functions
   */
 public:
  bool setVolume(uint8_t volume = 50) {
    // Set speaker volume
    sendAT(GF("+CLVL="), volume);
    return waitResponse() == 1;
  }

  uint8_t getVolume() {
    // Get speaker volume
    sendAT(GF("+CLVL?"));
    if (waitResponse(GF(AT_NL)) != 1) { return 0; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.replace("+CLVL:", "");
    res.trim();
    return res.toInt();
  }

  bool setMicVolume(uint8_t channel, uint8_t level) {
    if (channel > 4) { return 0; }
    sendAT(GF("+CMIC="), level);
    return waitResponse() == 1;
  }
  int newMessageInterrupt(String interrupt) {
    int Start = interrupt.indexOf(',');
    int Stop  = interrupt.indexOf('\n', Start);
    int index = interrupt.substring(Start + 1, Stop - 1).toInt();
    return index;
  }

  bool setAudioChannel(uint8_t channel) {
    sendAT(GF("+CHFA="), channel);
    return waitResponse() == 1;
  }

  bool playToolkitTone(uint8_t tone, uint32_t duration) {
    sendAT(GF("STTONE="), 1, tone);
    delay(duration);
    sendAT(GF("STTONE="), 0);
    return waitResponse();
  }
  String readSMS(int index, const bool changeStatusToRead = true) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    sendAT(GF("+CMGR="), index, GF(","),
           static_cast<const uint8_t>(!changeStatusToRead));
    String h = "";
    streamSkipUntil('\n');
    streamSkipUntil('\n');
    h = stream.readStringUntil('\n');
    return h;
  }
  int newMessageIndex(bool mode) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    sendAT(GF("+CMGL=\"REC UNREAD\",1"));
    String h = stream.readString();
    int    i;
    if (mode) {
      i = h.indexOf("+CMGL: ");
    } else {
      i = h.lastIndexOf("+CMGL: ");
    }

    int index = h.substring(i + 7, i + 9).toInt();
    if (index <= 0) return 0;
    return index;
  }
  bool emptySMSBuffer() {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    sendAT(GF("+CMGDA=\"DEL ALL\""));
    return waitResponse(60000L) == 1;
  }
  String getSenderID(int index, const bool changeStatusToRead = true) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    sendAT(GF("+CMGR="), index, GF(","),
           static_cast<const uint8_t>(changeStatusToRead));
    String h = "";
    streamSkipUntil('"');
    streamSkipUntil('"');
    streamSkipUntil('"');
    h = stream.readStringUntil('"');
    stream.readString();
    return h;
  }
  /*
   * Text messaging (SMS) functions
   */
  // Follows all text messaging (SMS) functions as inherited from TinyGsmSMS.tpp

  /*
   * GSM Location functions
   */
  // Depending on the exacty model and firmware revision, should return a
  // GSM-based location from CLBS as as inherited from TinyGsmGSMLocation.tpp
  // TODO(?):  Check number of digits in year (2 or 4)

  /*
   * GPS/GNSS/GLONASS location functions
   */
  // No functions of this type supported

  /*
   * Time functions
   */
  // Follows all clock functions as inherited from TinyGsmTime.tpp

  /*
   * NTP server functions
   */
  // Follows all NTP server functions as inherited from TinyGsmNTP.tpp

  /*
   * BLE functions
   */
  // No functions of this type supported

  /*
   * Battery functions
   */
  // Follows all battery functions as inherited from TinyGsmBattery.tpp

  /*
   * Temperature functions
   */
  // No functions of this type supported

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
    int8_t   rsp;
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
#if !defined(TINY_GSM_MODEM_SIM900)
    sendAT(GF("+CIPSSL="), ssl);
    rsp = waitResponse();
    if (ssl && rsp != 1) { return false; }
#ifdef TINY_GSM_SSL_CLIENT_AUTHENTICATION
    // set SSL options
    // +SSLOPT=<opt>,<enable>
    // <opt>
    //    0 (default) ignore invalid certificate
    //    1 client authentication
    // <enable>
    //    0 (default) close
    //    1 open
    sendAT(GF("+CIPSSL=1,1"));
    if (waitResponse() != 1) return false;
#endif
#endif
    sendAT(GF("+CIPSTART="), mux, ',', GF("\"TCP"), GF("\",\""), host,
           GF("\","), port);
    rsp = waitResponse(
        timeout_ms, GF("CONNECT OK" AT_NL), GF("CONNECT FAIL" AT_NL),
        GF("ALREADY CONNECT" AT_NL), GF("ERROR" AT_NL),
        GF("CLOSE OK" AT_NL));  // Happens when HTTPS handshake fails
    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(GF(AT_NL "DATA ACCEPT:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux
    return streamGetIntBefore('\n');
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) return 0;
#ifdef TINY_GSM_USE_HEX
    sendAT(GF("+CIPRXGET=3,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#else
    sendAT(GF("+CIPRXGET=2,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#endif
    streamSkipUntil(',');  // Skip Rx mode 2/normal or 3/HEX
    streamSkipUntil(',');  // Skip mux
    int16_t len_requested = streamGetIntBefore(',');
    //  ^^ Requested number of data bytes (1-1460 bytes)to be read
    int16_t len_confirmed = streamGetIntBefore('\n');
    // ^^ Confirmed number of data bytes to be read, which may be less than
    // requested. 0 indicates that no data can be read.
    // SRGD NOTE:  Contrary to above (which is copied from AT command manual)
    // this is actually be the number of bytes that will be remaining in the
    // buffer after the read.
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
      char c = strtol(buf, nullptr, 16);
#else
      while (!stream.available() &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_GSM_YIELD();
      }
      char c = stream.read();
#endif
      sockets[mux]->rx.put(c);
    }
    // DBG("### READ:", len_requested, "from", mux);
    // sockets[mux]->sock_available = modemGetAvailable(mux);
    sockets[mux]->sock_available = len_confirmed;
    waitResponse();
    return len_requested;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!sockets[mux]) return 0;
    sendAT(GF("+CIPRXGET=4,"), mux);
    size_t result = 0;
    if (waitResponse(GF("+CIPRXGET:")) == 1) {
      streamSkipUntil(',');  // Skip mode 4
      streamSkipUntil(',');  // Skip mux
      result = streamGetIntBefore('\n');
      waitResponse();
    }
    // DBG("### Available:", result, "on", mux);
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+CIPSTATUS="), mux);
    waitResponse(GF("+CIPSTATUS"));
    int8_t res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""),
                              GF(",\"CLOSING\""), GF(",\"REMOTE CLOSING\""),
                              GF(",\"INITIAL\""));
    waitResponse();
    return 1 == res;
  }

  /*
   * Utilities
   */
 public:
  bool handleURCs(String& data) {
    if (data.endsWith(GF(AT_NL "+CIPRXGET:"))) {
      int8_t mode = streamGetIntBefore(',');
      if (mode == 1) {
        int8_t mux = streamGetIntBefore('\n');
        if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
          sockets[mux]->got_data = true;
        }
        data = "";
        // DBG("### Got Data:", mux);
        return true;
      } else {
        data += mode;
        return false;
      }
    } else if (data.endsWith(GF(AT_NL "+RECEIVE:"))) {
      int8_t  mux = streamGetIntBefore(',');
      int16_t len = streamGetIntBefore('\n');
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        sockets[mux]->got_data = true;
        if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
      }
      data = "";
      // DBG("### Got Data:", len, "on", mux);
      return true;
    } else if (data.endsWith(GF("CLOSED" AT_NL))) {
      int8_t nl   = data.lastIndexOf(AT_NL, data.length() - 8);
      int8_t coma = data.indexOf(',', nl + 2);
      int8_t mux  = data.substring(nl + 2, coma).toInt();
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        sockets[mux]->sock_connected = false;
      }
      data = "";
      DBG("### Closed: ", mux);
      return true;
    } else if (data.endsWith(GF("*PSNWID:"))) {
      streamSkipUntil('\n');  // Refresh network name by network
      data = "";
      DBG("### Network name updated.");
      return true;
    } else if (data.endsWith(GF("*PSUTTZ:"))) {
      streamSkipUntil('\n');  // Refresh time and time zone by network
      data = "";
      DBG("### Network time and time zone updated.");
      return true;
    } else if (data.endsWith(GF("+CTZV:"))) {
      streamSkipUntil('\n');  // Refresh network time zone by network
      data = "";
      DBG("### Network time zone updated.");
      return true;
    } else if (data.endsWith(GF("DST:"))) {
      streamSkipUntil('\n');  // Refresh Network Daylight Saving Time by network
      data = "";
      DBG("### Daylight savings time state updated.");
      return true;
    }
    return false;
  }

 public:
  Stream& stream;

 protected:
  GsmClientSim800* sockets[TINY_GSM_MUX_COUNT];
};

#endif  // SRC_TINYGSMCLIENTSIM800_H_
