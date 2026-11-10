/**
 * @file       TinyGsmClientBG96.h
 * @brief      BG96 modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy and Aurelien BOUIN (SSL)
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Apr 2018, Aug 2023 (SSL)
 */
/**
 * @defgroup quectel_bg96 Quectel BG96 and BG95
 * @ingroup group_quectel
 * @brief Manufacturer: Quectel. Models: BG95, BG96.
 *
 * # Connection Information
 *
 * - Combined TCP/SSL sockets:
 *   - 12
 * - SSL contexts:
 *   - 6
 * - Socket Buffering:
 *   - The modem has an internal buffer for incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 *
 * @todo In operator `GsmClientBG96::read()`: Read directly into user buffer?
 * @todo In `modemEndSend()`: Wait for ACK? (AT+QISEND=id,0 or AT+QSSLSEND=id,0)
 * @todo In `modemGetConnected()`: Verify mux
 */

#ifndef SRC_TINYGSMCLIENTBG96_H_
#define SRC_TINYGSMCLIENTBG96_H_
#pragma message("TinyGSM:  TinyGsmClientBG96")

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmCellularData.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGPS.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"
#include "TinyGsmTemperature.tpp"

/// Registration status
/// @ingroup quectel_bg96
enum class BG96RegStatus {
  REG_NO_RESULT    = -1,  ///< No registration result
  REG_UNREGISTERED = 0,   ///< Not registered on the network
  REG_SEARCHING    = 2,   ///< Searching for network
  REG_DENIED       = 3,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered on the home network
  REG_OK_ROAMING   = 5,   ///< Registered on a roaming network
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/// Basic modem configurations for Quectel BG96 and BG95
/// @ingroup quectel_bg96
struct TinyGsmBG96ModemConfig : public TinyGsmModemConfigPreset<BG96RegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "Quectel";
#if defined(TINY_GSM_MODEM_BG95) || defined(TINY_GSM_MODEM_BG95SSL)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "BG95";
#else
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "BG96";
#endif
};

constexpr char TinyGsmBG96ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmBG96ModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for Quectel BG96 and BG95.
 *
 * The BG95/BG96 supports 12 sockets (0-11); any of them can be SSL.
 *
 * When sending  data QISEND and QSSLSEND both accept up to 1460 bytes of input.
 *
 * @ingroup quectel_bg96
 */
struct TinyGsmBG96TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 12,
          /*sendMaxSize*/ 1460,
          /*connectTimeoutS*/ 150> {};

/**
 * @brief SSL behavior and limits for Quectel BG96 and BG95.
 *
 * The module supports 6 SSL contexts (0-5).
 * @ingroup quectel_bg96
 */
struct TinyGsmBG96SslConfig : public TinyGsmSslConfigPreset<
                                  /*contextCount*/ 6> {};

/// Class for the Quectel BG96 and BG95
/// @ingroup quectel_bg96
class TinyGsmBG96 : public TinyGsmModem<TinyGsmBG96, TinyGsmBG96ModemConfig>,
                    public TinyGsmCellularData<TinyGsmBG96>,
                    public TinyGsmTCP<TinyGsmBG96, TinyGsmBG96TcpConfig>,
                    public TinyGsmSSL<TinyGsmBG96, TinyGsmBG96SslConfig>,
                    public TinyGsmCalling<TinyGsmBG96>,
                    public TinyGsmSMS<TinyGsmBG96>,
                    public TinyGsmGPS<TinyGsmBG96>,
                    public TinyGsmTime<TinyGsmBG96>,
                    public TinyGsmNTP<TinyGsmBG96>,
                    public TinyGsmBattery<TinyGsmBG96>,
                    public TinyGsmTemperature<TinyGsmBG96> {
  friend class TinyGsmModem<TinyGsmBG96, TinyGsmBG96ModemConfig>;
  friend class TinyGsmCellularData<TinyGsmBG96>;
  friend class TinyGsmTCP<TinyGsmBG96, TinyGsmBG96TcpConfig>;
  friend class GsmClient<TinyGsmBG96, TinyGsmBG96TcpConfig>;
  friend class TinyGsmSSL<TinyGsmBG96, TinyGsmBG96SslConfig>;
  friend class TinyGsmCalling<TinyGsmBG96>;
  friend class TinyGsmSMS<TinyGsmBG96>;
  friend class TinyGsmGPS<TinyGsmBG96>;
  friend class TinyGsmTime<TinyGsmBG96>;
  friend class TinyGsmNTP<TinyGsmBG96>;
  friend class TinyGsmBattery<TinyGsmBG96>;
  friend class TinyGsmTemperature<TinyGsmBG96>;

 public:
  using ModemConfig = TinyGsmBG96ModemConfig;
  using TcpConfig   = TinyGsmBG96TcpConfig;
  using SslConfig   = TinyGsmBG96SslConfig;
  using GsmSecureContext =
      TinyGsmSSL<TinyGsmBG96, TinyGsmBG96SslConfig>::gsmSecureContext;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup quectel_bg96
  class GsmClientBG96 : public GsmClient<TinyGsmBG96, TinyGsmBG96TcpConfig> {
    friend class TinyGsmBG96;

   public:
    using GsmClient<TinyGsmBG96, TinyGsmBG96TcpConfig>::connect;
    using GsmClient<TinyGsmBG96, TinyGsmBG96TcpConfig>::stop;
    using TcpConfig = TinyGsmBG96TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientBG96() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is identical to the identifier
     * the modem uses internally to identify the socket for the BG96.
     *
     * @note The BG96 allows you choose the multiplexing channel number, but if
     * the input mux channel number is already in use and other mux channels are
     * available, this library will select the next available one.  Use the
     * getMux() function to get the assigned multiplexing channel number after a
     * successful connection.
     */
    explicit GsmClientBG96(TinyGsmBG96& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmBG96, TinyGsmBG96TcpConfig>(modem, mux) {
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
    // No extra extended API functions
  };

  /*
   * Inner Secure Client
   */
 public:
  /// Inner secure client
  /// @ingroup quectel_bg96
  class GsmClientSecureBG96
      : public GsmClientSecure<GsmClientBG96, TinyGsmBG96> {
    friend class TinyGsmBG96;

   public:
    using GsmClientBG96::connect;
    using GsmClientBG96::stop;
    using GsmClientSecure::GsmClientSecure;
    using TcpConfig = TinyGsmBG96TcpConfig;

    /*
     * Arduino Client interface
     */

    // Because we have the same potential range of mux numbers for secure and
    // insecure connections, we don't need to re-check for mux number
    // availability.
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
        DBG("### The BG96 does not support SSL using pre-shared keys.");
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

    // NOTE: Unlike the unsecured client, we can't check the size of the buffer
    // for an SSL socket. This means we have to overwrite all of the
    // buffer-checking versions of functions with the read-without-size-check
    // versions.
    /// @copydoc GsmClient::available()
    int available() override {
      if (at == nullptr) { return 0; }
      is_mid_send = false;  // Any calls to the AT when mid-send will cause the
                            // send to fail
      // Returns the combined number of characters available in the TinyGSM
      // fifo and the modem chips internal fifo.
      if (!rx.size()) { at->maintain(); }
      return static_cast<uint16_t>(rx.size()) + sock_available;
    }

    /// @copydoc GsmClient::read(uint8_t*, size_t)
    int read(uint8_t* buf, size_t size) override {
      if (at == nullptr) { return -1; }
      is_mid_send = false;  // Any calls to the AT when mid-send will cause the
                            // send to fail
      size_t cnt  = 0;

      // Reads characters out of the TinyGSM fifo, and from the modem chip's
      // internal fifo if available.
      at->maintain();
      while (cnt < size) {
        size_t chunk = TinyGsmMin(size - cnt, rx.size());
        if (chunk > 0) {
          rx.get(buf, chunk);
          buf += chunk;
          cnt += chunk;
          continue;
        } /* TODO: Read directly into user buffer? */
        at->maintain();
        if (sock_available > 0) {
          int n = at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available),
                                mux);
          if (n == 0) break;
        } else {
          break;
        }
      }
      return cnt;
    }

    /// @copydoc GsmClient::read()
    int read() override {
      uint8_t c;
      if (read(&c, 1) == 1) { return c; }
      return -1;
    }

    /// @copydoc GsmClient::connected()
    uint8_t connected() override {
      if (at == nullptr) { return false; }
      if (is_mid_send) { return true; }  // Don't interrupt a send
      if (available()) { return true; }
      // If the modem doesn't have an internal buffer, or if we can't check how
      // many characters are in the buffer then the cascade won't happen.
      // We need to call modemGetConnected to check the sock state.
      return at->modemGetConnected(mux);
    }

    // NOTE: We do NOT need to re-write the peek() function for the secure
    // client, because the peek() function isn't different for buffers that can
    // and can't have the size checked - only for those with no buffer at all.
  };

  /*
   * Inner Secure Context
   */
 public:
  /// Inner secure context
  /// @ingroup quectel_bg96
  class GsmSecureContextBG96
      : public ::GsmSecureContext<TinyGsmBG96, TinyGsmBG96SslConfig> {
    friend class TinyGsmBG96;

   public:
    using ::GsmSecureContext<TinyGsmBG96,
                             TinyGsmBG96SslConfig>::GsmSecureContext;

    /// Unregister this context from its modem.
    ~GsmSecureContextBG96() {}
  };

  /*
   * Modem-scoped Client Aliases
   */
 public:
  /// Arduino Client type for this modem.
  using TinyGsmClient = GsmClientBG96;
  /// Secure Arduino Client type for this modem.
  using TinyGsmClientSecure = GsmClientSecureBG96;
  /// Security-context type for this modem.
  using TinyGsmSecureContext = GsmSecureContextBG96;

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmBG96(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientBG96"));

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

    // Enable automatic time zone update
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

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin) {
    if (!testAT()) { return false; }
    if (!setPhoneFunctionality(1, true)) { return false; }
    waitResponse(10000L, GF("APP RDY"));
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+QPOWD=1"));
    waitResponse(300);  // returns OK first
    return waitResponse(300, GF("POWERED DOWN")) == 1;
  }

  // When entering into sleep mode is enabled, DTR is pulled up, and WAKEUP_IN
  // is pulled up, the module can directly enter into sleep mode.If entering
  // into sleep mode is enabled, DTR is pulled down, and WAKEUP_IN is pulled
  // down, there is a need to pull the DTR pin and the WAKEUP_IN pin up first,
  // and then the module can enter into sleep mode.
  bool sleepEnableImpl(bool enable) {
    sendAT(GF("+QSCLK="), enable);
    return waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(15500L, GF("OK")) == 1;
  }

  /*
   * Generic network functions
   */
 protected:
  BG96RegStatus getRegistrationStatusImpl() {
    // Check first for EPS registration
    BG96RegStatus epsStatus =
        static_cast<BG96RegStatus>(getRegistrationStatusXREG("CEREG"));

    // If we're connected on EPS, great!
    if (epsStatus == BG96RegStatus::REG_OK_HOME ||
        epsStatus == BG96RegStatus::REG_OK_ROAMING) {
      return epsStatus;
    } else {
      // Otherwise, check generic network status
      return static_cast<BG96RegStatus>(getRegistrationStatusXREG("CREG"));
    }
  }

  bool isNetworkConnectedImpl() {
    BG96RegStatus s = this->getRegistrationStatus();
    return (s == BG96RegStatus::REG_OK_HOME ||
            s == BG96RegStatus::REG_OK_ROAMING);
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
 protected:
// NOTE: Some of the documentation suggests this set of commands for
// uploading certs, but the BG96 I have doesn't accept them.
#if 0
  // AT+QSECWRITE=<filename>,<filesize> [,<timeout>]
  // <filename> - The name of the certificate/key/password file. The file name
  // has to be prefixed with "RAM:" or "NVRAM:".
  bool loadCertificateImpl(const char* certificateName, const char* cert,
                           const uint16_t len) {
    sendAT(GF("+QSECWRITE=\"RAM:"), certificateName, GF("\","), len, GF(",10"));
    if (waitResponse(GF("CONNECT")) != 1) {
      return false;
    }
    streamWrite(cert, len);
    stream.flush();
    // After module switches to data mode, and the certificate or key data can
    // be inputted. When the size of the inputted data reaches <filesize> (unit:
    // byte) or module receives "+++" sequence from UART, module will return to
    // command mode and reply the following codes.
    // +QSECWRITE:<uploadsize>,<checksum>
    bool success = true;
    success &= waitResponse(GF("+QSECWRITE:")) == 1;
    if (success) {
      uint16_t len_confirmed = stream.parseInt();
      streamSkipUntil('\n');  // skip the checksum
      success &= len_confirmed == len;
    }
    return success && waitResponse() == 1;
  }

  // NOTE: You cannot print/view the content of a certificate after uploading it
  // to the modem This only prints the checksum
  bool printCertificateImpl(const char* filename, Stream& print_stream) {
    bool success = true;

    // Read the file's checksum and checksum match
    // AT+QSECREAD=<filename>
    // Responds with +QSECREAD=<good>,<checksum>
    sendAT(GF("+QSECREAD=\"RAM:"), filename, '"');
    success &= waitResponse(GF("+QSECREAD:")) == 1;
    if (success) { success = stream.parseInt() == 1; }
    if (!success) { return false; }

    // wait for some characters to be available
    uint32_t start = millis();
    while (!stream.available() && millis() - start < 10000) {}

    while (stream.available()) {
      int      c;
      uint32_t _startMillis = millis();
      do {
        c = stream.read();
        if (c >= 0) break;
      } while (millis() - _startMillis < 50);
      // Print the file to the buffer
#ifndef DUMP_AT_COMMANDS
      // NOTE: Only do this if we're not dumping the all AT, or we'll double
      // print
      print_stream.write(c);
#endif
      if (c < 0) { break; }  // if we run out of characters, stop
    }
    print_stream.flush();

    // wait for the ending OK
    success &= waitResponse() == 1;
    return success;
  }

  // NOTE: Deleting the certificate only happens by filename, the type of
  // certificate does not matter here
  // AT+QSECDEL=<filename>
  bool deleteCertificateImpl(const char* certificateName) {
    sendAT(GF("+QSECDEL=\"RAM:"), certificateName, '"');
    return waitResponse() == 1;
  }
#endif

  // AT+QFUPL=<filename>[,<file_size>[,<timeout>[,<ackmode>]]]
  template <typename TCert>
  bool loadCertificateImpl(const char* certificateName, TCert cert,
                           const uint16_t len) {
    sendAT(GF("+QFUPL=\""), certificateName, GF("\","), len, GF(",10"));
    if (waitResponse(GF("CONNECT")) != 1) { return false; }
    this->streamWrite(cert, len);
    stream.flush();
    // TA switches to the data mode (transparent access mode), and the binary
    // data of file can be inputted. When the total size of the inputted data
    // reaches <file_size> (unit: byte), TA will return to command mode and
    // reply the following codes:
    // +QFUPL: <upload_size>,<checksum>
    bool success = true;
    success &= waitResponse(GF("+QFUPL:")) == 1;
    if (success) {
      uint16_t len_confirmed = streamGetIntBefore(',');
      streamSkipUntil('\n');  // skip the checksum
      success &= len_confirmed == len;
    }
    return success && waitResponse() == 1;
  }

  bool printCertificateImpl(const char* filename, Stream& print_stream) {
    // Download the file
    // AT+QFDWL=<filename>
    // Responds with "CONNECT\r\n"
    // TA switches to data mode, and the binary data of the file will be
    // outputted. When the file is read over, TA will return to command mode and
    // reply the following codes:
    // +QFDWL: <download_size>,<checksum>
    // OK
    sendAT(GF("+QFDWL=\""), filename, '"');
    if (waitResponse(GF("CONNECT\r\n")) != 1) { return false; }

    const char metadataMarker[] = "\r\n+QFDWL:";
    uint8_t    markerIndex      = 0;
    uint32_t   print_len        = 0;
    while (markerIndex < sizeof(metadataMarker) - 1) {
      int c = streamTimedRead(10000L);
      if (c < 0) { return false; }
      if (c == metadataMarker[markerIndex]) {
        markerIndex++;
        continue;
      }

#ifndef DUMP_AT_COMMANDS
      // NOTE: Only do this if we're not dumping all AT, or we'll double print.
      for (uint8_t i = 0; i < markerIndex; i++) {
        print_stream.write(metadataMarker[i]);
      }
#endif
      print_len += markerIndex;
      markerIndex = 0;
      if (c == metadataMarker[0]) {
        markerIndex = 1;
      } else {
#ifndef DUMP_AT_COMMANDS
        print_stream.write(static_cast<uint8_t>(c));
#endif
        print_len++;
      }
    }
    print_stream.flush();

    uint32_t len_confirmed = streamGetULBefore(',');
    streamSkipUntil('\n');  // skip the checksum
    return len_confirmed == print_len && waitResponse() == 1;
  }

  // NOTE: Deleting the certificate only happens by filename, the type of
  // certificate does not matter here
  // AT+QFDEL=<filename>
  bool deleteCertificateImpl(const char* certificateName) {
    sendAT(GF("+QFDEL=\""), certificateName, '"');
    return waitResponse() == 1;
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
    dataDisconnect();

    // Configure the TCPIP Context
    sendAT(GF("+QICSGP=1,1,\""), apn, GF("\",\""), user, GF("\",\""), pwd, '"');
    if (waitResponse() != 1) { return false; }

    // Activate GPRS/CSD Context
    sendAT(GF("+QIACT=1"));
    if (waitResponse(150000L) != 1) { return false; }

    // Attach to Packet Domain service - is this necessary?
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  bool dataDisconnectImpl() {
    sendAT(GF("+QIDEACT=1"));  // Deactivate the bearer context
    if (waitResponse(40000L) != 1) { return false; }

    return true;
  }

  String getProviderImpl() {
    sendAT(GF("+QSPN?"));
    if (waitResponse(GF("+QSPN:")) != 1) { return ""; }
    streamSkipUntil('"');                      // Skip mode and format
    String res = stream.readStringUntil('"');  // read the provider
    waitResponse();                            // skip anything else
    return res;
  }

  /*
   * SIM card functions
   */
 protected:
  String getSimCCIDImpl() {
    sendAT(GF("+QCCID"));
    if (waitResponse(GF("+QCCID:")) != 1) { return ""; }
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
   * GSM location functions
   */
 protected:
  // NOTE:  As of application firmware version 01.016.01.016 triangulated
  // locations can be obtained via the QuecLocator service and accompanying AT
  // commands.  As this is a separate paid service which I do not have access
  // to, I am not implementing it here.

  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // enable GPS
  bool enableGPSImpl() {
    sendAT(GF("+QGPS=1"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool disableGPSImpl() {
    sendAT(GF("+QGPSEND"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  // get the RAW GPS output
  String getGPSrawImpl() {
    sendAT(GF("+QGPSLOC=2"));
    if (waitResponse(10000L, GF("+QGPSLOC: ")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  bool getGPSImpl(float* lat, float* lon, float* speed, float* alt, int* vsat,
                  int* usat, float* accuracy, int* year, int* month, int* day,
                  int* hour, int* minute, int* second) {
    sendAT(GF("+QGPSLOC=2"));
    if (waitResponse(10000L, GF("+QGPSLOC: ")) != 1) {
      // NOTE:  Will return an error if the position isn't fixed
      return false;
    }

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

    // UTC Time; Format: hhmmss.sss
    // Read the whole time portion into a single float, then parse that
    // into the hour, minute, and second portions.  By doing it this way, we
    // avoid issues of attempting to read a set number of characters for each
    // portion of the time if the time is missing or malformed.
    float itime = streamGetFloatBefore(',');
    if (itime >= 0.0F) {
      uint32_t itime_int      = static_cast<uint32_t>(itime);
      float    itime_fraction = itime - static_cast<float>(itime_int);
      ihour = static_cast<int16_t>(itime_int / 10000);       // Two digit hour
      imin = static_cast<int16_t>((itime_int / 100) % 100);  // Two digit minute
      secondWithSS = static_cast<float>(itime_int % 100) + itime_fraction;
    }

    ilat      = streamGetFloatBefore(',');  // Latitude
    ilon      = streamGetFloatBefore(',');  // Longitude
    iaccuracy = streamGetFloatBefore(',');  // Horizontal precision
    ialt      = streamGetFloatBefore(',');  // Altitude from sea level
    streamSkipUntil(',');                   // GNSS positioning mode
    streamSkipUntil(',');  // Course Over Ground based on true north
    streamSkipUntil(',');  // Speed Over Ground in Km/h
    ispeed = streamGetFloatBefore(',');  // Speed Over Ground in knots

    // UTC Date
    // Again, read the whole date portion into a single uint32_t, then parse it
    // into day, month, year.
    uint32_t idate = streamGetULBefore(',');
    iday           = static_cast<int16_t>(idate / 10000);  // Two digit day
    imonth = static_cast<int16_t>((idate / 100) % 100);    // Two digit month
    iyear  = static_cast<int16_t>(idate % 100);            // Two digit year

    iusat = streamGetIntBefore(',');  // Number of satellites,
    streamSkipUntil('\n');  // The error code of the operation. If it is not
                            // 0, it is the type of error.

    // Set pointers
    if (lat != nullptr) *lat = ilat;
    if (lon != nullptr) *lon = ilon;
    if (speed != nullptr) *speed = ispeed;
    if (alt != nullptr) *alt = ialt;
    if (vsat != nullptr) *vsat = 0;
    if (usat != nullptr) *usat = iusat;
    if (accuracy != nullptr) *accuracy = iaccuracy;
    if (iyear < 2000) iyear += 2000;
    if (year != nullptr) *year = iyear;
    if (month != nullptr) *month = imonth;
    if (day != nullptr) *day = iday;
    if (hour != nullptr) *hour = ihour;
    if (minute != nullptr) *minute = imin;
    if (second != nullptr) *second = static_cast<int>(secondWithSS);

    waitResponse();  // Final OK

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

  /*
   * Time functions
   */
 protected:
  String getGSMDateTimeImpl(TinyGSMDateTimeFormat format) {
    sendAT(GF("+QLTS=2"));
    if (waitResponse(2000L, GF("+QLTS: \"")) != 1) { return ""; }

    String res;

    switch (format) {
      case TinyGSMDateTimeFormat::DATE_FULL:
        res = stream.readStringUntil('"');
        break;
      case TinyGSMDateTimeFormat::DATE_TIME:
        streamSkipUntil(',');
        res = stream.readStringUntil('"');
        break;
      case TinyGSMDateTimeFormat::DATE_DATE:
        res = stream.readStringUntil(',');
        break;
    }
    waitResponse();  // Ends with OK
    return res;
  }

  // The BG96 returns UTC time instead of local time as other modules do in
  // response to CCLK, so we're using QLTS where we can specifically request
  // local time.
  bool getNetworkUTCTimeImpl(int* year, int* month, int* day, int* hour,
                             int* minute, int* second, float* timezone) {
    sendAT(GF("+QLTS=1"));
    if (waitResponse(2000L, GF("+QLTS: \"")) != 1) { return false; }

    int16_t iyear     = 0;
    int16_t imonth    = 0;
    int16_t iday      = 0;
    int16_t ihour     = 0;
    int16_t imin      = 0;
    int16_t isec      = 0;
    int16_t itimezone = 0;

    // Date & Time
    iyear     = streamGetIntBefore('/');
    imonth    = streamGetIntBefore('/');
    iday      = streamGetIntBefore(',');
    ihour     = streamGetIntBefore(':');
    imin      = streamGetIntBefore(':');
    isec      = streamGetIntLength(2);
    itimezone = streamGetIntBefore(',');
    streamSkipUntil('\n');  // DST flag

    // Set pointers
    if (iyear < 2000) iyear += 2000;
    if (year != nullptr) *year = iyear;
    if (month != nullptr) *month = imonth;
    if (day != nullptr) *day = iday;
    if (hour != nullptr) *hour = ihour;
    if (minute != nullptr) *minute = imin;
    if (second != nullptr) *second = isec;
    if (timezone != nullptr) *timezone = static_cast<float>(itimezone) / 4.0;

    // Final OK
    waitResponse();  // Ends with OK

#if 0
    DBG(GF("Year:"), iyear, GF("	Month:"), imonth, GF("	Day:"), iday);
    DBG(GF("Hour:"), ihour, GF("	Minute:"), imin, GF("	Second:"), isec);
    DBG(GF("Timezone:"), static_cast<float>(itimezone) / 4.0);
#endif

    // Validate parsed values
    if (!TinyGsmTime<TinyGsmBG96>::isValidDateTimeValues(
            iyear, imonth, iday, ihour, imin, isec, itimezone)) {
      return false;
    }

    return true;
  }

  // The BG96 returns UTC time instead of local time as other modules do in
  // response to CCLK, so we're using QLTS where we can specifically request
  // local time.
  bool getNetworkTimeImpl(int* year, int* month, int* day, int* hour,
                          int* minute, int* second, float* timezone) {
    sendAT(GF("+QLTS=2"));
    if (waitResponse(2000L, GF("+QLTS: \"")) != 1) { return false; }

    int16_t iyear     = 0;
    int16_t imonth    = 0;
    int16_t iday      = 0;
    int16_t ihour     = 0;
    int16_t imin      = 0;
    int16_t isec      = 0;
    int16_t itimezone = 0;

    // Date & Time
    iyear     = streamGetIntBefore('/');
    imonth    = streamGetIntBefore('/');
    iday      = streamGetIntBefore(',');
    ihour     = streamGetIntBefore(':');
    imin      = streamGetIntBefore(':');
    isec      = streamGetIntLength(2);
    itimezone = streamGetIntBefore(',');
    streamSkipUntil('\n');  // DST flag

    // Set pointers
    if (iyear < 2000) iyear += 2000;
    if (year != nullptr) *year = iyear;
    if (month != nullptr) *month = imonth;
    if (day != nullptr) *day = iday;
    if (hour != nullptr) *hour = ihour;
    if (minute != nullptr) *minute = imin;
    if (second != nullptr) *second = isec;
    if (timezone != nullptr) *timezone = static_cast<float>(itimezone) / 4.0;

    // Final OK
    waitResponse();  // Ends with OK

#if 0
    DBG(GF("Year:"), iyear, GF("	Month:"), imonth, GF("	Day:"), iday);
    DBG(GF("Hour:"), ihour, GF("	Minute:"), imin, GF("	Second:"), isec);
    DBG(GF("Timezone:"), static_cast<float>(itimezone) / 4.0);
#endif

    // Validate parsed values
    if (!TinyGsmTime<TinyGsmBG96>::isValidDateTimeValues(
            iyear, imonth, iday, ihour, imin, isec, itimezone)) {
      return false;
    }

    return true;
  }

  /*
   * NTP server functions
   */

  bool NTPServerSyncImpl(const char* server, byte) {
    // Request network synchronization
    // AT+QNTP=<contextID>,<server>[,<port>][,<autosettime>]
    sendAT(GF("+QNTP=1,\""), server, '"');
    // Response: OK\r\n+QNTP: <err>,<time>
    //   <err> -  Error code of operation - see chapter 4 of the
    // BG96_TCP(IP)_Application_Note
    // There's a long list of codes, but 0 is a success
    bool quick_sync = waitResponse(10000L) == 1;  // Wait for the OK response
    // NOTE: This only waits 10s, but the BG96 can take up to 125s to sync
    // with the NTP server.
    if (quick_sync) {
      // if we get the response within 10s, check the error code to see if it
      // was successful
      if (waitResponse(GF("+QNTP:")) == 1) {
        int16_t err_code = streamGetIntBefore(',');
        streamSkipUntil('\n');  // skip the rest of the response
        return err_code == 0;
      }
    }
    // if we don't get the response within 10s, or if the error code was not 0,
    // check if the time is still syncing.  If it is, we consider this a
    // success, as the time will be updated when the sync is complete.  If it is
    // not syncing, we consider this a failure.
    sendAT(GF("+QNTP?"));
    // If in the process of local time synchronization:
    // +QNTP: <server>,<port>]\r\nOK\r\n
    bool success = waitResponse(10000L, GF("+QNTP:")) == 1;
    if (success) { return waitResponse() == 1; }
    return false;
  }

  bool waitForTimeSyncImpl(uint16_t timeout_s) {
    // if we're not connected, we'll never get the time
    if (!isNetworkConnected()) {
      DBG(GF("### Not connected to network; cannot sync time!"));
      return false;
    }
    // if we're sure we should be able to get the time, wait for it
    uint32_t start_millis = millis();
    while (millis() - start_millis < static_cast<uint32_t>(timeout_s) * 1000) {
      // Request the time
      sendAT(GF("+QLTS"));
      bool got_time_rsp = waitResponse(500L, GF("+QLTS: \"")) == 1;
      if (got_time_rsp) {
        // if the year is at or after 2025, we assume the time is valid and
        // synced (the response uses a 2-digit year, so we immediately add 2000
        // to it)
        int16_t iyear = streamGetIntBefore('/');
        if (iyear < 2000) iyear += 2000;
        streamSkipUntil('\n');  // skip the rest of the response
        bool got_ok = waitResponse(500L) == 1;
        if (iyear >= 2025 && iyear <= 2099 && got_ok) { return true; }
      }
      delay(500);  // Wait 0.5s before trying again
    }
    return false;
  }

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
 protected:
  // get temperature in degree celsius
  float getTemperatureImpl() {
    sendAT(GF("+QTEMP"));
    if (waitResponse(GF("+QTEMP:")) != 1) { return -9999.0F; }
    // return temperature in C
    int16_t res =
        streamGetIntBefore(',');  // read PMIC (primary ic) temperature
    streamSkipUntil(',');         // skip XO temperature ??
    streamSkipUntil('\n');        // skip PA temperature ??
    // Wait for final OK
    if (waitResponse() != 1 || res == -9999) { return -9999.0F; }
    return static_cast<float>(res);
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

    // NOTE: The SSL context (<sslctxID>) is not the same as the connection
    // identifier.  The SSL context is the grouping of SSL settings, the
    // connection identifier is the mux/socket number.

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

    // apply the correct certificates to the connection
    if (CAcertName != nullptr &&
        strnlen(CAcertName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
        (sslAuthMode == SSLAuthMode::CA_VALIDATION ||
         sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION)) {
      // AT+QSSLCFG="cacert",<sslctxID>,<cacertpath>
      // <sslctxID> SSL Context ID, range 0-5
      // <cacertpath> certificate file path
      sendAT(GF("+QSSLCFG=\"cacert\","), context_id, GF(",\""), CAcertName,
             '"');
      success &= waitResponse(5000L) == 1;
    }
    // SRGD WARNING: UNTESTED!!
    if (clientCertName != nullptr &&
        strnlen(clientCertName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
        (sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION)) {
      // AT+QSSLCFG="clientcert",<sslctxID>,<client_cert_path>
      sendAT(GF("+QSSLCFG=\"clientcert\","), context_id, GF(",\""),
             clientCertName, '"');
      success &= waitResponse(5000L) == 1;
    }
    // SRGD WARNING: UNTESTED!!
    if (clientKeyName != nullptr &&
        strnlen(clientKeyName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
        (sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION)) {
      // AT+QSSLCFG="clientkey",<sslctxID>[,<client_key_path>]
      sendAT(GF("+QSSLCFG=\"clientkey\","), context_id, GF(",\""),
             clientKeyName, '"');
      success &= waitResponse(5000L) == 1;
    }

    // set the ssl cipher_suite
    // AT+QSSLCFG="ciphersuite",<sslctxID>,<cipher_suite>
    // <sslctxID> SSL Context ID, range 0-5
    // <cipher_suite> 0: TODO
    //              1: TODO
    //              0X0035: TLS_RSA_WITH_AES_256_CBC_SHA
    //              0XFFFF: ALL
    sendAT(GF("+QSSLCFG=\"ciphersuite\","), context_id,
           GF(",0XFFFF"));  // All available
    success &= waitResponse(5000L) == 1;

    // set the ssl version
    // AT+QSSLCFG="sslversion",<sslctxID>,<sslversion>
    // <sslctxID> SSL Context ID, range 0-5
    // <sslversion> 0: QAPI_NET_SSL_3.0
    //              1: QAPI_NET_SSL_PROTOCOL_TLS_1_0
    //              2: QAPI_NET_SSL_PROTOCOL_TLS_1_1
    //              3: QAPI_NET_SSL_PROTOCOL_TLS_1_2
    //              4: ALL
    int8_t q_ssl_version = 3;
    // convert the ssl version into the format for this command
    switch (sslVersion) {
      case SSLVersion::SSL3_0: {
        q_ssl_version = 0;
        break;
      }
      case SSLVersion::TLS1_0: {
        q_ssl_version = 1;
        break;
      }
      case SSLVersion::TLS1_1: {
        q_ssl_version = 2;
        break;
      }
      case SSLVersion::TLS1_2: {
        q_ssl_version = 3;
        break;
      }
      default: {
        q_ssl_version = 4;
        break;
      }
    }
    sendAT(GF("+QSSLCFG=\"sslversion\","), context_id, ',', q_ssl_version);
    success &= waitResponse(5000L) == 1;

    // set the ssl sec level
    // AT+QSSLCFG="seclevel",<sslctxID>,<sec_level>
    // <sslctxID> SSL Context ID, range 0-5
    // <sec_level> 0: No authentication (SSLAuthMode::NO_VALIDATION)
    //             1: Manage server authentication
    //             (SSLAuthMode::CA_VALIDATION)
    //             2: Manage server and client authentication if requested
    //             by the remote server (SSLAuthMode::MUTUAL_AUTHENTICATION)
    switch (sslAuthMode) {
      case SSLAuthMode::CA_VALIDATION: {
        sendAT(GF("+QSSLCFG=\"seclevel\","), context_id, GF(",1"));
        break;
      }
      case SSLAuthMode::MUTUAL_AUTHENTICATION: {
        sendAT(GF("+QSSLCFG=\"seclevel\","), context_id, GF(",2"));
        break;
      }
      default: {
        sendAT(GF("+QSSLCFG=\"seclevel\","), context_id, GF(",0"));
        break;
      }
    }
    success &= waitResponse(5000L) == 1;

    // Don't ignore the time - make sure you sync it first!
    // AT+QSSLCFG="ignorelocaltime",<SSL_ctxID>[,<ignore_ltime>]
    // <sslctxID> SSL Context ID, range 0-5
    // <ignore_ltime> 0 to use the time, 1 to ignore it (default 1)
    sendAT(GF("+QSSLCFG=\"ignorelocaltime\","), context_id, GF(",0"));
    success &= waitResponse() == 1;

    return success;
  }

  /*
   * Client-related functions
   */

 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t /*static*/ mux,
                    int timeout_s) {
    if (!isValidMux(mux)) { return false; }
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    bool     ssl        = sockets[mux]->is_secure;

    if (ssl) {
      // If we have a secure socket, use a static cast to get the context index.
      // This isn't really "safe" but since we've already checked that the
      // socket is a secure one, we're pretty sure of the type and it should
      // work.
      const GsmClientSecureBG96* thisClient =
          static_cast<const GsmClientSecureBG96*>(sockets[mux]);
      uint8_t sslCtxIndex = thisClient->sslCtxIndex;

      // AT+QSSLOPEN=<pdpctxID>,<sslctxID>,<clientID>,<serveraddr>,<server_port>[,<access_mode>]
      // <PDPcontextID>(1-16) - we always use 1, which we configured above
      // <sslctxID> SSL Context ID, range 0-5
      // <connectID>(0-11), - socket index (mux)
      // may need previous AT+QSSLCFG
      sendAT(GF("+QSSLOPEN=1,"), sslCtxIndex, ',', mux, GF(",\""), host,
             GF("\","), port, GF(",0"));
      if (waitResponse() != 1) return false;

      if (waitResponse(timeout_ms, GF("+QSSLOPEN:")) != 1) { return false; }
      // 20230629 -> +QSSLOPEN: <clientID>,<err>
      // clientID is mux
      // err must be 0
      if (streamGetIntBefore(',') != mux) { return false; }
      // Read status
      return (0 == streamGetIntBefore('\n'));
    } else {
      // AT+QIOPEN=1,0,"TCP","220.180.239.212",8009,0,0
      // <PDPcontextID>(1-16), <connectID>(0-11),
      // "TCP/UDP/TCP LISTENER/UDPSERVICE", "<IP_address>/<domain_name>",
      // <remote_port>,<local_port>,<access_mode>(0-2; 0=buffer)
      sendAT(GF("+QIOPEN=1,"), mux, GF(",\""), GF("TCP"), GF("\",\""), host,
             GF("\","), port, GF(",0,0"));
      waitResponse();

      if (waitResponse(timeout_ms, GF("+QIOPEN:")) != 1) { return false; }

      if (streamGetIntBefore(',') != mux) { return false; }
    }
    // Read status
    return (0 == streamGetIntBefore('\n'));
  }

  bool modemStop(uint8_t mux, uint32_t maxWaitMs) {
    if (!isValidMux(mux)) { return false; }
    bool ssl = sockets[mux]->is_secure;
    if (ssl) {
      sendAT(GF("+QSSLCLOSE="), mux);
    } else {
      sendAT(GF("+QICLOSE="), mux);
    }
    return waitResponse(maxWaitMs) == 1;
  }

  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    bool ssl = sockets[mux]->is_secure;
    if (ssl) {
      sendAT(GF("+QSSLSEND="), mux, ',', (uint16_t)len);
    } else {
      sendAT(GF("+QISEND="), mux, ',', (uint16_t)len);
    }
    return waitResponse(GF(">")) == 1;
  }
  // Between the modemBeginSend and modemEndSend, modemSend calls:
  // stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  // stream.flush();
  size_t modemEndSend(size_t len, uint8_t) {
    if (waitResponse(GF("SEND OK")) != 1) { return 0; }
    // TODO(?): Wait for ACK? (AT+QISEND=id,0 or AT+QSSLSEND=id,0)
    return len;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    int16_t len_reported = 0;
    size_t  len_read     = 0;
    bool    ssl          = sockets[mux]->is_secure;
    if (ssl) {
      sendAT(GF("+QSSLRECV="), mux, ',', (uint16_t)size);
      if (waitResponse(GF("+QSSLRECV:")) != 1) {
        sockets[mux]->sock_available = 0;
        DBG("### READ: For unknown reason close");
        return 0;
      }
      len_reported = streamGetIntBefore('\n');
      // We have no way of knowing in advance how much data will be in the
      // buffer so when data is received we always assume the buffer is
      // completely full. Chances are, this is not true and there's really not
      // that much there. In that case, make sure we make sure we re-set the
      // amount of data available.
      if (len_reported >= 0 && static_cast<size_t>(len_reported) < size) {
        sockets[mux]->sock_available = static_cast<uint16_t>(len_reported);
      }
      len_read = moveCharsFromStreamToFifo(mux, len_reported);
      sockets[mux]->sock_available -= TinyGsmMin(
          static_cast<size_t>(sockets[mux]->sock_available), len_read);
      // ^^ Decrease the characters available after moving from modem's FIFO to
      // our FIFO
      waitResponse();  // ends with an OK
    } else {
      sendAT(GF("+QIRD="), mux, ',', (uint16_t)size);
      if (waitResponse(GF("+QIRD:")) != 1) { return 0; }
      len_reported = streamGetIntBefore('\n');
      len_read     = moveCharsFromStreamToFifo(mux, len_reported);
      waitResponse();
      // For unsecured sockets, we can check how much is left in the buffer
      // after reading.
      sockets[mux]->sock_available = modemGetAvailable(mux);
    }
    return len_read;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    bool   ssl    = sockets[mux]->is_secure;
    size_t result = 0;
    if (ssl) {
      return 0;  // Sadly, we can't read the amount of buffered data for a
                 // secure socket
    } else {
      sendAT(GF("+QIRD="), mux, GF(",0"));
      if (waitResponse(GF("+QIRD:")) == 1) {
        streamSkipUntil(',');  // Skip total received
        streamSkipUntil(',');  // Skip have read
        result = streamGetIntBefore('\n');
        if (result) { DBG("### DATA AVAILABLE:", result, "on", mux); }
        waitResponse();
      }
    }
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    bool    ssl = sockets[mux]->is_secure;
    int16_t ret_mux;
    if (ssl) {
      sendAT(GF("+QSSLSTATE="), mux);
      // +QSSLSTATE:<clientID>,"SSLClient",<IP_address>,<remote_port>,<local_port>,<socket_state>,<pdpctxID>,<serverID>,<access_mode>,<AT_port>,<sslctxID>)

      if (waitResponse(GF("+QSSLSTATE:")) != 1) {
        waitResponse();  // just returns "OK" if the sock is closed
        return false;
      }

      ret_mux = streamGetIntBefore(',');     // clientID (mux)
      streamSkipUntil(',');                  // Skip "SSLClient"
      streamSkipUntil(',');                  // Skip remote ip
      streamSkipUntil(',');                  // Skip remote port
      streamSkipUntil(',');                  // Skip local port
      int8_t res = streamGetIntBefore(',');  // socket state

      waitResponse();

      // 0 Initial, 1 Opening, 2 Connected, 3 Listening, 4 Closing
      return 2 == res && isExpectedMux(ret_mux, mux);
    } else {
      sendAT(GF("+QISTATE=1,"), mux);
      // +QISTATE: 0,"TCP","151.139.237.11",80,5087,4,1,0,0,"uart1"

      if (waitResponse(GF("+QISTATE:")) != 1) {
        waitResponse();  // just returns "OK" if the sock is closed
        return false;
      }

      ret_mux = streamGetIntBefore(',');     //  mux
      streamSkipUntil(',');                  // Skip socket type
      streamSkipUntil(',');                  // Skip remote ip
      streamSkipUntil(',');                  // Skip remote port
      streamSkipUntil(',');                  // Skip local port
      int8_t res = streamGetIntBefore(',');  // socket state

      waitResponse();

      // 0 Initial, 1 Opening, 2 Connected, 3 Listening, 4 Closing
      return 2 == res && isExpectedMux(ret_mux, mux);
    }
  }

  /*
   * Utilities
   */
 protected:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+QIURC:"))) {
      streamSkipUntil('\"');
      String urc = stream.readStringUntil('\"');
      streamSkipUntil(',');
      if (urc == "recv") {
        int16_t mux = streamGetIntBefore('\n');
        DBG("### URC RECV:", mux);
        if (isValidMux(mux)) { sockets[mux]->got_data = true; }
      } else if (urc == "closed") {
        int16_t mux = streamGetIntBefore('\n');
        DBG("### URC CLOSE:", mux);
        if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
      } else {
        streamSkipUntil('\n');
      }
      data = "";
      return true;
    }
    if (data.endsWith(GF("+QSSLURC:"))) {
      streamSkipUntil('\"');
      String urc = stream.readStringUntil('\"');
      streamSkipUntil(',');
      if (urc == "recv") {
        int16_t mux = streamGetIntBefore('\n');
        DBG("### URC SSL RECV:", mux);
        if (isValidMux(mux)) {
          // We have no way of knowing how much data actually came in, so
          // we set the value to 1500, the maximum transmission unit for TCP.
          sockets[mux]->sock_available = 1500;
        }
      } else if (urc == "closed") {
        int16_t mux = streamGetIntBefore('\n');
        DBG("### URC CLOSE:", mux);
        if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
      } else {
        streamSkipUntil('\n');
      }
      data = "";
      return true;
    }
    return false;
  }

 public:
  /// Stream used to communicate with the modem.
  Stream& stream;

 protected:
  GsmClientBG96* sockets[TcpConfig::kMuxCount];
};

#endif  // SRC_TINYGSMCLIENTBG96_H_
