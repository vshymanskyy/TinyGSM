/**
 * @file       TinyGsmClientSIM7600.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSIM7600_H_
#define SRC_TINYGSMCLIENTSIM7600_H_

// #define TINY_GSM_DEBUG Serial
// #define TINY_GSM_USE_HEX

#define TINY_GSM_MUX_COUNT 10
#define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE
#ifdef AT_NL
#undef AT_NL
#endif
#define AT_NL "\r\n"

#ifdef MODEM_MANUFACTURER
#undef MODEM_MANUFACTURER
#endif
#define MODEM_MANUFACTURER "SIMCom"

#ifdef MODEM_MODEL
#undef MODEM_MODEL
#endif
#if defined(TINY_GSM_MODEM_SIM7500)
#define MODEM_MODEL "SIM7500";
#elif defined(TINY_GSM_MODEM_SIM7800)
#define MODEM_MODEL "SIM7800";
#else
#define MODEM_MODEL "SIM7600";
#endif

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmGPS.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"
#include "TinyGsmTemperature.tpp"


enum SIM7600RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};

class TinyGsmSim7600 : public TinyGsmModem<TinyGsmSim7600>,
                       public TinyGsmGPRS<TinyGsmSim7600>,
                       public TinyGsmTCP<TinyGsmSim7600, TINY_GSM_MUX_COUNT>,
                       public TinyGsmSMS<TinyGsmSim7600>,
                       public TinyGsmGSMLocation<TinyGsmSim7600>,
                       public TinyGsmGPS<TinyGsmSim7600>,
                       public TinyGsmTime<TinyGsmSim7600>,
                       public TinyGsmNTP<TinyGsmSim7600>,
                       public TinyGsmBattery<TinyGsmSim7600>,
                       public TinyGsmTemperature<TinyGsmSim7600>,
                       public TinyGsmCalling<TinyGsmSim7600> {
  friend class TinyGsmModem<TinyGsmSim7600>;
  friend class TinyGsmGPRS<TinyGsmSim7600>;
  friend class TinyGsmTCP<TinyGsmSim7600, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSMS<TinyGsmSim7600>;
  friend class TinyGsmGPS<TinyGsmSim7600>;
  friend class TinyGsmGSMLocation<TinyGsmSim7600>;
  friend class TinyGsmTime<TinyGsmSim7600>;
  friend class TinyGsmNTP<TinyGsmSim7600>;
  friend class TinyGsmBattery<TinyGsmSim7600>;
  friend class TinyGsmTemperature<TinyGsmSim7600>;
  friend class TinyGsmCalling<TinyGsmSim7600>;

  /*
   * Inner Client
   */
 public:
  class GsmClientSim7600 : public GsmClient {
    friend class TinyGsmSim7600;

   public:
    GsmClientSim7600() {}

    explicit GsmClientSim7600(TinyGsmSim7600& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmSim7600* modem, uint8_t mux = 0) {
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
  // NOT SUPPORTED

  /*
   * Constructor
   */
 public:
  explicit TinyGsmSim7600(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = nullptr) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM7600"));

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

    // Disable time and time zone URC's
    sendAT(GF("+CTZR=0"));
    if (waitResponse(10000L) != 1) { return false; }

    // Enable automatic time zome update
    sendAT(GF("+CTZU=1"));
    if (waitResponse(10000L) != 1) { return false; }

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

  bool factoryDefaultImpl() {  // these commands aren't supported
    return false;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = nullptr) {
    if (!testAT()) { return false; }
    sendAT(GF("+CRESET"));
    if (waitResponse(10000L) != 1) { return false; }
    delay(24000L);
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOF"));
    return waitResponse() == 1;
  }

  bool radioOffImpl() {
    if (!setPhoneFunctionality(4)) { return false; }
    delay(3000);
    return true;
  }

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
  SIM7600RegStatus getRegistrationStatus() {
    return (SIM7600RegStatus)getRegistrationStatusXREG("CGREG");
  }

 protected:
  bool isNetworkConnectedImpl() {
    SIM7600RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

 public:
  String getNetworkModes() {
    // Get the help string, not the setting value
    sendAT(GF("+CNMP=?"));
    if (waitResponse(GF(AT_NL "+CNMP:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  int16_t getNetworkMode() {
    sendAT(GF("+CNMP?"));
    if (waitResponse(GF(AT_NL "+CNMP:")) != 1) { return false; }
    int16_t mode = streamGetIntBefore('\n');
    waitResponse();
    return mode;
  }

  bool setNetworkMode(uint8_t mode) {
    sendAT(GF("+CNMP="), mode);
    return waitResponse() == 1;
  }

  bool getNetworkSystemMode(bool& n, int16_t& stat) {
    // n: whether to automatically report the system mode info
    // stat: the current service. 0 if it not connected
    sendAT(GF("+CNSMOD?"));
    if (waitResponse(GF(AT_NL "+CNSMOD:")) != 1) { return false; }
    n    = streamGetIntBefore(',') != 0;
    stat = streamGetIntBefore('\n');
    waitResponse();
    return true;
  }

  String getLocalIPImpl() {
    sendAT(GF("+IPADDR"));  // Inquire Socket PDP address
    // sendAT(GF("+CGPADDR=1"));  // Show PDP address
    String res;
    if (waitResponse(10000L, res) != 1) { return ""; }
    cleanResponseString(res);
    res.trim();
    return res;
  }

  /*
   * Secure socket layer (SSL) functions
   */
  // No functions of this type supported

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
    gprsDisconnect();  // Make sure we're not connected first

    // Define the PDP context

    // The CGDCONT commands set up the "external" PDP context

    // Set the external authentication
    if (user && strlen(user) > 0) {
      sendAT(GF("+CGAUTH=1,0,\""), pwd, GF("\",\""), user, '"');
      waitResponse();
    }

    // Define external PDP context 1
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"', ",\"0.0.0.0\",0,0");
    waitResponse();

    // Configure TCP parameters

    // Select TCP/IP application mode (command mode)
    sendAT(GF("+CIPMODE=0"));
    waitResponse();

    // Set Sending Mode - send without waiting for peer TCP ACK
    sendAT(GF("+CIPSENDMODE=0"));
    waitResponse();

    // Configure socket parameters
    // AT+CIPCCFG= <NmRetry>, <DelayTm>, <Ack>, <errMode>, <HeaderType>,
    //            <AsyncMode>, <TimeoutVal>
    // NmRetry = number of retransmission to be made for an IP packet
    //         = 10 (default)
    // DelayTm = number of milliseconds to delay before outputting received data
    //          = 0 (default)
    // Ack = sets whether reporting a string "Send ok" = 0 (don't report)
    // errMode = mode of reporting error result code = 0 (numberic values)
    // HeaderType = which data header of receiving data in multi-client mode
    //            = 1 (+RECEIVE,<link num>,<data length>)
    // AsyncMode = sets mode of executing commands
    //           = 0 (synchronous command executing)
    // TimeoutVal = minimum retransmission timeout in milliseconds = 75000
    sendAT(GF("+CIPCCFG=10,0,0,0,1,0,75000"));
    if (waitResponse() != 1) { return false; }

    // Configure timeouts for opening and closing sockets
    // AT+CIPTIMEOUT=<netopen_timeout> <cipopen_timeout>, <cipsend_timeout>
    sendAT(GF("+CIPTIMEOUT="), 75000, ',', 15000, ',', 15000);
    waitResponse();

    // Start the socket service

    // This activates and attaches to the external PDP context that is tied
    // to the embedded context for TCP/IP (ie AT+CGACT=1,1 and AT+CGATT=1)
    // Response may be an immediate "OK" followed later by "+NETOPEN: 0".
    // We to ignore any immediate response and wait for the
    // URC to show it's really connected.
    sendAT(GF("+NETOPEN"));
    if (waitResponse(75000L, GF(AT_NL "+NETOPEN: 0")) != 1) { return false; }

    return true;
  }

  bool gprsDisconnectImpl() {
    // Close all sockets and stop the socket service
    // Note: On the LTE models, this single command closes all sockets and the
    // service
    sendAT(GF("+NETCLOSE"));
    if (waitResponse(60000L, GF(AT_NL "+NETCLOSE: 0")) != 1) { return false; }

    return true;
  }

  bool isGprsConnectedImpl() {
    sendAT(GF("+NETOPEN?"));
    // May return +NETOPEN: 1, 0.  We just confirm that the first number is 1
    if (waitResponse(GF(AT_NL "+NETOPEN: 1")) != 1) { return false; }
    waitResponse();

    sendAT(GF("+IPADDR"));  // Inquire Socket PDP address
    // sendAT(GF("+CGPADDR=1")); // Show PDP address
    if (waitResponse() != 1) { return false; }

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
  // Gets the CCID of a sim card via AT+CCID
  String getSimCCIDImpl() {
    sendAT(GF("+CICCID"));
    if (waitResponse(GF(AT_NL "+ICCID:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone Call functions
   */
 protected:
  bool callHangupImpl() {
    sendAT(GF("+CHUP"));
    return waitResponse() == 1;
  }

  /*
   * Audio functions
   */
  //  No functions of this type supported

  /*
   * Text messaging (SMS) functions
   */
  // Follows all text messaging (SMS) functions as inherited from TinyGsmSMS.tpp

  /*
   * GSM Location functions
   */
  // Follows all GSM-based location functions as inherited from
  // TinyGsmGSMLocation.tpp

  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // enable GPS
  bool enableGPSImpl() {
    sendAT(GF("+CGPS=1"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool disableGPSImpl() {
    sendAT(GF("+CGPS=0"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  // get the RAW GPS output
  String getGPSrawImpl() {
    sendAT(GF("+CGNSSINFO"));
    if (waitResponse(GF(AT_NL "+CGNSSINFO:")) != 1) { return ""; }
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
    sendAT(GF("+CGNSSINFO"));
    if (waitResponse(GF(AT_NL "+CGNSSINFO:")) != 1) { return false; }

    uint8_t fixMode = streamGetIntBefore(',');  // mode 2=2D Fix or 3=3DFix
                                                // TODO(?) Can 1 be returned
    if (fixMode == 1 || fixMode == 2 || fixMode == 3) {
      // init variables
      float ilat = 0;
      char  north;
      float ilon = 0;
      char  east;
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

      streamSkipUntil(',');               // GPS satellite valid numbers
      streamSkipUntil(',');               // GLONASS satellite valid numbers
      streamSkipUntil(',');               // BEIDOU satellite valid numbers
      ilat  = streamGetFloatBefore(',');  // Latitude in ddmm.mmmmmm
      north = stream.readStringUntil(',').charAt(
          0);                            // N/S Indicator, N=north or S=south
      ilon = streamGetFloatBefore(',');  // Longitude in ddmm.mmmmmm
      east = stream.readStringUntil(',').charAt(
          0);  // E/W Indicator, E=east or W=west

      // Date. Output format is ddmmyy
      iday   = streamGetIntLength(2);    // Two digit day
      imonth = streamGetIntLength(2);    // Two digit month
      iyear  = streamGetIntBefore(',');  // Two digit year

      // UTC Time. Output format is hhmmss.s
      ihour = streamGetIntLength(2);  // Two digit hour
      imin  = streamGetIntLength(2);  // Two digit minute
      secondWithSS =
          streamGetFloatBefore(',');  // 4 digit second with subseconds

      ialt   = streamGetFloatBefore(',');  // MSL Altitude. Unit is meters
      ispeed = streamGetFloatBefore(',');  // Speed Over Ground. Unit is knots.
      streamSkipUntil(',');                // Course Over Ground. Degrees.
      iaccuracy = streamGetFloatBefore(',');  // Position Dilution Of Precision
      streamSkipUntil(',');   // Horizontal Dilution Of Precision
      streamSkipUntil('\n');  // Vertical Dilution Of Precision

      // Set pointers
      if (lat != nullptr)
        *lat = (floor(ilat / 100) + fmod(ilat, 100.) / 60) *
            (north == 'N' ? 1 : -1);
      if (lon != nullptr)
        *lon = (floor(ilon / 100) + fmod(ilon, 100.) / 60) *
            (east == 'E' ? 1 : -1);
      if (speed != nullptr) *speed = ispeed;
      if (alt != nullptr) *alt = ialt;
      if (vsat != nullptr) *vsat = ivsat;
      if (usat != nullptr) *usat = iusat;
      if (accuracy != nullptr) *accuracy = iaccuracy;
      if (iyear < 2000) iyear += 2000;
      if (year != nullptr) *year = iyear;
      if (month != nullptr) *month = imonth;
      if (day != nullptr) *day = iday;
      if (hour != nullptr) *hour = ihour;
      if (minute != nullptr) *minute = imin;
      if (second != nullptr) *second = static_cast<int>(secondWithSS);

      waitResponse();
      return true;
    }

    waitResponse();
    return false;
  }

  /**
   *  CGNSSMODE: <gnss_mode>,<dpo_mode>
   *  This command is used to configure GPS, GLONASS, BEIDOU and QZSS support
   * mode. 0 : GLONASS 1 : BEIDOU 2 : GALILEO 3 : QZSS dpo_mode: 1 enable , 0
   * disable
   */
  String setGNSSModeImpl(uint8_t mode, bool dpo) {
    String res;
    sendAT(GF("+CGNSSMODE="), mode, ",", dpo);
    if (waitResponse(10000L, res) != 1) { return ""; }
    res.replace(AT_NL, "");
    res.trim();
    return res;
  }

  uint8_t getGNSSModeImpl() {
    sendAT(GF("+CGNSSMODE?"));
    if (waitResponse(GF(AT_NL "+CGNSSMODE:")) != 1) { return 0; }
    return stream.readStringUntil(',').toInt();
  }


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
 protected:
  // returns volts, multiply by 1000 to get mV
  int16_t getBattVoltageImpl() {
    sendAT(GF("+CBC"));
    if (waitResponse(GF(AT_NL "+CBC:")) != 1) { return 0; }

    // get voltage in VOLTS
    float voltage = streamGetFloatBefore('\n');
    // Wait for final OK
    waitResponse();
    // Return millivolts
    uint16_t res = voltage * 1000;
    return res;
  }

  int8_t getBattPercentImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  int8_t getBattChargeStateImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    chargeState = 0;
    percent     = 0;
    milliVolts  = getBattVoltage();
    return true;
  }

  /*
   * Temperature functions
   */
 protected:
  // get temperature in degree celsius
  uint16_t getTemperatureImpl() {
    sendAT(GF("+CPMUTEMP"));
    if (waitResponse(GF(AT_NL "+CPMUTEMP:")) != 1) { return 0; }
    // return temperature in C
    uint16_t res = streamGetIntBefore('\n');
    // Wait for final OK
    waitResponse();
    return res;
  }

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 15) {
    if (ssl) { DBG("SSL not yet supported on this module!"); }
    // Make sure we'll be getting data manually on this connection
    sendAT(GF("+CIPRXGET=1"));
    if (waitResponse() != 1) { return false; }

    // Establish a connection in multi-socket mode
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    sendAT(GF("+CIPOPEN="), mux, ',', GF("\"TCP"), GF("\",\""), host, GF("\","),
           port);
    // The reply is OK followed by +CIPOPEN: <link_num>,<err> where <link_num>
    // is the mux number and <err> should be 0 if there's no error
    if (waitResponse(timeout_ms, GF(AT_NL "+CIPOPEN:")) != 1) { return false; }
    uint8_t opened_mux    = streamGetIntBefore(',');
    uint8_t opened_result = streamGetIntBefore('\n');
    if (opened_mux != mux || opened_result != 0) return false;
    return true;
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(GF(AT_NL "+CIPSEND:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux
    streamSkipUntil(',');  // Skip requested bytes to send
    // TODO(?):  make sure requested and confirmed bytes match
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
    streamSkipUntil(',');  // Skip mux/cid (connecion id)
    int16_t len_requested = streamGetIntBefore(',');
    //  ^^ Requested number of data bytes (1-1460 bytes)to be read
    int16_t len_confirmed = streamGetIntBefore('\n');
    // ^^ The data length which not read in the buffer
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
    // Read the status of all sockets at once
    sendAT(GF("+CIPCLOSE?"));
    if (waitResponse(GF("+CIPCLOSE:")) != 1) {
      // return false;  // TODO:  Why does this not read correctly?
    }
    for (int muxNo = 0; muxNo < TINY_GSM_MUX_COUNT; muxNo++) {
      // +CIPCLOSE:<link0_state>,<link1_state>,...,<link9_state>
      bool muxState = stream.parseInt();
      if (sockets[muxNo]) { sockets[muxNo]->sock_connected = muxState; }
    }
    waitResponse();  // Should be an OK at the end
    if (!sockets[mux]) return false;
    return sockets[mux]->sock_connected;
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
    } else if (data.endsWith(GF("+IPCLOSE:"))) {
      int8_t mux = streamGetIntBefore(',');
      streamSkipUntil('\n');  // Skip the reason code
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        sockets[mux]->sock_connected = false;
      }
      data = "";
      DBG("### Closed: ", mux);
      return true;
    } else if (data.endsWith(GF("+CIPEVENT:"))) {
      // Need to close all open sockets and release the network library.
      // User will then need to reconnect.
      DBG("### Network error!");
      if (!isGprsConnected()) { gprsDisconnect(); }
      data = "";
      return true;
    }
    return false;
  }

 public:
  Stream& stream;

 protected:
  GsmClientSim7600* sockets[TINY_GSM_MUX_COUNT];
};

#endif  // SRC_TINYGSMCLIENTSIM7600_H_
