/**
 * @file       TinyGsmGSMLocation.tpp
 * @brief      The CRTP parent class for GSM location functions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMGSMLOCATION_TPP_
#define SRC_TINYGSMGSMLOCATION_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_GSM_LOCATION
/// flag to indicate that the modem has GSM location functions
#define TINY_GSM_MODEM_HAS_GSM_LOCATION
#endif


/**
 * @class TinyGsmGSMLocation
 * @brief The CRTP parent class for GSM location functions.
 * @tparam modemType The derived modem class
 */
template <class modemType>
class TinyGsmGSMLocation {
 public:
  /// Compile-time capability flag indicating GSM-based location support
  static constexpr bool hasGSMLocation = true;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor gsm_location_functions
   * @name GSM location functions
   */
  /**@{*/

  /**
   * @brief Get the raw GSM location data.
   * @return The raw GSM location data as a String.
   */
  String getGsmLocationRaw() {
    return thisModem().getGsmLocationRawImpl();
  }

  /**
   * @brief Get the GSM location data.
   * @return The GSM location data as a String.
   */
  String getGsmLocation() {
    return thisModem().getGsmLocationRawImpl();
  }

  /**
   * @brief Get the GSM location data with detailed information.
   *
   * @param lat Pointer to store the latitude.
   * @param lon Pointer to store the longitude.
   * @param accuracy Pointer to store the accuracy (optional).
   * @param year Pointer to store the year (optional).
   * @param month Pointer to store the month (optional).
   * @param day Pointer to store the day (optional).
   * @param hour Pointer to store the hour (optional).
   * @param minute Pointer to store the minute (optional).
   * @param second Pointer to store the second (optional).
   *
   * @return True if the location was successfully retrieved, false otherwise.
   */
  bool getGsmLocation(float* lat, float* lon, float* accuracy = 0,
                      int* year = 0, int* month = 0, int* day = 0,
                      int* hour = 0, int* minute = 0, int* second = 0) {
    return thisModem().getGsmLocationImpl(lat, lon, accuracy, year, month, day,
                                          hour, minute, second);
  };

  /**
   * @brief Get the time attached to the GSM location data.
   *
   * @param year Pointer to store the year.
   * @param month Pointer to store the month.
   * @param day Pointer to store the day.
   * @param hour Pointer to store the hour.
   * @param minute Pointer to store the minute.
   * @param second Pointer to store the second.
   *
   * @return True if the time was successfully retrieved, false otherwise.
   */
  bool getGsmLocationTime(int* year, int* month, int* day, int* hour,
                          int* minute, int* second) {
    float lat      = 0;
    float lon      = 0;
    float accuracy = 0;
    return thisModem().getGsmLocation(&lat, &lon, &accuracy, year, month, day,
                                      hour, minute, second);
  }
  /**@}*/


 protected:
  // destructor (protected!)
  ~TinyGsmGSMLocation() {}

  /*
   * CRTP Helper
   */
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */
 protected:
  /*
   * GSM location functions
   * Template is based on SIMCOM commands
   */

  // String getGsmLocationImpl() {
  //   thisModem().sendAT(GF("+CIPGSMLOC=1,1"));
  //   if (thisModem().waitResponse(10000L, GF("+CIPGSMLOC:")) != 1) { return
  //   ""; } String res = thisModem().stream.readStringUntil('\n');
  //   thisModem().waitResponse();
  //   res.trim();
  //   return res;
  // }

  String getGsmLocationRawImpl() {
    // AT+CLBS=<type>,<cid>
    // <type> 1 = location using 3 cell's information
    //        3 = get number of times location has been accessed
    //        4 = Get longitude latitude and date time
    thisModem().sendAT(GF("+CLBS=1,1"));
    // Should get a location code of "0" indicating success
    if (thisModem().waitResponse(120000L, GF("+CLBS: ")) != 1) { return ""; }
    int8_t locationCode = thisModem().streamGetIntLength(2);
    // 0 = success, else, error
    if (locationCode != 0) {
      thisModem().waitResponse();  // should be an ok after the error
      return "";
    }
    String res = thisModem().stream.readStringUntil('\n');
    thisModem().waitResponse();
    res.trim();
    return res;
  }

  bool getGsmLocationImpl(float* lat, float* lon, float* accuracy, int* year,
                          int* month, int* day, int* hour, int* minute,
                          int* second) {
    // AT+CLBS=<type>,<cid>
    // <type> 1 = location using 3 cell's information
    //        3 = get number of times location has been accessed
    //        4 = Get longitude latitude and date time
    thisModem().sendAT(GF("+CLBS=4,1"));
    // Should get a location code of "0" indicating success
    if (thisModem().waitResponse(120000L, GF("+CLBS: ")) != 1) { return false; }
    int8_t locationCode = thisModem().streamGetIntLength(2);
    // 0 = success, else, error
    if (locationCode != 0) {
      thisModem().waitResponse();  // should be an ok after the error
      return false;
    }

    // init variables
    float   ilat      = 0;
    float   ilon      = 0;
    float   iaccuracy = 0;
    int16_t iyear     = 0;
    int16_t imonth    = 0;
    int16_t iday      = 0;
    int16_t ihour     = 0;
    int16_t imin      = 0;
    int16_t isec      = 0;

    ilat      = thisModem().streamGetFloatBefore(',');  // Latitude
    ilon      = thisModem().streamGetFloatBefore(',');  // Longitude
    iaccuracy = thisModem().streamGetIntBefore(',');    // Positioning accuracy

    // Date & Time
    iyear  = thisModem().streamGetIntBefore('/');
    imonth = thisModem().streamGetIntBefore('/');
    iday   = thisModem().streamGetIntBefore(',');
    ihour  = thisModem().streamGetIntBefore(':');
    imin   = thisModem().streamGetIntBefore(':');
    isec   = thisModem().streamGetIntBefore('\n');

    // Set pointers
    if (lat != nullptr) *lat = ilat;
    if (lon != nullptr) *lon = ilon;
    if (accuracy != nullptr) *accuracy = iaccuracy;
    if (iyear < 2000) iyear += 2000;
    if (year != nullptr) *year = iyear;
    if (month != nullptr) *month = imonth;
    if (day != nullptr) *day = iday;
    if (hour != nullptr) *hour = ihour;
    if (minute != nullptr) *minute = imin;
    if (second != nullptr) *second = isec;

#if 0
    DBG(GF("Latitude:"), String(ilat, 8), GF("\tLongitude:"), String(ilon, 8),
        GF("\tAccuracy:"), iaccuracy);
    DBG(GF("Year:"), iyear, GF("\tMonth:"), imonth, GF("\tDay:"), iday);
    DBG(GF("Hour:"), ihour, GF("\tMinute:"), imin, GF("\tSecond:"), isec);
#endif

    // Final OK
    thisModem().waitResponse();
    return true;
  }
};

#endif  // SRC_TINYGSMGSMLOCATION_TPP_
