/**
 * @file       TinyGsmGSMLocation.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMGSMLOCATION_H_
#define SRC_TINYGSMGSMLOCATION_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_GSM_LOCATION

template <class modemType>
class TinyGsmGSMLocation {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * GSM Location functions
   */
  String getGsmLocationRaw() {
    return thisModem().getGsmLocationRawImpl();
  }

  String getGsmLocation() {
    return thisModem().getGsmLocationRawImpl();
  }

  bool getGsmLocation(float* lat, float* lon, float* accuracy = 0,
                      int* year = 0, int* month = 0, int* day = 0,
                      int* hour = 0, int* minute = 0, int* second = 0) {
    return thisModem().getGsmLocationImpl(lat, lon, accuracy, year, month, day,
                                          hour, minute, second);
  };

  bool getGsmLocationTime(int* year, int* month, int* day, int* hour,
                          int* minute, int* second) {
    float lat      = 0;
    float lon      = 0;
    float accuracy = 0;
    return thisModem().getGsmLocation(&lat, &lon, &accuracy, year, month, day,
                                      hour, minute, second);
  }

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
  ~TinyGsmGSMLocation() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * GSM Location functions
   * Template is based on SIMCOM commands
   */
 protected:
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

  bool getGsmLocationImpl(float* lat, float* lon, float* accuracy = 0,
                          int* year = 0, int* month = 0, int* day = 0,
                          int* hour = 0, int* minute = 0, int* second = 0) {
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
    float ilat      = 0;
    float ilon      = 0;
    float iaccuracy = 0;
    int   iyear     = 0;
    int   imonth    = 0;
    int   iday      = 0;
    int   ihour     = 0;
    int   imin      = 0;
    int   isec      = 0;

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

    // Final OK
    thisModem().waitResponse();
    return true;
  }
};

#endif  // SRC_TINYGSMGSMLOCATION_H_
