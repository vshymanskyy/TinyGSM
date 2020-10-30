/**
 * @file       TinyGsmClientUBLOX.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTUBLOX_H_
#define SRC_TINYGSMCLIENTUBLOX_H_
// #pragma message("TinyGSM:  TinyGsmClientUBLOX")

// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MUX_COUNT 7
#define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE

#include "TinyGsmBattery.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmGPS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmModem.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmSSL.tpp"
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

class TinyGsmUBLOX : public TinyGsmModem<TinyGsmUBLOX>,
                     public TinyGsmGPRS<TinyGsmUBLOX>,
                     public TinyGsmTCP<TinyGsmUBLOX, TINY_GSM_MUX_COUNT>,
                     public TinyGsmSSL<TinyGsmUBLOX>,
                     public TinyGsmCalling<TinyGsmUBLOX>,
                     public TinyGsmSMS<TinyGsmUBLOX>,
                     public TinyGsmGSMLocation<TinyGsmUBLOX>,
                     public TinyGsmGPS<TinyGsmUBLOX>,
                     public TinyGsmTime<TinyGsmUBLOX>,
                     public TinyGsmBattery<TinyGsmUBLOX> {
  friend class TinyGsmModem<TinyGsmUBLOX>;
  friend class TinyGsmGPRS<TinyGsmUBLOX>;
  friend class TinyGsmTCP<TinyGsmUBLOX, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmUBLOX>;
  friend class TinyGsmCalling<TinyGsmUBLOX>;
  friend class TinyGsmSMS<TinyGsmUBLOX>;
  friend class TinyGsmGSMLocation<TinyGsmUBLOX>;
  friend class TinyGsmGPS<TinyGsmUBLOX>;
  friend class TinyGsmTime<TinyGsmUBLOX>;
  friend class TinyGsmBattery<TinyGsmUBLOX>;

  /*
   * Inner Client
   */
 public:
  class GsmClientUBLOX : public GsmClient {
    friend class TinyGsmUBLOX;

   public:
    GsmClientUBLOX() {}

    explicit GsmClientUBLOX(TinyGsmUBLOX& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmUBLOX* modem, uint8_t mux = 0) {
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
      // stop();  // DON'T stop!
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
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void stop(uint32_t maxWaitMs) {
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+USOCL="), mux);
      at->waitResponse();  // should return within 1s
      sock_connected = false;
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
  class GsmClientSecureUBLOX : public GsmClientUBLOX {
   public:
    GsmClientSecureUBLOX() {}

    explicit GsmClientSecureUBLOX(TinyGsmUBLOX& modem, uint8_t mux = 0)
        : GsmClientUBLOX(modem, mux) {}

   public:
    int connect(const char* host, uint16_t port, int timeout_s) override {
      // stop();  // DON'T stop!
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
    TINY_GSM_CLIENT_CONNECT_OVERRIDES
  };

  /*
   * Constructor
   */
 public:
  explicit TinyGsmUBLOX(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientUBLOX"));

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
    if (name.startsWith("u-blox SARA-R4") ||
        name.startsWith("u-blox SARA-N4")) {
      DBG("### WARNING:  You are using the wrong TinyGSM modem!");
    } else if (name.startsWith("u-blox SARA-N2")) {
      DBG("### SARA N2 NB-IoT modems not supported!");
    }

    return name;
  }

  bool factoryDefaultImpl() {
    sendAT(GF("+UFACTORY=0,1"));  // No factory restore, erase NVM
    waitResponse();
    return setPhoneFunctionality(16);  // Reset
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl() {
    if (!testAT()) { return false; }
    if (!setPhoneFunctionality(16)) { return false; }
    delay(3000);  // TODO(?):  Verify delay timing here
    return init();
  }

  bool powerOffImpl() {
    sendAT(GF("+CPWROFF"));
    return waitResponse(40000L) == 1;
  }

  bool sleepEnableImpl(bool enable = true) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 public:
  RegStatus getRegistrationStatus() {
    return (RegStatus)getRegistrationStatusXREG("CGREG");
  }

 protected:
  bool isNetworkConnectedImpl() {
    RegStatus s = getRegistrationStatus();
    if (s == REG_OK_HOME || s == REG_OK_ROAMING)
      return true;
    else if (s == REG_UNKNOWN)  // for some reason, it can hang at unknown..
      return isGprsConnected();
    else
      return false;
  }

  String getLocalIPImpl() {
    sendAT(GF("+UPSND=0,0"));
    if (waitResponse(GF(GSM_NL "+UPSND:")) != 1) { return ""; }
    streamSkipUntil(',');   // Skip PSD profile
    streamSkipUntil('\"');  // Skip request type
    String res = stream.readStringUntil('\"');
    if (waitResponse() != 1) { return ""; }
    return res;
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

    // Setting up the PSD profile/PDP context with the UPSD commands sets up an
    // "internal" PDP context, i.e. a data connection using the internal IP
    // stack and related AT commands for sockets.

    // Packet switched data configuration
    // AT+UPSD=<profile_id>,<param_tag>,<param_val>
    // profile_id = 0 - PSD profile identifier, in range 0-6 (NOT PDP context)
    // param_tag = 1: APN
    // param_tag = 2: username
    // param_tag = 3: password
    // param_tag = 7: IP address Note: IP address set as "0.0.0.0" means
    //    dynamic IP address assigned during PDP context activation
    sendAT(GF("+UPSD=0,1,\""), apn, '"');  // Set APN for PSD profile 0
    waitResponse();

    if (user && strlen(user) > 0) {
      sendAT(GF("+UPSD=0,2,\""), user, '"');  // Set user for PSD profile 0
      waitResponse();
    }
    if (pwd && strlen(pwd) > 0) {
      sendAT(GF("+UPSD=0,3,\""), pwd, '"');  // Set password for PSD profile 0
      waitResponse();
    }

    sendAT(GF("+UPSD=0,7,\"0.0.0.0\""));  // Dynamic IP on PSD profile 0
    waitResponse();

    // Packet switched data action
    // AT+UPSDA=<profile_id>,<action>
    // profile_id = 0: PSD profile identifier, in range 0-6 (NOT PDP context)
    // action = 3: activate; it activates a PDP context with the specified
    // profile, using the current parameters
    sendAT(GF(
        "+UPSDA=0,3"));  // Activate the PDP context associated with profile 0
    if (waitResponse(360000L) != 1) {  // Should return ok
      return false;
    }

    // Packet switched network-assigned data - Returns the current (dynamic)
    // network-assigned or network-negotiated value of the specified parameter
    // for the active PDP context associated with the specified PSD profile.
    // AT+UPSND=<profile_id>,<param_tag>
    // profile_id = 0: PSD profile identifier, in range 0-6 (NOT PDP context)
    // param_tag = 8: PSD profile status: if the profile is active the return
    // value is 1, 0 otherwise
    sendAT(GF("+UPSND=0,8"));  // Check if PSD profile 0 is now active
    int8_t res = waitResponse(GF(",8,1"), GF(",8,0"));
    waitResponse();  // Should return another OK
    if (res == 1) {
      return true;          // It's now active
    } else if (res == 2) {  // If it's not active yet, wait for the +UUPSDA URC
      if (waitResponse(180000L, GF("+UUPSDA: 0")) != 1) {  // 0=successful
        return false;
      }
      streamSkipUntil('\n');  // Ignore the IP address, if returned
    } else {
      return false;
    }

    return true;
  }

  bool gprsDisconnectImpl() {
    sendAT(GF(
        "+UPSDA=0,4"));  // Deactivate the PDP context associated with profile 0
    if (waitResponse(360000L) != 1) { return false; }

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
   * Phone Call functions
   */
 protected:
  // Can follow all of the phone call functions from the template

  /*
   * Messaging functions
   */
 protected:
  // Can follow all template functions

  /*
   * GSM/GPS/GNSS/GLONASS Location functions
   * NOTE:  u-blox modules use the same function to get location data from both
   * GSM tower triangulation and from dedicated GPS/GNSS/GLONASS receivers.  The
   * only difference in which sensor the data is requested from.  If a GNSS
   * location is requested from a modem without a GNSS receiver installed on the
   * I2C port, the GSM-based "Cell Locate" location will be returned instead.
   */
 protected:
  bool enableGPSImpl() {
    // AT+UGPS=<mode>[,<aid_mode>[,<GNSS_systems>]]
    // <mode> - 0: GNSS receiver powered off, 1: on
    // <aid_mode> - 0: no aiding (default)
    // <GNSS_systems> - 3: GPS + SBAS (default)
    sendAT(GF("+UGPS=1,0,3"));
    if (waitResponse(10000L, GF(GSM_NL "+UGPS:")) != 1) { return false; }
    return waitResponse(10000L) == 1;
  }
  bool disableGPSImpl() {
    sendAT(GF("+UGPS=0"));
    if (waitResponse(10000L, GF(GSM_NL "+UGPS:")) != 1) { return false; }
    return waitResponse(10000L) == 1;
  }
  String inline getUbloxLocationRaw(int8_t sensor) {
    // AT+ULOC=<mode>,<sensor>,<response_type>,<timeout>,<accuracy>
    // <mode> - 2: single shot position
    // <sensor> - 0: use the last fix in the internal database and stop the GNSS
    //          receiver
    //          - 1: use the GNSS receiver for localization
    //          - 2: use cellular CellLocate location information
    //          - 3: ?? use the combined GNSS receiver and CellLocate service
    //          information ?? - Docs show using sensor 3 and it's
    //          documented for the +UTIME command but not for +ULOC
    // <response_type> - 0: standard (single-hypothesis) response
    // <timeout> - Timeout period in seconds
    // <accuracy> - Target accuracy in meters (1 - 999999)
    sendAT(GF("+ULOC=2,"), sensor, GF(",0,120,1"));
    // wait for first "OK"
    if (waitResponse(10000L) != 1) { return ""; }
    // wait for the final result - wait full timeout time
    if (waitResponse(120000L, GF(GSM_NL "+UULOC:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }
  String getGsmLocationRawImpl() {
    return getUbloxLocationRaw(2);
  }
  String getGPSrawImpl() {
    return getUbloxLocationRaw(1);
  }

  inline bool getUbloxLocation(int8_t sensor, float* lat, float* lon,
                               float* speed = 0, float* alt = 0, int* vsat = 0,
                               int* usat = 0, float* accuracy = 0,
                               int* year = 0, int* month = 0, int* day = 0,
                               int* hour = 0, int* minute = 0,
                               int* second = 0) {
    // AT+ULOC=<mode>,<sensor>,<response_type>,<timeout>,<accuracy>
    // <mode> - 2: single shot position
    // <sensor> - 2: use cellular CellLocate location information
    //          - 0: use the last fix in the internal database and stop the GNSS
    //          receiver
    //          - 1: use the GNSS receiver for localization
    //          - 3: ?? use the combined GNSS receiver and CellLocate service
    //          information ?? - Docs show using sensor 3 and it's documented
    //          for the +UTIME command but not for +ULOC
    // <response_type> - 0: standard (single-hypothesis) response
    // <timeout> - Timeout period in seconds
    // <accuracy> - Target accuracy in meters (1 - 999999)
    sendAT(GF("+ULOC=2,"), sensor, GF(",0,120,1"));
    // wait for first "OK"
    if (waitResponse(10000L) != 1) { return false; }
    // wait for the final result - wait full timeout time
    if (waitResponse(120000L, GF(GSM_NL "+UULOC: ")) != 1) { return false; }

    // +UULOC: <date>, <time>, <lat>, <long>, <alt>, <uncertainty>, <speed>,
    // <direction>, <vertical_acc>, <sensor_used>, <SV_used>, <antenna_status>,
    // <jamming_status>

    // init variables
    float ilat         = 0;
    float ilon         = 0;
    float ispeed       = 0;
    float ialt         = 0;
    int   iusat        = 0;
    float iaccuracy    = 0;
    int   iyear        = 0;
    int   imonth       = 0;
    int   iday         = 0;
    int   ihour        = 0;
    int   imin         = 0;
    float secondWithSS = 0;

    // Date & Time
    iday         = streamGetIntBefore('/');    // Two digit day
    imonth       = streamGetIntBefore('/');    // Two digit month
    iyear        = streamGetIntBefore(',');    // Four digit year
    ihour        = streamGetIntBefore(':');    // Two digit hour
    imin         = streamGetIntBefore(':');    // Two digit minute
    secondWithSS = streamGetFloatBefore(',');  // 6 digit second with subseconds

    ilat = streamGetFloatBefore(',');  // Estimated latitude, in degrees
    ilon = streamGetFloatBefore(',');  // Estimated longitude, in degrees
    ialt = streamGetFloatBefore(
        ',');         // Estimated altitude, in meters - only forGNSS
                      // positioning, 0 in case of CellLocate
    if (ialt != 0) {  // values not returned for CellLocate
      iaccuracy =
          streamGetFloatBefore(',');       // Maximum possible error, in meters
      ispeed = streamGetFloatBefore(',');  // Speed over ground m/s3
      streamSkipUntil(',');  // Course over ground in degree (0 deg - 360 deg)
      streamSkipUntil(',');  // Vertical accuracy, in meters
      streamSkipUntil(',');  // Sensor used for the position calculation
      iusat = streamGetIntBefore(',');  // Number of satellite used
      streamSkipUntil(',');             // Antenna status
      streamSkipUntil('\n');            // Jamming status
    } else {
      iaccuracy =
          streamGetFloatBefore('\n');  // Maximum possible error, in meters
    }

    // Set pointers
    if (lat != NULL) *lat = ilat;
    if (lon != NULL) *lon = ilon;
    if (speed != NULL) *speed = ispeed;
    if (alt != NULL) *alt = ialt;
    if (vsat != NULL) *vsat = 0;  // Number of satellites viewed not reported;
    if (usat != NULL) *usat = iusat;
    if (accuracy != NULL) *accuracy = iaccuracy;
    if (iyear < 2000) iyear += 2000;
    if (year != NULL) *year = iyear;
    if (month != NULL) *month = imonth;
    if (day != NULL) *day = iday;
    if (hour != NULL) *hour = ihour;
    if (minute != NULL) *minute = imin;
    if (second != NULL) *second = static_cast<int>(secondWithSS);

    // final ok
    waitResponse();
    return true;
  }
  bool getGsmLocationImpl(float* lat, float* lon, float* accuracy = 0,
                          int* year = 0, int* month = 0, int* day = 0,
                          int* hour = 0, int* minute = 0, int* second = 0) {
    return getUbloxLocation(2, lat, lon, 0, 0, 0, 0, accuracy, year, month, day,
                            hour, minute, second);
  }
  bool getGPSImpl(float* lat, float* lon, float* speed = 0, float* alt = 0,
                  int* vsat = 0, int* usat = 0, float* accuracy = 0,
                  int* year = 0, int* month = 0, int* day = 0, int* hour = 0,
                  int* minute = 0, int* second = 0) {
    return getUbloxLocation(1, lat, lon, speed, alt, vsat, usat, accuracy, year,
                            month, day, hour, minute, second);
  }

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

    int8_t res     = streamGetIntBefore(',');
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

  // This would only available for a small number of modules in this group
  // (TOBY-L)
  float getTemperatureImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;

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
    *mux = streamGetIntBefore('\n');
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
    sendAT(GF("+USOCO="), *mux, ",\"", host, "\",", port);
    int8_t rsp = waitResponse(timeout_ms - (millis() - startMillis));
    return (1 == rsp);
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
    int16_t sent = streamGetIntBefore('\n');
    waitResponse();  // sends back OK after the confirmation of number sent
    return sent;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) return 0;
    sendAT(GF("+USORD="), mux, ',', (uint16_t)size);
    if (waitResponse(GF(GSM_NL "+USORD:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip mux
    int16_t len = streamGetIntBefore(',');
    streamSkipUntil('\"');

    for (int i = 0; i < len; i++) { moveCharFromStreamToFifo(mux); }
    streamSkipUntil('\"');
    waitResponse();
    // DBG("### READ:", len, "from", mux);
    sockets[mux]->sock_available = modemGetAvailable(mux);
    return len;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!sockets[mux]) return 0;
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
    sendAT(GF("+USORD="), mux, ",0");
    size_t  result = 0;
    uint8_t res    = waitResponse(GF(GSM_NL "+USORD:"));
    // Will give error "operation not allowed" when attempting to read a socket
    // that you have already told to close
    if (res == 1) {
      streamSkipUntil(',');  // Skip mux
      result = streamGetIntBefore('\n');
      // if (result) DBG("### DATA AVAILABLE:", result, "on", mux);
      waitResponse();
    }
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    // DBG("### AvailablE:", result, "on", mux);
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
    sendAT(GF("+USOCTL="), mux, ",10");
    uint8_t res = waitResponse(GF(GSM_NL "+USOCTL:"));
    if (res != 1) { return false; }

    streamSkipUntil(',');  // Skip mux
    streamSkipUntil(',');  // Skip type
    int8_t result = streamGetIntBefore('\n');
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
        } else if (data.endsWith(GF("+UUSORD:"))) {
          int8_t  mux = streamGetIntBefore(',');
          int16_t len = streamGetIntBefore('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data = true;
            // max size is 1024
            if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
          }
          data = "";
          DBG("### URC Data Received:", len, "on", mux);
        } else if (data.endsWith(GF("+UUSOCL:"))) {
          int8_t mux = streamGetIntBefore('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
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
  GsmClientUBLOX* sockets[TINY_GSM_MUX_COUNT];
  const char*     gsmNL = GSM_NL;
};

#endif  // SRC_TINYGSMCLIENTUBLOX_H_
