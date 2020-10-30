/**
 * @file       TinyGsmClientSIM7000.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSIM7000_H_
#define SRC_TINYGSMCLIENTSIM7000_H_

// #define TINY_GSM_DEBUG Serial
// #define TINY_GSM_USE_HEX

#define TINY_GSM_MUX_COUNT 8
#define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE

#include "TinyGsmBattery.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmGPS.tpp"
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

class TinyGsmSim7000 : public TinyGsmModem<TinyGsmSim7000>,
                       public TinyGsmGPRS<TinyGsmSim7000>,
                       public TinyGsmTCP<TinyGsmSim7000, TINY_GSM_MUX_COUNT>,
                       public TinyGsmSMS<TinyGsmSim7000>,
                       public TinyGsmGPS<TinyGsmSim7000>,
                       public TinyGsmTime<TinyGsmSim7000>,
                       public TinyGsmBattery<TinyGsmSim7000> {
  friend class TinyGsmModem<TinyGsmSim7000>;
  friend class TinyGsmGPRS<TinyGsmSim7000>;
  friend class TinyGsmTCP<TinyGsmSim7000, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSMS<TinyGsmSim7000>;
  friend class TinyGsmGPS<TinyGsmSim7000>;
  friend class TinyGsmTime<TinyGsmSim7000>;
  friend class TinyGsmBattery<TinyGsmSim7000>;

  /*
   * Inner Client
   */
 public:
  class GsmClientSim7000 : public GsmClient {
    friend class TinyGsmSim7000;

   public:
    GsmClientSim7000() {}

    explicit GsmClientSim7000(TinyGsmSim7000& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmSim7000* modem, uint8_t mux = 0) {
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
      at->sendAT(GF("+CIPCLOSE="), mux);
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

  /*TODO(?))
  class GsmClientSecureSIM7000 : public GsmClientSim7000
  {
  public:
    GsmClientSecure() {}

    GsmClientSecure(TinyGsmSim7000& modem, uint8_t mux = 0)
     : public GsmClient(modem, mux)
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
  explicit TinyGsmSim7000(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM7000"));

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

    // Enable Local Time Stamp for getting network time
    sendAT(GF("+CLTS=1"));
    if (waitResponse(10000L) != 1) { return false; }

    // Enable battery checks
    sendAT(GF("+CBATCHK=1"));
    if (waitResponse() != 1) { return false; }

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
    String name = "SIMCom SIM7000";

    sendAT(GF("+GMM"));
    String res2;
    if (waitResponse(1000L, res2) != 1) { return name; }
    res2.replace(GSM_NL "OK" GSM_NL, "");
    res2.replace("_", " ");
    res2.trim();

    name = res2;
    return name;
  }

  bool factoryDefaultImpl() {  // these commands aren't supported
    return false;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl() {
    if (!setPhoneFunctionality(0)) { return false; }
    if (!setPhoneFunctionality(1, true)) { return false; }
    waitResponse(10000L, GF("SMS Ready"), GF("RDY"));
    return init();
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOWD=1"));
    return waitResponse(GF("NORMAL POWER DOWN")) == 1;
  }

  // During sleep, the SIM7000 module has its serial communication disabled.
  // In order to reestablish communication pull the DRT-pin of the SIM7000
  // module LOW for at least 50ms. Then use this function to disable sleep mode.
  // The DTR-pin can then be released again.
  bool sleepEnableImpl(bool enable = true) {
    sendAT(GF("+CSCLK="), enable);
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
    RegStatus epsStatus = (RegStatus)getRegistrationStatusXREG("CEREG");
    // If we're connected on EPS, great!
    if (epsStatus == REG_OK_HOME || epsStatus == REG_OK_ROAMING) {
      return epsStatus;
    } else {
      // Otherwise, check GPRS network status
      // We could be using GPRS fall-back or the board could be being moody
      return (RegStatus)getRegistrationStatusXREG("CGREG");
    }
  }

 protected:
  bool isNetworkConnectedImpl() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

 public:
  String getNetworkModes() {
    sendAT(GF("+CNMP=?"));
    if (waitResponse(GF(GSM_NL "+CNMP:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  String setNetworkMode(uint8_t mode) {
    sendAT(GF("+CNMP="), mode);
    if (waitResponse(GF(GSM_NL "+CNMP:")) != 1) { return "OK"; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  String getPreferredModes() {
    sendAT(GF("+CMNB=?"));
    if (waitResponse(GF(GSM_NL "+CMNB:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  String setPreferredMode(uint8_t mode) {
    sendAT(GF("+CMNB="), mode);
    if (waitResponse(GF(GSM_NL "+CMNB:")) != 1) { return "OK"; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
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
  // Doesn't return the "+CCID" before the number
  String getSimCCIDImpl() {
    sendAT(GF("+CCID"));
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
  // Follows all messaging functions per template

  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // enable GPS
  bool enableGPSImpl() {
    sendAT(GF("+CGNSPWR=1"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool disableGPSImpl() {
    sendAT(GF("+CGNSPWR=0"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  // get the RAW GPS output
  String getGPSrawImpl() {
    sendAT(GF("+CGNSINF"));
    if (waitResponse(10000L, GF(GSM_NL "+CGNSINF:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  bool getGPSImpl(float* lat, float* lon, float* speed = 0, float* alt = 0,
                  int* vsat = 0, int* usat = 0, float* accuracy = 0,
                  int* year = 0, int* month = 0, int* day = 0, int* hour = 0,
                  int* minute = 0, int* second = 0) {
    sendAT(GF("+CGNSINF"));
    if (waitResponse(10000L, GF(GSM_NL "+CGNSINF:")) != 1) { return false; }

    streamSkipUntil(',');                // GNSS run status
    if (streamGetIntBefore(',') == 1) {  // fix status
      // init variables
      float ilat         = 0;
      float ilon         = 0;
      float ispeed       = 0;
      float ialt         = 0;
      int   ivsat        = 0;
      int   iusat        = 0;
      float iaccuracy    = 0;
      int   iyear        = 0;
      int   imonth       = 0;
      int   iday         = 0;
      int   ihour        = 0;
      int   imin         = 0;
      float secondWithSS = 0;

      // UTC date & Time
      iyear  = streamGetIntLength(4);  // Four digit year
      imonth = streamGetIntLength(2);  // Two digit month
      iday   = streamGetIntLength(2);  // Two digit day
      ihour  = streamGetIntLength(2);  // Two digit hour
      imin   = streamGetIntLength(2);  // Two digit minute
      secondWithSS =
          streamGetFloatBefore(',');  // 6 digit second with subseconds

      ilat   = streamGetFloatBefore(',');  // Latitude
      ilon   = streamGetFloatBefore(',');  // Longitude
      ialt   = streamGetFloatBefore(',');  // MSL Altitude. Unit is meters
      ispeed = streamGetFloatBefore(',');  // Speed Over Ground. Unit is knots.
      streamSkipUntil(',');                // Course Over Ground. Degrees.
      streamSkipUntil(',');                // Fix Mode
      streamSkipUntil(',');                // Reserved1
      iaccuracy =
          streamGetFloatBefore(',');    // Horizontal Dilution Of Precision
      streamSkipUntil(',');             // Position Dilution Of Precision
      streamSkipUntil(',');             // Vertical Dilution Of Precision
      streamSkipUntil(',');             // Reserved2
      ivsat = streamGetIntBefore(',');  // GNSS Satellites in View
      iusat = streamGetIntBefore(',');  // GNSS Satellites Used
      streamSkipUntil(',');             // GLONASS Satellites Used
      streamSkipUntil(',');             // Reserved3
      streamSkipUntil(',');             // C/N0 max
      streamSkipUntil(',');             // HPA
      streamSkipUntil('\n');            // VPA

      // Set pointers
      if (lat != NULL) *lat = ilat;
      if (lon != NULL) *lon = ilon;
      if (speed != NULL) *speed = ispeed;
      if (alt != NULL) *alt = ialt;
      if (vsat != NULL) *vsat = ivsat;
      if (usat != NULL) *usat = iusat;
      if (accuracy != NULL) *accuracy = iaccuracy;
      if (iyear < 2000) iyear += 2000;
      if (year != NULL) *year = iyear;
      if (month != NULL) *month = imonth;
      if (day != NULL) *day = iday;
      if (hour != NULL) *hour = ihour;
      if (minute != NULL) *minute = imin;
      if (second != NULL) *second = static_cast<int>(secondWithSS);

      waitResponse();
      return true;
    }

    streamSkipUntil('\n');  // toss the row of commas
    waitResponse();
    return false;
  }

  /*
   * Time functions
   */
  // Can follow CCLK as per template

  /*
   * Battery functions
   */
 protected:
  // Follows all battery functions per template

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
    if (ssl) { DBG("SSL not yet supported on this module!"); }

    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    sendAT(GF("+CIPSTART="), mux, ',', GF("\"TCP"), GF("\",\""), host,
           GF("\","), port);
    return (1 ==
            waitResponse(timeout_ms, GF("CONNECT OK" GSM_NL),
                         GF("CONNECT FAIL" GSM_NL),
                         GF("ALREADY CONNECT" GSM_NL), GF("ERROR" GSM_NL),
                         GF("CLOSE OK" GSM_NL)));
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(GF(GSM_NL "DATA ACCEPT:")) != 1) { return 0; }
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
        } else if (data.endsWith(GF(GSM_NL "+CIPRXGET:"))) {
          int8_t mode = streamGetIntBefore(',');
          if (mode == 1) {
            int8_t mux = streamGetIntBefore('\n');
            if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
              sockets[mux]->got_data = true;
            }
            data = "";
            // DBG("### Got Data:", mux);
          } else {
            data += mode;
          }
        } else if (data.endsWith(GF(GSM_NL "+RECEIVE:"))) {
          int8_t  mux = streamGetIntBefore(',');
          int16_t len = streamGetIntBefore('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data = true;
            if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
          }
          data = "";
          // DBG("### Got Data:", len, "on", mux);
        } else if (data.endsWith(GF("CLOSED" GSM_NL))) {
          int8_t nl   = data.lastIndexOf(GSM_NL, data.length() - 8);
          int8_t coma = data.indexOf(',', nl + 2);
          int8_t mux  = data.substring(nl + 2, coma).toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### Closed: ", mux);
        } else if (data.endsWith(GF("*PSNWID:"))) {
          streamSkipUntil('\n');  // Refresh network name by network
          data = "";
          DBG("### Network name updated.");
        } else if (data.endsWith(GF("*PSUTTZ:"))) {
          streamSkipUntil('\n');  // Refresh time and time zone by network
          data = "";
          DBG("### Network time and time zone updated.");
        } else if (data.endsWith(GF("+CTZV:"))) {
          streamSkipUntil('\n');  // Refresh network time zone by network
          data = "";
          DBG("### Network time zone updated.");
        } else if (data.endsWith(GF("DST: "))) {
          streamSkipUntil(
              '\n');  // Refresh Network Daylight Saving Time by network
          data = "";
          DBG("### Daylight savings time state updated.");
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
  GsmClientSim7000* sockets[TINY_GSM_MUX_COUNT];
  const char*       gsmNL = GSM_NL;
};

#endif  // SRC_TINYGSMCLIENTSIM7000_H_
