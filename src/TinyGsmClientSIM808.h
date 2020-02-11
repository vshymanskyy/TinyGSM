/**
 * @file     TinyGsmClientSIM808.h
 * @author   Volodymyr Shymanskyy
 * @license  LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date     Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSIM808_H_
#define SRC_TINYGSMCLIENTSIM808_H_
// #pragma message("TinyGSM:  TinyGsmClientSIM808")

#include "TinyGsmClientSIM800.h"
#include "TinyGsmGPS.tpp"

class TinyGsmSim808 : public TinyGsmSim800, public TinyGsmGPS<TinyGsmSim808> {
  friend class TinyGsmGPS<TinyGsmSim808>;

 public:
  explicit TinyGsmSim808(Stream& stream) : TinyGsmSim800(stream) {}


  /*
   * GPS location functions
   */
 protected:
  // enable GPS
  bool enableGPSImpl() {
    // uint16_t state;

    sendAT(GF("+CGNSPWR=1"));
    if (waitResponse() != 1) { return false; }

    return true;
  }

  bool disableGPSImpl() {
    // uint16_t state;

    sendAT(GF("+CGNSPWR=0"));
    if (waitResponse() != 1) { return false; }

    return true;
  }

  // get the RAW GPS output
  // works only with ans SIM808 V2
  String getGPSrawImpl() {
    sendAT(GF("+CGNSINF"));
    if (waitResponse(GF(GSM_NL "+CGNSINF:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  // works only with ans SIM808 V2
  bool getGPSImpl(float* lat, float* lon, float* speed = 0, int* alt = 0,
                  int* vsat = 0, int* usat = 0) {
    // String buffer = "";
    // char chr_buffer[12];
    bool fix = false;

    sendAT(GF("+CGNSINF"));
    if (waitResponse(GF(GSM_NL "+CGNSINF:")) != 1) { return false; }

    streamSkipUntil(',');                             // GNSS run status
    if (streamGetInt(',') == 1) fix = true;           // fix status
    streamSkipUntil(',');                             // UTC date & Time
    *lat = streamGetFloat(',');                       // Latitude
    *lon = streamGetFloat(',');                       // Longitude
    if (alt != NULL) *alt = streamGetFloat(',');      // MSL Altitude
    if (speed != NULL) *speed = streamGetFloat(',');  // Speed Over Ground
    streamSkipUntil(',');                             // Course Over Ground
    streamSkipUntil(',');                             // Fix Mode
    streamSkipUntil(',');                             // Reserved1
    streamSkipUntil(',');  // Horizontal Dilution Of Precision
    streamSkipUntil(',');  // Position Dilution Of Precision
    streamSkipUntil(',');  // Vertical Dilution Of Precision
    streamSkipUntil(',');  // Reserved2
    if (vsat != NULL) *vsat = streamGetInt(',');  // GNSS Satellites in View
    if (usat != NULL) *usat = streamGetInt(',');  // GNSS Satellites Used
    streamSkipUntil(',');                         // GLONASS Satellites Used
    streamSkipUntil(',');                         // Reserved3
    streamSkipUntil(',');                         // C/N0 max
    streamSkipUntil(',');                         // HPA
    streamSkipUntil('\n');                        // VPA

    waitResponse();

    return fix;
  }

  // get GPS time
  // works only with SIM808 V2
  bool getGPSTimeImpl(int* year, int* month, int* day, int* hour, int* minute,
                      int* second) {
    bool fix = false;
    char chr_buffer[12];
    sendAT(GF("+CGNSINF"));
    if (waitResponse(GF(GSM_NL "+CGNSINF:")) != 1) { return false; }

    for (int i = 0; i < 3; i++) {
      String buffer = stream.readStringUntil(',');
      buffer.toCharArray(chr_buffer, sizeof(chr_buffer));
      switch (i) {
        case 0:
          // mode
          break;
        case 1:
          // fixstatus
          if (buffer.toInt() == 1) { fix = buffer.toInt(); }
          break;
        case 2:
          *year   = buffer.substring(0, 4).toInt();
          *month  = buffer.substring(4, 6).toInt();
          *day    = buffer.substring(6, 8).toInt();
          *hour   = buffer.substring(8, 10).toInt();
          *minute = buffer.substring(10, 12).toInt();
          *second = buffer.substring(12, 14).toInt();
          break;

        default:
          // if nothing else matches, do the default
          // default is optional
          break;
      }
    }
    streamSkipUntil('\n');
    waitResponse();

    if (fix) {
      return true;
    } else {
      return false;
    }
  }
};

#endif  // SRC_TINYGSMCLIENTSIM808_H_
