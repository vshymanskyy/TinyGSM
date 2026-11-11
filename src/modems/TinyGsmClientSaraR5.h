/**
 * @file       TinyGsmClientSaraR5.h
 * @brief      u-blox SARA-R5 modem client and modem-trait definitions.
 * @author     Sebastian Bergner, Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Aug 2023
 */
/**
 * @defgroup ublox_sara_r5 u-blox SARA-R5
 * @ingroup group_ublox
 * @brief Manufacturer: u-blox. Models: SARA-R5.
 *
 * # Connection Information
 *
 * - Combined TCP/SSL sockets:
 *   - 7
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

#ifndef SRC_TINYGSMCLIENTSARAR5_H_
#define SRC_TINYGSMCLIENTSARAR5_H_
#pragma message("TinyGSM:  TinyGsmClientSaraR5")

#include "mixins/TinyGsmModem.tpp"
#include "mixins/TinyGsmTCP.tpp"

// NOTE: This module supports SSL, but we do not support any certificate
// management yet. Define TINY_GSM_MODEM_HAS_SSL here and do not include the SSL
// module.
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
/// @ingroup ublox_sara_r5
enum class SaraR5RegStatus {
  /// No registration result
  REG_NO_RESULT = -1,
  /// Not registered on the network
  REG_UNREGISTERED = 0,
  /// Searching for network
  REG_SEARCHING = 2,
  /// Registration denied
  REG_DENIED = 3,
  /// Registered on the home network
  REG_OK_HOME = 1,
  /// Registered on a roaming network
  REG_OK_ROAMING = 5,
  /// Unknown registration status
  REG_UNKNOWN = 4,
  /// Registered on the home network for SMS only
  REG_SMS_ONLY_HOME = 6,
  /// Registered on a roaming network for SMS only
  REG_SMS_ONLY_ROAMING = 7,
  /// ublox AT command manual states: attached for emergency bearer services
  /// only (see 3GPP TS 24.008 [85] and 3GPP TS 24.301 [120] that specify the
  /// condition when the MS is considered as attached for emergency bearer
  /// services)
  REG_EMERGENCY_ONLY = 8,
  /// not 100% certain, ublox AT command manual states: registered for "CSFB not
  /// preferred", home network (applicable only when AcTStatus indicates
  /// E-UTRAN)
  REG_NO_FALLBACK_LTE_HOME = 9,
  /// not 100% certain, ublox AT command manual states: registered for "CSFB not
  /// preferred", roaming (applicable only when AcTStatus indicates E-UTRAN)
  REG_NO_FALLBACK_LTE_ROAMING = 10
};

/// Basic modem configurations for the SARA-R5
/// @ingroup ublox_sara_r5
struct TinyGsmSaraR5ModemConfig
    : public TinyGsmModemConfigPreset<SaraR5RegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "u-blox";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SARA-R5";
};

constexpr char TinyGsmSaraR5ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmSaraR5ModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for the SARA-R5.
 *
 * Per the manual, 7 standard sockets can be managed.  I **think** all sockets
 * can be SSL, but the manual is not clear (to me).
 *
 * @warning You cannot control the socket mux number on this module! The module
 * opens the connection and returns the connection number it opened.
 *
 * The send data command, USOWR, accepts up to 1024 bytes in "normal" and
 * "binary extended" modes and up to 512 bytes in "HEX" mode.
 * @ingroup ublox_sara_r5
 */
struct TinyGsmSaraR5TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Dynamic,
          /*muxCount*/ 7,
          /*sendMaxSize*/ 1024,
          /*connectTimeoutS*/ 120> {};

#if 0
/**
 * @brief SSL behavior and limits for the u-blox SARA-R5.
 * The module supports 5 SSL contexts (called security profiles, numbered 0-4).
 * This library does not support selecting or modifying the security profiles in
 * any way.  The module will always use the default profile.
 * @ingroup ublox_sara_r5
 */
struct TinyGsmSaraR5SslConfig : public TinyGsmSslConfigPreset<
                                   /*contextCount*/ 5> {};
#endif

/// Class for the u-blox SARA-R5
/// @ingroup ublox_sara_r5
class TinyGsmSaraR5
    : public TinyGsmModem<TinyGsmSaraR5, TinyGsmSaraR5ModemConfig>,
      public TinyGsmCellularData<TinyGsmSaraR5>,
      public TinyGsmTCP<TinyGsmSaraR5, TinyGsmSaraR5TcpConfig>,
      public TinyGsmCalling<TinyGsmSaraR5>,
      public TinyGsmSMS<TinyGsmSaraR5>,
      public TinyGsmGSMLocation<TinyGsmSaraR5>,
      public TinyGsmGPS<TinyGsmSaraR5>,
      public TinyGsmTime<TinyGsmSaraR5>,
      public TinyGsmBattery<TinyGsmSaraR5> {
  friend class TinyGsmModem<TinyGsmSaraR5, TinyGsmSaraR5ModemConfig>;
  friend class TinyGsmCellularData<TinyGsmSaraR5>;
  friend class TinyGsmTCP<TinyGsmSaraR5, TinyGsmSaraR5TcpConfig>;
  friend class GsmClient<TinyGsmSaraR5, TinyGsmSaraR5TcpConfig>;
  friend class TinyGsmCalling<TinyGsmSaraR5>;
  friend class TinyGsmSMS<TinyGsmSaraR5>;
  friend class TinyGsmGSMLocation<TinyGsmSaraR5>;
  friend class TinyGsmGPS<TinyGsmSaraR5>;
  friend class TinyGsmTime<TinyGsmSaraR5>;
  friend class TinyGsmBattery<TinyGsmSaraR5>;

 public:
  using ModemConfig = TinyGsmSaraR5ModemConfig;
  using TcpConfig   = TinyGsmSaraR5TcpConfig;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup ublox_sara_r5
  class GsmClientSaraR5
      : public GsmClient<TinyGsmSaraR5, TinyGsmSaraR5TcpConfig> {
    friend class TinyGsmSaraR5;

   public:
    using GsmClient<TinyGsmSaraR5, TinyGsmSaraR5TcpConfig>::connect;
    using GsmClient<TinyGsmSaraR5, TinyGsmSaraR5TcpConfig>::stop;
    using TcpConfig = TinyGsmSaraR5TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSaraR5() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem.
     * @param modem Modem instance used by this client.
     *
     * @note The SARA-R5 does not allow you to specify the multiplexing channel.
     * The modem will automatically assign a channel when the client connects to
     * a server.  Use the getMux() function to get the assigned multiplexing
     * channel number after a successful connection.
     */
    explicit GsmClientSaraR5(TinyGsmSaraR5& modem, uint8_t /*mux*/ = 0)
        : GsmClient<TinyGsmSaraR5, TinyGsmSaraR5TcpConfig>(modem /*, mux*/) {
      is_secure = false;
      init(&modem);
    }

    /*
     * Arduino Client interface
     */
   public:
    /**
     * @copydoc GsmClient::connect(const char*, uint16_t, int)
     * @warning The SARA-R5 modem automatically assigns the multiplexing
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
  /// @ingroup ublox_sara_r5
  class GsmClientSecureSaraR5 : public GsmClientSaraR5 {
    friend class TinyGsmSaraR5;

   public:
    using GsmClientSaraR5::connect;
    using GsmClientSaraR5::stop;
    using TcpConfig = TinyGsmSaraR5TcpConfig;

    /*
     * Secure client constructors and initialization
     */
    /**
     * @brief Create a new secured TCP (SSL) client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSecureSaraR5() {
      is_secure = true;
    }
    /**
     * @brief Create a new secured TCP (SSL) client and bind it to a modem.
     * @copydetails GsmClientSaraR5::GsmClientSaraR5(TinyGsmSaraR5&, uint8_t)
     */
    explicit GsmClientSecureSaraR5(TinyGsmSaraR5& modem, uint8_t /*mux*/ = 0)
        : GsmClientSaraR5(modem) {
      is_secure = true;
    }
  };

  /*
   * Public Aliases
   */
 public:
  /// Typedef for backward compatibility
  using GsmClientSecureR5 = GsmClientSecureSaraR5;

  /// Arduino Client type for this modem.
  using TinyGsmClient = GsmClientSaraR5;
  /// Secure Arduino Client type for this modem.
  using TinyGsmClientSecure = GsmClientSecureSaraR5;

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmSaraR5(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSaraR5"));

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
  SaraR5RegStatus getRegistrationStatusImpl() {
    // Check first for EPS registration
    SaraR5RegStatus epsStatus =
        static_cast<SaraR5RegStatus>(getRegistrationStatusXREG("CEREG"));

    // If we're connected on EPS, great!
    if (epsStatus == SaraR5RegStatus::REG_OK_HOME ||
        epsStatus == SaraR5RegStatus::REG_OK_ROAMING) {
      return epsStatus;
    } else {
      // Otherwise, check generic network status
      return static_cast<SaraR5RegStatus>(getRegistrationStatusXREG("CREG"));
    }
  }

  bool isNetworkConnectedImpl() {
    SaraR5RegStatus s = this->getRegistrationStatus();
    if (s == SaraR5RegStatus::REG_OK_HOME ||
        s == SaraR5RegStatus::REG_OK_ROAMING ||
        s == SaraR5RegStatus::REG_SMS_ONLY_ROAMING ||
        s == SaraR5RegStatus::REG_SMS_ONLY_HOME)
      return true;
    else if (s == SaraR5RegStatus::REG_UNKNOWN)  // for some reason, it can hang
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
   * Possible values for selected and preferred are:
   * - 3: LTE
   * - 7: LTE Cat M1
   * - 8: LTE Cat NB1
   * - 9: GPRS / eGPRS
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
   * Possible values are:
   * - 3: LTE
   * - 7: LTE Cat M1
   * - 8: LTE Cat NB1
   * - 9: GPRS / eGPRS
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
  //  - Add `public TinyGsmSSL<TinyGsmSaraR5, TinyGsmSaraR5SslConfig>,` to the
  //  constructor's initializer list
  //  - Add `friend class TinyGsmSSL<TinyGsmSaraR5, TinyGsmSaraR5SslConfig>;` to
  //  the friend list
  //  - Make the secure client inherit from the secure client class in the SSL
  //  template.
  //  - Create an inner secure context class inheriting from
  //  TinyGsmSSLContext<TinyGsmSaraR5, TinyGsmSaraR5SslConfig>

  /*
   * Wifi functions
   */
  // No functions of this type supported

  /*
   * Cellular data functions
   */
 protected:
  bool dataConnectImpl(const char* apn, const char*, const char*) {
    sendAT(GF("+CGATT=1"));  // attach to Packet Domain service
    if (waitResponse(360000L) != 1) { return false; }

    // Setting up the PSD profile/PDP context with the UPSD commands sets up an
    // "internal" PDP context, i.e. a data connection using the internal IP
    // stack and related AT commands for sockets.

    // Packet switched data configuration
    // AT+UPSD=<profile_id>,<param_tag>,<param_val>
    // profile_id = 0 - PSD profile identifier, in range 0-6 (NOT PDP context)
    // param_tag = 1: APN
    // param_tag = 2: username  -> not working for SARA-R5
    // param_tag = 3: password  -> not working for SARA-R5
    // param_tag = 7: IP address Note: IP address set as "0.0.0.0" means
    //    dynamic IP address assigned during PDP context activation


    // check all available PDP context identifiers
    sendAT(GF("+CGDCONT?"));
    // Example Responses:
    // +CGDCONT: 0,"IP","payandgo.o2.co.uk","0.0.0.0",0,0,0,0,0,0,0,0,0,0
    // +CGDCONT:
    // 1,"IP","payandgo.o2.co.uk.mnc010.mcc234.gprs","10.160.182.234",0,0,0,2,0,0,0,0,0,0

    // While we're still getting responses starting with +CGDCONT:, keep reading
    // the lines and trying to match the APN. Continue through the final result
    // code after finding a match so the response is completely consumed.
    int    rcid = -1;
    char   strApn[128];
    bool   found = false;
    int8_t response;
    do {
      response = waitResponse(1000L, GF("+CGDCONT:"), GFP(ModemConfig::GSM_OK),
                              GFP(ModemConfig::GSM_ERROR));
      if (response != 1) { break; }

      int parsedCid = streamGetIntBefore(',');
      streamSkipUntil('\"');  // skip to the opening quote
      char strPdpType[10];
      // read the PDP type
      stream.readBytesUntil('\"', strPdpType, sizeof(strPdpType));
      streamSkipUntil('\"');  // skip to the next opening quote
      // read the APN
      size_t len  = stream.readBytesUntil('\"', strApn, sizeof(strApn) - 1);
      strApn[len] = '\0';
      streamSkipUntil('\n');  // throw away the rest of the line
      if (!found && parsedCid >= 0 && !strcmp(strApn, apn)) {
        rcid  = parsedCid;
        found = true;
      }
    } while (response == 1);

    // The query must end successfully and return a matching PDP context.
    if (response != 2 || !found) { return false; }

    sendAT(GF(
        "+UPSDA=0,4"));  // Deactivate the PDP context associated with profile 0
    waitResponse(360000L);  // Can return an error if previously not activated

    sendAT(GF("+UPSD=0,100,"),
           rcid);  // Deactivate the PDP context associated with profile 0
    waitResponse();

    sendAT(GF(
        "+UPSDA=0,3"));  // Activate the PDP context associated with profile 0
    if (waitResponse(360000L) != 1) { return false; }

    sendAT(GF("+UPSD=0,0,2"));  // Set protocol type to IPv4v6 with IPv4
                                // preferred for internal sockets
    waitResponse();

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
    DBG(GF("GPS on SARA-R5 cannot be enabled or disabled"));
    return true;
  }
  bool disableGPSImpl() {
    DBG(GF("GPS on SARA-R5 cannot be enabled or disabled"));
    return true;
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
    // waitResponse(10000L) ;
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
    // <response_type> - 0: standard (single-hypothesis) response  -> +UULOC:
    // <date>,<time>,<lat>,<long>,<alt>,<uncertainty>
    //                   1: detailed (single-hypothesis) response  -> +UULOC:
    //                   <date>,<time>,<lat>,<long>,<alt>,<uncertainty>,<speed>,<direction>,<vertical_acc>,<sensor_used>,<SV_used>,<antenna_status>,<jamming_status>
    // <timeout> - Timeout period in seconds
    // <accuracy> - Target accuracy in meters (1 - 999999)
    sendAT(GF("+ULOC=2,"), sensor, GF(",1,120,1"));
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

  bool modemStop(uint8_t mux, uint32_t maxWaitMs, bool newlyAllocated = false) {
    if (mux >= TcpConfig::kMuxCount || (!newlyAllocated && !isValidMux(mux))) {
      return false;
    }
    // Same command for both secure and non-secure sockets
    sendAT(GF("+USOCL="), mux);
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
  GsmClientSaraR5* sockets[TcpConfig::kMuxCount];
};

// cspell:words USOWR CSFB UTRAN

#endif  // SRC_TINYGSMCLIENTSARAR5_H_
