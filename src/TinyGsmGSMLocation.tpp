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
    float lat = 0;
    float lon = 0;
    return thisModem().getGsmLocation(lat, lon, 0, year, month, day, hour,
                                      minute, second);
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
    if (thisModem().waitResponse(120000L, GF("+CLBS:0,")) != 1) { return ""; }
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
    if (thisModem().waitResponse(120000L, GF("+CLBS:0,")) != 1) {
      return false;
    }
    *lat = thisModem().streamGetFloat(',');  // Latitude
    *lon = thisModem().streamGetFloat(',');  // Longitude
    if (accuracy != NULL)
      *accuracy = thisModem().streamGetInt(',');  // Positioning accuracy

    // Date & Time
    char dtSBuff[5] = {'\0'};
    thisModem().stream.readBytes(dtSBuff, 4);  // Four digit year
    dtSBuff[4] = '\0';                         // null terminate buffer
    if (year != NULL) *year = atoi(dtSBuff);   // Convert to int
    thisModem().streamSkipUntil('/');          // Throw out slash

    thisModem().stream.readBytes(dtSBuff, 2);  // Two digit month
    dtSBuff[2] = '\0';
    if (month != NULL) *month = atoi(dtSBuff);
    thisModem().streamSkipUntil('/');  // Throw out slash

    thisModem().stream.readBytes(dtSBuff, 2);  // Two digit day
    dtSBuff[2] = '\0';
    if (day != NULL) *day = atoi(dtSBuff);
    thisModem().streamSkipUntil(',');  // Throw out comma

    thisModem().stream.readBytes(dtSBuff, 2);  // Two digit hour
    dtSBuff[2] = '\0';
    if (hour != NULL) *hour = atoi(dtSBuff);
    thisModem().streamSkipUntil(':');  // Throw out colon

    thisModem().stream.readBytes(dtSBuff, 2);  // Two digit minute
    dtSBuff[2] = '\0';
    if (minute != NULL) *minute = atoi(dtSBuff);
    thisModem().streamSkipUntil(':');  // Throw out colon

    thisModem().stream.readBytes(dtSBuff, 2);  // Two digit second
    dtSBuff[2] = '\0';
    if (second != NULL) *second = atoi(dtSBuff);
    thisModem().streamSkipUntil('\n');  // Should be at the end of the line

    // Final OK
    thisModem().waitResponse();
    return true;
  }
};

#endif  // SRC_TINYGSMGSMLOCATION_H_
