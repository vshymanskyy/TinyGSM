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
#if defined(TINY_GSM_MODEM_SIM7070)
#define MODEM_MODEL "SIM7070";
#elif defined(TINY_GSM_MODEM_SIM7080)
#define MODEM_MODEL "SIM7080";
#elif defined(TINY_GSM_MODEM_SIM7090)
#define MODEM_MODEL "SIM7090";
#elif defined(TINY_GSM_MODEM_SIM7000) || defined(TINY_GSM_MODEM_SIM7000SSL)
#define MODEM_MODEL "SIM7000";
#else
#define MODEM_MODEL "SIM70xx";
#endif

#include "TinyGsmModem.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmGPS.tpp"

enum SIM70xxRegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};

template <class SIM70xxType>
class TinyGsmSim70xx : public TinyGsmModem<SIM70xxType>,
                       public TinyGsmGPRS<SIM70xxType>,
                       public TinyGsmGPS<SIM70xxType> {
  friend class TinyGsmModem<SIM70xxType>;
  friend class TinyGsmGPRS<SIM70xxType>;
  friend class TinyGsmGPS<SIM70xxType>;

  /*
   * CRTP Helper
   */
 protected:
  inline const SIM70xxType& thisModem() const {
    return static_cast<const SIM70xxType&>(*this);
  }
  inline SIM70xxType& thisModem() {
    return static_cast<SIM70xxType&>(*this);
  }
  ~TinyGsmSim70xx() {}

  /*
   * Constructor
   */
 public:
  explicit TinyGsmSim70xx(Stream& stream) : stream(stream) {}

  /*
   * Basic functions
   */
 protected:
  bool factoryDefaultImpl() {
    return false;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = nullptr) {
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
  SIM70xxRegStatus getRegistrationStatus() {
    SIM70xxRegStatus epsStatus =
        (SIM70xxRegStatus)thisModem().getRegistrationStatusXREG("CEREG");
    // If we're connected on EPS, great!
    if (epsStatus == REG_OK_HOME || epsStatus == REG_OK_ROAMING) {
      return epsStatus;
    } else {
      // Otherwise, check GPRS network status
      // We could be using GPRS fall-back or the board could be being moody
      return (SIM70xxRegStatus)thisModem().getRegistrationStatusXREG("CGREG");
    }
  }

 protected:
  bool isNetworkConnectedImpl() {
    SIM70xxRegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

 public:
  String getNetworkModes() {
    // Get the help string, not the setting value
    thisModem().sendAT(GF("+CNMP=?"));
    if (thisModem().waitResponse(GF(AT_NL "+CNMP:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
    return res;
  }

  int16_t getNetworkMode() {
    thisModem().sendAT(GF("+CNMP?"));
    if (thisModem().waitResponse(GF(AT_NL "+CNMP:")) != 1) { return false; }
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
    if (thisModem().waitResponse(GF(AT_NL "+CMNB:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
    return res;
  }

  int16_t getPreferredMode() {
    thisModem().sendAT(GF("+CMNB?"));
    if (thisModem().waitResponse(GF(AT_NL "+CMNB:")) != 1) { return false; }
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
    if (thisModem().waitResponse(GF(AT_NL "+CNSMOD:")) != 1) { return false; }
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

  /*
   * GPRS functions
   */
 protected:
  // should implement in sub-classes

  /*
   * SIM card functions
   */
 protected:
  // Doesn't return the "+CCID" before the number
  String getSimCCIDImpl() {
    thisModem().sendAT(GF("+CCID"));
    if (thisModem().waitResponse(GF(AT_NL)) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
    res.trim();
    return res;
  }

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
    if (thisModem().waitResponse(10000L, GF(AT_NL "+CGNSINF:")) != 1) {
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
    if (thisModem().waitResponse(10000L, GF(AT_NL "+CGNSINF:")) != 1) {
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
      if (lat != nullptr) *lat = ilat;
      if (lon != nullptr) *lon = ilon;
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

      thisModem().waitResponse();
      return true;
    }

    thisModem().streamSkipUntil('\n');  // toss the row of commas
    thisModem().waitResponse();
    return false;
  }
  bool handleURCs(String& data) {
    return thisModem().handleURCs(data);
  }

 public:
  Stream& stream;
};

#endif  // SRC_TINYGSMCLIENTSIM70XX_H_
