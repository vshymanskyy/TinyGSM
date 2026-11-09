/**
 * @file       TinyGsmClientSIM7600.h
 * @brief      SIM7600 modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup simcom_sim7600 SIMCom SIM7500, SIM7600, SIM7800
 * @ingroup group_simcom
 * @brief Manufacturer: SIMCom. Models: SIM7500, SIM7600, SIM7800.
 *
 * # Connection Information
 *
 * - TCP sockets:
 *   - 10
 * - SSL sockets:
 *   - 2
 * - SSL contexts:
 *   - 10
 * - Socket Buffering:
 *   - The modem has an internal buffer for incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 *
 * @todo In GsmClientSim7600 constructor: Ensure the secure socket mux isn't out
 * of range
 * @todo In `dataDisconnectImpl()`: Should CCHSTOP come before NETCLOSE?  Is it
 * needed in addition to NETCLOSE?
 * @todo In `getGPSImpl()`: Can 1 be returned
 * @todo In `GsmClientSecureSim7600::connect()`: verify CCHOPEN
 * @todo In `modemSend()`: make sure requested and confirmed bytes match
 */

#ifndef SRC_TINYGSMCLIENTSIM7600_H_
#define SRC_TINYGSMCLIENTSIM7600_H_
#pragma message("TinyGSM:  TinyGsmClientSIM7600")

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmCellularData.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmGPS.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"
#include "TinyGsmTemperature.tpp"

/// Registration status
/// @ingroup simcom_sim7600
enum class Sim7600RegStatus {
  REG_NO_RESULT    = -1,  ///< No registration result
  REG_UNREGISTERED = 0,   ///< Not registered on the network
  REG_SEARCHING    = 2,   ///< Searching for network
  REG_DENIED       = 3,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered on the home network
  REG_OK_ROAMING   = 5,   ///< Registered on a roaming network
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/// Basic modem configurations for the SIMCom SIM7600 modem family
/// @ingroup simcom_sim7600
struct TinyGsmSim7600ModemConfig
    : public TinyGsmModemConfigPreset<Sim7600RegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "SIMCom";
#if defined(TINY_GSM_MODEM_SIM7500)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM7500";
#elif defined(TINY_GSM_MODEM_SIM7800)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM7800";
#else
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM7600";
#endif
};

constexpr char TinyGsmSim7600ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmSim7600ModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for the SIMCom SIM7600 modem family.
 *
 * The module supports 10 TCP sockets or 2 SSL sockets.
 *
 * @todo I think the number of TCP and SSL sockets are independent of each other
 * and managed completely differently.  That is, I think there can be two
 * connection 0's, one using the SSL application on the module and the other
 * using the TCP application on the module.  Could someone who has a SIM7600
 * module test the real number of TCP and SSL connections that can be made at
 * once?
 *
 * @todo Handle the different number of sockets for TCP and SSL in the code.
 *
 * The secure send data command, CCHSEND, can handle up to 2048 bytes of input,
 * but the unsecured CIPSEND command will only accept 1500, so we'll take the
 * smaller number as the maximum send size.
 * @ingroup simcom_sim7600
 */
struct TinyGsmSim7600TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 10> {};

/**
 * @brief SSL behavior and limits for SIMCom SIM7600 modules.
 * The module supports 10 SSL contexts.
 * @ingroup simcom_sim7600
 */
struct TinyGsmSim7600SslConfig : public TinyGsmSslConfigPreset<
                                     /*contextCount*/ 10> {};

/// Class for the SIMCOM SIM7600, SIM7500, and SIM7800
/// @ingroup simcom_sim7600
class TinyGsmSim7600
    : public TinyGsmModem<TinyGsmSim7600, TinyGsmSim7600ModemConfig>,
      public TinyGsmCellularData<TinyGsmSim7600>,
      public TinyGsmTCP<TinyGsmSim7600, TinyGsmSim7600TcpConfig>,
      public TinyGsmSSL<TinyGsmSim7600, TinyGsmSim7600SslConfig>,
      public TinyGsmSMS<TinyGsmSim7600>,
      public TinyGsmGSMLocation<TinyGsmSim7600>,
      public TinyGsmGPS<TinyGsmSim7600>,
      public TinyGsmTime<TinyGsmSim7600>,
      public TinyGsmNTP<TinyGsmSim7600>,
      public TinyGsmBattery<TinyGsmSim7600>,
      public TinyGsmTemperature<TinyGsmSim7600>,
      public TinyGsmCalling<TinyGsmSim7600> {
  friend class TinyGsmModem<TinyGsmSim7600, TinyGsmSim7600ModemConfig>;
  friend class TinyGsmCellularData<TinyGsmSim7600>;
  friend class TinyGsmTCP<TinyGsmSim7600, TinyGsmSim7600TcpConfig>;
  friend class GsmClient<TinyGsmSim7600, TinyGsmSim7600TcpConfig>;
  friend class TinyGsmSSL<TinyGsmSim7600, TinyGsmSim7600SslConfig>;
  friend class TinyGsmSMS<TinyGsmSim7600>;
  friend class TinyGsmGPS<TinyGsmSim7600>;
  friend class TinyGsmGSMLocation<TinyGsmSim7600>;
  friend class TinyGsmTime<TinyGsmSim7600>;
  friend class TinyGsmNTP<TinyGsmSim7600>;
  friend class TinyGsmBattery<TinyGsmSim7600>;
  friend class TinyGsmTemperature<TinyGsmSim7600>;
  friend class TinyGsmCalling<TinyGsmSim7600>;

 public:
  using ModemConfig = TinyGsmSim7600ModemConfig;
  using TcpConfig   = TinyGsmSim7600TcpConfig;
  using SslConfig   = TinyGsmSim7600SslConfig;
  using GsmSecureContext =
      TinyGsmSSL<TinyGsmSim7600, TinyGsmSim7600SslConfig>::gsmSecureContext;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup simcom_sim7600
  class GsmClientSim7600
      : public GsmClient<TinyGsmSim7600, TinyGsmSim7600TcpConfig> {
    friend class TinyGsmSim7600;

   public:
    using GsmClient<TinyGsmSim7600, TinyGsmSim7600TcpConfig>::connect;
    using GsmClient<TinyGsmSim7600, TinyGsmSim7600TcpConfig>::stop;
    using TcpConfig = TinyGsmSim7600TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSim7600() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is identical to the identifier
     * the modem uses internally to identify the socket for the SIM7600.
     *
     * @note The SIM7600 and similar variants allow you choose the multiplexing
     * channel number, but if the input mux channel number is already in use and
     * other mux channels are available, this library will select the next
     * available one.  Use the getMux() function to get the assigned
     * multiplexing channel number after a successful connection.
     */
    explicit GsmClientSim7600(TinyGsmSim7600& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmSim7600, TinyGsmSim7600TcpConfig>(modem, mux) {
      is_secure = false;
      init(&modem, mux);
    }

    /*
     * Arduino Client interface
     */
    TINY_GSM_STATIC_TCP_CONNECT

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
  /// @ingroup simcom_sim7600
  class GsmClientSecureSim7600
      : public GsmSecureClient<GsmClientSim7600, TinyGsmSim7600> {
    friend class TinyGsmSim7600;

   public:
    using GsmSecureClient<GsmClientSim7600, TinyGsmSim7600>::init;
    using GsmClientSim7600::connect;
    using GsmClientSim7600::stop;
    using TcpConfig = TinyGsmSim7600TcpConfig;

    /*
     * Secure client constructors and initialization
     */

    /**
     * @brief Create a new secured TCP (SSL) client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSecureSim7600() : GsmSecureClient() {}

    /**
     * @brief Create a new secured TCP (SSL) client and bind it to a modem and
     * optionally a multiplexing channel.
     * @copydetails GsmClientSim7600::GsmClientSim7600(TinyGsmSim7600&, uint8_t)
     */
    explicit GsmClientSecureSim7600(TinyGsmSim7600& modem, uint8_t mux = 0)
        : GsmSecureClient() {
      init(&modem, mux);
    }

    /**
     * @brief Create a new secured TCP (SSL) client with an explicit SSL
     * context and bind it to a modem and multiplexing channel.
     * @param modem The modem that owns the socket and SSL contexts.
     * @param mux The socket identifier.
     * @param sslCtxIndex The hardware SSL context index.
     */
    explicit GsmClientSecureSim7600(TinyGsmSim7600& modem, uint8_t mux,
                                    uint8_t sslCtxIndex)
        : GsmSecureClient() {
      setSSLContextIndex(sslCtxIndex);
      init(&modem, mux);
    }

    /**
     * @brief Initialize this client with modem context and multiplexing
     * channel.
     *
     * @param modem Pointer to the modem instance.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.
     * @param strictAttach If true, require the requested mux for static mux
     * clients.
     * @return true if initialization completed.
     * @important The SSL client for the A7672x only supports multiplexing
     * channel numbers 0 and 1 unlike the 10 channels supported for non-SSL
     * sockets.
     */
    bool init(TinyGsmSim7600* modem, uint8_t mux = 0,
              bool strictAttach = false) override {
      return this->attachToModem(this, modem, mux, 2, strictAttach);
    }

    /*
     * Arduino Client interface
     */

    int connect(const char* host, uint16_t port, int timeout_s) override {
      if (at == nullptr) { return 0; }
      stop(TcpConfig::kStopTimeoutS * 1000L);
      rx.clear();
      GsmSecureContext* sslContext = at->getSSLContext(sslCtxIndex);
      // check for a valid SSL context
      if (sslContext == nullptr) {
        DBG("### No SSL context is registered at index", sslCtxIndex);
        return 0;
      }
      // check that the authentication mode is supported
      if (sslContext->sslAuthMode == SSLAuthMode::PRE_SHARED_KEYS) {
        DBG("### The SIM7600 does not support SSL using pre-shared keys.");
        return 0;
      }
      // check that the context has the required credentials
      if (!at->validateSSLContextCredentials(sslContext)) {
        DBG("### The SSL context is missing required credentials!");
        return 0;
      }
      // configure the context
      if (!at->configureSSLContext(sslCtxIndex)) {
        DBG("### Failed to configure the SSL context!");
        return 0;
      }
      // make the connection
      sock_connected = at->modemConnect(host, port, mux, timeout_s);
      return sock_connected;
    }
  };

  /*
   * Inner Secure Context
   */
 public:
  /// Inner secure context
  /// @ingroup simcom_sim7600
  class GsmSecureContextSim7600
      : public ::GsmSecureContext<TinyGsmSim7600, TinyGsmSim7600SslConfig> {
    friend class TinyGsmSim7600;

   public:
    using ::GsmSecureContext<TinyGsmSim7600,
                             TinyGsmSim7600SslConfig>::GsmSecureContext;

    /// Unregister this context from its modem.
    ~GsmSecureContextSim7600() {}
  };

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmSim7600(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM7600"));

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

    // Disable time and time zone URC's
    sendAT(GF("+CTZR=0"));
    if (waitResponse(10000L) != 1) { return false; }

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

  bool factoryDefaultImpl() {  // these commands aren't supported
    return false;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin) {
    if (!testAT()) { return false; }
    sendAT(GF("+CRESET"));
    if (waitResponse(10000L) != 1) { return false; }
    delay(16000L);
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOF"));
    return waitResponse() == 1;
  }

  bool radioOffImpl() {
    if (!setPhoneFunctionality(4)) { return false; }
    delay(3000);
    return true;
  }

  bool sleepEnableImpl(bool enable) {
    sendAT(GF("+CSCLK="), enable);
    return waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 protected:
  Sim7600RegStatus getRegistrationStatusImpl() {
    return static_cast<Sim7600RegStatus>(getRegistrationStatusXREG("CGREG"));
  }

  bool isNetworkConnectedImpl() {
    Sim7600RegStatus s = this->getRegistrationStatus();
    return (s == Sim7600RegStatus::REG_OK_HOME ||
            s == Sim7600RegStatus::REG_OK_ROAMING);
  }

  String getLocalIPImpl() {
    sendAT(GF("+IPADDR"));  // Inquire Socket PDP address
    // sendAT(GF("+CGPADDR=1"));  // Show PDP address
    String res;
    if (waitResponse(10000L, res) != 1) { return ""; }
    cleanResponseString(res);
    res.trim();
    return res;
  }

  /*
   * Network mode / type / technology functions
   */
 public:
  /**
   * @brief Get the available network modes of the modem.
   * @return A string representing the available network modes.
   */
  String getNetworkModes() {
    // Get the help string, not the setting value
    sendAT(GF("+CNMP=?"));
    if (waitResponse(GF("+CNMP:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  /**
   * @brief Get the current network mode of the modem.
   * @return The current network mode as an integer.
   */
  int16_t getNetworkMode() {
    sendAT(GF("+CNMP?"));
    if (waitResponse(GF("+CNMP:")) != 1) { return false; }
    int16_t mode = streamGetIntBefore('\n');
    waitResponse();
    return mode;
  }

  /**
   * @brief Set the network mode of the modem.
   * @param mode The network mode to set.
   * @return True if the operation was successful, false otherwise.
   */
  bool setNetworkMode(uint8_t mode) {
    sendAT(GF("+CNMP="), mode);
    return waitResponse() == 1;
  }

  /**
   * @brief Get the network system mode of the modem.
   * @param n A reference to a boolean that will be set to true if the modem is
   * in automatic reporting mode, false otherwise.
   * @param stat A reference to an integer that will be set to the current
   * service status. 0 if not connected.
   * @return True if the operation was successful, false otherwise.
   */
  bool getNetworkSystemMode(bool& n, int16_t& stat) {
    // n: whether to automatically report the system mode info
    // stat: the current service. 0 if it not connected
    sendAT(GF("+CNSMOD?"));
    if (waitResponse(GF("+CNSMOD:")) != 1) { return false; }
    n    = streamGetIntBefore(',') != 0;
    stat = streamGetIntBefore('\n');
    waitResponse();
    return true;
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
 protected:
  template <typename TCert>
  bool loadCertificateImpl(const char* certificateName, TCert cert,
                           const uint16_t len) {
    sendAT(GF("+CCERTDOWN=\""), certificateName, GF("\","), len);
    if (waitResponse(5000L, GF(">")) != 1) { return false; }
    this->streamWrite(cert, len);
    stream.flush();
    return waitResponse(5000L) == 1;
  }

  bool deleteCertificateImpl(const char* certificateName) {
    sendAT(GF("+CCERTDELE=\""), certificateName, '"');
    return waitResponse(5000L) == 1;
  }

  // no certificate conversion needed
  bool convertCertificateImpl(CertificateType, const char*) {
    return true;
  }
  bool convertCACertificateImpl(const char*) {
    return true;
  }
  bool convertClientCertificatesImpl(const char*, const char*) {
    return true;
  }
  bool convertPSKandIDImpl(const char*, const char*) {
    return true;
  }
  bool convertPSKTableImpl(const char*) {
    return true;
  }

  /*
   * Wifi functions
   */
  // No functions of this type supported

  /*
   * Cellular data functions
   */
 protected:
  bool dataConnectImpl(const char* apn, const char* user, const char* pwd) {
    dataDisconnect();  // Make sure we're not connected first

    // Define the PDP context

    // The CGDCONT commands set up the "external" PDP context

    // Set the external authentication
    if (user && strlen(user) > 0) {
      sendAT(GF("+CGAUTH=1,0,\""), pwd, GF("\",\""), user, '"');
      waitResponse();
    }

    // Define external PDP context 1
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"', ",\"0.0.0.0\",0,0");
    waitResponse();

    // Configure TCP parameters

    // Select TCP/IP application mode (command mode)
    //  0 = non transparent mode
    sendAT(GF("+CIPMODE=0"));
    waitResponse();

    // Set Sending Mode - send without waiting for peer TCP ACK
    sendAT(GF("+CIPSENDMODE=0"));
    waitResponse();

    // Configure socket parameters
    // AT+CIPCCFG= <NmRetry>, <DelayTm>, <Ack>, <errMode>, <HeaderType>,
    //            <AsyncMode>, <TimeoutVal>
    // NmRetry = number of retransmission to be made for an IP packet
    //         = 10 (default)
    // DelayTm = number of milliseconds to delay before outputting received data
    //          = 0 (default)
    // Ack = sets whether reporting a string "Send ok" = 0 (don't report)
    // errMode = mode of reporting error result code = 0 (numberic values)
    // HeaderType = which data header of receiving data in multi-client mode
    //            = 1 (+RECEIVE,<link num>,<data length>)
    // AsyncMode = sets mode of executing commands
    //           = 0 (synchronous command executing)
    // TimeoutVal = minimum retransmission timeout in milliseconds
    sendAT(GF("+CIPCCFG=10,0,0,0,1,0,"),
           TcpConfig::kRetransmissionTimeoutS * 1000L);
    if (waitResponse() != 1) { return false; }

    // Configure timeouts for opening and closing sockets
    // AT+CIPTIMEOUT=<netopen_timeout> <cipopen_timeout>, <cipsend_timeout>
    sendAT(GF("+CIPTIMEOUT="), 75000, ',', TcpConfig::kConnectTimeoutS * 1000L,
           ',', TcpConfig::kSendTimeoutS * 1000L);
    waitResponse();

    // Set to get data manually on TCP (unsecured) sockets
    sendAT(GF("+CIPRXGET=1"));
    if (waitResponse() != 1) { return false; }

    // Start the (unsecured) socket service

    // This activates and attaches to the external PDP context that is tied
    // to the embedded context for TCP/IP (ie AT+CGACT=1,1 and AT+CGATT=1)
    // Response may be an immediate "OK" followed later by "+NETOPEN: 0".
    // We to ignore any immediate response and wait for the
    // URC to show it's really connected.
    sendAT(GF("+NETOPEN"));
    int8_t res = waitResponse(75000L, GF("+NETOPEN: 0"),
                              GF("+IP ERROR: Network is already opened"),
                              GFP(ModemConfig::GSM_ERROR));
    if (res != 1 && res != 2) { return false; }

    // Set the module to require manual reading of rx buffer data on SSL sockets

    // Configure the mode of sending and receiving data
    // CCHMODE=<mode>
    // <mode>
    //  0 – Normal
    //  1 – Transparent mode
    // This command must be called before AT+CCHSTART.
    sendAT(GF("+CCHMODE=0"));
    waitResponse(5000L);  // allowed to fail

    // Configure the report mode of sending and receiving data
    // +CCHSET=<report_send_result>,<recv_mode>
    // <report_send_result> Whether to report result of CCHSEND, the default
    //   value is 0: 0 No. 1 Yes. Module will report +CCHSEND:
    // <session_id>,<err> to MCU when complete sending data.
    // <recv_mode> The receiving mode, the default value is 0:
    //   0 Output the data to MCU whenever received data.
    //   1 Module caches the received data and notifies MCU with+CCHEVENT:
    // <session_id>,RECV EVENT. MCU can use AT+CCHRECV to receive the cached
    //   data (only in manual receiving mode).
    // This command must be called before AT+CCHSTART.
    sendAT(GF("+CCHSET=1,1"));
    waitResponse(5000L);  // allowed to fail

    // Start the SSL client service (doesn't mean we have to use it!)
    sendAT(GF("+CCHSTART"));
    waitResponse(5000L);  // allowed to fail

    return true;
  }

  bool dataDisconnectImpl() {
    // Close all sockets and stop the socket service
    // Note: On the LTE models, this single command closes all sockets and the
    // service and deactivates the PDP context
    sendAT(GF("+NETCLOSE"));
    waitResponse(60000L);  // allowed to fail

    // Stop the SSL client
    sendAT(GF("+CCHSTOP"));
    waitResponse(60000L, GF("+CCHSTOP: 0"));  // allowed to fail

    // TODO: Should CCHSTOP come before NETCLOSE?  Is it needed in addition to
    // NETCLOSE?

    sendAT(GF("+CGATT=0"));  // Detach from Packet Domain service
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  bool isDataConnectedImpl() {
    sendAT(GF("+NETOPEN?"));
    // May return +NETOPEN: 1, 0.  We just confirm that the first number is 1
    if (waitResponse(GF("+NETOPEN: 1")) != 1) { return false; }
    waitResponse();

    sendAT(GF("+IPADDR"));  // Inquire Socket PDP address
    // sendAT(GF("+CGPADDR=1")); // Show PDP address
    if (waitResponse() != 1) { return false; }

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
  // Gets the CCID of a sim card via AT+CCID
  String getSimCCIDImpl() {
    sendAT(GF("+CICCID"));
    if (waitResponse(GF("+ICCID:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone call functions
   */
 protected:
  bool callHangupImpl() {
    sendAT(GF("+CHUP"));
    return waitResponse() == 1;
  }

  /*
   * Audio functions
   */
  // No functions of this type supported

  /*
   * Text messaging (SMS) functions
   */
  // Follows all text messaging (SMS) functions as inherited from TinyGsmSMS.tpp

  /*
   * GSM location functions
   */
  // Follows all GSM-based location functions as inherited from
  // TinyGsmGSMLocation.tpp

  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // enable GPS
  bool enableGPSImpl() {
    sendAT(GF("+CGPS=1"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool disableGPSImpl() {
    sendAT(GF("+CGPS=0"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  // get the RAW GPS output
  String getGPSrawImpl() {
    sendAT(GF("+CGNSSINFO"));
    if (waitResponse(GF("+CGNSSINFO:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  bool getGPSImpl(float* lat, float* lon, float* speed, float* alt, int* vsat,
                  int* usat, float* accuracy, int* year, int* month, int* day,
                  int* hour, int* minute, int* second) {
    sendAT(GF("+CGNSSINFO"));
    if (waitResponse(GF("+CGNSSINFO:")) != 1) { return false; }

    uint8_t fixMode = streamGetIntBefore(',');  // mode 2=2D Fix or 3=3DFix
                                                // TODO(?) Can 1 be returned

    // init variables
    float   ilat = 0;
    char    north;
    float   ilon = 0;
    char    east;
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

    streamSkipUntil(',');               // GPS satellite valid numbers
    streamSkipUntil(',');               // GLONASS satellite valid numbers
    streamSkipUntil(',');               // BEIDOU satellite valid numbers
    ilat  = streamGetFloatBefore(',');  // Latitude in ddmm.mmmmmm
    north = stream.readStringUntil(',').charAt(
        0);                            // N/S Indicator, N=north or S=south
    ilon = streamGetFloatBefore(',');  // Longitude in ddmm.mmmmmm
    east = stream.readStringUntil(',').charAt(
        0);  // E/W Indicator, E=east or W=west

    // Date. Output format is ddmmyy
    // Read the whole date portion into a single uint32_t, then parse it into
    // day, month, year.  By doing it this way, we avoid issues of attempting to
    // read a set number of characters for each portion of the date if the date
    // is missing or malformed.
    uint32_t idate = streamGetULBefore(',');
    iday           = static_cast<int16_t>(idate / 10000);  // Two digit day
    imonth = static_cast<int16_t>((idate / 100) % 100);    // Two digit month
    iyear  = static_cast<int16_t>(idate % 100);            // Two digit year

    // UTC Time. Output format is hhmmss.s
    // Again, read the whole time portion into a single float, then parse that
    // into the hour, minute, and second portions.
    float itime = streamGetFloatBefore(',');
    if (itime >= 0.0F) {
      uint32_t itime_int      = static_cast<uint32_t>(itime);
      float    itime_fraction = itime - static_cast<float>(itime_int);
      ihour = static_cast<int16_t>(itime_int / 10000);       // Two digit hour
      imin = static_cast<int16_t>((itime_int / 100) % 100);  // Two digit minute
      secondWithSS = static_cast<float>(itime_int % 100) + itime_fraction;
    }

    ialt   = streamGetFloatBefore(',');     // MSL Altitude. Unit is meters
    ispeed = streamGetFloatBefore(',');     // Speed Over Ground. Unit is knots.
    streamSkipUntil(',');                   // Course Over Ground. Degrees.
    iaccuracy = streamGetFloatBefore(',');  // Position Dilution Of Precision
    streamSkipUntil(',');                   // Horizontal Dilution Of Precision
    streamSkipUntil('\n');                  // Vertical Dilution Of Precision

    // Set pointers
    if (lat != nullptr)
      *lat = (floor(ilat / 100) + fmod(ilat, 100.) / 60) *
          (north == 'N' ? 1 : -1);
    if (lon != nullptr)
      *lon = (floor(ilon / 100) + fmod(ilon, 100.) / 60) *
          (east == 'E' ? 1 : -1);
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
    DBG(GF("Latitude:"), String(ilat, 8), GF("\tLongitude:"), String(ilon, 8),
        GF("\tAltitude:"), String(ialt, 4));
    DBG(GF("VSAT:"), ivsat, GF("\tUSAT:"), iusat, GF("\tAccuracy:"), iaccuracy);
    DBG(GF("Year:"), iyear, GF("\tMonth:"), imonth, GF("\tDay:"), iday);
    DBG(GF("Hour:"), ihour, GF("\tMinute:"), imin, GF("\tSecond:"),
        String(secondWithSS, 3));
#endif

    return (fixMode == 1 || fixMode == 2 || fixMode == 3);
  }

  /*
   *  CGNSSMODE: <gnss_mode>,<dpo_mode>
   * This command is used to configure GPS, GLONASS, BEIDOU and QZSS support
   * mode. 0 : GLONASS 1 : BEIDOU 2 : GALILEO 3 : QZSS dpo_mode: 1 enable , 0
   * disable
   */
  String setGNSSModeImpl(uint8_t mode, bool dpo) {
    String res;
    sendAT(GF("+CGNSSMODE="), mode, ',', dpo);
    if (waitResponse(10000L, res) != 1) { return ""; }
    res.replace(String(GFP(ModemConfig::GSM_NL)), "");
    res.trim();
    return res;
  }

  uint8_t getGNSSModeImpl() {
    sendAT(GF("+CGNSSMODE?"));
    if (waitResponse(GF("+CGNSSMODE:")) != 1) { return 0; }
    return stream.readStringUntil(',').toInt();
  }


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
 protected:
  // returns volts, multiply by 1000 to get mV
  int16_t getBattVoltageImpl() {
    sendAT(GF("+CBC"));
    if (waitResponse(GF("+CBC:")) != 1) { return 0; }

    // get voltage in VOLTS
    float voltage = streamGetFloatBefore('\n');
    // Wait for final OK
    waitResponse();
    // Return millivolts
    uint16_t res = voltage * 1000;
    return res;
  }

  int8_t getBattPercentImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  int8_t getBattChargeStateImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    chargeState = 0;
    percent     = 0;
    milliVolts  = getBattVoltage();
    return true;
  }

  /*
   * Temperature functions
   */
 protected:
  // get temperature in degree celsius
  float getTemperatureImpl() {
    sendAT(GF("+CPMUTEMP"));
    if (waitResponse(GF("+CPMUTEMP:")) != 1) { return -9999.0f; }
    // return temperature in C
    float temp = streamGetFloatBefore('\n');
    // Wait for final OK
    waitResponse();
    return temp;
  }

  /*
   * SSL context functions
   */
 public:
  /**
   * @brief Configure the SSL context for the modem.
   * @param context_id The SSL context ID.
   * @param sslAuthMode The SSL authentication mode.
   * @param sslVersion The SSL version.
   * @param CAcertName The CA certificate name.
   * @param clientCertName The client certificate name.
   * @param clientKeyName The client key name.
   * @return True if the operation was successful, false otherwise.
   */
  bool configureSSLContext(uint8_t context_id, SSLAuthMode sslAuthMode,
                           SSLVersion sslVersion, const char* CAcertName,
                           const char* clientCertName,
                           const char* clientKeyName) {
    // store the configuration in the SSL context object
    GsmSecureContext* sslContext = getSSLContext(context_id);
    if (sslContext == nullptr) { return false; }
    sslContext->sslAuthMode = sslAuthMode;
    sslContext->sslVersion  = sslVersion;
    strncpy(sslContext->CAcertName, CAcertName ? CAcertName : "",
            sizeof(sslContext->CAcertName) - 1);
    sslContext->CAcertName[sizeof(sslContext->CAcertName) - 1] = '\0';
    strncpy(sslContext->clientCertName, clientCertName ? clientCertName : "",
            sizeof(sslContext->clientCertName) - 1);
    sslContext->clientCertName[sizeof(sslContext->clientCertName) - 1] = '\0';
    strncpy(sslContext->clientKeyName, clientKeyName ? clientKeyName : "",
            sizeof(sslContext->clientKeyName) - 1);
    sslContext->clientKeyName[sizeof(sslContext->clientKeyName) - 1] = '\0';

    return configureSSLContext(context_id);
  }

  /**
   * @brief Configure the SSL context for the modem.
   * @param context_id The SSL context ID.
   * @return True if the operation was successful, false otherwise.
   */
  bool configureSSLContext(uint8_t context_id) {
    bool success = true;

    // List the certs available
    //   sendAT(GF("+CCERTLIST"));
    //   waitResponse(5000L);

    // NOTE: The SSL context (<ssl_ctx_index>) is not the same as the
    // connection identifier.  The SSL context is the grouping of SSL
    // settings, the connection identifier is the mux/socket number.

    // CSSLCFG commands reference the SSL context number; C**A**SSLCFG
    // commands reference the connection number (aka, the mux).

    // Retrieve the SSL context object for the given context ID.
    GsmSecureContext* sslContext  = getSSLContext(context_id);
    SSLAuthMode       sslAuthMode = sslContext ? sslContext->sslAuthMode
                                               : SSLAuthMode::NO_VALIDATION;
    SSLVersion        sslVersion  = sslContext ? sslContext->sslVersion
                                               : SSLVersion::ALL_SSL;
    const char* CAcertName     = sslContext ? sslContext->CAcertName : nullptr;
    const char* clientCertName = sslContext ? sslContext->clientCertName
                                            : nullptr;
    const char* clientKeyName  = sslContext ? sslContext->clientKeyName
                                            : nullptr;

    DBG("### SSL context index:", context_id);
    DBG("### SSL auth mode:", (int)sslAuthMode);
    DBG("### SSL version:", (int)sslVersion);
    DBG("### CA cert name:", CAcertName);
    DBG("### Client cert name:", clientCertName);
    DBG("### Client key name:", clientKeyName);

    // set the ssl version
    // AT+CSSLCFG="sslversion",<ssl_ctx_index>,<sslversion>
    // <ssl_ctx_index> The SSL context ID. The range is 0-9.
    // <sslversion> 0: SSL3.0
    //              1: TLS1.0
    //              2: TLS1.1
    //              3: TLS1.2
    //              4: All
    if (static_cast<int8_t>(sslVersion) < 0 ||
        static_cast<int8_t>(sslVersion) > 4) {
      // Not supported; select "ALL" and hope for the best
      sslVersion = SSLVersion::ALL_SSL;
    }
    sendAT(GF("+CSSLCFG=\"sslversion\","), context_id, ',',
           static_cast<int8_t>(sslVersion));
    success &= waitResponse(5000L) == 1;

    // set authentication mode
    // AT+CSSLCFG="authmode",<ssl_ctx_index>,<authmode>
    // <ssl_ctx_index> The SSL context ID. The range is 0-9.
    // <authmode> 0: No authentication (SSLAuthMode::NO_VALIDATION)
    //            1: server authentication (SSLAuthMode::CA_VALIDATION)
    //            2: server and client authentication
    //            (SSLAuthMode::MUTUAL_AUTHENTICATION)
    //            3: client authentication and no server authentication
    //            (SSLAuthMode::CLIENT_VALIDATION)
    // Because the auth mode used by the SIM7600 is different than the
    // SSLAuthMode enum, we need to translate it to the correct value.
    int8_t authModeValue = 0;
    switch (sslAuthMode) {
      case SSLAuthMode::NO_VALIDATION: authModeValue = 0; break;
      case SSLAuthMode::CA_VALIDATION: authModeValue = 1; break;
      case SSLAuthMode::MUTUAL_AUTHENTICATION: authModeValue = 2; break;
      case SSLAuthMode::CLIENT_VALIDATION: authModeValue = 3; break;
      default:
        authModeValue = 0;  // Default to no validation if unknown
        break;
    }
    sendAT(GF("+CSSLCFG=\"authmode\","), context_id, ',',
           static_cast<int8_t>(authModeValue));
    success &= waitResponse(5000L) == 1;

    // apply the correct certificates to the connection
    if (CAcertName != nullptr &&
        strnlen(CAcertName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
        (sslAuthMode == SSLAuthMode::CA_VALIDATION ||
         sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION)) {
      /* Configure the server root CA of the specified SSL context
      AT + CSSLCFG = "cacert", <ssl_ctx_index>,<ca_file> */
      sendAT(GF("+CSSLCFG=\"cacert\","), context_id, GF(",\""), CAcertName,
             '"');
      success &= waitResponse(5000L) == 1;
    }
    if (clientCertName != nullptr &&
        strnlen(clientCertName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
        (sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION ||
         sslAuthMode == SSLAuthMode::CLIENT_VALIDATION)) {
      sendAT(GF("+CSSLCFG=\"clientcert\","), context_id, GF(",\""),
             clientCertName, '"');
      success &= waitResponse(5000L) == 1;
    }
    if (clientKeyName != nullptr &&
        strnlen(clientKeyName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
        (sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION ||
         sslAuthMode == SSLAuthMode::CLIENT_VALIDATION)) {
      sendAT(GF("+CSSLCFG=\"clientkey\","), context_id, GF(",\""),
             clientKeyName, '"');
      success &= waitResponse(5000L) == 1;
    }

    return success;
  }

  /**
   * @brief Link the SSL context to a specific connection (mux).
   * @param mux The **zero-indexed** position of the client in the modem's
   * socket array.
   * @param context_id The SSL context ID.
   * @return True if the operation was successful, false otherwise.
   */
  bool linkSSLContext(uint8_t mux, uint8_t context_id) {
    // set the configured SSL context for the session
    // AT+CCHSSLCFG=<session_id>,<ssl_ctx_index>
    sendAT(GF("+CCHSSLCFG="), mux, ',', context_id);
    return waitResponse(5000L) == 1;
  }

  /*
   * Client-related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t /*static*/ mux,
                    int timeout_s) {
    if (!isValidMux(mux)) { return false; }
    bool     success    = false;
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    bool     ssl        = sockets[mux]->is_secure;

    if (ssl) {
      // If we have a secure socket, use a static cast to get the context index.
      // This isn't really "safe" but since we've already checked that the
      // socket is a secure one, we're pretty sure of the type and it should
      // work.
      const GsmClientSecureSim7600* thisClient =
          static_cast<const GsmClientSecureSim7600*>(sockets[mux]);
      uint8_t sslCtxIndex = thisClient->sslCtxIndex;

      if (!linkSSLContext(mux, sslCtxIndex)) {
        DBG("### Failed to link the SSL context to mux", mux);
        return false;
      }

      // Establish a connection in multi-socket mode
      // AT+CCHOPEN=<session_id>,"<host>",<port>[,<client_type>,[<bind_port>]]
      sendAT(GF("+CCHOPEN="), mux, GF(",\""), host, GF("\","), port, GF(",2"));
      // The reply is OK or ERROR followed by +CCHOPEN: <session_id>,<err> where
      // <session_id> is the mux number and <err> should be 0 if there's no
      // error
      success = waitResponse(timeout_ms) == 1;  // capture the OK or ERROR
      if (success) {
        success &= waitResponse(timeout_ms, GF("+CCHOPEN:")) == 1;
      }
      // TODO: verify this
    } else {
      // AT+CIPOPEN=<link_num>,"TCP",<serverIP>,<serverPort>[,<localPort>]
      sendAT(GF("+CIPOPEN="), mux, ',', GF("\"TCP"), GF("\",\""), host,
             GF("\","), port);
      // The reply is OK or ERROR followed by +CIPOPEN: <link_num>,<err> where
      // <link_num> is the mux number and <err> should be 0 if there's no
      // error
      // There may also be an ERROR returned after the +CIPOPEN: line if the PDP
      // context wasn't activated first. We ignore this case.
      success = waitResponse(timeout_ms) == 1;  // capture the OK or ERROR
      if (success) {
        success &= waitResponse(timeout_ms, GF("+CIPOPEN:")) == 1;
      }
    }

    // Since both CIPOPEN and CCHOPEN return the same response, we can handle it
    // here
    if (success) {
      int16_t opened_mux    = streamGetIntBefore(',');
      uint8_t opened_result = streamGetIntBefore('\n');
      if (!isExpectedMux(opened_mux, mux) || opened_result != 0) return false;
    }
    return success;
  }


  // NOTE: The implementations of modemSend(...), modemRead(...), and
  // modemGetAvailable(...) are almost completely different for SSL and
  // unsecured sockets.
  bool modemStop(uint8_t mux, uint32_t /*maxWaitMs*/) {
    if (!isValidMux(mux)) { return false; }
    bool ssl = sockets[mux]->is_secure;
    if (ssl) {
      sendAT(GF("+CCHCLOSE="), mux);
    } else {
      sendAT(GF("+CIPCLOSE="), mux);
    }
    return waitResponse() == 1;  // should return within 1s
  }

  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    bool ssl = sockets[mux]->is_secure;
    if (ssl) {
      sendAT(GF("+CCHSEND="), mux, ',', (uint16_t)len);
    } else {
      sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    }
    return waitResponse(GF(">")) == 1;
  }
  // Between the modemBeginSend and modemEndSend, modemSend calls:
  // stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  // stream.flush();
  size_t modemEndSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    if (waitResponse() != 1) { return 0; }
    bool ssl = sockets[mux]->is_secure;
    if (ssl) {
      // Because we set CCHSET to return the send result, we should get a
      // +CCHSEND: <session_id>,<err>
      if (waitResponse(10000L, GF("+CCHSEND:"), GFP(ModemConfig::GSM_ERROR),
                       GF("CLOSE OK\r\n")) != 1) {
        return 0;
      }
      int16_t ret_mux = streamGetIntBefore(',');        // check mux
      bool    result  = streamGetIntBefore('\n') == 0;  // check error code
      if (isExpectedMux(ret_mux, mux) && result) { return len; }
      return 0;
    } else {
      // after OK, returns +CIPSEND: <link_num>,<reqSendLength>,<cnfSendLength>
      if (waitResponse(GF("+CIPSEND:")) != 1) { return 0; }
      int16_t ret_mux = streamGetIntBefore(',');  // check mux
      streamSkipUntil(',');  // Skip requested bytes to send
      // TODO(?):  make sure requested and confirmed bytes match
      int16_t sent = streamGetIntBefore('\n');  // check send length
      if (isExpectedMux(ret_mux, mux) && sent >= 0) {
        return static_cast<size_t>(sent);
      }
      return 0;
    }
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    bool    ssl           = sockets[mux]->is_secure;
    int16_t len_reported  = 0;
    int16_t len_remaining = 0;
    int16_t ret_mux;

    if (ssl) {
#if defined(TINY_GSM_USE_HEX) && defined(TINY_GSM_DEBUG)
      DBG("### ERROR: SSL sockets do not support reading in HEX mode, reading "
          "in ASCII mode");
#endif
      // AT+CCHRECV=<session_id>[,<max_recv_len>]
      sendAT(GF("+CCHRECV="), mux, ',', (uint16_t)size);
      // response is +CCHRECV: DATA, <session_id>,<len>\n<data>
      if (waitResponse(GF("+CCHRECV:")) != 1) { return 0; }
      streamSkipUntil(',');                    // Skip the word "DATA"
      ret_mux      = streamGetIntBefore(',');  // mux/cid (connecion id)
      len_reported = streamGetIntBefore('\n');
    } else {
#ifdef TINY_GSM_USE_HEX
      // <mode> - 3 – read data in HEX form, the max read length is 750
      sendAT(GF("+CIPRXGET=3,"), mux, ',', (uint16_t)size);
#else
      // <mode> - 2 – read data in ASCII, the max read length is 1500
      sendAT(GF("+CIPRXGET=2,"), mux, ',', (uint16_t)size);
#endif
      if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
      //+CIPRXGET: <mode>,<link_num>,<len_read>,<rest_len><data_in_hex>
      //<len_read> Integer type, the length of data that has been read.
      //<rest_len> Integer type, the length of data which has not been read in
      // the buffer.
      streamSkipUntil(',');                    // Skip Rx mode 2/normal or 3/HEX
      ret_mux      = streamGetIntBefore(',');  // mux/cid (connecion id)
      len_reported = streamGetIntBefore(',');
      // ^^ Integer type, the length of data that has been read.
      len_remaining = streamGetIntBefore('\n');
      // ^^ Integer type, the length of data which has not been read in the
      // buffer.
    }

    size_t len_read = 0;
    if (isValidMux(ret_mux)) {
      // Move the data to the socket buffer of the returned mux as long as the
      // returned mux is valid, even if it doesn't match the expected mux.
#ifdef TINY_GSM_USE_HEX
      // CCHRECV returns raw TLS data even when CIPRXGET uses hex mode.
      if (!ssl) {
        len_read = moveCharsFromStreamToFifo<true>(ret_mux, len_reported);
      } else
#endif
      {
        len_read = moveCharsFromStreamToFifo(ret_mux, len_reported);
      }
    }

    if (ssl) {
      if (isValidMux(ret_mux)) {
        // Returns +CCHRECV: {mux},0 after the data
        if (waitResponse(GF("+CCHRECV: ")) == 1) {
          streamSkipUntil('\n');  // skip the mux and error code
          waitResponse();         // final OK
        }
        // we need to check how much is left after the read
        sockets[ret_mux]->sock_available = (uint16_t)modemGetAvailable(ret_mux);
      } else {
        // if the mux is invalid we need to eat everything in the stream
        streamClear();
      }
    } else {
      if (isValidMux(ret_mux)) {
        // the read call already told us how much is left
        sockets[ret_mux]->sock_available = len_remaining;
      }
      waitResponse();  // ending OK after non-SSL request; the waitResponse
                       // function will toss all the characters before the OK if
                       // the mux was invalid
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
    bool   ssl    = sockets[mux]->is_secure;
    size_t result = 0;
    if (ssl) {
      // NOTE: Only two SSL sockets are supported (0 and 1) and AT+CCHRECV?
      // returns the number of characters availalable on both.
      sendAT(GF("+CCHRECV?"));
      // +CCHRECV: LEN,<cache_len_0>,<cache_len_1>
      // <cache_len_0> = The length of RX data cached for connection 0.
      // <cache_len_1> = The length of RX data cached for connection 1.
      if (waitResponse(GF("+CCHRECV: ")) != 1) { return 0; }
      streamSkipUntil(',');                        // Skip the text "LEN"
      size_t len_on_0 = streamGetIntBefore(',');   // read cache_len_0
      size_t len_on_1 = streamGetIntBefore('\n');  // read cache_len_1

      // set the sock available for both sockets (if they exist)
      if (mux == 1) {
        result                 = len_on_1;
        GsmClientSim7600* sock = sockets[mux];
        if (sock) { sock->sock_available = len_on_1; }
      } else if (mux == 0) {
        result                 = len_on_0;
        GsmClientSim7600* sock = sockets[mux];
        if (sock) { sock->sock_available = len_on_0; }
      } else {
        DBG("### ERROR: Invalid mux number");
        result = 0;
      }
    } else {
      sendAT(GF("+CIPRXGET=4,"), mux);
      if (waitResponse(GF("+CIPRXGET:")) == 1) {
        streamSkipUntil(',');                       // Skip returned mode (4)
        int16_t ret_mux = streamGetIntBefore(',');  // mux
        result          = streamGetIntBefore('\n');
        if (isValidMux(ret_mux)) {
          // set the sock available as long as the received mux is valid
          sockets[ret_mux]->sock_available = result;
        }
        // but if we somehow got an unexpected mux, set the return result to 0
        if (!isExpectedMux(ret_mux, mux)) { result = 0; }
      }
    }
    waitResponse();  // final ok
    // DBG("### Available:", result, "on", mux);
    if (result == 0) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    if (sockets[mux]->is_secure) {
      sendAT(GF("+CCHOPEN?"));
      // If connect to a server, it will show the connected information.
      // Otherwise, the connected information is empty.
      // +CCHOPEN: 0,”<host>”,<port>,<client_type>,[<bind_port>]
      // +CCHOPEN: 1,”<host>”,<port>,<client_type>,[<bind_port>]
      // […]
      // OK
      // <client_type> The type of client:
      //   - 1 – TCP client.
      //   - 2 – SSL/TLS client.
      if (waitResponse(GF("+CCHOPEN:")) != 1) { return false; }
      // for (uint8_t muxNo = 0; muxNo < SslConfig::kMuxCount; muxNo++) {
      for (uint8_t muxNo = 0; muxNo < 2; muxNo++) {
        String state = stream.readStringUntil('\n');
        if (sockets[muxNo] && sockets[muxNo]->is_secure) {
          // if there's at least one comma, there's connection info which
          // means it's connected
          sockets[muxNo]->sock_connected = (state.indexOf(',') > 0);
        }
      }
    } else {
      sendAT(GF("+CIPOPEN?"));
      // If connect to a server, it will show the connected information.
      // Otherwise, the connected information is empty.
      // +CIPOPEN:<mux>,<State or blank...>
      // +CIPOPEN: <link_num>[,<type>,<serverIP>,<serverPort>,<index>]
      // +CIPOPEN: <link_num>[,<type>,<serverIP>,<serverPort>,<index>]
      // +CIPOPEN: <link_num> // no further content for closed connections
      // […]
      // OK
      if (waitResponse(GF("+CIPOPEN:")) != 1) { return false; }
      for (uint8_t muxNo = 0; muxNo < TcpConfig::kMuxCount; muxNo++) {
        String state = stream.readStringUntil('\n');
        if (sockets[muxNo] && !sockets[muxNo]->is_secure) {
          // if there's at least one comma, there's connection info which
          // means it's connected
          sockets[muxNo]->sock_connected = (state.indexOf(',') > 0);
        }
      }
    }

    waitResponse();  // Should be an OK at the end

    return sockets[mux]->sock_connected;
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
    } else if (data.endsWith(GF("+IPCLOSE:"))) {
      int16_t mux = streamGetIntBefore(',');
      streamSkipUntil('\n');  // Skip the reason code
      if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
      data = "";
      DBG("### Closed: ", mux);
      return true;
    } else if (data.endsWith(GF("+CIPEVENT:"))) {
      // Need to close all open sockets and release the network library.
      // User will then need to reconnect.
      DBG("### Network error!");
      if (!isDataConnected()) { dataDisconnect(); }
      data = "";
      return true;
    }
    return false;
  }

 public:
  /// Stream used to communicate with the modem.
  Stream& stream;

 protected:
  GsmClientSim7600* sockets[TcpConfig::kMuxCount];
  // TODO(SRGD): I suspect we need to have two separate socket arrays, a secure
  // and not secure one
};

// cspell:words CCHSEND CCHSTOP CCHOPEN

#endif  // SRC_TINYGSMCLIENTSIM7600_H_
