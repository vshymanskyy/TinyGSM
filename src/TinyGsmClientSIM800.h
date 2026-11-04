/**
 * @file       TinyGsmClientSIM800.h
 * @brief      SIM800-family modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup simcom_sim800 SIMCom SIM800, SIM900 and variants
 * @ingroup group_simcom
 * @brief Manufacturer: SIMCom. Models: SIM800 series, SIM900.
 *
 * # Connection Information
 *
 * - TCP sockets:
 *   - 8
 * - SSL sockets:
 *   - 5
 * - Socket Buffering:
 *   - The modem has an internal buffer for incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 */

#ifndef SRC_TINYGSMCLIENTSIM800_H_
#define SRC_TINYGSMCLIENTSIM800_H_
#pragma message("TinyGSM:  TinyGsmClientSIM800")

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"

// NOTE: This module supports SSL, but we do not support any certificate
// management yet. Define TINY_GSM_MODEM_HAS_SSL here and do no include the SSL
// module.
// See the example examples/more/SIM800_SslSetCert/SIM800_SslSetCert.ino for an
// example of how to manage the certificates
#if !defined(TINY_GSM_MODEM_SIM900)
#ifndef TINY_GSM_MODEM_HAS_SSL
/// flag to indicate that the modem has Secure Socket Layer (SSL) functions
#define TINY_GSM_MODEM_HAS_SSL
#endif
#endif

#include "TinyGsmCellularData.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"

/// Registration status
/// @ingroup simcom_sim800
enum class Sim800RegStatus {
  REG_NO_RESULT    = -1,  ///< No registration result
  REG_UNREGISTERED = 0,   ///< Not registered on the network
  REG_SEARCHING    = 2,   ///< Searching for network
  REG_DENIED       = 3,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered on the home network
  REG_OK_ROAMING   = 5,   ///< Registered on a roaming network
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/// Basic modem configurations for SIMCom SIM800 variants
/// @ingroup simcom_sim800
struct TinyGsmSim800ModemConfig
    : public TinyGsmModemConfigPreset<Sim800RegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "SIMCom";
#if defined(TINY_GSM_MODEM_SIM808)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM808";
#elif defined(TINY_GSM_MODEM_SIM868)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM868";
#elif defined(TINY_GSM_MODEM_SIM900)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM900";
#else
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM800";
#endif
};

constexpr char TinyGsmSim800ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmSim800ModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for SIMCom SIM800 variants.
 * @ingroup simcom_sim800
 *
 * @todo Handle the different number of sockets for TCP and SSL in the code.
 */
struct TinyGsmSim800TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 8> {};

#if 0
/**
 * @brief SSL behavior and limits for SIMCom SIM800 variants.
 * This library does not support selecting or modifying the SSL contexts in any
 * way.  The module will always use the default context.
 * @ingroup simcom_sim800
 */
struct TinyGsmSim800SslConfig : public TinyGsmSslConfigPreset<
                                   /*contextCount*/ ?> {};
#endif

/// Class for the SIMCOM SIM800 and SIM900 (with some limitations)
/// @ingroup simcom_sim800
class TinyGsmSim800
    : public TinyGsmModem<TinyGsmSim800, TinyGsmSim800ModemConfig>,
      public TinyGsmCellularData<TinyGsmSim800>,
      public TinyGsmTCP<TinyGsmSim800, TinyGsmSim800TcpConfig>,
      public TinyGsmCalling<TinyGsmSim800>,
      public TinyGsmSMS<TinyGsmSim800>,
      public TinyGsmGSMLocation<TinyGsmSim800>,
      public TinyGsmTime<TinyGsmSim800>,
      public TinyGsmNTP<TinyGsmSim800>,
      public TinyGsmBattery<TinyGsmSim800> {
  friend class TinyGsmModem<TinyGsmSim800, TinyGsmSim800ModemConfig>;
  friend class TinyGsmCellularData<TinyGsmSim800>;
  friend class TinyGsmTCP<TinyGsmSim800, TinyGsmSim800TcpConfig>;
  friend class GsmClient<TinyGsmSim800, TinyGsmSim800TcpConfig>;
  friend class TinyGsmCalling<TinyGsmSim800>;
  friend class TinyGsmSMS<TinyGsmSim800>;
  friend class TinyGsmGSMLocation<TinyGsmSim800>;
  friend class TinyGsmTime<TinyGsmSim800>;
  friend class TinyGsmNTP<TinyGsmSim800>;
  friend class TinyGsmBattery<TinyGsmSim800>;

 public:
  using ModemConfig = TinyGsmSim800ModemConfig;
  using TcpConfig   = TinyGsmSim800TcpConfig;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup simcom_sim800
  class GsmClientSim800
      : public GsmClient<TinyGsmSim800, TinyGsmSim800TcpConfig> {
    friend class TinyGsmSim800;

   public:
    using GsmClient<TinyGsmSim800, TinyGsmSim800TcpConfig>::connect;
    using GsmClient<TinyGsmSim800, TinyGsmSim800TcpConfig>::stop;
    using TcpConfig = TinyGsmSim800TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSim800() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is identical to the identifier
     * the modem uses internally to identify the socket for the SIM800.
     *
     * @note The SIM800 variants allow you choose the multiplexing channel
     * number, but if the input mux channel number is already in use and other
     * mux channels are available, this library will select the next available
     * one.  Use the getMux() function to get the assigned multiplexing channel
     * number after a successful connection.
     */
    explicit GsmClientSim800(TinyGsmSim800& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmSim800, TinyGsmSim800TcpConfig>(modem, mux) {
      is_secure = false;
      init(&modem, mux);
    }

    /**
     * @brief Initialize the TCP client with a modem and optionally a
     * multiplexing channel.
     * @return true if initialization was successful, false otherwise.
     * @copydetails GsmClientSim800::GsmClientSim800(TinyGsmSim800&, uint8_t)
     */
    bool init(TinyGsmSim800* modem, uint8_t mux = 0) override {
      return this->attachToModem(this, modem, mux);
    }

    /*
     * Arduino Client interface
     */
   public:
    TINY_GSM_STATIC_TCP_CONNECT


    /*
     * Extended Client API
     */
    // No extra extended API functions
  };

#if !defined(TINY_GSM_MODEM_SIM900)
  /*
   * Inner Secure Client
   */
 public:
  /// Inner secure client
  /// @ingroup simcom_sim800
  class GsmClientSecureSim800 : public GsmClientSim800 {
    friend class TinyGsmSim800;

   public:
    using GsmClientSim800::connect;
    using GsmClientSim800::stop;
    using TcpConfig = TinyGsmSim800TcpConfig;

    /*
     * Secure client constructors and initialization
     */
    /**
     * @brief Create a new secured TCP (SSL) client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSecureSim800() {
      is_secure = true;
    }
    /**
     * @brief Create a new secured TCP (SSL) client and bind it to a modem and
     * optionally a multiplexing channel.
     * @copydetails GsmClientSim800::GsmClientSim800(TinyGsmSim800&, uint8_t)
     */
    explicit GsmClientSecureSim800(TinyGsmSim800& modem, uint8_t mux = 0)
        : GsmClientSim800(modem, mux) {
      is_secure = true;
    }

    // Because we have the same potential range of mux numbers for secure and
    // insecure connections, we don't need to re-check for mux number
    // availability.
  };
#endif

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmSim800(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM800"));

    if (!testAT()) { return false; }

    // sendAT(GF("&FZ"));  // Factory + Reset
    // waitResponse();

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

    // Enable Local Time Stamp for getting network time
    sendAT(GF("+CLTS=1"));
    if (waitResponse(10000L) != 1) { return false; }

    // Enable battery checks
    sendAT(GF("+CBATCHK=1"));
    waitResponse();

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

  bool factoryDefaultImpl() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("+IPR=0"));  // Auto-baud
    waitResponse();
    sendAT(GF("+IFC=0,0"));  // No Flow Control
    waitResponse();
    sendAT(GF("+ICF=3,3"));  // 8 data 0 parity 1 stop
    waitResponse();
    sendAT(GF("+CSCLK=0"));  // Disable Slow Clock
    waitResponse();
    sendAT(GF("&W"));  // Write configuration
    return waitResponse() == 1;
  }

  /*
    bool thisHasSSL() {
  #if defined(TINY_GSM_MODEM_SIM900)
      return false;
  #else
      sendAT(GF("+CIPSSL=?"));
      if (waitResponse(GF("+CIPSSL:")) != 1) { return false; }
      return waitResponse() == 1;
  #endif
    }
    */

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin) {
    if (!testAT()) { return false; }
    sendAT(GF("&W"));
    waitResponse();
    if (!setPhoneFunctionality(0)) { return false; }
    if (!setPhoneFunctionality(1, true)) { return false; }
    delay(3000);
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOWD=1"));
    return waitResponse(10000L, GF("NORMAL POWER DOWN")) == 1;
  }

  // During sleep, the SIM800 module has its serial communication disabled. In
  // order to reestablish communication pull the DRT-pin of the SIM800 module
  // LOW for at least 50ms. Then use this function to disable sleep mode. The
  // DTR-pin can then be released again.
  bool sleepEnableImpl(bool enable) {
    sendAT(GF("+CSCLK="), enable);
    return waitResponse() == 1;
  }

  // <fun> 0 Minimum functionality
  // <fun> 1 Full functionality (Default)
  // <fun> 4 Disable phone both transmit and receive RF circuits.
  // <rst> Reset the MT before setting it to <fun> power level.
  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 protected:
  Sim800RegStatus getRegistrationStatusImpl() {
    return static_cast<Sim800RegStatus>(getRegistrationStatusXREG("CREG"));
  }

  bool isNetworkConnectedImpl() {
    Sim800RegStatus s = this->getRegistrationStatus();
    return (s == Sim800RegStatus::REG_OK_HOME ||
            s == Sim800RegStatus::REG_OK_ROAMING);
  }

  String getLocalIPImpl() {
    sendAT(GF("+CIFSR;E0"));
    String res;
    if (waitResponse(10000L, res) != 1) { return ""; }
    cleanResponseString(res);
    return res;
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
  //  - Add `public TinyGsmSSL<TinyGsmSim800, TinyGsmSim800SslConfig>,` to the
  //  constructor's initializer list
  //  - Add `friend class TinyGsmSSL<TinyGsmSim800, TinyGsmSim800SslConfig>;` to
  //  the friend list
  //  - Make the secure client inherit from the secure client class in the SSL
  //  template.
  //  - Create an inner secure context class inheriting from
  //  TinyGsmSSLContext<TinyGsmSim800, TinyGsmSim800SslConfig>

  /*
   * Wifi functions
   */
  // No functions of this type supported

  /*
   * Cellular data functions
   */
 protected:
  bool dataConnectImpl(const char* apn, const char* user, const char* pwd) {
    dataDisconnect();

    // Bearer settings for applications based on IP
    // Set the connection type to GPRS
    sendAT(GF("+SAPBR=3,1,\"Contype\",\"GPRS\""));
    waitResponse();

    // Set the APN
    sendAT(GF("+SAPBR=3,1,\"APN\",\""), apn, '"');
    waitResponse();

    // Set the user name
    if (user && strlen(user) > 0) {
      sendAT(GF("+SAPBR=3,1,\"USER\",\""), user, '"');
      waitResponse();
    }
    // Set the password
    if (pwd && strlen(pwd) > 0) {
      sendAT(GF("+SAPBR=3,1,\"PWD\",\""), pwd, '"');
      waitResponse();
    }

    // Define the PDP context
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Activate the PDP context
    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    // Open the definied GPRS bearer context
    sendAT(GF("+SAPBR=1,1"));
    waitResponse(85000L);
    // Query the GPRS bearer context status
    sendAT(GF("+SAPBR=2,1"));
    if (waitResponse(30000L) != 1) { return false; }

    // Attach to Packet Domain service
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // Set to multi-IP
    sendAT(GF("+CIPMUX=1"));
    if (waitResponse() != 1) { return false; }

    // Put in "quick send" mode (thus no extra "Send OK")
    sendAT(GF("+CIPQSEND=1"));
    if (waitResponse() != 1) { return false; }

    // Set to get data manually
    sendAT(GF("+CIPRXGET=1"));
    if (waitResponse() != 1) { return false; }

    // Start Task and Set APN, USER NAME, PASSWORD
    sendAT(GF("+CSTT=\""), apn, GF("\",\""), user, GF("\",\""), pwd, '"');
    if (waitResponse(60000L) != 1) { return false; }

    // Bring Up Wireless Connection with GPRS or CSD
    sendAT(GF("+CIICR"));
    if (waitResponse(60000L) != 1) { return false; }

    // Get Local IP Address, only assigned after connection
    sendAT(GF("+CIFSR;E0"));
    if (waitResponse(10000L) != 1) { return false; }

#if TINY_GSM_USE_PUBLIC_DNS
    // Configure Domain Name Server (DNS)
    sendAT(GF("+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\""));
    if (waitResponse() != 1) { return false; }
#endif

    return true;
  }

  bool dataDisconnectImpl() {
    // Shut the TCP/IP connection
    // CIPSHUT will close *all* open connections
    sendAT(GF("+CIPSHUT"));
    if (waitResponse(60000L) != 1) { return false; }

    sendAT(GF("+CGATT=0"));  // Detach from Packet Domain service
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  String getProviderImpl() {
    sendAT(GF("+CSPN?"));
    if (waitResponse(GF("+CSPN:")) != 1) { return ""; }
    streamSkipUntil('"'); /* Skip mode and format */
    String res = stream.readStringUntil('"');
    waitResponse();
    return res;
  }

  /*
   * SIM card functions
   */
 protected:
  // May not return the "+CCID" before the number
  String getSimCCIDImpl() {
    sendAT(GF("+CCID"));
    if (waitResponse(GFP(ModemConfig::GSM_NL)) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    // Trim out the CCID header in case it is there
    res.replace("CCID:", "");
    res.trim();
    return res;
  }

  /*
   * Phone call functions
   */
 public:
  /**
   * @brief Set the busy status of the modem for calling.
   * @param busy True to set the modem as busy, false to set it as not busy.
   * @return True if the operation was successful, false otherwise.
   */
  bool setGsmBusy(bool busy = true) {
    sendAT(GF("+GSMBUSY="), busy ? 1 : 0);
    return waitResponse() == 1;
  }

  /*
   * Audio functions
   */
 public:
  /**
   * @brief Set the speaker volume of the modem.
   * @param volume The desired volume level (0-100).
   * @return True if the operation was successful, false otherwise.
   */
  bool setVolume(uint8_t volume = 50) {
    // Set speaker volume
    sendAT(GF("+CLVL="), volume);
    return waitResponse() == 1;
  }

  /**
   * @brief Get the current speaker volume of the modem.
   * @return The current volume level (0-100).
   */
  uint8_t getVolume() {
    // Get speaker volume
    sendAT(GF("+CLVL?"));
    if (waitResponse(GF("+CLVL:")) != 1) { return 0; }
    int16_t res = streamGetIntBefore('\n');
    waitResponse();
    return static_cast<uint8_t>(res);
  }

  /**
   * @brief Set the microphone volume of the modem.
   * @param channel The microphone channel (0-4).
   * @param level The desired volume level (0-100).
   * @return True if the operation was successful, false otherwise.
   */
  bool setMicVolume(uint8_t channel, uint8_t level) {
    if (channel > 4) { return false; }
    sendAT(GF("+CMIC="), channel, ',', level);
    return waitResponse() == 1;
  }

  /**
   * @brief Set the audio channel of the modem.
   * @param channel The desired audio channel.
   * @return True if the operation was successful, false otherwise.
   */
  bool setAudioChannel(uint8_t channel) {
    sendAT(GF("+CHFA="), channel);
    return waitResponse() == 1;
  }

  /**
   * @brief Play a toolkit tone on the modem.
   * @param tone The tone to play.
   * @param duration The duration of the tone in milliseconds.
   * @return True if the operation was successful, false otherwise.
   */
  bool playToolkitTone(uint8_t tone, uint32_t duration) {
    sendAT(GF("STTONE="), 1, tone);
    delay(duration);
    sendAT(GF("STTONE="), 0);
    return waitResponse();
  }

  /*
   * Text messaging (SMS) functions
   */
  // Follows all text messaging (SMS) functions as inherited from TinyGsmSMS.tpp

  /*
   * GSM Location functions
   */
  // Depending on the exacty model and firmware revision, should return a
  // GSM-based location from CLBS as as inherited from TinyGsmGSMLocation.tpp
  // TODO(?):  Check number of digits in year (2 or 4)

  /*
   * GPS/GNSS/GLONASS location functions
   */
  // No functions of this type supported

  /*
   * Time functions
   */
  // Follows all clock functions as inherited from TinyGsmTime.tpp

  /*
   * NTP server functions
   */
  // Follows all NTP server functions as inherited from TinyGsmNTP.tpp

  /*
   * BLE functions
   */
  // No functions of this type supported

  /*
   * Battery functions
   */
  // Follows all battery functions as inherited from TinyGsmBattery.tpp

  /*
   * Temperature functions
   */
  // No functions of this type supported

  /*
   * Client-related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t /*static*/ mux,
                    int timeout_s) {
    if (!isValidMux(mux)) { return false; }
    int8_t   rsp;
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
#if !defined(TINY_GSM_MODEM_SIM900)
    bool ssl = sockets[mux]->is_secure;
    sendAT(GF("+CIPSSL="), ssl);
    rsp = waitResponse();
    if (ssl && rsp != 1) { return false; }
#ifdef TINY_GSM_SSL_CLIENT_AUTHENTICATION
    // set SSL options
    // +SSLOPT=<opt>,<enable>
    // <opt>
    //    0 (default) ignore invalid certificate
    //    1 client authentication
    // <enable>
    //    0 (default) close
    //    1 open
    sendAT(GF("+CIPSSL=1,1"));
    if (waitResponse() != 1) return false;
#endif
#endif
    sendAT(GF("+CIPSTART="), mux, ',', GF("\"TCP"), GF("\",\""), host,
           GF("\","), port);
    rsp =
        waitResponse(timeout_ms, GF("CONNECT OK\r\n"), GF("CONNECT FAIL\r\n"),
                     GF("ALREADY CONNECT\r\n"), GFP(ModemConfig::GSM_ERROR),
                     GF("CLOSE OK\r\n"));  // Happens when HTTPS handshake fails
    return (1 == rsp || 3 == rsp);         // OK or ALREADY CONNECT
  }

  bool modemStop(uint8_t mux, uint32_t /*maxWaitMs*/) {
    if (!isValidMux(mux)) { return false; }
    // Same command for both secure and non-secure sockets
    sendAT(GF("+CIPCLOSE="), mux, GF(",1"));  // Quick close
    return waitResponse() == 1;               // should return within 1s
  }

  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    return waitResponse(GF(">")) == 1;
  }
  // Between the modemBeginSend and modemEndSend, modemSend calls:
  // stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  // stream.flush();
  size_t modemEndSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    if (waitResponse(GF("DATA ACCEPT:")) != 1) { return 0; }
    int16_t  ret_mux = streamGetIntBefore(',');   // check mux
    uint16_t sent    = streamGetIntBefore('\n');  // check send length
    if (sent != len) { DBG("### Sent:", sent, "of", len, "on", mux); }
    if (isExpectedMux(ret_mux, mux)) { return sent; }
    return 0;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    size_t len_read = 0;

#ifdef TINY_GSM_USE_HEX
    sendAT(GF("+CIPRXGET=3,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#else
    sendAT(GF("+CIPRXGET=2,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#endif

    streamSkipUntil(',');  // Skip Rx mode 2/normal or 3/HEX
    int16_t ret_mux      = streamGetIntBefore(',');  // mux
    int16_t len_reported = streamGetIntBefore(',');
    //  ^^ Requested number of data bytes (1-1460 bytes) to be read
    int16_t len_remaining = streamGetIntBefore('\n');
    // ^^ Confirmed number of data bytes to be read, which may be less than
    // requested. 0 indicates that no data can be read.
    // SRGD NOTE:  Contrary to above (which is copied from AT command manual)
    // the first number is the number of bytes returned and the second is the
    // number of bytes that will be remaining in the buffer after the read.
    if (isValidMux(ret_mux)) {
      // Move the data to the socket buffer of the returned mux as long as the
      // returned mux is valid, even if it doesn't match the expected mux.
#ifdef TINY_GSM_USE_HEX
      len_read = moveCharsFromStreamToFifo<true>(ret_mux, len_reported);
#else
      len_read = moveCharsFromStreamToFifo(ret_mux, len_reported);
#endif
    }
    waitResponse();  // ending OK; the waitResponse function will toss all the
                     // characters before the OK if the mux was invalid

    if (isValidMux(ret_mux)) {
      // get the amount available after reading
      sockets[ret_mux]->sock_available = len_remaining >= 0 ? len_remaining : 0;
      // sockets[mux]->sock_available = modemGetAvailable(mux);
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
    sendAT(GF("+CIPRXGET=4,"), mux);
    size_t result = 0;
    if (waitResponse(GF("+CIPRXGET:")) == 1) {
      streamSkipUntil(',');                       // Skip mode 4
      int16_t ret_mux = streamGetIntBefore(',');  // mux
      result          = streamGetIntBefore('\n');
      waitResponse();
      if (!isExpectedMux(ret_mux, mux)) { result = 0; }
    }
    // DBG("### Available:", result, "on", mux);
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPSTATUS="), mux);
    waitResponse(GF("+CIPSTATUS"));
    int8_t res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""),
                              GF(",\"CLOSING\""), GF(",\"REMOTE CLOSING\""),
                              GF(",\"INITIAL\""));
    waitResponse();
    return 1 == res;
  }

  /*
   * Utilities
   */
 protected:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+CIPRXGET:"))) {
      int8_t mode = streamGetIntBefore(',');
      if (mode == 1) {
        int16_t mux = streamGetIntBefore('\n');
        if (isValidMux(mux)) { sockets[mux]->got_data = true; }
        data = "";
        // DBG("### Got Data:", mux);
        return true;
      } else {
        data += mode;
        return false;
      }
    } else if (data.endsWith(GF("+RECEIVE:"))) {
      int16_t mux = streamGetIntBefore(',');
      int16_t len = streamGetIntBefore('\n');
      if (isValidMux(mux)) {
        sockets[mux]->got_data = true;
        if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
      }
      data = "";
      // DBG("### Got Data:", len, "on", mux);
      return true;
    } else if (data.endsWith(GF("CLOSED\r\n"))) {
      int16_t nl = TinyGsmMax(0,
                              data.lastIndexOf(String(GFP(ModemConfig::GSM_NL)),
                                               data.length() - 8));
      int16_t coma = data.indexOf(',', nl + 2);
      if (coma > nl + 2) {
        int16_t mux = data.substring(nl + 2, coma).toInt();
        if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
        DBG("### Closed: ", mux);
      }
      data = "";
      return true;
    } else if (data.endsWith(GF("*PSNWID:"))) {
      streamSkipUntil('\n');  // Refresh network name by network
      data = "";
      DBG("### Network name updated.");
      return true;
    } else if (data.endsWith(GF("*PSUTTZ:"))) {
      streamSkipUntil('\n');  // Refresh time and time zone by network
      data = "";
      DBG("### Network time and time zone updated.");
      return true;
    } else if (data.endsWith(GF("+CTZV:"))) {
      streamSkipUntil('\n');  // Refresh network time zone by network
      data = "";
      DBG("### Network time zone updated.");
      return true;
    } else if (data.endsWith(GF("DST:"))) {
      streamSkipUntil('\n');  // Refresh Network Daylight Saving Time by network
      data = "";
      DBG("### Daylight savings time state updated.");
      return true;
    }
    return false;
  }

 public:
  /// Stream used to communicate with the modem.
  Stream& stream;

 protected:
  GsmClientSim800* sockets[TcpConfig::kMuxCount];
};

#endif  // SRC_TINYGSMCLIENTSIM800_H_
