/**
 * @file       TinyGsmClientSIM70xx.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSIM70XX_H_
#define SRC_TINYGSMCLIENTSIM70XX_H_

// #define TINY_GSM_DEBUG Serial
// #define TINY_GSM_USE_HEX

#include "TinyGsmBattery.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmGPS.tpp"
#include "TinyGsmModem.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmGSMLocation.tpp"

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

template <class modemType>
class TinyGsmSim70xx : public TinyGsmModem<TinyGsmSim70xx<modemType>>,
                       public TinyGsmGPRS<TinyGsmSim70xx<modemType>>,
                       public TinyGsmSMS<TinyGsmSim70xx<modemType>>,
                       public TinyGsmGPS<TinyGsmSim70xx<modemType>>,
                       public TinyGsmTime<TinyGsmSim70xx<modemType>>,
                       public TinyGsmNTP<TinyGsmSim70xx<modemType>>,
                       public TinyGsmBattery<TinyGsmSim70xx<modemType>>,
                       public TinyGsmGSMLocation<TinyGsmSim70xx<modemType>> {
  friend class TinyGsmModem<TinyGsmSim70xx<modemType>>;
  friend class TinyGsmGPRS<TinyGsmSim70xx<modemType>>;
  friend class TinyGsmSMS<TinyGsmSim70xx<modemType>>;
  friend class TinyGsmGPS<TinyGsmSim70xx<modemType>>;
  friend class TinyGsmTime<TinyGsmSim70xx<modemType>>;
  friend class TinyGsmNTP<TinyGsmSim70xx<modemType>>;
  friend class TinyGsmBattery<TinyGsmSim70xx<modemType>>;
  friend class TinyGsmGSMLocation<TinyGsmSim70xx<modemType>>;

  /*
   * CRTP Helper
   */
 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }

  /*
   * Constructor
   */
 public:
  explicit TinyGsmSim70xx(Stream& stream) : stream(stream) {}

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    return thisModem().initImpl(pin);
  }

  String getModemNameImpl() {
    String name = "SIMCom SIM7000";

    thisModem().sendAT(GF("+GMM"));
    String res2;
    if (thisModem().waitResponse(5000L, res2) != 1) { return name; }
    res2.replace(GSM_NL "OK" GSM_NL, "");
    res2.replace("_", " ");
    res2.trim();

    name = res2;
    return name;
  }

  bool factoryDefaultImpl() {           // these commands aren't supported
    thisModem().sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    thisModem().waitResponse();
    thisModem().sendAT(GF("+IPR=0"));  // Auto-baud
    thisModem().waitResponse();
    thisModem().sendAT(GF("+IFC=0,0"));  // No Flow Control
    thisModem().waitResponse();
    thisModem().sendAT(GF("+ICF=3,3"));  // 8 data 0 parity 1 stop
    thisModem().waitResponse();
    thisModem().sendAT(GF("+CSCLK=0"));  // Disable Slow Clock
    thisModem().waitResponse();
    thisModem().sendAT(GF("&W"));  // Write configuration
    return thisModem().waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = NULL) {
    thisModem().sendAT(GF("E0"));  // Echo Off
    thisModem().waitResponse();
    if (!thisModem().setPhoneFunctionality(0)) { return false; }
    if (!thisModem().setPhoneFunctionality(1, true)) { return false; }
    thisModem().waitResponse(30000L, GF("SMS Ready"));
    return thisModem().initImpl(pin);
  }

  bool powerOffImpl() {
    thisModem().sendAT(GF("+CPOWD=1"));
    return thisModem().waitResponse(GF("NORMAL POWER DOWN")) == 1;
  }

  // During sleep, the SIM70xx module has its serial communication disabled.
  // In order to reestablish communication pull the DRT-pin of the SIM70xx
  // module LOW for at least 50ms. Then use this function to disable sleep
  // mode. The DTR-pin can then be released again.
  bool sleepEnableImpl(bool enable = true) {
    thisModem().sendAT(GF("+CSCLK="), enable);
    return thisModem().waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false) {
    thisModem().sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return thisModem().waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 public:
  RegStatus getRegistrationStatus() {
    RegStatus epsStatus =
        (RegStatus)thisModem().getRegistrationStatusXREG("CEREG");
    // If we're connected on EPS, great!
    if (epsStatus == REG_OK_HOME || epsStatus == REG_OK_ROAMING) {
      return epsStatus;
    } else {
      // Otherwise, check GPRS network status
      // We could be using GPRS fall-back or the board could be being moody
      return (RegStatus)thisModem().getRegistrationStatusXREG("CGREG");
    }
  }

 protected:
  bool isNetworkConnectedImpl() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

 public:
  String getNetworkModes() {
    // Get the help string, not the setting value
    thisModem().sendAT(GF("+CNMP=?"));
    if (thisModem().waitResponse(GF(GSM_NL "+CNMP:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
    return res;
  }

  int16_t getNetworkMode() {
    thisModem().sendAT(GF("+CNMP?"));
    if (thisModem().waitResponse(GF(GSM_NL "+CNMP:")) != 1) { return false; }
    int16_t mode = thisModem().streamGetIntBefore('\n');
    thisModem().waitResponse();
    return mode;
  }

  bool setNetworkMode(uint8_t mode) {
    // 2 Automatic
    // 13 GSM only
    // 38 LTE only
    // 51 GSM and LTE only
    thisModem().sendAT(GF("+CNMP="), mode);
    return thisModem().waitResponse() == 1;
  }

  String getPreferredModes() {
    // Get the help string, not the setting value
    thisModem().sendAT(GF("+CMNB=?"));
    if (thisModem().waitResponse(GF(GSM_NL "+CMNB:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
    return res;
  }

  int16_t getPreferredMode() {
    thisModem().sendAT(GF("+CMNB?"));
    if (thisModem().waitResponse(GF(GSM_NL "+CMNB:")) != 1) { return false; }
    int16_t mode = thisModem().streamGetIntBefore('\n');
    thisModem().waitResponse();
    return mode;
  }

  bool setPreferredMode(uint8_t mode) {
    // 1 CAT-M
    // 2 NB-IoT
    // 3 CAT-M and NB-IoT
    thisModem().sendAT(GF("+CMNB="), mode);
    return thisModem().waitResponse() == 1;
  }

  bool getNetworkSystemMode(bool& n, int16_t& stat) {
    // n: whether to automatically report the system mode info
    // stat: the current service. 0 if it not connected
    thisModem().sendAT(GF("+CNSMOD?"));
    if (thisModem().waitResponse(GF(GSM_NL "+CNSMOD:")) != 1) { return false; }
    n    = thisModem().streamGetIntBefore(',') != 0;
    stat = thisModem().streamGetIntBefore('\n');
    thisModem().waitResponse();
    return true;
  }

  bool setNetworkSystemMode(bool n) {
    // n: whether to automatically report the system mode info
    thisModem().sendAT(GF("+CNSMOD="), int8_t(n));
    return thisModem().waitResponse() == 1;
  }

  String getLocalIPImpl() {
    return thisModem().getLocalIPImpl();
  }

  /*
   * GPRS functions
   */
 protected:
  // should implement in sub-classes
  bool gprsConnectImpl(const char* apn, const char* user = NULL,
                       const char* pwd = NULL) {
    return thisModem().gprsConnectImpl(apn, user, pwd);
  }

  bool gprsDisconnectImpl() {
    return thisModem().gprsDisconnectImpl();
  }

  /*
   * SIM card functions
   */
 protected:
  // Doesn't return the "+CCID" before the number
  String getSimCCIDImpl() {
    thisModem().sendAT(GF("+CCID"));
    if (thisModem().waitResponse(GF(GSM_NL)) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
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
    thisModem().sendAT(GF("+CGNSPWR=1"));
    if (thisModem().waitResponse() != 1) { return false; }
    return true;
  }

  bool disableGPSImpl() {
    thisModem().sendAT(GF("+CGNSPWR=0"));
    if (thisModem().waitResponse() != 1) { return false; }
    return true;
  }

  // get the RAW GPS output
  String getGPSrawImpl() {
    thisModem().sendAT(GF("+CGNSINF"));
    if (thisModem().waitResponse(10000L, GF(GSM_NL "+CGNSINF:")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  bool getGPSImpl(float* lat, float* lon, float* speed = 0, float* alt = 0,
                  int* vsat = 0, int* usat = 0, float* accuracy = 0,
                  int* year = 0, int* month = 0, int* day = 0, int* hour = 0,
                  int* minute = 0, int* second = 0) {
    thisModem().sendAT(GF("+CGNSINF"));
    if (thisModem().waitResponse(10000L, GF(GSM_NL "+CGNSINF:")) != 1) {
      return false;
    }

    thisModem().streamSkipUntil(',');                // GNSS run status
    if (thisModem().streamGetIntBefore(',') == 1) {  // fix status
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
      iyear        = thisModem().streamGetIntLength(4);  // Four digit year
      imonth       = thisModem().streamGetIntLength(2);  // Two digit month
      iday         = thisModem().streamGetIntLength(2);  // Two digit day
      ihour        = thisModem().streamGetIntLength(2);  // Two digit hour
      imin         = thisModem().streamGetIntLength(2);  // Two digit minute
      secondWithSS = thisModem().streamGetFloatBefore(
          ',');  // 6 digit second with subseconds

      ilat = thisModem().streamGetFloatBefore(',');  // Latitude
      ilon = thisModem().streamGetFloatBefore(',');  // Longitude
      ialt = thisModem().streamGetFloatBefore(
          ',');  // MSL Altitude. Unit is meters
      ispeed = thisModem().streamGetFloatBefore(
          ',');                          // Speed Over Ground. Unit is knots.
      thisModem().streamSkipUntil(',');  // Course Over Ground. Degrees.
      thisModem().streamSkipUntil(',');  // Fix Mode
      thisModem().streamSkipUntil(',');  // Reserved1
      iaccuracy = thisModem().streamGetFloatBefore(
          ',');                          // Horizontal Dilution Of Precision
      thisModem().streamSkipUntil(',');  // Position Dilution Of Precision
      thisModem().streamSkipUntil(',');  // Vertical Dilution Of Precision
      thisModem().streamSkipUntil(',');  // Reserved2
      ivsat = thisModem().streamGetIntBefore(',');  // GNSS Satellites in View
      iusat = thisModem().streamGetIntBefore(',');  // GNSS Satellites Used
      thisModem().streamSkipUntil(',');             // GLONASS Satellites Used
      thisModem().streamSkipUntil(',');             // Reserved3
      thisModem().streamSkipUntil(',');             // C/N0 max
      thisModem().streamSkipUntil(',');             // HPA
      thisModem().streamSkipUntil('\n');            // VPA

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

      thisModem().waitResponse();
      return true;
    }

    thisModem().streamSkipUntil('\n');  // toss the row of commas
    thisModem().waitResponse();
    return false;
  }

  /*
   * Time functions
   */
  // Can follow CCLK as per template

  /*
   * NTP server functions
   */
  // Can sync with server using CNTP as per template

  /*
   * Battery functions
   */
 protected:
  // Follows all battery functions per template

  /*
   * Client related functions
   */
  // should implement in sub-classes

  /*
   * Utilities
   */
 public:
  // should implement in sub-classes
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
    return thisModem().waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
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
    return thisModem().waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
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
    return thisModem().waitResponse(1000, r1, r2, r3, r4, r5);
  }

 public:
  Stream& stream;

 protected:
  const char* gsmNL = GSM_NL;
};

#endif  // SRC_TINYGSMCLIENTSIM70XX_H_
