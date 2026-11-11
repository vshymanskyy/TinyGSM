/**
 * @file       TinyGsmClientSIM70xx.h
 * @brief      Shared SIM70xx modem client base types and helpers.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 *
 * @defgroup simcom_sim70xx SIMCom SIM70xx (SIM7000, SIM7070, SIM7080, SIM7090)
 * @ingroup group_simcom
 * @brief Manufacturer: SIMCom. Models: SIM7000, SIM7070, SIM7080, SIM7090.
 */
#ifndef SRC_TINYGSMCLIENTSIM70XX_H_
#define SRC_TINYGSMCLIENTSIM70XX_H_
#pragma message("TinyGSM:  TinyGsmClientSIM70xx")

#include "mixins/TinyGsmModem.tpp"
#include "mixins/TinyGsmCellularData.tpp"
#include "mixins/TinyGsmGPS.tpp"

/// Registration status
/// @ingroup simcom_sim70xx
enum class Sim70xxRegStatus {
  REG_NO_RESULT    = -1,  ///< No registration result
  REG_UNREGISTERED = 0,   ///< Not registered on the network
  REG_SEARCHING    = 2,   ///< Searching for network
  REG_DENIED       = 3,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered on the home network
  REG_OK_ROAMING   = 5,   ///< Registered on a roaming network
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/**
 * @brief Parent class for the SIMCom SIM70xx family of modems
 *
 * @tparam SIM70xxType The derived class type
 * @tparam SIM70xxModemConfig The modem configuration type
 *
 * @ingroup simcom_sim70xx
 */
template <class SIM70xxType, class SIM70xxModemConfig>
class TinyGsmSim70xx : public TinyGsmModem<SIM70xxType, SIM70xxModemConfig>,
                       public TinyGsmCellularData<SIM70xxType>,
                       public TinyGsmGPS<SIM70xxType> {
  friend class TinyGsmModem<SIM70xxType, SIM70xxModemConfig>;
  friend class TinyGsmCellularData<SIM70xxType>;
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
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
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
  bool restartImpl(const char* pin) {
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
  bool sleepEnableImpl(bool enable) {
    thisModem().sendAT(GF("+CSCLK="), enable);
    return thisModem().waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset) {
    thisModem().sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return thisModem().waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 protected:
  Sim70xxRegStatus getRegistrationStatusImpl() {
    Sim70xxRegStatus epsStatus = static_cast<Sim70xxRegStatus>(
        thisModem().getRegistrationStatusXREG("CEREG"));
    // If we're connected on EPS, great!
    if (epsStatus == Sim70xxRegStatus::REG_OK_HOME ||
        epsStatus == Sim70xxRegStatus::REG_OK_ROAMING) {
      return epsStatus;
    } else {
      // Otherwise, check GPRS network status
      // We could be using GPRS fall-back or the board could be being moody
      return static_cast<Sim70xxRegStatus>(
          thisModem().getRegistrationStatusXREG("CGREG"));
    }
  }

  bool isNetworkConnectedImpl() {
    Sim70xxRegStatus s = this->getRegistrationStatus();
    return (s == Sim70xxRegStatus::REG_OK_HOME ||
            s == Sim70xxRegStatus::REG_OK_ROAMING);
  }

  /*
   * Network mode / type / technology functions
   */
 public:
  /**
   * @brief Get the available network modes of the modem.
   * @return A string representing the available network modes.
   */
  String getNetworkModes() {
    // Get the help string, not the setting value
    thisModem().sendAT(GF("+CNMP=?"));
    if (thisModem().waitResponse(GF("+CNMP:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
    return res;
  }

  /**
   * @brief Get the current network mode of the modem.
   * @return The current network mode as an integer.
   */
  int16_t getNetworkMode() {
    thisModem().sendAT(GF("+CNMP?"));
    if (thisModem().waitResponse(GF("+CNMP:")) != 1) { return false; }
    int16_t mode = thisModem().streamGetIntBefore('\n');
    thisModem().waitResponse();
    return mode;
  }

  /**
   * @brief Set the network mode of the modem.
   * @param mode The network mode to set.
   * @return True if the operation was successful, false otherwise.
   */
  bool setNetworkMode(uint8_t mode) {
    // 2 Automatic
    // 13 GSM only
    // 38 LTE only
    // 51 GSM and LTE only
    thisModem().sendAT(GF("+CNMP="), mode);
    return thisModem().waitResponse() == 1;
  }

  /**
   * @brief Get the available preferred network modes of the modem.
   * @return A string representing the available preferred network modes.
   */
  String getPreferredModes() {
    // Get the help string, not the setting value
    thisModem().sendAT(GF("+CMNB=?"));
    if (thisModem().waitResponse(GF("+CMNB:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
    return res;
  }

  /**
   * @brief Get the current preferred network mode of the modem.
   * @return The current preferred network mode as an integer.
   */
  int16_t getPreferredMode() {
    thisModem().sendAT(GF("+CMNB?"));
    if (thisModem().waitResponse(GF("+CMNB:")) != 1) { return false; }
    int16_t mode = thisModem().streamGetIntBefore('\n');
    thisModem().waitResponse();
    return mode;
  }

  /**
   * @brief Set the preferred network mode of the modem.
   * @param mode The preferred network mode to set.
   *  - 1 CAT-M
   *  - 2 NB-IoT
   *  - 3 CAT-M and NB-IoT
   * @return True if the operation was successful, false otherwise.
   */
  bool setPreferredMode(uint8_t mode) {
    thisModem().sendAT(GF("+CMNB="), mode);
    return thisModem().waitResponse() == 1;
  }

  /**
   * @brief Get the network system mode of the modem.
   * @param n A reference to a boolean that will be set to true if the modem is
   * in automatic reporting mode, false otherwise.
   * @param stat A reference to an integer that will be set to the current
   * service status. 0 if not connected.
   * @return True if the operation was successful, false otherwise.
   */
  bool getNetworkSystemMode(bool& n, int16_t& stat) {
    // n: whether to automatically report the system mode info
    // stat: the current service. 0 if it not connected
    thisModem().sendAT(GF("+CNSMOD?"));
    if (thisModem().waitResponse(GF("+CNSMOD:")) != 1) { return false; }
    n    = thisModem().streamGetIntBefore(',') != 0;
    stat = thisModem().streamGetIntBefore('\n');
    thisModem().waitResponse();
    return true;
  }

  /**
   * @brief Set the network system mode of the modem.
   * @param n A boolean indicating whether to enable automatic reporting of the
   * system mode info.
   * @return True if the operation was successful, false otherwise.
   */
  bool setNetworkSystemMode(bool n) {
    // n: whether to automatically report the system mode info
    thisModem().sendAT(GF("+CNSMOD="), int8_t(n));
    return thisModem().waitResponse() == 1;
  }

  /*
   * Cellular data functions
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
    if (thisModem().waitResponse(GFP(SIM70xxModemConfig::GSM_NL)) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
    res.trim();
    return res;
  }

  // Gets the IMEI of the modem with AT+CGSN
  String getIMEIImpl() {
    thisModem().sendAT(GF("+CGSN"));
    if (thisModem().waitResponse(GFP(SIM70xxModemConfig::GSM_NL)) != 1) {
      return "";
    }
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
    if (thisModem().waitResponse(10000L, GF("+CGNSINF:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    thisModem().waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  bool getGPSImpl(float* lat, float* lon, float* speed, float* alt, int* vsat,
                  int* usat, float* accuracy, int* year, int* month, int* day,
                  int* hour, int* minute, int* second) {
    thisModem().sendAT(GF("+CGNSINF"));
    if (thisModem().waitResponse(10000L, GF("+CGNSINF:")) != 1) {
      return false;
    }
    // <GNSS run status>
    //  - 0 GNSS off.
    //  - 1 GNSS on.
    // <Fix status>
    //  - 0 Not fixed position.
    //  - 1 Fixed position.

#if defined(TINY_GSM_DEBUG) && 0
    int16_t run_status = thisModem().streamGetIntBefore(',');
#else
    thisModem().streamSkipUntil(',');  // GNSS run status
#endif
    int16_t fix_status = thisModem().streamGetIntBefore(',');  // fix status

    // init variables
    float   ilat         = 0;
    float   ilon         = 0;
    float   ispeed       = 0;
    float   ialt         = 0;
    int16_t ivsat        = 0;
    int16_t iusat        = 0;
    float   iaccuracy    = 0;
    int16_t iyear        = 0;
    int16_t imonth       = 0;
    int16_t iday         = 0;
    int16_t ihour        = 0;
    int16_t imin         = 0;
    float   secondWithSS = 0;

    // UTC date & Time
    // If the modem hasn't gotten a fix, the date and time might be missing.
    // Before we parse the date and time by the expecte d number of characters,
    // check if the date and time is present by checking if the next character
    // is a comma.  If it is a comma, then the date and time is missing and we
    // will skip parsing it.
    // NOTE: only wait for the response inter-character timeout, not a whole
    // timeout. This character is in the middle of a long string sent by the
    // modem in a single chunk.
    bool hasDateTime =
        thisModem().streamTimedPeek(TINY_GSM_INTERCHAR_TIMEOUT_MS) != ',';
    if (hasDateTime) {
      iyear        = thisModem().streamGetIntLength(4);  // Four digit year
      imonth       = thisModem().streamGetIntLength(2);  // Two digit month
      iday         = thisModem().streamGetIntLength(2);  // Two digit day
      ihour        = thisModem().streamGetIntLength(2);  // Two digit hour
      imin         = thisModem().streamGetIntLength(2);  // Two digit minute
      secondWithSS = thisModem().streamGetFloatBefore(
          ',');  // 6 digit second with subseconds
    } else {
      stream.read();  // Throw away the comma
    }

    ilat = thisModem().streamGetFloatBefore(',');  // Latitude
    ilon = thisModem().streamGetFloatBefore(',');  // Longitude
    ialt =
        thisModem().streamGetFloatBefore(',');  // MSL Altitude. Unit is meters
    ispeed = thisModem().streamGetFloatBefore(',');
    //^ Speed Over Ground. Unit is km/h (AT manuals, table "AT+CGNSINF return
    // Parameters").
    thisModem().streamSkipUntil(',');  // Course Over Ground. Degrees.
    thisModem().streamSkipUntil(',');  // Fix Mode
    thisModem().streamSkipUntil(',');  // Reserved1
    iaccuracy = thisModem().streamGetFloatBefore(
        ',');                          // Horizontal Dilution Of Precision
    thisModem().streamSkipUntil(',');  // Position Dilution Of Precision
    thisModem().streamSkipUntil(',');  // Vertical Dilution Of Precision
    thisModem().streamSkipUntil(',');  // Reserved2
    ivsat = thisModem().streamGetIntBefore(',');  // GNSS Satellites in View

    // The rest of the line differs between modules. A SIM7000 goes on with
    // <GNSS Satellites Used>,<GLONASS Satellites Used>,<Reserved3>,
    // <C/N0 max>,<HPA>,<VPA>; a SIM7070/SIM7080/SIM7090 answers only
    // <Reserved3>,<HPA>,<VPA> (SIM7070_SIM7080_SIM7090 Series AT Command
    // Manual V1.04, section 8.2.2). Skipping six fields on the short layout
    // waited out a full stream timeout on each missing field and on the OK,
    // five seconds per call, so read the remainder as one line and let its
    // field count pick the layout.
    String rest   = thisModem().stream.readStringUntil('\n');
    int    fields = 1;
    for (unsigned int i = 0; i < rest.length(); i++) {
      if (rest[i] == ',') { fields++; }
    }
    if (fields >= 6) {  // SIM7000 layout: the first remaining field is "used"
      iusat = rest.substring(0, rest.indexOf(',')).toInt();
    }

    // Set pointers **even if the fix is not valid!!**
    // The module returns avlues even without a valid GNSS fix, so we set the
    // pointers regardless. Allow the user to decide whether to use the data
    // based on the fix status.
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

#if 0
    DBG("GNSS run status:", run_status);
    DBG("Fix status:", fix_status);
    DBG(GF("Latitude:"), String(ilat, 8), GF("\tLongitude:"), String(ilon, 8),
        GF("\tAltitude:"), String(ialt, 4));
    DBG(GF("VSAT:"), ivsat, GF("\tUSAT:"), iusat, GF("\tAccuracy:"), iaccuracy);
    DBG(GF("Year:"), iyear, GF("\tMonth:"), imonth, GF("\tDay:"), iday);
    DBG(GF("Hour:"), ihour, GF("\tMinute:"), imin, GF("\tSecond:"),
        String(secondWithSS, 3));
#endif

    return fix_status == 1;  // return true if we have a fix
  }

  /*
   * Utilities
   */
 protected:
 public:
  /// Stream used to communicate with the modem.
  Stream& stream;
};

#endif  // SRC_TINYGSMCLIENTSIM70XX_H_
