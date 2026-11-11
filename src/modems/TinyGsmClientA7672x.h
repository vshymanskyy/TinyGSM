/**
 * @file       TinyGsmClientA7672x.h
 * @brief      A7672x modem client and modem-trait definitions.
 * @author     Giovanni de Rosso Unruh
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2022 Giovanni de Rosso Unruh
 * @date       Oct 2022
 */
/**
 * @defgroup simcom_a7672x SIMCom A7672 Series
 * @ingroup group_simcom
 * @brief Manufacturer: SIMCom. Models: A7672x (A7672E, A7672SA, A7672S,
 * A7672G)
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
 * @todo In GsmClientA7672X constructor: Ensure the secure socket mux isn't out
 * of range
 * @todo In `getLocalIPSecure()`: figure out when to use each command properly
 * @todo In `deleteCertificateImpl()`: test
 * @todo In `dataDisconnectImpl()`: Should CCHSTOP come before NETCLOSE?  Is it
 * needed in addition to NETCLOSE?
 * @todo In `connect()` (secure path): verify the socket returned by CCHOPEN
 * @todo In `modemSend()`: make sure requested and confirmed bytes match
 */

#ifndef SRC_TINYGSMCLIENTA7672X_H_
#define SRC_TINYGSMCLIENTA7672X_H_
#pragma message("TinyGSM:  TinyGsmClientA7672x")

#include "mixins/TinyGsmModem.tpp"
#include "mixins/TinyGsmTCP.tpp"
#include "mixins/TinyGsmSSL.tpp"
#include "mixins/TinyGsmCellularData.tpp"
#include "mixins/TinyGsmCalling.tpp"
#include "mixins/TinyGsmSMS.tpp"
#include "mixins/TinyGsmGSMLocation.tpp"
#include "mixins/TinyGsmTime.tpp"
#include "mixins/TinyGsmNTP.tpp"
#include "mixins/TinyGsmBattery.tpp"
#include "mixins/TinyGsmTemperature.tpp"

/// Registration status
/// @ingroup simcom_a7672x
enum class A7672XRegStatus {
  REG_NO_RESULT    = -1,  ///< No registration result
  REG_UNREGISTERED = 0,   ///< Not registered on the network
  REG_SEARCHING    = 2,   ///< Searching for network
  REG_DENIED       = 3,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered on the home network
  REG_OK_ROAMING   = 5,   ///< Registered on a roaming network
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/// Basic modem configurations for SIMCom A7672 modules
/// @ingroup simcom_a7672x
struct TinyGsmA7672XModemConfig
    : public TinyGsmModemConfigPreset<A7672XRegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "SIMCom";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "A7672x";
};

constexpr char TinyGsmA7672XModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmA7672XModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for SIMCom A7672 modules.
 *
 * The module supports 10 TCP sockets or 2 SSL sockets.
 *
 * @todo I think the number of TCP and SSL sockets are independent of each other
 * and managed completely differently.  That is, I think there can be two
 * connection 0's, one using the SSL application on the module and the other
 * using the TCP application on the module.  Could someone who has a A7672x
 * module test the real number of TCP and SSL connections that can be made at
 * once?
 *
 * @todo Handle the different number of sockets for TCP and SSL in the code.
 *
 * The secure send data command, CCHSEND, can handle up to 2048 bytes of input,
 * but the unsecured CIPSEND command will only accept 1500, so we'll take the
 * smaller number as the maximum send size.
 * @ingroup simcom_a7672x
 */
struct TinyGsmA7672XTcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 10> {};

/**
 * @brief SSL behavior and limits for SIMCom A7672 modules.
 * The module supports 10 SSL contexts.
 * @ingroup simcom_a7672x
 */
struct TinyGsmA7672XSslConfig : public TinyGsmSslConfigPreset<
                                    /*contextCount*/ 10> {};

/**
 * @brief Class for the SIMCom A7672X modem, which is a 4G LTE Cat-M1/NB-IoT
 * modem with GPS and SSL support.
 * @ingroup simcom_a7672x
 */
class TinyGsmA7672X
    : public TinyGsmModem<TinyGsmA7672X, TinyGsmA7672XModemConfig>,
      public TinyGsmCellularData<TinyGsmA7672X>,
      public TinyGsmTCP<TinyGsmA7672X, TinyGsmA7672XTcpConfig>,
      public TinyGsmSSL<TinyGsmA7672X, TinyGsmA7672XSslConfig>,
      public TinyGsmCalling<TinyGsmA7672X>,
      public TinyGsmSMS<TinyGsmA7672X>,
      public TinyGsmGSMLocation<TinyGsmA7672X>,
      public TinyGsmTime<TinyGsmA7672X>,
      public TinyGsmNTP<TinyGsmA7672X>,
      public TinyGsmBattery<TinyGsmA7672X>,
      public TinyGsmTemperature<TinyGsmA7672X> {
  friend class TinyGsmModem<TinyGsmA7672X, TinyGsmA7672XModemConfig>;
  friend class TinyGsmCellularData<TinyGsmA7672X>;
  friend class TinyGsmTCP<TinyGsmA7672X, TinyGsmA7672XTcpConfig>;
  friend class GsmClient<TinyGsmA7672X, TinyGsmA7672XTcpConfig>;
  friend class TinyGsmSSL<TinyGsmA7672X, TinyGsmA7672XSslConfig>;
  friend class TinyGsmCalling<TinyGsmA7672X>;
  friend class TinyGsmSMS<TinyGsmA7672X>;
  friend class TinyGsmGSMLocation<TinyGsmA7672X>;
  friend class TinyGsmTime<TinyGsmA7672X>;
  friend class TinyGsmNTP<TinyGsmA7672X>;
  friend class TinyGsmBattery<TinyGsmA7672X>;
  friend class TinyGsmTemperature<TinyGsmA7672X>;

 public:
  using ModemConfig = TinyGsmA7672XModemConfig;
  using TcpConfig   = TinyGsmA7672XTcpConfig;
  using SslConfig   = TinyGsmA7672XSslConfig;
  using GsmSecureContext =
      TinyGsmSSL<TinyGsmA7672X, TinyGsmA7672XSslConfig>::gsmSecureContext;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup simcom_a7672x
  class GsmClientA7672X
      : public GsmClient<TinyGsmA7672X, TinyGsmA7672XTcpConfig> {
    friend class TinyGsmA7672X;

   public:
    using GsmClient<TinyGsmA7672X, TinyGsmA7672XTcpConfig>::connect;
    using GsmClient<TinyGsmA7672X, TinyGsmA7672XTcpConfig>::stop;
    using TcpConfig = TinyGsmA7672XTcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientA7672X() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is identical to the identifier
     * the modem uses internally to identify the socket for the A7672x.
     *
     * @note The A7672x allows you choose the multiplexing channel number, but
     * if the input mux channel number is already in use and other mux channels
     * are available, this library will select the next available one.  Use the
     * getMux() function to get the assigned multiplexing channel number after a
     * successful connection.
     */
    explicit GsmClientA7672X(TinyGsmA7672X& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmA7672X, TinyGsmA7672XTcpConfig>(modem, mux) {
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
  /// @ingroup simcom_a7672x
  class GsmClientSecureA7672X
      : public GsmClientSecure<GsmClientA7672X, TinyGsmA7672X> {
    friend class TinyGsmA7672X;

   public:
    using GsmClientSecure<GsmClientA7672X, TinyGsmA7672X>::init;
    using GsmClientA7672X::connect;
    using GsmClientA7672X::stop;
    using TcpConfig = TinyGsmA7672XTcpConfig;

    /*
     * Secure client constructors and initialization
     */

    /**
     * @brief Create a new secured TCP (SSL) client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSecureA7672X() : GsmClientSecure() {}

    /**
     * @brief Create a new secured TCP (SSL) client and bind it to a modem and
     * optionally a multiplexing channel.
     * @copydetails GsmClientA7672X::GsmClientA7672X(TinyGsmA7672X&, uint8_t)
     */
    explicit GsmClientSecureA7672X(TinyGsmA7672X& modem, uint8_t mux = 0)
        : GsmClientSecure() {
      init(&modem, mux);
    }

    /**
     * @brief Create a new secured TCP (SSL) client with an explicit SSL
     * context and bind it to a modem and multiplexing channel.
     * @param modem The modem that owns the socket and SSL contexts.
     * @param mux The socket identifier.
     * @param sslCtxIndex The hardware SSL context index.
     */
    explicit GsmClientSecureA7672X(TinyGsmA7672X& modem, uint8_t mux,
                                   uint8_t sslCtxIndex)
        : GsmClientSecure() {
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
    bool init(TinyGsmA7672X* modem, uint8_t mux = 0,
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
        DBG("### The A7672x does not support SSL using pre-shared keys.");
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
  /// @ingroup simcom_a7672x
  class GsmSecureContextA7672X
      : public ::GsmSecureContext<TinyGsmA7672X, TinyGsmA7672XSslConfig> {
    friend class TinyGsmA7672X;

   public:
    using ::GsmSecureContext<TinyGsmA7672X,
                             TinyGsmA7672XSslConfig>::GsmSecureContext;

    /// Unregister this context from its modem.
    ~GsmSecureContextA7672X() {}
  };

  /*
   * Modem-scoped Client Aliases
   */
 public:
  /// Arduino Client type for this modem.
  using TinyGsmClient = GsmClientA7672X;
  /// Secure Arduino Client type for this modem.
  using TinyGsmClientSecure = GsmClientSecureA7672X;
  /// Security-context type for this modem.
  using TinyGsmSecureContext = GsmSecureContextA7672X;

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmA7672X(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  A7672x"));

    if (!testAT(2000)) { return false; }

    // sendAT(GF("&FZ"));  // Factory + Reset
    // waitResponse();

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("V1"));  // turn on verbose error codes
#else
    sendAT(GF("V0"));  // turn off error codes
#endif
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

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
    sendAT(GF("&F"));  // Factory + Reset
    waitResponse();
    sendAT(GF("+IFC=0,0"));  // No Flow Control
    waitResponse();
    sendAT(GF("+ICF=2,2"));  // 8 data 0 parity 1 stop
    waitResponse();
    sendAT(GF("+CSCLK=0"));  // Control UART Sleep always work
    waitResponse();
    sendAT(GF("&W"));  // Write configuration
    return waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin) {
    if (!testAT()) { return false; }
    sendAT(GF("+CRESET"));
    waitResponse();
    if (!setPhoneFunctionality(0)) { return false; }
    if (!setPhoneFunctionality(1, true)) { return false; }
    delay(3000);
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOF"));
    return waitResponse(10000L) == 1;
  }

  //  This command is used to enable UART Sleep or always work. If set to 0,
  //  UART always work. If set to 1, ensure that DTR is pulled high and the
  //  module can go to DTR sleep. If set to 2, the module will enter RXsleep. RX
  //  wakeup directly sends data through the serial port (for example: AT) to
  //  wake up
  bool sleepEnableImpl(bool enable) {
    sendAT(GF("+CSCLK="),
           enable ? '2' : '1');  // 2: RXsleep (at wakeup) 1: DTR sleep
    return waitResponse() == 1;
  }

  // <fun> 0 minimum functionality
  // <fun> 1 full functionality, online mode
  // <fun> 4 disable phone both transmit and receive RF circuits
  // <fun> 5 Factory Test Mode (The A7600's 5 and 1 have the same function)
  // <fun> 6 Reset
  // <fun> 7 Offline Mode
  // <rst> 0 do not reset the ME before setting it to <fun> power level
  // <rst> 1 reset the ME before setting it to <fun> power level. This
  // valueonlytakes effect when <fun> equals 1
  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : ",0");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 public:
  /**
   * @brief Get the local IP address of the modem when using a secure
   * connection.
   * @return The local IP address as a String.
   */
  String getLocalIPSecure() {
    // TODO: figure out when to use each command properly
    // AT+CCHADDR is used to get the IPv4 address after calling AT+CCHSTART (ie,
    // when using the SSL application on the module) AT+CGPADDR is used to get
    // the IPv4 address for the packet domain service (PDP)
    sendAT(GF("+CCHADDR"));
    if (waitResponse(GF("+CCHADDR:")) != 1) { return ""; }
    streamSkipUntil(',');  // Skip context id
    String res = stream.readStringUntil('\r');
    if (waitResponse() != 1) { return ""; }
    return res;
  }

 protected:
  A7672XRegStatus getRegistrationStatusImpl() {
    return static_cast<A7672XRegStatus>(getRegistrationStatusXREG("CREG"));
  }

  bool isNetworkConnectedImpl() {
    A7672XRegStatus s = this->getRegistrationStatus();
    return (s == A7672XRegStatus::REG_OK_HOME ||
            s == A7672XRegStatus::REG_OK_ROAMING);
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
 protected:
  // The name of the certificate/key/password file. The file name must
  // have type like ".pem" or ".der".
  // The certificate like - const char ca_cert[] PROGMEM =  R"EOF(-----BEGIN...
  // len of certificate like - sizeof(ca_cert)
  // NOTE: Uploading the certificate only happens by filename, the type of
  // certificate does not matter here
  template <typename TCert>
  bool loadCertificateImpl(const char* certificateName, TCert cert,
                           const uint16_t len) {
    sendAT(GF("+CCERTDOWN=\""), certificateName, GF("\","), len);
    if (waitResponse(5000L, GF(">")) != 1) { return false; }
    this->streamWrite(cert, len);
    stream.flush();
    return waitResponse(5000L) == 1;
  }

  // NOTE: Deleting the certificate only happens by filename, the type of
  // certificate does not matter here
  bool deleteCertificateImpl(const char* certificateName) {  // todo test
    sendAT(GF("+CCERTDELE="), certificateName);
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
  // No functions of this type supported (but the modem does support WiFi)

  /*
   * Cellular data functions
   */
 protected:
  bool dataConnectImpl(const char* apn, const char*, const char*) {
    dataDisconnect();

    // Define the PDP context
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Activate the PDP context
    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    // Attach to Packet Domain service
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // Get Local IP Address, only assigned after connection
    sendAT(GF("+CGPADDR=1"));
    if (waitResponse(10000L) != 1) { return false; }

#if TINY_GSM_USE_PUBLIC_DNS
    // Configure Domain Name Server (DNS)
    sendAT(GF("+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\""));
    if (waitResponse() != 1) { return false; }
#endif

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

    // Start the TCP/IP socket service

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
    // Shut the TCP/IP connection
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
  SimStatus getSimStatusImpl(uint32_t timeout_ms) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      sendAT(GF("+CPIN?"));
      if (waitResponse(GF("+CPIN:")) != 1) {
        delay(1000);
        continue;
      }
      int8_t status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"),
                                   GF("SIM not inserted"), GF("SIM REMOVED"));
      waitResponse();
      switch (status) {
        case 2:
        case 3: return SIM_LOCKED;
        case 1: return SIM_READY;
        default: return SIM_ERROR;
      }
    }
    return SIM_ERROR;
  }

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
 public:
  /**
   * @brief Set the busy status of the modem for calling.
   * @param busy True to set the modem as busy, false to set it as not busy.
   * @return True if the operation was successful, false otherwise.
   */
  bool setGsmBusy(bool busy = true) {
    sendAT(GF("+CCFC=1,"), busy ? 1 : 0);
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
  // No functions of this type supported
  /*
   * GPS/GNSS/GLONASS location functions
   */
  // No functions of this type supported

  /*
   * Time functions
   */
  // No functions of this type supported
  /*
   * NTP server functions
   */
  // No functions of this type supported

  /*
   * BLE functions
   */
  // No functions of this type supported, but the module does support
  // Bluetooth/BLE

  /*
   * Battery functions
   */
 protected:
  // AT+CBC reports the power supply voltage in volts instead of millivolts.
  int16_t getBattVoltageImpl() {
    sendAT(GF("+CBC"));
    // Return -9999 for no response
    if (waitResponse(GF("+CBC:")) != 1) { return -9999; }
    // get voltage in volts
    float voltage = streamGetFloatBefore('\n');
    // Wait for final OK
    bool validResponse = waitResponse() == 1;
    // Return -9999 for an invalid value
    if (!validResponse || voltage <= 0.0F || voltage > 32.767F) {
      return -9999;
    }
    // Return millivolts, adding 0.5 for rounding
    return static_cast<int16_t>(voltage * 1000.0F + 0.5F);
  }

  int8_t getBattPercentImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  int8_t getBattChargeStateImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    // the only battery value we can get is voltage, so defer to
    // getBattVoltageImpl() and set the others to default invalid values
    chargeState = -99;
    percent     = -99;
    milliVolts  = getBattVoltageImpl();
    return milliVolts != -9999;
  }

  /*
   * Temperature functions
   */
 protected:
  float getTemperatureImpl() {
    sendAT(GF("+CPMUTEMP"));
    if (waitResponse(GF("+CPMUTEMP:")) != 1) { return -9999.0F; }
    // return temperature in C
    float temp = streamGetFloatBefore('\n');
    // Wait for final OK
    if (waitResponse() != 1 || temp == -9999.0F) { return -9999.0F; }
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
    // Because the auth mode used by the A7672x is different than the
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

    // enable SSL SNI (server name indication)
    // AT+CSSLCFG="enableSNI",<ssl_ctx_index>,<enableSNI_flag>
    sendAT(GF("+CSSLCFG=\"enableSNI\","), context_id, GF(",1"));
    success &= waitResponse(2000L) == 1;

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
    // set the connection identifier that the above SSL context settings
    // apply to (ie, tie connection mux to SSL context)
    // AT+CCHSSLCFG=<session_id>,<ssl_ctx_index>
    sendAT(GF("+CCHSSLCFG="), mux, ',', context_id);
    return waitResponse(2000L) == 1;
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

    // Configure TCP heartbeat
    // +CTCPKA:<keepalive>,<keepidle>,<keepcount>,<keepinterval>
    sendAT(GF("+CTCPKA=1,2,5,1"));
    if (waitResponse(2000L) != 1) { return false; }

    if (ssl) {
      // If we have a secure socket, use a static cast to get the context index.
      // This isn't really "safe" but since we've already checked that the
      // socket is a secure one, we're pretty sure of the type and it should
      // work.
      const GsmClientSecureA7672X* thisClient =
          static_cast<const GsmClientSecureA7672X*>(sockets[mux]);
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

  bool modemStop(uint8_t mux, uint32_t /*maxWaitMs*/) {
    if (!isValidMux(mux)) { return false; }
    bool ssl = sockets[mux]->is_secure;
    if (ssl) {
      sendAT(GF("+CCHCLOSE="), mux);  //, GF(",1"));  // Quick close
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
    int16_t ret_mux       = 0;
    size_t  len_read      = 0;

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
      streamSkipUntil(',');                    // Skip Rx mode 2/normal or 3/HEX
      ret_mux      = streamGetIntBefore(',');  // mux/cid (connecion id)
      len_reported = streamGetIntBefore(',');
      // ^^ Integer type, the length of data that has been read.
      len_remaining = streamGetIntBefore('\n');
      // ^^ Integer type, the length of data which has not been read in the
      // buffer.
    }
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
        // reading the characters should have cleared the stream, now check how
        // much is left after the read
        sockets[ret_mux]->sock_available = modemGetAvailable(ret_mux);
      } else {
        // there's no OK or closing notation for SSL reads, so if the mux is
        // invalid we need to eat the rest of the characters in the stream
        streamClear();
      }
    } else {
      if (isValidMux(ret_mux)) {
        // for non-SSL if the mux is what we expected then the read call already
        // told us how much is left
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
      if (waitResponse(GF("+CCHRECV:")) != 1) { return 0; }
      streamSkipUntil(',');                        // Skip the text "LEN"
      size_t len_on_0 = streamGetIntBefore(',');   // read cache_len_0
      size_t len_on_1 = streamGetIntBefore('\n');  // read cache_len_1

      // set the sock available for both sockets (if they exist)
      if (mux == 1) {
        result                = len_on_1;
        GsmClientA7672X* sock = sockets[mux];
        if (sock) { sock->sock_available = len_on_1; }
      } else if (mux == 0) {
        result                = len_on_0;
        GsmClientA7672X* sock = sockets[mux];
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
#if 0
    if (!isValidMux(mux)) { return false; }
    // TODO(SRGD): Does this work?  It's not the right command by the manual
    int8_t res = 0;
    bool   ssl = sockets[mux]->is_secure;
    if (ssl) {
      bool connected = this->sockets[mux]->sock_connected;
      // DBG("### Connected:", connected);
      return connected;
    } else {
      // Query TCP Connection Data Transmitting Status
      sendAT(GF("+CIPACK="), mux);
      // +CIPACK: <sent_data_size>,<ack_data_size>,<recv_data_size>
      waitResponse(GF("+CIPACK:"));
      res = waitResponse(2000L);
      waitResponse();
    }
    return 1 == res;
#endif
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
    } else if (data.endsWith(GF("RECV EVENT\r\n"))) {
#if USE_SUBMITTED_A7672X
      // WHAT??? No, no, no, you can't issue a sendAT/waitResponse here!! The
      // handle URC's function is the module-unique part of the general purpose
      // waitResponse function.
      // TODO: This is a problem, we can't issue a sendAT/waitResponse here.
      // Totally sloppy QC on my part to let this through.
      sendAT(GF("+CCHRECV?"));
      String res = "";
      waitResponse(2000L, res);
      int8_t  mux = res.substring(res.lastIndexOf(',') + 1).toInt();
      int16_t len =
          res.substring(res.indexOf(',') + 1, res.lastIndexOf(',')).toInt();
      if (isValidMux(mux)) {
        sockets[mux]->got_data = true;
        if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
      }
      data = "";
      DBG("### Got Data:", len, "on", mux);
      return true;
#endif
      // SRGD Note: I don't have this module to test this, but the above code is
      // what was submitted by a user.  I think if we get this receive event
      // notification without any detail, we should just set the got_data flag
      // and reset the prev_check so the next read or available call will get
      // the data.
      for (uint8_t mux = 0; mux < TcpConfig::kMuxCount; mux++) {
        if (isValidMux(mux)) {
          sockets[mux]->got_data   = true;
          sockets[mux]->prev_check = 0;
        }
      }
      data = "";
      DBG("### Got receive event");
      return true;
    } else if (data.endsWith(GF("+CCHRECV:"))) {
      int16_t mux = streamGetIntBefore(',');
      streamSkipUntil('\n');  // skip the error code
      if (isValidMux(mux)) { sockets[mux]->sock_connected = true; }
      data = "";
      DBG("### ACK:", mux);
      return true;
    } else if (data.endsWith(GF("+IPCLOSE:"))) {
      int16_t mux = streamGetIntBefore(',');
      streamSkipUntil('\n');
      if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
      data = "";
      DBG("### TCP Closed: ", mux);
      return true;
    } else if (data.endsWith(GF("+CCHCLOSE:"))) {
      int16_t mux = streamGetIntBefore(',');
      if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
      data = "";
      streamSkipUntil('\n');
      DBG("### SSL Closed: ", mux);
      return true;
    } else if (data.endsWith(GF("+CCH_PEER_CLOSED:"))) {
      int16_t mux = streamGetIntBefore('\n');
      if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
      data = "";
      DBG("### SSL Closed: ", mux);
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
  GsmClientA7672X* sockets[TcpConfig::kMuxCount];
  // TODO(SRGD): I suspect we need to have two separate socket arrays, a secure
  // and not secure one
};

/// Typedef for backward compatibility
typedef TinyGsmA7672X TinyGsmA7672x;

// cspell:words CCHSEND CCHSET CCHSTART CCHOPEN CCHSTOP

#endif  // SRC_TINYGSMCLIENTA7672X_H_
