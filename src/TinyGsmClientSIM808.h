/**
 * @file     TinyGsmClientSIM808.h
 * @brief      SIM808 modem client and modem-trait definitions.
 * @author   Volodymyr Shymanskyy
 * @license  LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date     Nov 2016
 */
/**
 * @defgroup simcom_sim808 SIMCom SIM808 and SIM868
 * @ingroup simcom_sim800
 * @brief Manufacturer: SIMCom. Models: SIM808, SIM868.
 *
 * # Connection Information
 *
 * - TCP sockets: 8.
 * - SSL sockets: 5.
 * - Socket Buffering:
 *   - The modem has an internal buffer for incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 * - Note: This module inherits TCP/SSL behavior from TinyGsmClientSIM800.h.
 */

#ifndef SRC_TINYGSMCLIENTSIM808_H_
#define SRC_TINYGSMCLIENTSIM808_H_
#pragma message("TinyGSM:  TinyGsmClientSIM808")

#include "TinyGsmClientSIM800.h"
#include "TinyGsmGPS.tpp"
#include "TinyGsmBluetooth.tpp"

/**
 * @brief Class for the SIMCOM SIM808 and SIM868, which extends the SIM800 with
 * GPS and Bluetooth support
 * @ingroup simcom_sim808
 */
class TinyGsmSim808 : public TinyGsmSim800,
                      public TinyGsmGPS<TinyGsmSim808>,
                      public TinyGsmBluetooth<TinyGsmSim808> {
  friend class TinyGsmGPS<TinyGsmSim808>;
  friend class TinyGsmBluetooth<TinyGsmSim808>;

  /*
   * Public Aliases
   */
 public:
  /// Arduino Client type inherited from the SIM800 implementation.
  using TinyGsmClient = TinyGsmSim800::TinyGsmClient;
#if !defined(TINY_GSM_MODEM_SIM900)
  /// Secure Arduino Client type inherited from the SIM800 implementation.
  using TinyGsmClientSecure = TinyGsmSim800::TinyGsmClientSecure;
#endif

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmSim808(Stream& stream) : TinyGsmSim800(stream) {}


  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // enable GPS
  bool enableGPSImpl() {
    sendAT(GF("+CGNSPWR=1"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool disableGPSImpl() {
    sendAT(GF("+CGNSPWR=0"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  // get the RAW GPS output
  // works only with ans SIM808 V2
  String getGPSrawImpl() {
    sendAT(GF("+CGNSINF"));
    if (waitResponse(10000L, GF("+CGNSINF:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  // works only with ans SIM808 V2
  bool getGPSImpl(float* lat, float* lon, float* speed, float* alt, int* vsat,
                  int* usat, float* accuracy, int* year, int* month, int* day,
                  int* hour, int* minute, int* second) {
    sendAT(GF("+CGNSINF"));
    if (waitResponse(10000L, GF("+CGNSINF:")) != 1) { return false; }

#if defined(TINY_GSM_DEBUG) && 0
    int16_t run_status = streamGetIntBefore(',');
#else
    streamSkipUntil(',');  // GNSS run status
#endif
    int16_t fix_status = streamGetIntBefore(',');  // fix status

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
    bool hasDateTime = streamTimedPeek(TINY_GSM_INTERCHAR_TIMEOUT_MS) != ',';
    if (hasDateTime) {
      iyear  = streamGetIntLength(4);  // Four digit year
      imonth = streamGetIntLength(2);  // Two digit month
      iday   = streamGetIntLength(2);  // Two digit day
      ihour  = streamGetIntLength(2);  // Two digit hour
      imin   = streamGetIntLength(2);  // Two digit minute
      secondWithSS =
          streamGetFloatBefore(',');  // 6 digit second with subseconds
    } else {
      stream.read();  // Throw away the comma
    }

    ilat   = streamGetFloatBefore(',');     // Latitude
    ilon   = streamGetFloatBefore(',');     // Longitude
    ialt   = streamGetFloatBefore(',');     // MSL Altitude. Unit is meters
    ispeed = streamGetFloatBefore(',');     // Speed Over Ground. Unit is knots.
    streamSkipUntil(',');                   // Course Over Ground. Degrees.
    streamSkipUntil(',');                   // Fix Mode
    streamSkipUntil(',');                   // Reserved1
    iaccuracy = streamGetFloatBefore(',');  // Horizontal Dilution Of Precision
    streamSkipUntil(',');                   // Position Dilution Of Precision
    streamSkipUntil(',');                   // Vertical Dilution Of Precision
    streamSkipUntil(',');                   // Reserved2
    ivsat = streamGetIntBefore(',');        // GNSS Satellites in View
    iusat = streamGetIntBefore(',');        // GNSS Satellites Used
    streamSkipUntil(',');                   // GLONASS Satellites Used
    streamSkipUntil(',');                   // Reserved3
    streamSkipUntil(',');                   // C/N0 max
    streamSkipUntil(',');                   // HPA
    streamSkipUntil('\n');                  // VPA

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

    waitResponse();

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
   * Bluetooth functions
   */

  bool enableBluetoothImpl() {
    sendAT(GF("+BTPOWER=1"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool disableBluetoothImpl() {
    sendAT(GF("+BTPOWER=0"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool setBluetoothVisibilityImpl(bool visible) {
    sendAT(GF("+BTVIS="), visible);
    if (waitResponse() != 1) { return false; }

    return true;
  }

  bool setBluetoothHostNameImpl(const char* name) {
    sendAT(GF("+BTHOST="), name);
    if (waitResponse() != 1) { return false; }

    return true;
  }
};

#endif  // SRC_TINYGSMCLIENTSIM808_H_
