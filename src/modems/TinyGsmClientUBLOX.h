/**
 * @file       TinyGsmClientUBLOX.h
 * @brief      u-blox modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup ublox_cellular u-blox 2G and 3G Cellular Modules
 * @ingroup group_ublox
 * @brief Manufacturer: u-blox. Models: 2G and 3G cellular modules, including
 * LARA, LEON, LISA, MPCI, SARA-G, SARA-U, and TOBY modules.
 *
 * # Connection Information
 *
 * - Combined TCP/SSL sockets:
 *   - 7
 *   - a few u-blox modules support more, but this library does not
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

#ifndef SRC_TINYGSMCLIENTUBLOX_H_
#define SRC_TINYGSMCLIENTUBLOX_H_
#pragma message("TinyGSM:  TinyGsmClientUBLOX")

#include "mixins/TinyGsmModem.tpp"
#include "mixins/TinyGsmTCP.tpp"

// NOTE: This module supports SSL, but we do not support any certificate
// management yet. We define TINY_GSM_MODEM_HAS_SSL here and do not include the
// SSL module.
#ifndef TINY_GSM_MODEM_HAS_SSL
/// flag to indicate that the modem has Secure Socket Layer (SSL) functions
#define TINY_GSM_MODEM_HAS_SSL
#endif

#include "mixins/TinyGsmCellularData.tpp"
#include "mixins/TinyGsmCalling.tpp"
#include "mixins/TinyGsmSMS.tpp"
#include "mixins/TinyGsmGSMLocation.tpp"
#include "mixins/TinyGsmGPS.tpp"
#include "mixins/TinyGsmTime.tpp"
#include "mixins/TinyGsmBattery.tpp"

/// Registration status
/// @ingroup ublox_cellular
enum class UBLOXRegStatus {
  REG_NO_RESULT    = -1,  ///< No registration result
  REG_UNREGISTERED = 0,   ///< Not registered on the network
  REG_SEARCHING    = 2,   ///< Searching for network
  REG_DENIED       = 3,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered on the home network
  REG_OK_ROAMING   = 5,   ///< Registered on a roaming network
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/// Basic modem configurations for the UBLOX modem family
/// @ingroup ublox_cellular
struct TinyGsmUBLOXModemConfig
    : public TinyGsmModemConfigPreset<UBLOXRegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "u-blox";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "unknown";
};

constexpr char TinyGsmUBLOXModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmUBLOXModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for the u-blox family.
 *
 * Most modules using this protocol support 7 sockets, the LEON-G1 supports 16.
 * I **think** all sockets can be SSL, but the manual is not clear (to me).
 *
 * @warning You cannot control the socket mux number on this module! The module
 * opens the connection and returns the connection number it opened.
 *
 * The send data command, USOWR, accepts up to 1024 bytes in "normal" and
 * "binary extended" modes and up to 512 bytes in "HEX" mode.
 * @ingroup ublox_cellular
 */
struct TinyGsmUBLOXTcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Dynamic,
          /*muxCount*/ 7,
          /*sendMaxSize*/ 1024,
          /*connectTimeoutS*/ 120> {};

#if 0
/**
 * @brief SSL behavior and limits for the u-blox family
 * The module supports 5 SSL contexts (called security profiles, numbered 0-4).
 * This library does not support selecting or modifying the security profiles in
 * any way.  The module will always use the default profile.
 * @ingroup ublox_cellular
 */
struct TinyGsmUBLOXSslConfig : public TinyGsmSslConfigPreset<
                                  /*contextCount*/ 5> {};
#endif

/// Class for the u-blox family of modems
/// @ingroup ublox_cellular
class TinyGsmUBLOX : public TinyGsmModem<TinyGsmUBLOX, TinyGsmUBLOXModemConfig>,
                     public TinyGsmCellularData<TinyGsmUBLOX>,
                     public TinyGsmTCP<TinyGsmUBLOX, TinyGsmUBLOXTcpConfig>,
                     public TinyGsmCalling<TinyGsmUBLOX>,
                     public TinyGsmSMS<TinyGsmUBLOX>,
                     public TinyGsmGSMLocation<TinyGsmUBLOX>,
                     public TinyGsmGPS<TinyGsmUBLOX>,
                     public TinyGsmTime<TinyGsmUBLOX>,
                     public TinyGsmBattery<TinyGsmUBLOX> {
  friend class TinyGsmModem<TinyGsmUBLOX, TinyGsmUBLOXModemConfig>;
  friend class TinyGsmCellularData<TinyGsmUBLOX>;
  friend class TinyGsmTCP<TinyGsmUBLOX, TinyGsmUBLOXTcpConfig>;
  friend class GsmClient<TinyGsmUBLOX, TinyGsmUBLOXTcpConfig>;
  friend class TinyGsmCalling<TinyGsmUBLOX>;
  friend class TinyGsmSMS<TinyGsmUBLOX>;
  friend class TinyGsmGSMLocation<TinyGsmUBLOX>;
  friend class TinyGsmGPS<TinyGsmUBLOX>;
  friend class TinyGsmTime<TinyGsmUBLOX>;
  friend class TinyGsmBattery<TinyGsmUBLOX>;

 public:
  using ModemConfig = TinyGsmUBLOXModemConfig;
  using TcpConfig   = TinyGsmUBLOXTcpConfig;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup ublox_cellular
  class GsmClientUBLOX : public GsmClient<TinyGsmUBLOX, TinyGsmUBLOXTcpConfig> {
    friend class TinyGsmUBLOX;

   public:
    using GsmClient<TinyGsmUBLOX, TinyGsmUBLOXTcpConfig>::connect;
    using GsmClient<TinyGsmUBLOX, TinyGsmUBLOXTcpConfig>::stop;
    using TcpConfig = TinyGsmUBLOXTcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientUBLOX() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem.
     * @param modem Modem instance used by this client.
     *
     * @note The u-blox modules do not allow you to specify the multiplexing
     * channel.  The modem will automatically assign a channel when the client
     * connects to a server.  Use the getMux() function to get the assigned
     * multiplexing channel number after a successful connection.
     */
    explicit GsmClientUBLOX(TinyGsmUBLOX& modem, uint8_t /*mux*/ = 0)
        : GsmClient<TinyGsmUBLOX, TinyGsmUBLOXTcpConfig>(modem /*, mux*/) {
      is_secure = false;
      init(&modem);
    }

    /*
     * Arduino Client interface
     */
   public:
    /**
     * @copydoc GsmClient::connect(const char*, uint16_t, int)
     * @warning The u-blox modems automatically assign the multiplexing
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
      // Start with an invalid mux so cleanup only closes a newly allocated
      // socket.
      uint8_t assignedMux = static_cast<uint8_t>(-1);
      // modemConnect will validate the mux number returned by the modem.
      sock_connected = at->modemConnect(host, port, &assignedMux, timeout_s,
                                        is_secure);
      // stop the socket if the connection failed or we could not register the
      // assigned mux
      if (!sock_connected && assignedMux < TcpConfig::kMuxCount) {
        at->modemStop(assignedMux, TcpConfig::kStopTimeoutS * 1000L, true);
      } else if (sock_connected &&
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
  /// @ingroup ublox_cellular
  class GsmClientSecureUBLOX : public GsmClientUBLOX {
    friend class TinyGsmUBLOX;

   public:
    using GsmClientUBLOX::connect;
    using GsmClientUBLOX::stop;
    using TcpConfig = TinyGsmUBLOXTcpConfig;

    /*
     * Secure client constructors and initialization
     */
    /**
     * @brief Create a new secured TCP (SSL) client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSecureUBLOX() {
      is_secure = true;
    }
    /**
     * @brief Create a new secured TCP (SSL) client and bind it to a modem.
     * @copydetails GsmClientUBLOX::GsmClientUBLOX(TinyGsmUBLOX&, uint8_t)
     */
    explicit GsmClientSecureUBLOX(TinyGsmUBLOX& modem, uint8_t /*mux*/ = 0)
        : GsmClientUBLOX(modem) {
      is_secure = true;
    }
  };

  /*
   * Public Aliases
   */
 public:
  /// Arduino Client type for this modem.
  using TinyGsmClient = GsmClientUBLOX;
  /// Secure Arduino Client type for this modem.
  using TinyGsmClientSecure = GsmClientSecureUBLOX;

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmUBLOX(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientUBLOX"));

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

    // Enable automatic time zome update
    sendAT(GF("+CTZU=1"));
    waitResponse(10000L);
    // Ignore the response, in case the network doesn't support it.
    // if (waitResponse(10000L) != 1) { return false; }

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
    if (name.startsWith("u-blox SARA-R4") ||
        name.startsWith("u-blox SARA-N4")) {
      DBG("### WARNING:  You are using the wrong TinyGSM modem!");
    } else if (name.startsWith("u-blox SARA-N2")) {
      DBG("### SARA N2 NB-IoT modems not supported!");
    }
#endif
    return name;
  }

  bool factoryDefaultImpl() {
    sendAT(GF("+UFACTORY=0,1"));  // No factory restore, erase NVM
    waitResponse();
    return setPhoneFunctionality(16);  // Reset
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin) {
    if (!testAT()) { return false; }
    if (!setPhoneFunctionality(16)) { return false; }
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
  UBLOXRegStatus getRegistrationStatusImpl() {
    return static_cast<UBLOXRegStatus>(getRegistrationStatusXREG("CGREG"));
  }

  bool isNetworkConnectedImpl() {
    UBLOXRegStatus s = this->getRegistrationStatus();
    if (s == UBLOXRegStatus::REG_OK_HOME || s == UBLOXRegStatus::REG_OK_ROAMING)
      return true;
    else if (s == UBLOXRegStatus::REG_UNKNOWN)  // for some reason, it can hang
                                                // at unknown..
      return isDataConnected();
    else
      return false;
  }

  String getLocalIPImpl() {
    sendAT(GF("+UPSND=0,0"));
    if (waitResponse(GF("+UPSND:")) != 1) { return ""; }
    streamSkipUntil(',');   // Skip PSD profile
    streamSkipUntil('\"');  // Skip request type
    String res = stream.readStringUntil('\"');
    if (waitResponse() != 1) { return ""; }
    return res;
  }

  /*
   * Network mode / type / technology functions
   */
 public:
  /**
   * @brief Set the radio access technology (RAT) for the modem.
   *
   * Possible values for selected are:
   * - 0: GSM / GPRS / eGPRS (single mode)
   * - 1: GSM / UMTS (dual mode)
   * - 2: UMTS (single mode)
   * - 3: LTE (single mode)
   * - 4: GSM / UMTS / LTE (tri mode)
   * - 5: GSM / LTE (dual mode)
   * - 6: UMTS / LTE (dual mode)
   *
   * Possible values for preferred are:
   * - 0: GSM / GPRS / eGPRS
   * - 2: UTRAN
   * - 3: LTE
   *
   * @param selected The selected RAT mode.
   * @param preferred The preferred RAT mode.
   * @return True if the command was successful, false otherwise.
   */
  bool setRadioAccessTechnology(uint8_t selected, uint8_t preferred) {
    sendAT(GF("+URAT="), selected, ',', preferred);
    if (waitResponse() != 1) { return false; }
    return true;
  }

  /**
   * @brief Get the current radio access technology (RAT) of the modem.
   * @return The current RAT mode, or -1 if the command or parsing failed.
   */
  int8_t getCurrentRadioAccessTechnology() {
    sendAT(GF("+URAT?"));
    if (waitResponse(10000L, GF("+URAT:")) != 1) { return -1; }

    int16_t parsedRat = streamGetIntBefore('\n');
    if (waitResponse() != 1 || parsedRat == -9999) { return -1; }

    return static_cast<int8_t>(parsedRat);
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
  // This module supports SSL, but no certificate management functions are
  // supported yet.
  // If you wish to add certificate management for this module you must (in
  // addition to adding the functions here):
  //  - Add `#include "mixins/TinyGsmSSL.tpp` to the top of the file
  //  - Remove `#define TINY_GSM_MODEM_HAS_SSL` from the top of the file
  //  - Remove the #if 0 / #endif statements bracketing the SSL configuration
  //  structure.
  //  - Add `public TinyGsmSSL<TinyGsmUBLOX, TinyGsmUBLOXSslConfig>,` to the
  //  constructor's initializer list
  //  - Add `friend class TinyGsmSSL<TinyGsmUBLOX, TinyGsmUBLOXSslConfig>;` to
  //  the friend list
  //  - Make the secure client inherit from the secure client class in the SSL
  //  template.
  //  - Create an inner secure context class inheriting from
  //  TinyGsmSSLContext<TinyGsmUBLOX, TinyGsmUBLOXSslConfig>

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

    // Setting up the PSD profile/PDP context with the UPSD commands sets up an
    // "internal" PDP context, i.e. a data connection using the internal IP
    // stack and related AT commands for sockets.

    // Packet switched data configuration
    // AT+UPSD=<profile_id>,<param_tag>,<param_val>
    // profile_id = 0 - PSD profile identifier, in range 0-6 (NOT PDP context)
    // param_tag = 1: APN
    // param_tag = 2: username
    // param_tag = 3: password
    // param_tag = 7: IP address Note: IP address set as "0.0.0.0" means
    //    dynamic IP address assigned during PDP context activation
    sendAT(GF("+UPSD=0,1,\""), apn, '"');  // Set APN for PSD profile 0
    waitResponse();

    if (user && strlen(user) > 0) {
      sendAT(GF("+UPSD=0,2,\""), user, '"');  // Set user for PSD profile 0
      waitResponse();
    }
    if (pwd && strlen(pwd) > 0) {
      sendAT(GF("+UPSD=0,3,\""), pwd, '"');  // Set password for PSD profile 0
      waitResponse();
    }

    sendAT(GF("+UPSD=0,7,\"0.0.0.0\""));  // Dynamic IP on PSD profile 0
    waitResponse();

    // Packet switched data action
    // AT+UPSDA=<profile_id>,<action>
    // profile_id = 0: PSD profile identifier, in range 0-6 (NOT PDP context)
    // action = 3: activate; it activates a PDP context with the specified
    // profile, using the current parameters
    sendAT(GF(
        "+UPSDA=0,3"));  // Activate the PDP context associated with profile 0
    if (waitResponse(360000L) != 1) {  // Should return ok
      return false;
    }

    // Packet switched network-assigned data - Returns the current (dynamic)
    // network-assigned or network-negotiated value of the specified parameter
    // for the active PDP context associated with the specified PSD profile.
    // AT+UPSND=<profile_id>,<param_tag>
    // profile_id = 0: PSD profile identifier, in range 0-6 (NOT PDP context)
    // param_tag = 8: PSD profile status: if the profile is active the return
    // value is 1, 0 otherwise
    sendAT(GF("+UPSND=0,8"));  // Check if PSD profile 0 is now active
    int8_t res = waitResponse(GF(",8,1"), GF(",8,0"));
    waitResponse();  // Should return another OK
    if (res == 1) {
      return true;          // It's now active
    } else if (res == 2) {  // If it's not active yet, wait for the +UUPSDA URC
      if (waitResponse(180000L, GF("+UUPSDA: 0")) != 1) {  // 0=successful
        return false;
      }
      streamSkipUntil('\n');  // Ignore the IP address, if returned
    } else {
      return false;
    }

    return true;
  }

  bool dataDisconnectImpl() {
    sendAT(GF(
        "+UPSDA=0,4"));  // Deactivate the PDP context associated with profile 0
    if (waitResponse(360000L) != 1) { return false; }

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
  // Follows all Phone call functions as inherited from TinyGsmCalling.tpp

  /*
   * Audio functions
   */
  // No functions of this type supported

  /*
   * Text messaging (SMS) functions
   */
  // Follows all text messaging (SMS) functions as inherited from TinyGsmSMS.tpp

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

#if 0
    DBG(GF("Latitude:"), String(ilat, 8), GF("\tLongitude:"), String(ilon, 8),
        GF("\tAltitude:"), String(ialt, 4));
    DBG(GF("VSAT:"), 0, GF("\tUSAT:"), iusat, GF("\tAccuracy:"), iaccuracy);
    DBG(GF("Year:"), iyear, GF("\tMonth:"), imonth, GF("\tDay:"), iday);
    DBG(GF("Hour:"), ihour, GF("\tMinute:"), imin, GF("\tSecond:"),
        String(secondWithSS, 3));
#endif

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
    // get indication states related to network status, battery information and
    // so on
    sendAT(GF("+CIND?"));
    // Response:
    // +CIND:battchg,signal,service,sounder,message,call,roam,smsfull,gprs,callsetup,callheld,simind
    // OK
    // battchg = battery charge level, 0-5 (always 0 if charging isnt supported
    // signal:
    // - 0: < -105 dBm
    // - 1: < -93 dBm
    // - 2: < -81 dBm
    // - 3: < -69 dBm
    // - 4: < - 57 dBm
    // - 5: >= -57 dBm
    // see the manual for the rest of the "indications"
    if (waitResponse(GF("+CIND:")) != 1) { return -99; }

    int16_t res = streamGetIntBefore(',');  // return is 0-5
    // Wait for final OK
    bool validResponse = waitResponse() == 1;
    if (!validResponse || res < 0 || res > 5) { return -99; }
    return static_cast<int8_t>(res * 20);
  }

  int8_t getBattChargeStateImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    // the only battery value we can get is percent, so defer to
    // getBattPercentImpl() and set the others to default invalid values
    chargeState = -99;
    percent     = getBattPercentImpl();
    milliVolts  = -9999;
    return percent != -99;
  }

  /*
   * Temperature functions
   */
  // This would only available for a small number of modules in this group
  // (TOBY-L)
  float getTemperatureImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Client-related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t* dynamicMux,
                    int timeout_s, bool isSecure) {
    uint32_t timeout_ms  = ((uint32_t)timeout_s) * 1000;
    bool     ssl         = isSecure;
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
      // If no profile is set a default USECMNG profile is used
      sendAT(GF("+USOSEC="), *dynamicMux, ",1");
      waitResponse();
    }

    // Enable NODELAY
    // AT+USOSO=<socket>,<level>,<opt_name>,<opt_val>[,<opt_val2>]
    // <level> - 0 for IP, 6 for TCP, 65535 for socket level options
    // <opt_name> TCP/1 = no delay (do not delay send to coalesce packets)
    // NOTE:  Enabling this may increase data plan usage
    // sendAT(GF("+USOSO="), *dynamicMux, GF(",6,1,1"));
    // waitResponse();

    // Enable KEEPALIVE, 30 sec
    // sendAT(GF("+USOSO="), *dynamicMux, GF(",6,2,30000"));
    // waitResponse();

    // connect on the allocated socket
    sendAT(GF("+USOCO="), *dynamicMux, GF(",\""), host, GF("\","), port);
    uint32_t elapsed = millis() - startMillis;
    if (elapsed >= timeout_ms) { return false; }
    int8_t rsp = waitResponse(timeout_ms - elapsed);
    return (1 == rsp);
  }

  bool modemStop(uint8_t mux, uint32_t /*maxWaitMs*/,
                 bool    newlyAllocated = false) {
    if (mux >= TcpConfig::kMuxCount || (!newlyAllocated && !isValidMux(mux))) {
      return false;
    }
    // Same command for both secure and non-secure sockets
    sendAT(GF("+USOCL="), mux);
    return waitResponse() == 1;  // should return within 1s
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
    if (isExpectedMux(ret_mux, mux) && success) return sent;
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
    // DBG("### AvailablE:", result, "on", mux);
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
    return (isExpectedMux(ret_mux, mux) && result != 0);
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
      // DBG("### URC Data Received:", len, "on", mux);
      return true;
    } else if (data.endsWith(GF("+UUSOCL:"))) {
      int16_t mux = streamGetIntBefore('\n');
      if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
      data = "";
      DBG("### URC Sock Closed: ", mux);
      return true;
    }
    return false;
  }

 public:
  /// Stream used to communicate with the modem.
  Stream& stream;

 protected:
  GsmClientUBLOX* sockets[TcpConfig::kMuxCount];
};
// cspell:words USOWR

#endif  // SRC_TINYGSMCLIENTUBLOX_H_
