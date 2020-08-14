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
   * GPS/GNSS/GLONASS location functions
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
  bool getGPS(float* lat, float* lon, float* speed = 0, float* alt = 0,
              int* vsat = 0, int* usat = 0, float* accuracy = 0, int* year = 0,
              int* month = 0, int* day = 0, int* hour = 0, int* minute = 0,
              int* second = 0) {
    return thisModem().getGPSImpl(lat, lon, speed, alt, vsat, usat, accuracy,
                                  year, month, day, hour, minute, second);
  }
  bool getGPSTime(int* year, int* month, int* day, int* hour, int* minute,
                  int* second) {
    float lat = 0;
    float lon = 0;
    return thisModem().getGPSImpl(lat, lon, 0, 0, 0, 0, 0, year, month, day,
                                  hour, minute, second);
  }

  String setGNSSMode(uint8_t mode,bool dpo)
  {
    return thisModem().setGNSSModeImpl(mode,dpo);
  }

  uint8_t getGNSSMode()
  {
    return thisModem().getGNSSModeImpl();
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
   * GPS/GNSS/GLONASS location functions
   */

  bool   enableGPSImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool   disableGPSImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  String getGPSrawImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool   getGPSImpl(float* lat, float* lon, float* speed = 0, float* alt = 0,
                    int* vsat = 0, int* usat = 0, float* accuracy = 0,
                    int* year = 0, int* month = 0, int* day = 0, int* hour = 0,
                    int* minute = 0,
                    int* second = 0) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  String setGNSSModeImpl(uint8_t mode,bool dpo) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  uint8_t getGNSSModeImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
};


#endif  // SRC_TINYGSMGPS_H_
