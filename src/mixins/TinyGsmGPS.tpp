/**
 * @file       TinyGsmGPS.tpp
 * @brief      GNSS/GPS helper mixin for location and satellite data.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMGPS_TPP_
#define SRC_TINYGSMGPS_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_GPS
/// flag to indicate that the modem has GPS functions
#define TINY_GSM_MODEM_HAS_GPS
#endif

/**
 * @class TinyGsmGPS
 * @brief The CRTP parent class for GPS functions.
 * @tparam modemType The derived modem class
 */
template <class modemType>
class TinyGsmGPS {
 public:
  /// Compile-time capability flag indicating GPS/GNSS support
  static constexpr bool hasGPS = true;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor gps_functions
   * @name GPS (GNSS, GLONASS) functions
   */
  /**@{*/
  /**
   * @brief Enable the GPS module.
   * @return True if the GPS module was successfully enabled, false otherwise.
   */
  bool enableGPS() {
    return thisModem().enableGPSImpl();
  }
  /**
   * @brief Disable the GPS module.
   * @return True if the GPS module was successfully disabled, false otherwise.
   */
  bool disableGPS() {
    return thisModem().disableGPSImpl();
  }
  /**
   * @brief Get the raw GPS data string.
   * @return The raw GPS data string.
   */
  String getGPSraw() {
    return thisModem().getGPSrawImpl();
  }
  /**
   * @brief Get the GPS location data.
   * @param lat Pointer to store the latitude.
   * @param lon Pointer to store the longitude.
   * @param speed Pointer to store the speed (optional).
   * @param alt Pointer to store the altitude (optional).
   * @param vsat Pointer to store the number of visible satellites (optional).
   * @param usat Pointer to store the number of used satellites (optional).
   * @param accuracy Pointer to store the accuracy (optional).
   * @param year Pointer to store the year (optional).
   * @param month Pointer to store the month (optional).
   * @param day Pointer to store the day (optional).
   * @param hour Pointer to store the hour (optional).
   * @param minute Pointer to store the minute (optional).
   * @param second Pointer to store the second (optional).
   * @return True if the GPS data was successfully retrieved, false otherwise.
   */
  bool getGPS(float* lat, float* lon, float* speed = 0, float* alt = 0,
              int* vsat = 0, int* usat = 0, float* accuracy = 0, int* year = 0,
              int* month = 0, int* day = 0, int* hour = 0, int* minute = 0,
              int* second = 0) {
    return thisModem().getGPSImpl(lat, lon, speed, alt, vsat, usat, accuracy,
                                  year, month, day, hour, minute, second);
  }
  /**
   * @brief Get the time associated with the GPS data.
   * @param year Pointer to store the year.
   * @param month Pointer to store the month.
   * @param day Pointer to store the day.
   * @param hour Pointer to store the hour.
   * @param minute Pointer to store the minute.
   * @param second Pointer to store the second.
   * @return True if the GPS time data was successfully retrieved, false
   * otherwise.
   */
  bool getGPSTime(int* year, int* month, int* day, int* hour, int* minute,
                  int* second) {
    float lat = 0;
    float lon = 0;
    return thisModem().getGPSImpl(&lat, &lon, 0, 0, 0, 0, 0, year, month, day,
                                  hour, minute, second);
  }

  /**
   * @brief Set the GNSS mode.
   * @param mode The GNSS mode to set.
   * @param dpo Whether to enable or disable DPO.
   * @return The result of the operation.
   */
  String setGNSSMode(uint8_t mode, bool dpo) {
    return thisModem().setGNSSModeImpl(mode, dpo);
  }

  /**
   * @brief Get the current GNSS mode.
   * @return The current GNSS mode.
   */
  uint8_t getGNSSMode() {
    return thisModem().getGNSSModeImpl();
  }
  /**@}*/


 protected:
  // destructor (protected!)
  ~TinyGsmGPS() {}

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
   * GPS/GNSS/GLONASS location functions
   */

  bool   enableGPSImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool   disableGPSImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  String getGPSrawImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool   getGPSImpl(float* lat, float* lon, float* speed, float* alt, int* vsat,
                    int* usat, float* accuracy, int* year, int* month, int* day,
                    int* hour, int* minute,
                    int* second) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  String setGNSSModeImpl(uint8_t mode, bool dpo) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  uint8_t getGNSSModeImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;
};


#endif  // SRC_TINYGSMGPS_TPP_
