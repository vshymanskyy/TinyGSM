/**
 * @file       TinyGsmGPS.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMGPS_H_
#define SRC_TINYGSMGPS_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_GPS

template <class modemType>
class TinyGsmGPS {
 public:
  /*
   * GPS location functions
   */
  bool enableGPS() {
    return thisModem().enableGPSImpl();
  }
  bool disableGPS() {
    return thisModem().disableGPSImpl();
  }
  String getGPSraw() {
    return thisModem().getGPSrawImpl();
  }
  bool getGPSTime(int* year, int* month, int* day, int* hour, int* minute,
                  int* second) {
    return thisModem().getGPSTimeImpl(year, month, day, hour, minute, second);
  }
  bool getGPS(float* lat, float* lon, float* speed = 0, int* alt = 0) {
    return thisModem().getGPSImpl(lat, lon, speed, alt);
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
   * GPS location functions
   */

  bool   enableGPSImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool   disableGPSImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  String getGPSrawImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool   getGPSTimeImpl(int* year, int* month, int* day, int* hour, int* minute,
                        int* second) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool   getGPSImpl(float* lat, float* lon, float* speed = 0,
                    int* alt = 0) TINY_GSM_ATTR_NOT_IMPLEMENTED;
};


#endif  // SRC_TINYGSMGPS_H_
