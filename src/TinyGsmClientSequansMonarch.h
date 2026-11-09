/**
 * @file       TinyGsmClientSequansMonarch.h
 * @brief      Sequans Monarch modem client and modem-trait definitions.
 * @author     Michael Krumpus
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2019 Michael Krumpus
 * @date       Jan 2019
 */
/**
 * @defgroup sequans_monarch Sequans Monarch
 * @ingroup group_sequans
 * @brief Manufacturer: Sequans. Models: Monarch.
 *
 * # Connection Information
 *
 * - Combined TCP/SSL sockets:
 *   - 6
 *   - Unlike most modules, the sockets are numbered starting at 1, not 0.
 * - SSL contexts (security profiles):
 *   - 6
 * - Socket Buffering:
 *   - The modem has an internal buffer for incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 */

#ifndef SRC_TINYGSMCLIENTSEQUANSMONARCH_H_
#define SRC_TINYGSMCLIENTSEQUANSMONARCH_H_
#pragma message("TinyGSM:  TinyGsmClientSequansMonarch")

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"

// NOTE: This module supports SSL, but we do not support any certificate
// management yet. Define TINY_GSM_MODEM_HAS_SSL here and do not include the SSL
// module.
#ifndef TINY_GSM_MODEM_HAS_SSL
/// flag to indicate that the modem has Secure Socket Layer (SSL) functions
#define TINY_GSM_MODEM_HAS_SSL
#endif

/// The default SSL context to use for SSL connections.
#define TINY_GSM_MONARCH_DEFAULT_SSL_CTX 1

#include "TinyGsmCellularData.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmTemperature.tpp"

/// Registration status
/// @ingroup sequans_monarch
enum class MonarchRegStatus {
  REG_NO_RESULT    = -1,  ///< No registration result
  REG_UNREGISTERED = 0,   ///< Not registered on the network
  REG_SEARCHING    = 2,   ///< Searching for network
  REG_DENIED       = 3,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered on the home network
  REG_OK_ROAMING   = 5,   ///< Registered on a roaming network
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/// Basic modem configurations for the Sequans Monarch
/// @ingroup sequans_monarch
struct TinyGsmSequansMonarchModemConfig
    : public TinyGsmModemConfigPreset<MonarchRegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "Sequans";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "Monarch";
};

constexpr char TinyGsmSequansMonarchModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmSequansMonarchModemConfig::MODEM_MODEL[]
    __attribute__((weak));

/**
 * @brief TCP behavior and limits for the Sequans Monarch.
 *
 * @note Unlike most modules, the sockets are numbered starting at 1, not 0.
 *
 * The send data command, SQNSSENDEXT, accepts up to 1500 bytes of input, but
 * this is configured to send HEX, so only 1/2 of that is available.
 * @ingroup sequans_monarch
 */
struct TinyGsmSequansMonarchTcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 6,
          /*sendMaxSize*/ 750> {};

#if 0
/**
 * @brief SSL behavior and limits for the Sequans Monarch.
 * The module supports 6 security profiles / SSL contexts, but this library does
 * not support any functions to configure a profile or upload certificates.  The
 * SSL context can only be selected by modifying the
 * TINY_GSM_MONARCH_DEFAULT_SSL_CTX variable.
 * @ingroup sequans_monarch
 */
struct TinyGsmSequansMonarchSslConfig : public TinyGsmSslConfigPreset<
                                   /*contextCount*/ 6> {};
#endif

/// Socket status
enum class SocketStatus {
  SOCK_CLOSED                 = 0,  ///< Socket is closed
  SOCK_ACTIVE_DATA            = 1,  ///< Socket is active and has data
  SOCK_SUSPENDED              = 2,  ///< Socket is suspended
  SOCK_SUSPENDED_PENDING_DATA = 3,  ///< Socket is suspended with pending data
  SOCK_LISTENING = 4,  ///< Socket is listening for incoming connections
  SOCK_INCOMING  = 5,  ///< Socket has an incoming connection
  SOCK_OPENING   = 6,  ///< Socket is in the process of opening
};

/// Class for the Sequans Monarch
/// @ingroup sequans_monarch
class TinyGsmSequansMonarch
    : public TinyGsmModem<TinyGsmSequansMonarch,
                          TinyGsmSequansMonarchModemConfig>,
      public TinyGsmCellularData<TinyGsmSequansMonarch>,
      public TinyGsmTCP<TinyGsmSequansMonarch, TinyGsmSequansMonarchTcpConfig>,
      public TinyGsmCalling<TinyGsmSequansMonarch>,
      public TinyGsmSMS<TinyGsmSequansMonarch>,
      public TinyGsmTime<TinyGsmSequansMonarch>,
      public TinyGsmTemperature<TinyGsmSequansMonarch> {
  friend class TinyGsmModem<TinyGsmSequansMonarch,
                            TinyGsmSequansMonarchModemConfig>;
  friend class TinyGsmCellularData<TinyGsmSequansMonarch>;
  friend class TinyGsmTCP<TinyGsmSequansMonarch,
                          TinyGsmSequansMonarchTcpConfig>;
  friend class GsmClient<TinyGsmSequansMonarch, TinyGsmSequansMonarchTcpConfig>;
  friend class TinyGsmCalling<TinyGsmSequansMonarch>;
  friend class TinyGsmSMS<TinyGsmSequansMonarch>;
  friend class TinyGsmTime<TinyGsmSequansMonarch>;
  friend class TinyGsmTemperature<TinyGsmSequansMonarch>;

 public:
  using ModemConfig = TinyGsmSequansMonarchModemConfig;
  using TcpConfig   = TinyGsmSequansMonarchTcpConfig;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup sequans_monarch
  class GsmClientSequansMonarch
      : public GsmClient<TinyGsmSequansMonarch,
                         TinyGsmSequansMonarchTcpConfig> {
    friend class TinyGsmSequansMonarch;

   public:
    using GsmClient<TinyGsmSequansMonarch,
                    TinyGsmSequansMonarchTcpConfig>::connect;
    using GsmClient<TinyGsmSequansMonarch,
                    TinyGsmSequansMonarchTcpConfig>::stop;
    using TcpConfig = TinyGsmSequansMonarchTcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSequansMonarch() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is **NOT** the identifier the
     * Monarch uses internally to identify the socket.  The Monarch uses
     * 1-indexed multiplexing channel numbers, so this library will convert the
     * 0-indexed mux number to a 1-indexed multiplexing channel number for the
     * Monarch.
     *
     * @note The Monarch allows you choose the multiplexing channel number, but
     * if the input mux channel number is already in use and other mux channels
     * are available, this library will select the next available one.  Use the
     * getMux() function to get the assigned multiplexing channel number after a
     * successful connection.
     */
    explicit GsmClientSequansMonarch(TinyGsmSequansMonarch& modem,
                                     uint8_t                mux = 0)
        : GsmClient<TinyGsmSequansMonarch, TinyGsmSequansMonarchTcpConfig>(
              modem, mux) {
      is_secure = false;
      init(&modem, mux);
    }

    /*
     * Arduino Client interface
     */
   public:
    TINY_GSM_STATIC_TCP_CONNECT

    /*
     * Extended Client API
     */
   public:
    // NOTE: Not virtual overrides - GsmClient::beginWrite()/endWrite() are
    // non-virtual, so these simply hide the base versions and the base
    // versions (which reference modemBeginSend()/modemEndSend(), not
    // implemented for this modem) are never instantiated.
    bool beginWrite(uint16_t) TINY_GSM_ATTR_NOT_IMPLEMENTED {
      return false;
    };
    bool endWrite(uint16_t) TINY_GSM_ATTR_NOT_IMPLEMENTED {
      return false;
    };
  };

  /*
   * Inner Secure Client
   */
 public:
  /// Inner secure client
  /// @ingroup sequans_monarch
  class GsmClientSecureSequansMonarch : public GsmClientSequansMonarch {
    friend class TinyGsmSequansMonarch;

   public:
    using GsmClientSequansMonarch::connect;
    using GsmClientSequansMonarch::stop;
    using TcpConfig = TinyGsmSequansMonarchTcpConfig;

    /*
     * Secure client constructors and initialization
     */
    /**
     * @brief Create a new secured TCP (SSL) client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSecureSequansMonarch() {
      is_secure = true;
    }
    /**
     * @brief Create a new secured TCP (SSL) client and bind it to a modem and
     * optionally a multiplexing channel.
     * @copydetails GsmClientSequansMonarch::GsmClientSequansMonarch(
     * TinyGsmSequansMonarch&, uint8_t)
     */
    explicit GsmClientSecureSequansMonarch(TinyGsmSequansMonarch& modem,
                                           uint8_t                mux = 0)
        : GsmClientSequansMonarch(modem, mux) {
      is_secure = true;
    }

   protected:
    bool    strictSSL   = false;
    uint8_t sslCtxIndex = TINY_GSM_MONARCH_DEFAULT_SSL_CTX;

   public:
    /*
     * Arduino Client interface
     */
    int connect(const char* host, uint16_t port, int timeout_s) override {
      if (at == nullptr) { return 0; }
      stop(TcpConfig::kStopTimeoutS * 1000L);
      rx.clear();

      // configure security profile 1 with parameters:
      // +SQNSPCFG:<spId>,<version>,<cipherSpecs>,<certValidLevel>,
      //           <caCertificateID>,<clientCertificateID>,
      //           <clientPrivateKeyID>,<psk>
      if (strictSSL) {
        // require minimum of TLS 1.2 (3)
        // only support cipher suite 0x3D: TLS_RSA_WITH_AES_256_CBC_SHA256
        // verify server certificate against imported CA certs 0 and enforce
        // validity period (3)
        at->sendAT(GF("+SQNSPCFG="), sslCtxIndex,
                   GF(",3,\"0x3D\",3,0,,,\"\",\"\""));
      } else {
        // use TLS 1.0 or higher (1)
        // support wider variety of cipher suites
        // do not verify server certificate (0)
        at->sendAT(GF("+SQNSPCFG="), sslCtxIndex,
                   GF(",1,\"0x2F;0x35;0x3C;0x3D\",0,,,,\"\",\"\""));
      }
      if (at->waitResponse() != 1) {
        DBG("failed to configure security profile");
        return false;
      }

      sock_connected = at->modemConnect(host, port, mux, timeout_s);
      return sock_connected;
    }

    /*
     * Secure client configuration
     */

    /**
     * @brief Require minimum of TLS 1.2
     * @param strict True to require TLS 1.2, false to allow lower versions.
     */
    void setStrictSSL(bool strict) {
      strictSSL = strict;
    }
  };

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmSequansMonarch(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSequansMonarch"));

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

    // Make sure the module is enabled. Unlike others, the VZN20Q powers on
    // with CFUN=0 not CFUN=1 (that is, at minimum functionality instead of full
    // functionality The module cannot even detect the sim card if the cellular
    // functionality is disabled so unless we explicitly enable the
    // functionality the init will fail.
    sendAT(GF("+CFUN=1"));
    waitResponse();

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

  bool factoryDefaultImpl() {
    sendAT(GF("&F0"));  // Factory
    waitResponse();
    sendAT('Z');  // default configuration
    waitResponse();
    sendAT(GF("+IPR=0"));  // Auto-baud
    return waitResponse() == 1;
  }

  void maintainImpl() {
    bool check_connections = false;
    for (uint8_t mux = 0; mux < TcpConfig::kMuxCount; mux++) {
      GsmClientSequansMonarch* sock = sockets[mux];
      if (sock && sock->got_data) {
        sock->got_data       = false;
        sock->sock_available = modemGetAvailable(mux);
        check_connections    = true;
      }
    }
    if (check_connections) {
      // modemGetConnected() always checks the state of ALL socks
      modemGetConnected(1);
    }
    while (stream.available()) { waitResponse(15, nullptr, nullptr); }
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin) {
    if (!testAT()) { return false; }

    sendAT(GF("+CFUN=0"));
    int8_t res = waitResponse(20000L, GFP(ModemConfig::GSM_OK),
                              GFP(ModemConfig::GSM_ERROR), GF("+SYSSTART"));
    if (res != 1 && res != 3) { return false; }

    sendAT(GF("+CFUN=1,1"));
    res = waitResponse(20000L, GF("+SYSSTART"), GFP(ModemConfig::GSM_ERROR));
    if (res != 1 && res != 3) { return false; }
    delay(1000);
    return init(pin);
  }

  bool powerOffImpl() {
    // NOTE:  The only way to turn the modem back on after this shutdown is with
    // a hard reset
    sendAT(GF("+SQNSSHDN"));
    return waitResponse();
  }

  //  When power saving is enabled, UART0 interface is activated with sleep mode
  //  support. Module power state is controlled by RTS0 line.  When no activity
  //  on UART, CTS line will be set to OFF state (driven high level) <timeout>
  //  milliseconds (100ms to 10s, default 5s) after the last sent character,
  //  then module will go to sleep mode as soon as DTE set RTS line to OFF state
  //  (driver high level).
  bool sleepEnableImpl(bool enable) {
    sendAT(GF("+SQNIPSCFG="), enable);
    return waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun,
                                 bool    reset) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 protected:
  MonarchRegStatus getRegistrationStatusImpl() {
    return static_cast<MonarchRegStatus>(getRegistrationStatusXREG("CEREG"));
  }

  bool isNetworkConnectedImpl() {
    MonarchRegStatus s = this->getRegistrationStatus();
    return (s == MonarchRegStatus::REG_OK_HOME ||
            s == MonarchRegStatus::REG_OK_ROAMING);
  }
  String getLocalIPImpl() {
    sendAT(GF("+CGPADDR=3"));
    if (waitResponse(10000L, GF("+CGPADDR: 3,\"")) != 1) { return ""; }
    String res = stream.readStringUntil('\"');
    waitResponse();
    return res;
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
  // This module supports SSL, but no certificate management functions are
  // supported yet.
  // If you wish to add certificate management for this module you must (in
  // addition to adding the functions here):
  //  - Add `#include "TinyGsmSSL.tpp` to the top of the file
  //  - Remove `#define TINY_GSM_MODEM_HAS_SSL` from the top of the file
  //  - Remove the #if 0 / #endif statements bracketing the SSL configuration
  //  structure.
  //  - Add `public TinyGsmSSL<TinyGsmSequansMonarch,
  //  TinyGsmSequansMonarchSslConfig>,` to the constructor's initializer list
  //  - Add `friend class TinyGsmSSL<TinyGsmSequansMonarch,
  //  TinyGsmSequansMonarchSslConfig>;` to the friend list
  //  - Make the secure client inherit from the secure client class in the SSL
  //  template.
  //  - Create an inner secure context class inheriting from
  //  TinyGsmSSLContext<TinyGsmSequansMonarch, TinyGsmSequansMonarchSslConfig>

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

    // Define the PDP context (This uses context #3!)
    sendAT(GF("+CGDCONT=3,\"IPV4V6\",\""), apn, '"');
    waitResponse();

    // Set authentication
    if (user && strlen(user) > 0) {
      sendAT(GF("+CGAUTH=3,1,\""), user, GF("\",\""), pwd, '"');
      waitResponse();
    }

    // Activate the PDP context
    sendAT(GF("+CGACT=1,3"));
    waitResponse(60000L);

    // Attach to Packet Domain service
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  bool dataDisconnectImpl() {
    // Detach from PS network
    sendAT(GF("+CGATT=0"));
    if (waitResponse(60000L) != 1) { return false; }
    // Dectivate all PDP contexts
    sendAT(GF("+CGACT=0"));
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  /*
   * SIM card functions
   */
 protected:
  String getSimCCIDImpl() {
    sendAT(GF("+SQNCCID"));
    if (waitResponse(GF("+SQNCCID:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone call functions
   */
 protected:
  bool callAnswerImpl() TINY_GSM_ATTR_NOT_AVAILABLE;
  bool dtmfSendImpl(char cmd, int duration_ms) TINY_GSM_ATTR_NOT_AVAILABLE;


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
  // No functions of this type supportedp

  /*
   * Temperature functions
   */
 protected:
  float getTemperatureImpl() {
    sendAT(GF("+SMDTH"));
    if (waitResponse(10000L, GF("+SMDTH: ")) != 1) { return -9999.0f; }
    String res;
    if (waitResponse(1000L, res) != 1) { return -9999.0f; }
    if (res.indexOf("ERROR") >= 0) { return -9999.0f; }
    return res.toFloat();
  }

  /*
   * Client-related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t /*static*/ mux,
                    int timeout_s) {
    if (!isValidMux(mux)) { return false; }
    int8_t   rsp;
    uint32_t timeout_ms  = ((uint32_t)timeout_s) * 1000;
    bool     ssl         = sockets[mux]->is_secure;
    uint8_t  sslCtxIndex = TINY_GSM_MONARCH_DEFAULT_SSL_CTX;
    uint32_t startMillis = millis();
    uint8_t  connId      = muxToConnectionId(mux);

    if (ssl) {
      // enable SSL and use security profile 1
      // AT+SQNSSCFG=<connId>,<enable>,<spId>
      sendAT(GF("+SQNSSCFG="), connId, GF(",1,"), sslCtxIndex);
      if (waitResponse() != 1) {
        DBG("### WARNING: failed to configure secure socket");
        return false;
      }
    }

    // Socket configuration
    // AT+SQNSCFG:<connId1>, <cid1>, <pktSz1>, <maxTo1>, <connTo1>, <txTo1>
    // <connId1> = Connection ID = mux
    // <cid1> = PDP context ID = 3 - this is number set up above in the
    // dataConnect function
    // <pktSz1> = Packet Size, used for online data mode only = 300 (default)
    // <maxTo1> = Max timeout in seconds = 90 (default)
    // <connTo1> = Connection timeout in hundreds of milliseconds
    //           = 600 (default)
    // <txTo1> = Data sending timeout in hundreds of milliseconds,
    // used for online data mode only = 50 (default)
    sendAT(GF("+SQNSCFG="), connId, GF(",3,300,90,600,50"));
    waitResponse(5000L);

    // Socket configuration extended
    // AT+SQNSCFGEXT:<connId1>, <srMode1>, <recvDataMode1>, <keepalive1>,
    // <listenAutoRsp1>, <sendDataMode1>
    // <connId1> = Connection ID = mux
    // <srMode1> = Send/Receive URC model = 1 - data amount mode
    // <recvDataMode1> = Receive data mode = 0  - data as text (1 for hex)
    // <keepalive1> = unused = 0
    // <listenAutoRsp1> = Listen auto-response mode = 0 - deactivated
    // <sendDataMode1> = Send data mode = 1  - data as hex (0 for text)
    sendAT(GF("+SQNSCFGEXT="), connId, GF(",1,0,0,0,1"));
    waitResponse(5000L);

    // Socket dial
    // AT+SQNSD=<connId>,<txProt>,<rPort>,<IPaddr>[,<closureType>[,<lPort>[,<connMode>[,acceptAnyRemote]]]]
    // <connId> = Connection ID = mux
    // <txProt> = Transmission protocol = 0 - TCP (1 for UDP)
    // <rPort> = Remote host port to contact
    // <IPaddr> = Any valid IP address in the format xxx.xxx.xxx.xxx or any
    // host name solved with a DNS query
    // <closureType> = Socket closure behaviour for TCP, has no effect for UDP
    //               = 0 - local port closes when remote does (default)
    // <lPort> = UDP connection local port, has no effect for TCP connections.
    // <connMode> = Connection mode = 1 - command mode connection
    // <acceptAnyRemote> = Applies to UDP only
    sendAT(GF("+SQNSD="), connId, GF(",0,"), port, ',', '"', host, '"',
           GF(",0,0,1"));
    rsp = waitResponse((timeout_ms - (millis() - startMillis)),
                       GFP(ModemConfig::GSM_OK), GFP(ModemConfig::GSM_ERROR),
                       GF("NO CARRIER\r\n"));

    // creation of socket failed immediately.
    if (rsp != 1) { return false; }

    // wait until we get a good status
    bool connected = false;
    while (!connected && ((millis() - startMillis) < timeout_ms)) {
      connected = modemGetConnected(mux);
      delay(100);  // socket may be in opening state
    }
    return connected;
  }

  bool modemStop(uint8_t mux, uint32_t /*maxWaitMs*/) {
    if (!isValidMux(mux)) { return false; }
    uint8_t connId = muxToConnectionId(mux);
    // Same command for both secure and non-secure sockets
    sendAT(GF("+SQNSH="), connId);
    return waitResponse() == 1;  // should return within 1s
  }

  size_t modemSend(const uint8_t* buff, size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    if (sockets[mux]->sock_connected == false) {
      DBG("### Sock closed, cannot send data!");
      return 0;
    }

    uint8_t connId = muxToConnectionId(mux);

    // Pointer to where in the buffer we're up to
    // A const cast is need to cast-away the constant-ness of the buffer (ie,
    // modify it).
    uint8_t* txPtr     = const_cast<uint8_t*>(buff);
    size_t   bytesSent = 0;

    do {
      // make no more than 3 attempts at the single send command
      int8_t send_attempts = 0;
      bool   send_success  = false;
      while (send_attempts < 3 && !send_success) {
        // Number of bytes to send from buffer in this command
        size_t sendLength = TcpConfig::kSendMaxSize;
        // Ensure the program doesn't read past the allocated memory
        if (txPtr + TcpConfig::kSendMaxSize >
            const_cast<uint8_t*>(buff) + len) {
          sendLength = const_cast<uint8_t*>(buff) + len - txPtr;
        }

        sendAT(GF("+SQNSSENDEXT="), connId, ',', (uint16_t)sendLength);
        send_success = waitResponse(10000L, GF("\r\n> ")) == 1;
        if (!send_success) {
          send_attempts++;
          continue;
        }

        // Translate bytes into char to be able to send them as a hex string
        static const char hex[] = "0123456789ABCDEF";
        char              char_command[2];

        for (size_t i = 0; i < sendLength; i++) {
          uint8_t b       = txPtr[i];
          char_command[0] = hex[b >> 4];
          char_command[1] = hex[b & 0x0F];
          stream.write(char_command, 2);
        }

        stream.flush();
        send_success = waitResponse() == 1;
        if (send_success) {
          bytesSent += sendLength;  // bump up number of bytes sent
          txPtr += sendLength;      // bump up the pointer
        }
        // The payload is already on the wire. Do not repeat this chunk,
        // because the modem may have accepted it without returning OK.
        break;
      }
      // if we failed after 3 attempts at the same chunk, bail from the whole
      // thing
      if (!send_success) { break; }
    } while (bytesSent < len && sockets[mux]->sock_connected);
    return bytesSent;

    // uint8_t nAttempts = 5;
    // bool gotPrompt = false;
    // while (nAttempts > 0 && !gotPrompt) {
    //   sendAT(GF("+SQNSSEND="), connId);
    //   if (waitResponse(5000, GF("\r\n> ")) == 1) {
    //     gotPrompt = true;
    //   }
    //   nAttempts--;
    //   delay(50);
    // }
    // if (gotPrompt) {
    //   stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    //   stream.write(reinterpret_cast<const char*>(0x1A), 1);
    //   stream.flush();
    //   if (waitResponse() != 1) {
    //     DBG("### no OK after send");
    //     return 0;
    //   }
    //   return len;
    // }
    // return 0;
  }

#if 0
  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    uint8_t connId = muxToConnectionId(mux);
    sendAT(GF("+SQNSSENDEXT="), connId, ',', (uint16_t)len);
    return waitResponse(10000L, GF("\r\n> ")) == 1;
  }
  size_t modemEndSend(size_t len, uint8_t) {
    if (waitResponse() != 1) {
      DBG("### no OK after send");
      return 0;
    }
    return len;
  }
#endif

  size_t modemRead(size_t size, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    uint8_t connId   = muxToConnectionId(mux);
    size_t  len_read = 0;

    sendAT(GF("+SQNSRECV="), connId, ',', (uint16_t)size);
    if (waitResponse(GF("+SQNSRECV: ")) != 1) { return 0; }

    int16_t ret_connId   = streamGetIntBefore(',');        // connection id
    int16_t ret_mux      = connectionIdToMux(ret_connId);  // convert to mux
    int16_t len_reported = streamGetIntBefore('\n');
    if (isValidMux(ret_mux)) {
      // Move the data to the socket buffer of the returned mux as long as the
      // returned mux is valid, even if it doesn't match the expected mux.
      len_read = moveCharsFromStreamToFifo(ret_mux, len_reported);
    }
    waitResponse();  // ending OK; the waitResponse function will toss all the
                     // characters before the OK if the mux was invalid

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
    uint8_t connId = muxToConnectionId(mux);

    sendAT(GF("+SQNSI="), connId);
    size_t  result     = 0;
    int16_t ret_mux    = -1;
    int16_t ret_connId = -1;
    if (waitResponse(GF("+SQNSI:")) == 1) {
      ret_connId = streamGetIntBefore(',');        // connection id
      ret_mux    = connectionIdToMux(ret_connId);  // convert to mux
      streamSkipUntil(',');                        // Skip total sent
      streamSkipUntil(',');                        // Skip total received
      result = streamGetIntBefore(',');            // keep data not yet read
      waitResponse();
    }
    // DBG("### Available:", result, "on", mux);
    if (isExpectedMux(ret_mux, mux)) { return result; }
    return 0;
  }

  bool modemGetConnected(uint8_t mux) {
    // This single command always returns the connection status of all
    // six possible sockets.
    // Because this will give us all the sockets and we want to be able to call
    // it and have it populate all mux connection statuses without knowing which
    // muxes are valid within maintain(), we intentionally don't validate the
    // provided mux until the end.
    sendAT(GF("+SQNSS"));
    for (uint8_t connId = 1; connId <= TcpConfig::kMuxCount; connId++) {
      if (waitResponse(GFP(ModemConfig::GSM_OK), GF("+SQNSS: ")) != 2) {
        break;
      }
      SocketStatus status = SocketStatus::SOCK_CLOSED;
      // if (streamGetIntBefore(',') != connId) { // check the mux no
      //   DBG("### Warning: misaligned mux numbers!");
      // }
      streamSkipUntil(',');  // skip connection ID [use connId]
      status = static_cast<SocketStatus>(stream.parseInt());  // Read the status
      // if mux is in use, will have comma then other info after the status
      // if not, there will be new line immediately after status
      // streamSkipUntil('\n'); // Skip port and IP info
      // SOCK_CLOSED                 = 0,
      // SOCK_ACTIVE_DATA            = 1,
      // SOCK_SUSPENDED              = 2,
      // SOCK_SUSPENDED_PENDING_DATA = 3,
      // SOCK_LISTENING              = 4,
      // SOCK_INCOMING               = 5,
      // SOCK_OPENING                = 6,
      uint8_t muxNo = connectionIdToMux(connId);  // convert to mux
      GsmClientSequansMonarch* sock = sockets[muxNo];
      if (sock) {
        sock->sock_connected = ((status != SocketStatus::SOCK_CLOSED) &&
                                (status != SocketStatus::SOCK_INCOMING) &&
                                (status != SocketStatus::SOCK_OPENING));
      }
    }
    waitResponse();  // Should be an OK at the end
    if (!isValidMux(mux)) { return false; }
    GsmClientSequansMonarch* thisSock = sockets[mux];
    return thisSock ? thisSock->sock_connected : false;
  }

  /*
   * Utilities
   */
 protected:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+SQNSRING:"))) {
      int16_t connId = streamGetIntBefore(',');
      uint8_t mux    = connectionIdToMux(connId);
      int16_t len    = streamGetIntBefore('\n');
      if (isValidMux(mux)) {
        sockets[mux]->got_data       = true;
        sockets[mux]->sock_available = len;
      }
      data = "";
      DBG("### URC Data Received:", len, "on", mux);
      return true;
    } else if (data.endsWith(GF("SQNSH: "))) {
      int16_t connId = streamGetIntBefore('\n');
      uint8_t mux    = connectionIdToMux(connId);
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
  GsmClientSequansMonarch* sockets[TcpConfig::kMuxCount];
  // The modem's NL is \r\n (ModemConfig::GSM_NL) but that is not accepted
  // with SQNSSENDEXT in data mode so use \n
  const char* gsmNL = "\n";

  /**
   * @brief Convert a multiplexing channel number to the modem's internal
   * connection identifier.
   * @param mux The multiplexing channel number (0-indexed).
   * @return The modem's internal connection identifier (1-indexed).
   */
  inline uint8_t muxToConnectionId(uint8_t mux) {
    return mux + 1;
  };
  /**
   * @brief Convert the modem's internal connection identifier to a multiplexing
   * channel number.
   * @param connId The modem's internal connection identifier (1-indexed).
   * @return The multiplexing channel number (0-indexed).
   */
  inline uint8_t connectionIdToMux(uint8_t connId) {
    return connId - 1;
  };
};

// cspell:words SQNSSENDEXT

#endif  // SRC_TINYGSMCLIENTSEQUANSMONARCH_H_
