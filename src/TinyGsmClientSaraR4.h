/**
 * @file       TinyGsmClientSaraR4.h
 * @brief      u-blox SARA-R4 modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup ublox_sara_r4 u-blox SARA-R4
 * @ingroup group_ublox
 * @brief Manufacturer: u-blox. Models: SARA-R4.
 *
 * # Connection Information
 *
 * - TCP sockets:
 *   - 7
 * - SSL sockets:
 *   - 4
 * - SSL contexts (security profiles):
 *   - 5
 * - Socket Buffering:
 *   - The modem has an internal buffer for incoming data.
 * - Socket Numbering:
 *   - The modem will automatically assign a channel number when the client
 * connects to a server.
 *
 * @todo In `restartImpl()`: Verify delay timing here
 */

#ifndef SRC_TINYGSMCLIENTSARAR4_H_
#define SRC_TINYGSMCLIENTSARAR4_H_
#pragma message("TinyGSM:  TinyGsmClientSaraR4")

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"

// NOTE: This module supports SSL, but we do not support any certificate
// management yet. Define TINY_GSM_MODEM_HAS_SSL here and do no include the SSL
// module.
#ifndef TINY_GSM_MODEM_HAS_SSL
/// flag to indicate that the modem has Secure Socket Layer (SSL) functions
#define TINY_GSM_MODEM_HAS_SSL
#endif

#include "TinyGsmCellularData.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmGPS.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmBattery.tpp"
#include "TinyGsmTemperature.tpp"

/// Registration status
/// @ingroup ublox_sara_r4
enum class SaraR4RegStatus {
  REG_NO_RESULT    = -1,  ///< No registration result
  REG_UNREGISTERED = 0,   ///< Not registered on the network
  REG_SEARCHING    = 2,   ///< Searching for network
  REG_DENIED       = 3,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered on the home network
  REG_OK_ROAMING   = 5,   ///< Registered on a roaming network
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/// Basic modem configurations for the u-blox SARA-R4
/// @ingroup ublox_sara_r4
struct TinyGsmSaraR4ModemConfig
    : public TinyGsmModemConfigPreset<SaraR4RegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "u-blox";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SARA-R4";
};

constexpr char TinyGsmSaraR4ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmSaraR4ModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for the u-blox SARA-R4.
 *
 * Per the manual, 7 standard sockets or 4 SSL sockets can be managed.
 *
 * @todo Handle the different number of sockets for TCP and SSL in the code.
 *
 * @warning You cannot control the socket mux number on this module! The module
 * opens the connection and returns the connection number it opened.
 *
 * The send data command, USOWR, accepts up to 1024 bytes in "normal" and
 * "binary extended" modes and up to 512 bytes in "HEX" mode.
 * @ingroup ublox_sara_r4
 */
struct TinyGsmSaraR4TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Dynamic,
          /*muxCount*/ 7,
          /*sendMaxSize*/ 1024,
          /*connectTimeoutS*/ 120,
          /*stopTimeoutS*/ 135> {};

#if 0
/**
 * @brief SSL behavior and limits for the u-blox SARA-R4.
 * The module supports 5 SSL contexts (called security profiles, numbered 0-4).
 * This library does not support selecting or modifying the security profiles in
 * any way.  The module will always use the default profile.
 * @ingroup ublox_sara_r4
 */
struct TinyGsmSaraR4SslConfig : public TinyGsmSslConfigPreset<
                                   /*contextCount*/ 5> {};
#endif

/// Class for the u-blox SARA-R4
/// @ingroup ublox_sara_r4
class TinyGsmSaraR4
    : public TinyGsmModem<TinyGsmSaraR4, TinyGsmSaraR4ModemConfig>,
      public TinyGsmCellularData<TinyGsmSaraR4>,
      public TinyGsmTCP<TinyGsmSaraR4, TinyGsmSaraR4TcpConfig>,
      public TinyGsmSMS<TinyGsmSaraR4>,
      public TinyGsmGSMLocation<TinyGsmSaraR4>,
      public TinyGsmGPS<TinyGsmSaraR4>,
      public TinyGsmTime<TinyGsmSaraR4>,
      public TinyGsmBattery<TinyGsmSaraR4>,
      public TinyGsmTemperature<TinyGsmSaraR4> {
  friend class TinyGsmModem<TinyGsmSaraR4, TinyGsmSaraR4ModemConfig>;
  friend class TinyGsmCellularData<TinyGsmSaraR4>;
  friend class TinyGsmTCP<TinyGsmSaraR4, TinyGsmSaraR4TcpConfig>;
  friend class GsmClient<TinyGsmSaraR4, TinyGsmSaraR4TcpConfig>;
  friend class TinyGsmSMS<TinyGsmSaraR4>;
  friend class TinyGsmGSMLocation<TinyGsmSaraR4>;
  friend class TinyGsmGPS<TinyGsmSaraR4>;
  friend class TinyGsmTime<TinyGsmSaraR4>;
  friend class TinyGsmTemperature<TinyGsmSaraR4>;
  friend class TinyGsmBattery<TinyGsmSaraR4>;

 public:
  using ModemConfig = TinyGsmSaraR4ModemConfig;
  using TcpConfig   = TinyGsmSaraR4TcpConfig;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup ublox_sara_r4
  class GsmClientSaraR4
      : public GsmClient<TinyGsmSaraR4, TinyGsmSaraR4TcpConfig> {
    friend class TinyGsmSaraR4;

   public:
    using GsmClient<TinyGsmSaraR4, TinyGsmSaraR4TcpConfig>::connect;
    using GsmClient<TinyGsmSaraR4, TinyGsmSaraR4TcpConfig>::stop;
    using TcpConfig = TinyGsmSaraR4TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSaraR4() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem.
     * @param modem Modem instance used by this client.
     *
     * @note The SARA-R4 does not allow you to specify the multiplexing channel.
     * The modem will automatically assign a channel when the client connects to
     * a server.  Use the getMux() function to get the assigned multiplexing
     * channel number after a successful connection.
     */
    explicit GsmClientSaraR4(TinyGsmSaraR4& modem, uint8_t /*mux*/ = 0)
        : GsmClient<TinyGsmSaraR4, TinyGsmSaraR4TcpConfig>(modem /*, mux*/) {
      is_secure = false;
      init(&modem);
    }

    /**
     * @brief Initialize the TCP client with a modem.
     * @return true if initialization was successful, false otherwise.
     * @copydetails GsmClientSaraR4::GsmClientSaraR4(TinyGsmSaraR4&, uint8_t)
     */
    bool init(TinyGsmSaraR4* modem, uint8_t /*mux*/ = 0) override {
      // This is a placeholder registration. The modem assigns the actual mux
      // during connect(), and the client registration is moved at that time.
      return this->attachToModem(this, modem, 0);
    }

    /*
     * Arduino Client interface
     */
   public:
    /**
     * @copydoc GsmClient::connect(const char*, uint16_t, int)
     * @warning The SARA-R4 modem automatically assigns the multiplexing
     * channel.  When the modem assigns the mux, the client will be registered
     * there.  An attempt will be made to move any other client that had been at
     * that position in the modem's socket array.  If it cannot be moved, the
     * client at the assigned mux will be forcibly overwritten.  This function
     * assumes that the client you just asked to connect is more important than
     * any client that might already be at the assigned mux.
     */
    int connect(const char* host, uint16_t port, int timeout_s) override {
      if (at == nullptr) { return 0; }
      is_mid_send = false;
#if 0
      // stop if and only if the mux number is valid, the socket pointer is not
      // null, and the socket is connected
      if (mux < TcpConfig::kMuxCount && at->sockets[mux] != nullptr && sock_connected) {
        stop(TcpConfig::kStopTimeoutS * 1000L);
      }
#endif
      rx.clear();
      // attempt to use the requested mux number first
      uint8_t assignedMux = mux;
      // modemConnect will validate the mux number returned by the modem and
      // return false and set the assignedMux to -1 if the mux number is invalid
      // or the connection fails
      sock_connected = at->modemConnect(host, port, &assignedMux, timeout_s);
      if (sock_connected &&
          !this->registerAssignedMux(this, assignedMux, true)) {
        at->modemStop(assignedMux, TcpConfig::kStopTimeoutS * 1000L);
        sock_connected = false;
      }
      at->maintain();
      return sock_connected;
    }

    /*
     * Extended Client API
     */
    // No extra extended API functions
  };

  /*
   * Inner Secure Client
   */
 public:
  /// Inner secure client
  /// @ingroup ublox_sara_r4
  class GsmClientSecureSaraR4 : public GsmClientSaraR4 {
    friend class TinyGsmSaraR4;

   public:
    using GsmClientSaraR4::connect;
    using GsmClientSaraR4::stop;
    using TcpConfig = TinyGsmSaraR4TcpConfig;

    /*
     * Secure client constructors and initialization
     */
    /**
     * @brief Create a new secured TCP (SSL) client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSecureSaraR4() {
      is_secure = true;
    }
    /**
     * @brief Create a new secured TCP (SSL) client and bind it to a modem.
     * @copydetails GsmClientSaraR4::GsmClientSaraR4(TinyGsmSaraR4&, uint8_t)
     */
    explicit GsmClientSecureSaraR4(TinyGsmSaraR4& modem, uint8_t /*mux*/ = 0)
        : GsmClientSaraR4(modem) {
      is_secure = true;
    }
  };
  /// Typedef for backward compatibility
  using GsmClientSecureR4 = GsmClientSecureSaraR4;

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmSaraR4(Stream& stream)
      : stream(stream),
        has2GFallback(false),
        supportsAsyncSockets(false) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSaraR4"));

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    String modemName = getModemName();
    DBG(GF("### Modem:"), modemName);
    if (modemName.startsWith("u-blox SARA-R412")) {
      has2GFallback = true;
    } else {
      has2GFallback = false;
    }
    if (modemName.startsWith("u-blox SARA-R404M") ||
        modemName.startsWith("u-blox SARA-R410M-01B")) {
      supportsAsyncSockets = false;
    } else {
      supportsAsyncSockets = true;
    }

    // Enable automatic time zome update
    sendAT(GF("+CTZU=1"));
    if (waitResponse(10000L) != 1) { return false; }

    SimStatus ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != nullptr && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    } else {
      // if the sim is ready, or it's locked but no pin has been provided,
      // return true
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  // only difference in implementation is the warning on the wrong type
  String getModemNameImpl() {
    String manufacturer = getModemManufacturer();
    String model        = getModemModel();
    String name         = manufacturer + ' ' + model;
#ifdef TINY_GSM_DEBUG
    DBG("### Modem:", name);
    if (!name.startsWith("u-blox SARA-R4") &&
        !name.startsWith("u-blox SARA-N4")) {
      DBG("### WARNING:  You are using the wrong TinyGSM modem!");
    }
#endif
    return name;
  }

  bool factoryDefaultImpl() {
    sendAT(GF("&F"));  // Resets the current profile, other NVM not affected
    return waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  // using +CFUN=15 instead of the more common CFUN=1,1
  bool restartImpl(const char* pin) {
    if (!testAT()) { return false; }
    if (!setPhoneFunctionality(15)) { return false; }
    delay(3000);  // TODO(?):  Verify delay timing here
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPWROFF"));
    return waitResponse(40000L) == 1;
  }

  bool sleepEnableImpl(bool enable) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 protected:
  SaraR4RegStatus getRegistrationStatusImpl() {
    // Check first for EPS registration
    SaraR4RegStatus epsStatus =
        static_cast<SaraR4RegStatus>(getRegistrationStatusXREG("CEREG"));

    // If we're connected on EPS, great!
    if (epsStatus == SaraR4RegStatus::REG_OK_HOME ||
        epsStatus == SaraR4RegStatus::REG_OK_ROAMING) {
      return epsStatus;
    } else {
      // Otherwise, check generic network status
      return static_cast<SaraR4RegStatus>(getRegistrationStatusXREG("CREG"));
    }
  }

  bool isNetworkConnectedImpl() {
    SaraR4RegStatus s = this->getRegistrationStatus();
    return (s == SaraR4RegStatus::REG_OK_HOME ||
            s == SaraR4RegStatus::REG_OK_ROAMING);
  }

  /*
   * Network mode / type / technology functions
   */
 public:
  /**
   * @brief Set the radio access technology (RAT) for the modem.
   *
   * @param urat The selected RAT mode.
   * - 3: LTE
   * - 7: LTE Cat M1
   * - 8: LTE Cat NB1
   * - 9: GPRS / eGPRS
   * @return True if the command was successful, false otherwise.
   */
  bool setRadioAccessTechnology(uint8_t urat, uint8_t) {
    // AT+URAT=<SelectedAcT>[,<PreferredAct>[,<2ndPreferredAct>]]

    sendAT(GF("+COPS=2"));  // Deregister from network
    if (waitResponse() != 1) { return false; }
    sendAT(GF("+URAT="), urat);  // Radio Access Technology (RAT) selection
    if (waitResponse() != 1) { return false; }
    sendAT(GF("+COPS=0"));  // Auto-register to the network
    if (waitResponse() != 1) { return false; }
    return restart();
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
  // This module supports SSL, but there are no certificate management functions
  // are supported yet.
  // If you wish to add certificate management for this module you must (in
  // addition to adding the functions here):
  //  - Add `#include "TinyGsmSSL.tpp` to the top of the file
  //  - Remove `#define TINY_GSM_MODEM_HAS_SSL` from the top of the file
  //  - Remove the #if 0 / #endif statements bracketing the SSL configuration
  //  structure.
  //  - Add `public TinyGsmSSL<TinyGsmSaraR4, TinyGsmSaraR4SslConfig>,` to the
  //  constructor's initializer list
  //  - Add `friend class TinyGsmSSL<TinyGsmSaraR4, TinyGsmSaraR4SslConfig>;` to
  //  the friend list
  //  - Make the secure client inherit from the secure client class in the SSL
  //  template.
  //  - Create an inner secure context class inheriting from
  //  TinyGsmSSLContext<TinyGsmSaraR4, TinyGsmSaraR4SslConfig>

  /*
   * Wifi functions
   */
  // No functions of this type supported

  /*
   * Cellular data functions
   */
 protected:
  bool dataConnectImpl(const char* apn, const char* user, const char* pwd) {
    // dataDisconnect();

    sendAT(GF("+CGATT=1"));  // attach to Packet Domain service
    if (waitResponse(360000L) != 1) { return false; }

    // Using CGDCONT sets up an "external" PCP context, i.e. a data connection
    // using the external IP stack (e.g. Windows dial up) and PPP link over the
    // serial interface.  This is the only command set supported by the LTE-M
    // and LTE NB-IoT modules (SARA-R4xx, SARA-N4xx)

    // Set the authentication
    if (user && strlen(user) > 0) {
      sendAT(GF("+CGAUTH=1,0,\""), user, GF("\",\""), pwd, '"');
      waitResponse();
    }

    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');  // Define PDP context 1
    waitResponse();

    sendAT(GF("+CGACT=1,1"));  // activate PDP profile/context 1
    if (waitResponse(150000L) != 1) { return false; }

    return true;
  }

  bool dataDisconnectImpl() {
    // Mark all the sockets as closed
    // This ensures that asynchronously closed sockets are marked closed
    for (uint8_t mux = 0; mux < TcpConfig::kMuxCount; mux++) {
      GsmClientSaraR4* sock = sockets[mux];
      if (sock && sock->sock_connected) { sock->sock_connected = false; }
    }

    // sendAT(GF("+CGACT=0,1"));  // Deactivate PDP context 1
    sendAT(GF("+CGACT=0"));  // Deactivate all contexts
    if (waitResponse(40000L) != 1) {
      // return false;
    }

    sendAT(GF("+CGATT=0"));  // Detach from Packet Domain service
    if (waitResponse(360000L) != 1) { return false; }

    return true;
  }

  /*
   * SIM card functions
   */
 protected:
  // This uses "CGSN" instead of "GSN"
  String getIMEIImpl() {
    sendAT(GF("+CGSN"));
    if (waitResponse(GFP(ModemConfig::GSM_NL)) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone call functions
   */
  // No functions of this type supported

  /*
   * Audio functions
   */
  // No functions of this type supported

  /*
   * Text messaging (SMS) functions
   */
 protected:
  String sendUSSDImpl(const String& code) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool   sendSMS_UTF16Impl(const String& number, const void* text,
                           size_t len) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * GSM/GPS/GNSS/GLONASS Location functions
   * NOTE:  u-blox modules use the same function to get location data from both
   * GSM tower triangulation and from dedicated GPS/GNSS/GLONASS receivers.  The
   * only difference in which sensor the data is requested from.  If a GNSS
   * location is requested from a modem without a GNSS receiver installed on the
   * I2C port, the GSM-based "Cell Locate" location will be returned instead.
   */
 protected:
  bool enableGPSImpl() {
    // AT+UGPS=<mode>[,<aid_mode>[,<GNSS_systems>]]
    // <mode> - 0: GNSS receiver powered off, 1: on
    // <aid_mode> - 0: no aiding (default)
    // <GNSS_systems> - 3: GPS + SBAS (default)
    sendAT(GF("+UGPS=1,0,3"));
    if (waitResponse(10000L, GF("+UGPS:")) != 1) { return false; }
    return waitResponse(10000L) == 1;
  }
  bool disableGPSImpl() {
    sendAT(GF("+UGPS=0"));
    if (waitResponse(10000L, GF("+UGPS:")) != 1) { return false; }
    return waitResponse(10000L) == 1;
  }
  String inline getUbloxLocationRaw(int8_t sensor) {
    // AT+ULOC=<mode>,<sensor>,<response_type>,<timeout>,<accuracy>
    // <mode> - 2: single shot position
    // <sensor> - 0: use the last fix in the internal database and stop the GNSS
    //          receiver
    //          - 1: use the GNSS receiver for localization
    //          - 2: use cellular CellLocate location information
    //          - 3: ?? use the combined GNSS receiver and CellLocate service
    //          information ?? - Docs show using sensor 3 and it's
    //          documented for the +UTIME command but not for +ULOC
    // <response_type> - 0: standard (single-hypothesis) response
    // <timeout> - Timeout period in seconds
    // <accuracy> - Target accuracy in meters (1 - 999999)
    sendAT(GF("+ULOC=2,"), sensor, GF(",0,120,1"));
    // wait for first "OK"
    if (waitResponse(10000L) != 1) { return ""; }
    // wait for the final result - wait full timeout time
    if (waitResponse(120000L, GF("+UULOC:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }
  String getGsmLocationRawImpl() {
    return getUbloxLocationRaw(2);
  }
  String getGPSrawImpl() {
    return getUbloxLocationRaw(1);
  }

  inline bool getUbloxLocation(int8_t sensor, float* lat, float* lon,
                               float* speed = 0, float* alt = 0, int* vsat = 0,
                               int* usat = 0, float* accuracy = 0,
                               int* year = 0, int* month = 0, int* day = 0,
                               int* hour = 0, int* minute = 0,
                               int* second = 0) {
    // AT+ULOC=<mode>,<sensor>,<response_type>,<timeout>,<accuracy>
    // <mode> - 2: single shot position
    // <sensor> - 2: use cellular CellLocate location information
    //          - 0: use the last fix in the internal database and stop the GNSS
    //          receiver
    //          - 1: use the GNSS receiver for localization
    //          - 3: ?? use the combined GNSS receiver and CellLocate service
    //          information ?? - Docs show using sensor 3 and it's documented
    //          for the +UTIME command but not for +ULOC
    // <response_type> - 0: standard (single-hypothesis) response
    // <timeout> - Timeout period in seconds
    // <accuracy> - Target accuracy in meters (1 - 999999)
    sendAT(GF("+ULOC=2,"), sensor, GF(",0,120,1"));
    // wait for first "OK"
    if (waitResponse(10000L) != 1) { return false; }
    // wait for the final result - wait full timeout time
    if (waitResponse(120000L, GF("+UULOC: ")) != 1) { return false; }

    // +UULOC: <date>, <time>, <lat>, <long>, <alt>, <uncertainty>, <speed>,
    // <direction>, <vertical_acc>, <sensor_used>, <SV_used>, <antenna_status>,
    // <jamming_status>

    // init variables
    float   ilat         = 0;
    float   ilon         = 0;
    float   ispeed       = 0;
    float   ialt         = 0;
    int16_t iusat        = 0;
    float   iaccuracy    = 0;
    int16_t iyear        = 0;
    int16_t imonth       = 0;
    int16_t iday         = 0;
    int16_t ihour        = 0;
    int16_t imin         = 0;
    float   secondWithSS = 0;

    // Date & Time
    iday         = streamGetIntBefore('/');    // Two digit day
    imonth       = streamGetIntBefore('/');    // Two digit month
    iyear        = streamGetIntBefore(',');    // Four digit year
    ihour        = streamGetIntBefore(':');    // Two digit hour
    imin         = streamGetIntBefore(':');    // Two digit minute
    secondWithSS = streamGetFloatBefore(',');  // 6 digit second with subseconds

    ilat = streamGetFloatBefore(',');  // Estimated latitude, in degrees
    ilon = streamGetFloatBefore(',');  // Estimated longitude, in degrees
    ialt = streamGetFloatBefore(
        ',');         // Estimated altitude, in meters - only forGNSS
                      // positioning, 0 in case of CellLocate
    if (ialt != 0) {  // values not returned for CellLocate
      iaccuracy =
          streamGetFloatBefore(',');       // Maximum possible error, in meters
      ispeed = streamGetFloatBefore(',');  // Speed over ground m/s3
      streamSkipUntil(',');  // Course over ground in degree (0 deg - 360 deg)
      streamSkipUntil(',');  // Vertical accuracy, in meters
      streamSkipUntil(',');  // Sensor used for the position calculation
      iusat = streamGetIntBefore(',');  // Number of satellite used
      streamSkipUntil(',');             // Antenna status
      streamSkipUntil('\n');            // Jamming status
    } else {
      iaccuracy =
          streamGetFloatBefore('\n');  // Maximum possible error, in meters
    }

    // Set pointers
    if (lat != nullptr) *lat = ilat;
    if (lon != nullptr) *lon = ilon;
    if (speed != nullptr) *speed = ispeed;
    if (alt != nullptr) *alt = ialt;
    if (vsat != nullptr)
      *vsat = 0;  // Number of satellites viewed not reported;
    if (usat != nullptr) *usat = iusat;
    if (accuracy != nullptr) *accuracy = iaccuracy;
    if (iyear < 2000) iyear += 2000;
    if (year != nullptr) *year = iyear;
    if (month != nullptr) *month = imonth;
    if (day != nullptr) *day = iday;
    if (hour != nullptr) *hour = ihour;
    if (minute != nullptr) *minute = imin;
    if (second != nullptr) *second = static_cast<int>(secondWithSS);

    // final ok
    waitResponse();
    return true;
  }
  bool getGsmLocationImpl(float* lat, float* lon, float* accuracy, int* year,
                          int* month, int* day, int* hour, int* minute,
                          int* second) {
    return getUbloxLocation(2, lat, lon, 0, 0, 0, 0, accuracy, year, month, day,
                            hour, minute, second);
  }
  bool getGPSImpl(float* lat, float* lon, float* speed, float* alt, int* vsat,
                  int* usat, float* accuracy, int* year, int* month, int* day,
                  int* hour, int* minute, int* second) {
    return getUbloxLocation(1, lat, lon, speed, alt, vsat, usat, accuracy, year,
                            month, day, hour, minute, second);
  }

  /*
   * Time functions
   */
  // Follows all clock functions as inherited from TinyGsmTime.tpp

  /*
   * NTP server functions
   */
  // No functions of this type supported

  /*
   * BLE functions
   */
  // No functions of this type supported

  /*
   * Battery functions
   */
 protected:
  int16_t getBattVoltageImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  int8_t getBattPercentImpl() {
    sendAT(GF("+CIND?"));
    if (waitResponse(GF("+CIND:")) != 1) { return 0; }

    int8_t res     = streamGetIntBefore(',');
    int8_t percent = res * 20;  // return is 0-5
    // Wait for final OK
    waitResponse();
    return percent;
  }

  int8_t getBattChargeStateImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    chargeState = 0;
    percent     = getBattPercent();
    milliVolts  = 0;
    return true;
  }

  /*
   * Temperature functions
   */
 protected:
  float getTemperatureImpl() {
    // First make sure the temperature is set to be in celsius
    sendAT(GF("+UTEMP=0"));  // Would use 1 for Fahrenheit
    if (waitResponse() != 1) { return static_cast<float>(-9999); }
    sendAT(GF("+UTEMP?"));
    if (waitResponse(GF("+UTEMP:")) != 1) { return static_cast<float>(-9999); }
    int16_t res  = streamGetIntBefore('\n');
    float   temp = -9999;
    if (res != -1) { temp = (static_cast<float>(res)) / 10; }
    return temp;
  }

  /*
   * Client-related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t* dynamicMux,
                    int timeout_s) {
    // Validate dynamicMux before accessing sockets array
    if (*dynamicMux >= TcpConfig::kMuxCount || !sockets[*dynamicMux]) {
      return false;
    }
    uint32_t timeout_ms  = ((uint32_t)timeout_s) * 1000;
    bool     ssl         = sockets[*dynamicMux]->is_secure;
    uint32_t startMillis = millis();

    // create a socket
    sendAT(GF("+USOCR=6"));
    // reply is +USOCR: ## of socket created
    if (waitResponse(GF("+USOCR:")) != 1) { return false; }
    int16_t connected_mux = streamGetIntBefore('\n');
    waitResponse();
    // Validate the returned mux
    if (connected_mux < 0 || connected_mux >= TcpConfig::kMuxCount) {
      DBG(GF("ERROR: Modem returned invalid mux"));
      *dynamicMux = static_cast<uint8_t>(-1);  // Set mux to invalid value
      return false;  // Return failure when mux is out of range
    }
    *dynamicMux = static_cast<uint8_t>(connected_mux);

    if (ssl) {
      // AT+USOSEC=<socket>,<ssl_tls_status>[,<usecmng_profile_id>]
      // Must be issued before the +USOCO command
      // support depends on firmware version
      // If no profile is set a default USECMNG profile is used
      sendAT(GF("+USOSEC="), *dynamicMux, ",1" /*, ',', sslCtxIndex*/);
      if (waitResponse() != 1) { return false; }
    }

    // Enable NODELAY
    // AT+USOSO=<socket>,<level>,<opt_name>,<opt_val>[,<opt_val2>]
    // <level> - 0 for IP, 6 for TCP, 65535 for socket level options
    // <opt_name> TCP/1 = no delay (do not delay send to coalesce packets)
    // NOTE:  Enabling this may increase data plan usage
    // sendAT(GF("+USOSO="), *mux, GF(",6,1,1"));
    // waitResponse();

    // Enable KEEPALIVE, 30 sec
    // sendAT(GF("+USOSO="), *mux, GF(",6,2,30000"));
    // waitResponse();

    // connect on the allocated socket

    // Use an asynchronous open to reduce the number of terminal freeze-ups
    // This is still blocking until the URC arrives
    // The SARA-R410M-02B with firmware revisions prior to L0.0.00.00.05.08
    // has a nasty habit of locking up when opening a socket, especially if
    // the cellular service is poor.
    // NOT supported on SARA-R404M / SARA-R410M-01B
    if (supportsAsyncSockets) {
      DBG("### Opening socket asynchronously!  Socket cannot be used yet; "
          "blocking until the URC '+UUSOCO' appears.");
      sendAT(GF("+USOCO="), *dynamicMux, GF(",\""), host, GF("\","), port,
             GF(",1"));
      uint32_t elapsed = millis() - startMillis;
      if (elapsed >= timeout_ms) { return false; }
      if (waitResponse(timeout_ms - elapsed, GF("+UUSOCO:")) == 1) {
        streamGetIntBefore(',');  // skip repeated mux
        int8_t connection_status = streamGetIntBefore('\n');
        DBG("### Waited", millis() - startMillis, "ms for socket to open");
        return (0 == connection_status);
      } else {
        DBG("### Waited", millis() - startMillis,
            "but never got socket open notice");
        return false;
      }
    } else {
      // use synchronous open
      sendAT(GF("+USOCO="), *dynamicMux, GF(",\""), host, GF("\","), port);
      uint32_t elapsed = millis() - startMillis;
      if (elapsed >= timeout_ms) { return false; }
      int8_t rsp = waitResponse(timeout_ms - elapsed);
      return (1 == rsp);
    }
  }

  bool modemStop(uint8_t mux, uint32_t maxWaitMs) {
    if (!isValidMux(mux)) { return false; }
    // We want to use an async socket close because the synchronous close of
    // an open socket is INCREDIBLY SLOW and the modem can freeze up.  But we
    // only attempt the async close if we already KNOW the socket is open
    // because calling the async close on a closed socket and then attempting
    // opening a new socket causes the board to lock up for 2-3 minutes and
    // then finally return with a "new" socket that is immediately closed.
    // Attempting to close a socket that is already closed with a synchronous
    // close quickly returns an error.
    if (supportsAsyncSockets && sockets[mux]->sock_connected) {
      DBG("### Closing socket asynchronously!  Socket might remain open "
          "until arrival of +UUSOCL:",
          mux);
      // faster asynchronous close
      // NOT supported on SARA-R404M / SARA-R410M-01B
      // Same command for both secure and non-secure sockets
      sendAT(GF("+USOCL="), mux, GF(",1"));
      // NOTE: Even though we're closing asynchronously and not waiting for the
      // close to complete, we let the stop function set the sock as
      // disconnected right away because it can no longer be used
      // NOTE: Even asynchronously, it can take up to 120s to get a response
    } else {
      // synchronous close
      // Same command for both secure and non-secure sockets
      sendAT(GF("+USOCL="), mux);
      // NOTE:  can take up to 120s to get a response
    }
    return waitResponse(maxWaitMs) == 1;
  }

  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+USOWR="), mux, ',', (uint16_t)len);
    if (waitResponse(GF("@")) != 1) { return 0; }
    // 50ms delay, see AT manual section 25.10.4
    delay(50);
    return true;
  }
  // Between the modemBeginSend and modemEndSend, modemSend calls:
  // stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  // stream.flush();
  size_t modemEndSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    if (waitResponse(GF("+USOWR:")) != 1) { return 0; }
    int16_t  ret_mux = streamGetIntBefore(',');   // check mux
    uint16_t sent    = streamGetIntBefore('\n');  // check send length
    bool     success = waitResponse() ==
        1;  // sends back OK after the confirmation of number sent
    if (sent != len) { DBG("### Sent:", sent, "of", len, "on", mux); }
    if (success && isExpectedMux(ret_mux, mux)) return sent;
    return 0;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    size_t len_read = 0;

    sendAT(GF("+USORD="), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+USORD:")) != 1) { return 0; }

    int16_t ret_mux      = streamGetIntBefore(',');  // mux
    int16_t len_reported = streamGetIntBefore(',');
    streamSkipUntil('\"');
    if (isValidMux(ret_mux)) {
      // Move the data to the socket buffer of the returned mux as long as the
      // returned mux is valid, even if it doesn't match the expected mux.
      len_read = moveCharsFromStreamToFifo(ret_mux, len_reported);
    }
    streamSkipUntil('\"');
    // ^^ if the returned mux is invalid, this will skip the entire line, if it
    // was valid this will only skip the closing quote
    waitResponse();

    if (isValidMux(ret_mux)) {
      // get the amount available after reading
      sockets[ret_mux]->sock_available = modemGetAvailable(ret_mux);
    }
    if (!isExpectedMux(ret_mux, mux)) {
      // if we didn't get a read from the expected mux, set the read length to 0
      // and update the available data for the mux that was requested
      len_read                     = 0;
      sockets[mux]->sock_available = modemGetAvailable(mux);
    }
    return len_read;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
    sendAT(GF("+USORD="), mux, ",0");
    size_t  result = 0;
    uint8_t res    = waitResponse(GF("+USORD:"));
    // Will give error "operation not allowed" when attempting to read a socket
    // that you have already told to close
    if (res == 1) {
      int16_t ret_mux = streamGetIntBefore(',');  // mux
      result          = streamGetIntBefore('\n');
      // if (result) DBG("### DATA AVAILABLE:", result, "on", mux);
      waitResponse();
      if (!isExpectedMux(ret_mux, mux)) { result = 0; }
    }
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    // DBG("### Available:", result, "on", mux);
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
    sendAT(GF("+USOCTL="), mux, ",10");
    uint8_t res = waitResponse(GF("+USOCTL:"));
    if (res != 1) { return false; }

    int16_t ret_mux = streamGetIntBefore(',');  // mux
    streamSkipUntil(',');                       // Skip type
    int8_t result = streamGetIntBefore('\n');
    // 0: the socket is in INACTIVE status (it corresponds to CLOSED status
    // defined in RFC793 "TCP Protocol Specification" [112])
    // 1: the socket is in LISTEN status
    // 2: the socket is in SYN_SENT status
    // 3: the socket is in SYN_RCVD status
    // 4: the socket is in ESTABILISHED status
    // 5: the socket is in FIN_WAIT_1 status
    // 6: the socket is in FIN_WAIT_2 status
    // 7: the sokcet is in CLOSE_WAIT status
    // 8: the socket is in CLOSING status
    // 9: the socket is in LAST_ACK status
    // 10: the socket is in TIME_WAIT status
    waitResponse();
    return (result != 0 && isExpectedMux(ret_mux, mux));
  }

  /*
   * Utilities
   */
 protected:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+UUSORD:"))) {
      int16_t mux = streamGetIntBefore(',');
      int16_t len = streamGetIntBefore('\n');
      if (isValidMux(mux)) {
        sockets[mux]->got_data = true;
        // max size is 1024
        if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
      }
      data = "";
      DBG("### URC Data Received:", len, "on", mux);
      return true;
    } else if (data.endsWith(GF("+UUSOCL:"))) {
      int16_t mux = streamGetIntBefore('\n');
      if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
      data = "";
      DBG("### URC Sock Closed: ", mux);
      return true;
    } else if (data.endsWith(GF("+UUSOCO:"))) {
      int16_t mux          = streamGetIntBefore(',');
      int16_t socket_error = streamGetIntBefore('\n');
      if (isValidMux(mux) && socket_error == 0) {
        sockets[mux]->sock_connected = true;
      }
      data = "";
      DBG("### URC Sock Opened: ", mux);
      return true;
    }
    return false;
  }

 public:
  /// Stream used to communicate with the modem.
  Stream& stream;

 protected:
  GsmClientSaraR4* sockets[TcpConfig::kMuxCount];
  bool             has2GFallback;
  bool             supportsAsyncSockets;
};

// cspell:words USOWR

#endif  // SRC_TINYGSMCLIENTSARAR4_H_
