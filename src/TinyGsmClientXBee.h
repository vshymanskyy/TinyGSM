/**
 * @file       TinyGsmClientXBee.h
 * @brief      XBee modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy, XBee module by Sara
 * Damiano
 * @date       Nov 2016
 */
/**
 * @defgroup digi_xbee Digi XBee and XBee3 cellular and WiFi modules
 * @ingroup group_digi
 * @brief Manufacturer: Digi. Models: XBee and XBee3 cellular and WiFi modules.
 *
 * # Connection Information
 *
 * - Combined TCP/SSL sockets:
 *   - 1
 *   - The much more complicated (and unsupported) API mode is needed for
 * multiplexing.
 *   - 3 TLS profiles can be used (0, 1, 2)
 * - Socket Buffering:
 *   - The modem does **not** have an internal buffer for incoming data.
 *   - You must set the buffer size in this library to be larger than your
 * largest expected incoming packet, or you will lose incoming data.
 * - Socket Numbering:
 *   - Only one socket is available in transparent/command mode, and it is
 * always channel 0.
 *   - The mux argument on the client constructor and init() is ignored.
 *
 * @todo In operator `GsmClientXBee::read()`: Read directly into user buffer?
 * @todo In `enterCommandMode()`: optimize this
 * @todo Add support for network time through the 'DT' command.
 * @todo Support uploading certificates using the
 * [XModem library](https://github.com/gilman88/xmodem-lib)
 * @todo Implement baud rate forcing using a 6s serial break to temporarily
 * enter command mode at 9600 and then switch to the desired baud rate.
 */

#ifndef SRC_TINYGSMCLIENTXBEE_H_
#define SRC_TINYGSMCLIENTXBEE_H_
#pragma message("TinyGSM:  TinyGsmClientXBee")


#ifdef TINY_GSM_XBEE_GUARD_TIME
#undef TINY_GSM_XBEE_GUARD_TIME
#endif
/**
 * @brief The manufacturer's  standard "guard time" for XBee modules, in
 * milliseconds. This is the required wait time before and after sending the
 * '+++' to the module to enter command mode.  XBee's have a default guard time
 * of 1 second (1000ms, 10 extra for safety here)
 */
#define TINY_GSM_XBEE_GUARD_TIME 1010


#ifdef TINY_GSM_XBEE_WORKING_GUARD_TIME
#undef TINY_GSM_XBEE_WORKING_GUARD_TIME
#endif
/**
 * @brief The working "guard time" for XBee modules for when this library is
 * actively communicating with the module, in milliseconds. This is the required
 * wait time before and after sending the '+++' to the module to enter command
 * mode.  When this library is active, it shortens the guard time to improve
 * communication efficiency.
 */
#define TINY_GSM_XBEE_WORKING_GUARD_TIME 0x64

#ifdef TINY_GSM_XBEE_DROPOUT_TIME
#undef TINY_GSM_XBEE_DROPOUT_TIME
#endif
/**
 * @brief The command mode drop-out time for XBee modules, in milliseconds /
 * 100.
 *
 * This is the amount of time the module will wait after the last command before
 * automatically exiting command mode.
 *
 * Use the default drop out time of 0x64 x 100ms (10 seconds)
 */
#define TINY_GSM_XBEE_DROPOUT_TIME 0x64

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmWifi.tpp"
#include "TinyGsmCellularData.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmTemperature.tpp"
#include "TinyGsmBattery.tpp"

/**
 * @brief function decorator to enter command mode for XBee modules, if not
 * already in command mode, and exit command mode after the function call if it
 * was not already in command mode.
 *
 * Use this to avoid too many entrances and exits from command mode.  The
 * cellular Bee's often freeze up and won't respond when attempting to enter
 * command mode too many times.
 *
 */
#define XBEE_COMMAND_START_DECORATOR(nAttempts, failureReturn)                \
  bool wasInCommandMode = inCommandMode &&                                    \
      millis() - lastCommandModeMillis <=                                     \
          static_cast<uint32_t>(guardTime * 2);                               \
  if (!wasInCommandMode) { /* don't re-enter command mode if already in it */ \
    if (!commandMode(nAttempts))                                              \
      return failureReturn; /* Return immediately if fails */                 \
  }
/// Function decorator to exit command mode for XBee modules, if not already in
/// command mode, after the function call if it was not already in command mode.
#define XBEE_COMMAND_END_DECORATOR                                 \
  bool stillInCommandMode = inCommandMode &&                       \
      millis() - lastCommandModeMillis <=                          \
          static_cast<uint32_t>(TINY_GSM_XBEE_DROPOUT_TIME) * 100; \
  if (!wasInCommandMode && stillInCommandMode) {                   \
    /* only exit if we weren't in command mode and now are */      \
    exitCommand();                                                 \
  }
/// A null IP address
#define NULL_IP IPAddress(0, 0, 0, 0)

/// Registration status
/// @ingroup digi_xbee
enum class XBeeRegStatus {
  REG_OK           = 0,  ///< Registered on the network
  REG_UNREGISTERED = 1,  ///< Not registered on the network
  REG_SEARCHING    = 2,  ///< Searching for network
  REG_DENIED       = 3,  ///< Registration denied
  REG_UNKNOWN      = 4,  ///< Unknown registration status
};

/// Basic modem configurations for the Digi XBee and XBee3
/// @ingroup digi_xbee
struct TinyGsmXBeeModemConfig : public TinyGsmModemConfigPreset<XBeeRegStatus> {
  /// The newline character(s) used in AT commands.
  static constexpr char GSM_NL[] TINY_GSM_PROGMEM = "\r";
  /// The OK string.
  static constexpr char GSM_OK[] TINY_GSM_PROGMEM = "OK\r";
  /// The ERROR string
  static constexpr char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR\r";
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "Digi";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "XBee";
};

constexpr char TinyGsmXBeeModemConfig::GSM_NL[] __attribute__((weak));
constexpr char TinyGsmXBeeModemConfig::GSM_OK[] __attribute__((weak));
constexpr char TinyGsmXBeeModemConfig::GSM_ERROR[] __attribute__((weak));
constexpr char TinyGsmXBeeModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmXBeeModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for the Digi XBee and XBee3.
 *
 * XBees do not support multiple sockets in transparent/command mode.  The much
 * more complicated API mode is needed.  In API mode, 4-10 sockets can be
 * supported, depending on the specific module.
 * @ingroup digi_xbee
 */
struct TinyGsmXBeeTcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::NoModemBuffer,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 1,
          /*sendMaxSize*/ 1500,    // default
          /*connectTimeoutS*/ 75,  // default
          /*stopTimeoutS*/ 5> {};

/**
 * @brief SSL behavior and limits for Digi XBee and XBee3.
 * All supported XBee modules support 3 SSL contexts in API mode, but only 1
 * ($0) in transparent mode.
 * @ingroup digi_xbee
 */
struct TinyGsmXBeeSslConfig : public TinyGsmSslConfigPreset<
                                  /*contextCount*/ 1> {};

/// The known types of XBees
/// The values are responses to the HS command to get "hardware series"
/// @ingroup digi_xbee
enum class XBeeType {
  XBEE_UNKNOWN   = 0,      ///< Unknown XBee type
  XBEE_S6B_WIFI  = 0x601,  ///< Digi XBee Wi-Fi
  XBEE_LTE1_VZN  = 0xB01,  ///< Digi XBee Cellular LTE Cat 1
  XBEE_3G        = 0xB02,  ///< Digi XBee Cellular 3G
  XBEE3_LTE1_ATT = 0xB06,  ///< Digi XBee3 Cellular LTE CAT 1
  XBEE3_LTEM_ATT = 0xB08,  ///< Digi XBee3 Cellular LTE-M
  XBEE3_LTEM3    = 0xB0E,  ///< Digi XBee3 Cellular LTE-M3
};

/// Class for the Digi XBee family of modems
/// @ingroup digi_xbee
class TinyGsmXBee : public TinyGsmModem<TinyGsmXBee, TinyGsmXBeeModemConfig>,
                    public TinyGsmCellularData<TinyGsmXBee>,
                    public TinyGsmWifi<TinyGsmXBee>,
                    public TinyGsmTCP<TinyGsmXBee, TinyGsmXBeeTcpConfig>,
                    public TinyGsmSSL<TinyGsmXBee, TinyGsmXBeeSslConfig>,
                    public TinyGsmSMS<TinyGsmXBee>,
                    public TinyGsmBattery<TinyGsmXBee>,
                    public TinyGsmTemperature<TinyGsmXBee> {
  friend class TinyGsmModem<TinyGsmXBee, TinyGsmXBeeModemConfig>;
  friend class TinyGsmCellularData<TinyGsmXBee>;
  friend class TinyGsmWifi<TinyGsmXBee>;
  friend class TinyGsmTCP<TinyGsmXBee, TinyGsmXBeeTcpConfig>;
  friend class GsmClient<TinyGsmXBee, TinyGsmXBeeTcpConfig>;
  friend class TinyGsmSSL<TinyGsmXBee, TinyGsmXBeeSslConfig>;
  friend class TinyGsmSMS<TinyGsmXBee>;
  friend class TinyGsmBattery<TinyGsmXBee>;
  friend class TinyGsmTemperature<TinyGsmXBee>;

 public:
  using ModemConfig = TinyGsmXBeeModemConfig;
  using TcpConfig   = TinyGsmXBeeTcpConfig;
  using SslConfig   = TinyGsmXBeeSslConfig;
  using GsmSecureContext =
      TinyGsmSSL<TinyGsmXBee, TinyGsmXBeeSslConfig>::gsmSecureContext;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup digi_xbee
  class GsmClientXBee : public GsmClient<TinyGsmXBee, TinyGsmXBeeTcpConfig> {
    friend class TinyGsmXBee;

   public:
    using GsmClient<TinyGsmXBee, TinyGsmXBeeTcpConfig>::connect;
    using GsmClient<TinyGsmXBee, TinyGsmXBeeTcpConfig>::stop;
    using TcpConfig = TinyGsmXBeeTcpConfig;
    using Print::write;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientXBee() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem.
     * @param modem Modem instance used by this client.
     *
     * @note The XBee does not support multiplexing in transparent/command mode.
     * The much more complicated API mode is needed for multiplexing.
     */
    explicit GsmClientXBee(TinyGsmXBee& modem, uint8_t /*mux*/ = 0)
        : GsmClient<TinyGsmXBee, TinyGsmXBeeTcpConfig>(modem /*, mux*/) {
      is_secure = false;
      init(&modem);
    }

    /*
     * Arduino Client interface
     */
   public:
    /**
     * @copydoc GsmClient::connect(const char*, uint16_t, int)
     *
     * @note The XBee saves all connection information (ssid/pwd etc) except IP
     * address and port number, in flash (NVM). The NVM is be updated only when
     * it is initialized. The TCP connection itself is not opened until you
     * attempt to send data. Because all settings are saved to flash, it is
     * possible (or likely) that you could send data even if you haven't "made"
     * any connection.
     */
    int connect(const char* host, uint16_t port, int timeout_s) override {
      if (at == nullptr) { return 0; }
      // NOTE:  Not calling stop() or yield() here
      at->streamClear();  // Empty anything in the buffer before starting
      sock_connected      = at->modemConnect(host, port, mux, timeout_s);
      lastConnectionCheck = millis();
      return sock_connected;
    }
    /// @copydoc GsmClient::connect(const char*, uint16_t)
    int connect(const char* host, uint16_t port) override {
      if (at == nullptr) { return 0; }
      return connect(host, port, TcpConfig::kConnectTimeoutS);
    }

    /// @copydoc GsmClient::connect(IPAddress, uint16_t, int)
    int connect(IPAddress ip, uint16_t port, int timeout_s) override {
      if (at == nullptr) { return 0; }
      if (timeout_s != 0) {
        DBG("Timeout [", timeout_s, "] doesn't apply here.");
      }
      // NOTE:  Not calling stop() or yield() here
      at->streamClear();  // Empty anything in the buffer before starting
      sock_connected      = at->modemConnect(ip, port, mux, timeout_s);
      lastConnectionCheck = millis();
      return sock_connected;
    }
    /// @copydoc GsmClient::connect(IPAddress, uint16_t)
    int connect(IPAddress ip, uint16_t port) override {
      if (at == nullptr) { return 0; }
      return connect(ip, port, 0);
    }

    /**
     * @copydoc GsmClient::stop(uint32_t)
     *
     * @note Because settings are saved in flash, the XBEE will attempt to
     * reconnect to the previous socket if it receives any outgoing data.
     * Setting sock_connected to false after the stop ensures that connected()
     * will return false after a stop has been ordered.  This makes it play much
     * more nicely with libraries like PubSubClient.
     */
    void stop(uint32_t maxWaitMs) override {
      if (at == nullptr) { return; }
      at->streamClear();  // Empty anything in the buffer
      // empty the saved currently-in-use destination address
      at->modemStop(0, maxWaitMs);
      at->streamClear();  // Empty anything in the buffer
      is_mid_send    = false;
      sock_connected = false;
    }

    /// @copydoc GsmClient::write(const uint8_t*, size_t)
    size_t write(const uint8_t* buf, size_t size) override {
      if (at == nullptr) { return 0; }
      return at->stream.write(buf, size);
    }

    /// @copydoc GsmClient::write(uint8_t)
    size_t write(uint8_t c) override {
      if (at == nullptr) { return 0; }
      return at->stream.write(c);
    }

    /// @copydoc GsmClient::available()
    int available() override {
      if (at == nullptr) { return 0; }
      return at->stream.available();
      /*
      if (!rx.size() || at->stream.available()) {
        at->maintain();
      }
      return at->stream.available() + rx.size();
      */
    }

    /// @copydoc GsmClient::read(uint8_t*, size_t)
    int read(uint8_t* buf, size_t size) override {
      if (at == nullptr) { return -1; }
      return at->stream.readBytes(reinterpret_cast<char*>(buf), size);
    }

    /// @copydoc GsmClient::read()
    int read() override {
      if (at == nullptr) { return -1; }
      return at->stream.read();
    }

    /// @copydoc GsmClient::peek()
    int peek() override {
      if (at == nullptr) { return -1; }
      return at->stream.peek();
    }
    /// @copydoc GsmClient::flush()
    void flush() override {
      if (at == nullptr) { return; }
      at->stream.flush();
    }

    /// @copydoc GsmClient::connected()
    uint8_t connected() override {
      if (at == nullptr) { return false; }
      if (available()) { return true; }
      if (millis() - lastConnectionCheck > 5000L) {
        // NOTE: Only go into command mode if it has been more than 5 seconds
        // since the last check
        lastConnectionCheck = millis();
        sock_connected      = at->modemGetConnected(mux);
      }
      return sock_connected;
    }
    /// @copydoc GsmClient::operator bool()
    operator bool() override {
      return connected();
    }

    /*
     * Extended Client API
     */

    /// @copydoc GsmClient::remoteIP()
    /// @note This function hides the non-virtual remoteIP() function in the
    /// base class, which is not virtual.
    String remoteIP() {
      if (at == nullptr) { return ""; }
      IPAddress atLastIP = at->savedIP;
      return TinyGsmStringFromIp(atLastIP);
    }

    /// The last time the connection was checked. Since the modem doesn't give
    /// us any URC's in transparent mode, we use this to check periodically if
    /// the connection is still alive.
    uint32_t lastConnectionCheck = 0;
  };

  /*
   * Inner Secure Client
   */
 public:
  /// Inner secure client
  /// @ingroup digi_xbee
  class GsmClientSecureXBee
      : public GsmSecureClient<GsmClientXBee, TinyGsmXBee> {
    friend class TinyGsmXBee;

   protected:
    /**
     * @brief Validates and configures the SSL context for the XBee modem.
     */
    bool validateAndConfigureSSLContext() {
      if (at == nullptr) { return false; }
      // NOTE:  Not calling stop() or yield() here
      at->streamClear();  // Empty anything in the buffer before starting

      GsmSecureContext* sslContext = at->getSSLContext(sslCtxIndex);
      // check for a valid SSL context
      if (sslContext == nullptr) {
        DBG("### No SSL context is registered at index", sslCtxIndex);
        return false;
      }
      // check that the authentication mode is supported
      if (sslContext->sslAuthMode == SSLAuthMode::PRE_SHARED_KEYS) {
        DBG("### The XBee does not support SSL using pre-shared keys.");
        return false;
      }
      // check that the context has the required credentials
      if (!at->validateSSLContextCredentials(sslContext)) {
        DBG("### The SSL context is missing required credentials!");
        return false;
      }
      // configure the context
      if (!at->configureSSLContext(sslCtxIndex)) {
        DBG("### Failed to configure the SSL context!");
        return false;
      }
      return true;
    }

   public:
    using GsmClientXBee::connect;
    using GsmClientXBee::stop;
    using GsmSecureClient::GsmSecureClient;
    using TcpConfig = TinyGsmXBeeTcpConfig;

    /*
     * Arduino Client interface
     */

    int connect(const char* host, uint16_t port, int timeout_s) override {
      // NOTE:  Not calling stop() or yield() here
      if (at == nullptr) { return 0; }
      if (!validateAndConfigureSSLContext()) { return 0; }
      // do the rest of the connection process
      sock_connected      = at->modemConnect(host, port, mux, timeout_s);
      lastConnectionCheck = millis();
      return sock_connected;
    }

    int connect(IPAddress ip, uint16_t port, int timeout_s) override {
      // NOTE:  Not calling stop() or yield() here
      if (at == nullptr) { return 0; }
      if (!validateAndConfigureSSLContext()) { return 0; }
      if (timeout_s != 0) {
        DBG("Timeout [", timeout_s, "] doesn't apply here.");
      }
      sock_connected      = at->modemConnect(ip, port, mux, timeout_s);
      lastConnectionCheck = millis();
      return sock_connected;
    }
  };

  /*
   * Inner Secure Context
   */
 public:
  /// Inner secure context
  /// @ingroup digi_xbee
  class GsmSecureContextXBee
      : public ::GsmSecureContext<TinyGsmXBee, TinyGsmXBeeSslConfig> {
    friend class TinyGsmXBee;

   public:
    using ::GsmSecureContext<TinyGsmXBee,
                             TinyGsmXBeeSslConfig>::GsmSecureContext;

    /// Unregister this context from its modem.
    ~GsmSecureContextXBee() {}
  };

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmXBee(Stream& stream) : TinyGsmXBee(stream, -1) {
    // Delegate to main constructor
  }

  /**
   * @brief Construct a modem wrapper around a stream transport and a reset pin.
   * @param stream Stream used to communicate with the modem.
   * @param resetPin Physical pin on the MCU that is connected to the XBee reset
   * pin .
   */
  TinyGsmXBee(Stream& stream, int8_t resetPin)
      : stream(stream),
        guardTime(TINY_GSM_XBEE_GUARD_TIME),
        beeType(XBeeType::XBEE_UNKNOWN),
        resetPin(resetPin),
        savedIP(NULL_IP),
        savedHost(""),
        lastConnectedIP(NULL_IP),
        inCommandMode(false),
        lastCommandModeMillis(0),
        lastHostLookupMillis(0) {
    // Start not knowing what kind of bee it is
    // Start with the default guard time of 1 second
    memset(sockets, 0, sizeof(sockets));
    // reserve space for the saved host
    savedHost.reserve(128);
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientXBee"));

    if (resetPin >= 0) {
      pinMode(resetPin, OUTPUT);
      digitalWrite(resetPin, HIGH);
    }

    XBEE_COMMAND_START_DECORATOR(10, false)

    bool changesMade = false;
    bool ret_val     = true;

    // if there's a pin, we need to re-write to flash each time
    if (pin && strlen(pin) > 0) {
      sendAT(GF("PN"), pin);
      if (waitResponse() != 1) {
        ret_val = false;
      } else {
        changesMade = true;
      }
    }

    // Put in transparent mode, if it isn't already
    changesMade |= changeSettingIfNeeded(GF("AP"), 0x0);

    // shorten the guard time to 100ms, if it was anything else
    changesMade |= changeSettingIfNeeded(GF("GT"),
                                         TINY_GSM_XBEE_WORKING_GUARD_TIME);
    guardTime = TINY_GSM_XBEE_WORKING_GUARD_TIME + 10;

    // Make sure the command mode drop-out time is long enough that we won't
    // fall out of command mode without intentionally leaving it.  This is the
    // default drop out time of 0x64 x 100ms (10 seconds)
    changesMade |= changeSettingIfNeeded(GF("CT"), 0x64);

    // Set the text delimiter for packetizing in transparent mode
    // line feed (\n) is 0x0A, carriage return (\r) is 0x0D
    // Set to zero to disable text delimiter checking. (default)
    changesMade |= changeSettingIfNeeded(GF("TD"), 0x0);

    // Set the packetization timeout - the number of character times of
    // inter-character silence required before transmission begins when
    // operating in Transparent mode.  Set RO to 0 to transmit characters as
    // they arrive instead of buffering them into one RF packet.
    // The default value is 0x03 (three character spaces)
    changesMade |= changeSettingIfNeeded(GF("RO"), 0x03);

    if (changesMade) { ret_val &= writeChanges(); }

    getSeries();  // Get the "Hardware Series";

    XBEE_COMMAND_END_DECORATOR

    return ret_val;
  }

  String getModemNameImpl() {
    String result;
    result.reserve(26);
    result = GFP(TinyGsmXBeeModemConfig::MODEM_MANUFACTURER);
    result += ' ';
    result += getModemModelImpl();
    return result;
  }

  String getModemModelImpl() {
    PGM_P suffix;
    switch (beeType) {
      case XBeeType::XBEE_S6B_WIFI: suffix = PSTR(" Wi-Fi"); break;
      case XBeeType::XBEE_LTE1_VZN: suffix = PSTR(" Cellular LTE Cat 1"); break;
      case XBeeType::XBEE_3G: suffix = PSTR(" Cellular 3G"); break;
      case XBeeType::XBEE3_LTE1_ATT:
        suffix = PSTR("3 Cellular LTE CAT 1");
        break;
      case XBeeType::XBEE3_LTEM_ATT: suffix = PSTR("3 Cellular LTE-M"); break;
      case XBeeType::XBEE3_LTEM3: suffix = PSTR("3 Cellular LTE-M3"); break;
      default: suffix = PSTR(" Unknown");
    }
    String result;
    result.reserve(30);
    result = GFP(TinyGsmXBeeModemConfig::MODEM_MODEL);
    result += GFP(suffix);
    return result;
  }

  // Gets the modem serial number
  String getModemSerialNumberImpl() {
    String result;
    result.reserve(24);
    result = sendATGetString(GF("SL"));  // Request Module MAC/Serial Number Low
    result += ' ';
    result +=
        sendATGetString(GF("SH"));  // Request Module MAC/Serial Number High
    return result;
  }

  // Gets the modem hardware version
  String getModemHardwareVersion() {
    return sendATGetString(GF("HV"));
  }

  // Gets the modem firmware version
  String getModemFirmwareVersion() {
    return sendATGetString(GF("VR"));
  }

  // Gets the modem combined version
  String getModemRevisionImpl() {
    String result;
    result.reserve(24);
    result = getModemHardwareVersion();
    result += ' ';
    result += getModemFirmwareVersion();
    return result;
  }

  bool setBaudImpl(uint32_t baud) {
    XBEE_COMMAND_START_DECORATOR(5, false)
    bool changesMade = false;
    switch (baud) {
      case 2400: changesMade |= changeSettingIfNeeded(GF("BD"), 0x1); break;
      case 4800: changesMade |= changeSettingIfNeeded(GF("BD"), 0x2); break;
      case 9600: changesMade |= changeSettingIfNeeded(GF("BD"), 0x3); break;
      case 19200: changesMade |= changeSettingIfNeeded(GF("BD"), 0x4); break;
      case 38400: changesMade |= changeSettingIfNeeded(GF("BD"), 0x5); break;
      case 57600: changesMade |= changeSettingIfNeeded(GF("BD"), 0x6); break;
      case 115200: changesMade |= changeSettingIfNeeded(GF("BD"), 0x7); break;
      case 230400: changesMade |= changeSettingIfNeeded(GF("BD"), 0x8); break;
      case 460800: changesMade |= changeSettingIfNeeded(GF("BD"), 0x9); break;
      case 921600: changesMade |= changeSettingIfNeeded(GF("BD"), 0xA); break;
      default: {
        DBG(GF("Specified baud rate is unsupported! Setting to 9600 baud."));
        changesMade |= changeSettingIfNeeded(GF("BD"),
                                             0x3);  // Set to default of 9600
        break;
      }
    }
    if (changesMade) { writeChanges(); }
    XBEE_COMMAND_END_DECORATOR
    return true;
  }

  bool testATImpl(uint32_t timeout_ms) {
    uint32_t start   = millis();
    bool     success = false;
    while (!success && millis() - start < timeout_ms) {
      if (!inCommandMode) {
        success = commandMode();
        if (success) exitCommand();
      } else {
        sendAT();
        if (waitResponse(200) == 1) {
          success = true;
        } else {
          // if we didn't respond to the AT, assume we're not in command mode
          inCommandMode = false;
        }
      }
      delay(250);
    }
    return success;
  }

  void maintainImpl() {
    // this only happens OUTSIDE command mode, so if we're getting characters
    // they should be data received from the TCP connection
  }

  bool factoryDefaultImpl() {
    XBEE_COMMAND_START_DECORATOR(5, false)
    sendAT(GF("RE"));
    bool ret_val = waitResponse() == 1;
    ret_val &= writeChanges();
    XBEE_COMMAND_END_DECORATOR
    // Make sure the guard time for the modem object is set back to default
    // otherwise communication would fail after the reset
    guardTime = 1010;
    return ret_val;
  }

  String getModemInfoImpl() {
    return sendATGetString(GF("HS"));
  }

  /*
   * XBee identifier functions
   */
 public:
  /**
   * @brief Get the type of XBee module currently in use.
   * @return The current Bee Type.
   */
  XBeeType getBeeType() {
    return beeType;
  }

  /**
   * @brief Get the name of the XBee module currently in use.
   * @return The name of the current XBee module.
   */
  String getBeeName() {
    return getModemNameImpl();
  }

  /// Get the series of the XBee module and store it internally
  void getSeries(void) {
    sendAT(GF("HS"));  // Get the "Hardware Series";
    int16_t intRes = readResponseInt();
    // if no response from module, then try again
    if (intRes == 0xff || intRes == -9999) {
      sendAT(GF("HS"));  // Get the "Hardware Series";
      intRes = readResponseInt();
      if (intRes == 0xff || intRes == -9999) {
        // Still no response, leave a known value - should reset
        intRes = static_cast<int16_t>(XBeeType::XBEE_UNKNOWN);
      }
    }
    beeType = static_cast<XBeeType>(intRes);
    DBG(GF("### Modem: "), getModemName(), intRes);
  }

  /*
   * Power functions
   */
 protected:
  // The XBee's have a bad habit of getting into an unresponsive funk
  // This uses the board's hardware reset pin to force it to reset
  void pinReset(const char* pin = nullptr) {
    if (resetPin >= 0) {
      DBG("### Forcing a modem reset!\r\n");
      digitalWrite(resetPin, LOW);
      delay(1);
      digitalWrite(resetPin, HIGH);
    } else {
      DBG("### Attempting a modem software restart");
      restart(pin);
    }
  }

  bool restartImpl(const char* pin) {
    if (!commandMode()) {
      DBG("### XBee not in command mode for restart; Exit");
      return false;
    }  // Return immediately

    if (beeType == XBeeType::XBEE_UNKNOWN)
      getSeries();  // how we restart depends on this

    if (beeType != XBeeType::XBEE_S6B_WIFI) {
      sendAT(GF("AM1"));  // Digi suggests putting cellular modules into
                          // airplane mode before restarting This allows the
                          // sockets and connections to close cleanly
      if (waitResponse() != 1) return exitAndFail();
      if (!writeChanges()) return exitAndFail();
    }

    sendAT(GF("FR"));
    if (waitResponse() != 1)
      return exitAndFail();
    else
      inCommandMode = false;  // Reset effectively exits command mode

    if (beeType == XBeeType::XBEE_S6B_WIFI)
      delay(2000);  // Wifi module actually resets about 2 seconds later
    else
      delay(100);  // cellular modules wait 100ms before reset happens

    // Wait until reboot completes and XBee responds to command mode call again
    for (uint32_t start = millis(); millis() - start < 60000L;) {
      if (commandMode(1)) break;
      delay(250);  // wait a litle before trying again
    }

    if (beeType != XBeeType::XBEE_S6B_WIFI) {
      sendAT(GF("AM0"));  // Turn off airplane mode
      if (waitResponse() != 1) return exitAndFail();
      if (!writeChanges()) return exitAndFail();
    }

    exitCommand();

    return init(pin);
  }

  void setupPinSleep(bool maintainAssociation = false) {
    XBEE_COMMAND_START_DECORATOR(5, )

    if (beeType == XBeeType::XBEE_UNKNOWN)
      getSeries();  // Command depends on series

    bool changesMade = false;

    // Pin sleep
    changesMade |= changeSettingIfNeeded(GF("SM"), 0x1);

    if (beeType == XBeeType::XBEE_S6B_WIFI && !maintainAssociation) {
      // For lowest power, dissassociated deep sleep
      changesMade |= changeSettingIfNeeded(GF("SO"), 0x200);
    } else if (!maintainAssociation) {
      // For supported cellular modules, maintain association
      // Not supported by all modules, will return "ERROR"
      changesMade |= changeSettingIfNeeded(GF("SO"), 0x1);
    }

    if (changesMade) { writeChanges(); }
    XBEE_COMMAND_END_DECORATOR
  }

  bool
  powerOffImpl() {  // NOTE:  Not supported for WiFi or older cellular firmware
    XBEE_COMMAND_START_DECORATOR(5, false)
    sendAT(GF("SD"));
    bool ret_val = waitResponse(120000L) == 1;
    // make sure we're really shut down
    if (ret_val) { ret_val &= (sendATGetString(GF("AI")) == "2D"); }
    XBEE_COMMAND_END_DECORATOR
    return ret_val;
  }

  // Enable airplane mode
  bool radioOffImpl() {
    bool success     = true;
    bool changesMade = false;
    XBEE_COMMAND_START_DECORATOR(5, false)
    changesMade = changeSettingIfNeeded(GF("AM"), 0x1, 5000L);
    if (changesMade) { success = writeChanges(); }
    XBEE_COMMAND_END_DECORATOR
    return success;
  }

  bool sleepEnableImpl(bool enable) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool setPhoneFunctionalityImpl(uint8_t fun,
                                 bool    reset) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 protected:
  XBeeRegStatus parseWifiRegStatus(int16_t code) {
    // 0x00 Successfully joined an access point, established IP addresses and
    // IP listening sockets
    if (code == 0x00) return XBeeRegStatus::REG_OK;
    // 0x01 Wi-Fi transceiver initialization in progress.
    // 0x02 Wi-Fi transceiver initialized, but not yet scanning for access
    // point.
    // 0x40 Waiting for WPA or WPA2 Authentication.
    // 0x41 Device joined a network and is waiting for IP configuration to
    // complete
    // 0x42 Device is joined, IP is configured, and listening sockets are being
    // set up.
    // 0xFF Device is currently scanning for the configured SSID.
    if (code == 0x01 || code == 0x02 || code == 0x40 || code == 0x41 ||
        code == 0x42 || code == 0xFF)
      return XBeeRegStatus::REG_SEARCHING;
    // 0x13 Disconnecting from access point.
    if (code == 0x13) {
      restart();  // S6B tends to get stuck "disconnecting"
      return XBeeRegStatus::REG_UNREGISTERED;
    }
    // 0x23 SSID not configured.
    if (code == 0x23) return XBeeRegStatus::REG_UNREGISTERED;
    // 0x24 Encryption key invalid (either NULL or invalid length for WEP).
    // 0x27 SSID was found, but join failed.
    if (code == 0x24 || code == 0x27) return XBeeRegStatus::REG_DENIED;
    return XBeeRegStatus::REG_UNKNOWN;
  }

  XBeeRegStatus parseCellularRegStatus(int16_t code) {
    // 0x00 Connected to the Internet.
    if (code == 0x00) return XBeeRegStatus::REG_OK;
    // 0x22 Registering to cellular network.
    // 0x23 Connecting to the Internet.
    // 0xFF Initializing.
    if (code == 0x22 || code == 0x23 || code == 0xFF) {
      return XBeeRegStatus::REG_SEARCHING;
    }
    // 0x25 Cellular network registration denied.
    if (code == 0x25) return XBeeRegStatus::REG_DENIED;
    // 0x2A Airplane mode.
    if (code == 0x2A) {
      sendAT(GF("AM0"));  // Turn off airplane mode
      waitResponse();
      writeChanges();
      return XBeeRegStatus::REG_UNKNOWN;
    }
    // 0x2F Bypass mode active.
    if (code == 0x2F) {
      sendAT(GF("AP0"));  // Set back to transparent mode
      waitResponse();
      writeChanges();
      return XBeeRegStatus::REG_UNKNOWN;
    }
    // 0x24 The cellular component is missing, corrupt, or otherwise in error.
    // 0x2B USB Direct active.
    // 0x2C Cellular component is in PSM (power save mode).
    // All other codes default to REG_UNKNOWN
    return XBeeRegStatus::REG_UNKNOWN;
  }

  XBeeRegStatus getRegistrationStatusImpl() {
    XBEE_COMMAND_START_DECORATOR(5, XBeeRegStatus::REG_UNKNOWN)

    if (!inCommandMode) return XBeeRegStatus::REG_UNKNOWN;
    if (beeType == XBeeType::XBEE_UNKNOWN) getSeries();

    sendAT(GF("AI"));
    int16_t       intRes = readResponseInt(10000L);
    XBeeRegStatus stat   = (beeType == XBeeType::XBEE_S6B_WIFI)
        ? parseWifiRegStatus(intRes)
        : parseCellularRegStatus(intRes);

    XBEE_COMMAND_END_DECORATOR
    return stat;
  }

  int8_t getSignalQualityImpl() {
    XBEE_COMMAND_START_DECORATOR(5, 0);

    if (beeType == XBeeType::XBEE_UNKNOWN)
      getSeries();  // Need to know what type of bee so we know how to ask

    if (beeType == XBeeType::XBEE_S6B_WIFI)
      sendAT(GF("LM"));  // ask for the "link margin" - the dB above sensitivity
                         // (~RSSI)
    else
      sendAT(GF("DB"));  // ask for the cell strength in dBm (~RSSI)
    int16_t intRes = readResponseInt();

    XBEE_COMMAND_END_DECORATOR

    if (beeType == XBeeType::XBEE3_LTEM_ATT && intRes == 105)
      intRes = 0;  // tends to reply with "69" when signal is unknown

    if (beeType == XBeeType::XBEE_S6B_WIFI) {
      if (intRes == 0xFF || intRes == -9999) {
        return 0;  // 0xFF returned for unknown
      } else {
        return -93 + intRes;  // the maximum sensitivity is -93dBm
      }
    } else {
      return -1 * intRes;  // need to convert to negative number
    }
  }

  bool isNetworkConnectedImpl() {
    // first check for association indicator
    XBeeRegStatus s = this->getRegistrationStatus();
    if (s == XBeeRegStatus::REG_OK) {
      if (beeType == XBeeType::XBEE_S6B_WIFI) {
        // For wifi bees, if the association indicator is ok, check that a both
        // a local IP and DNS have been allocated
        IPAddress ip  = localIP();
        IPAddress dns = getDNSAddress();
        if (ip != NULL_IP && dns != NULL_IP) {
          return true;
        } else {
          return false;
        }
      } else {
        return true;
      }
    } else {
      return false;
    }
  }

  bool waitForNetworkImpl(uint32_t timeout_ms, bool check_signal) {
    bool retVal = false;
    XBEE_COMMAND_START_DECORATOR(5, false)
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      if (check_signal) { getSignalQuality(); }
      if (isNetworkConnected()) {
        retVal = true;
        break;
      }
      delay(250);  // per Neil H. - more stable with delay
    }
    XBEE_COMMAND_END_DECORATOR
    return retVal;
  }

  String getLocalIPImpl() {
    XBEE_COMMAND_START_DECORATOR(5, "")
    sendAT(GF("MY"));
    String IPaddr;
    IPaddr.reserve(16);
    // wait for the response - this response can be very slow
    IPaddr = readResponseString(30000);
    XBEE_COMMAND_END_DECORATOR
    IPaddr.trim();
    return IPaddr;
  }

  String getDNS() {
    XBEE_COMMAND_START_DECORATOR(5, "")
    switch (beeType) {
      case XBeeType::XBEE_S6B_WIFI: {
        sendAT(GF("NS"));
        break;
      }
      default: {
        sendAT(GF("N1"));
        break;
      }
    }
    String DNSaddr;
    DNSaddr.reserve(16);
    // wait for the response - this response can be very slow
    DNSaddr = readResponseString(30000);
    XBEE_COMMAND_END_DECORATOR
    DNSaddr.trim();
    return DNSaddr;
  }

  IPAddress getDNSAddress() {
    return TinyGsmIpFromString(getDNS());
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
  // Uses the secure client inherited from TinyGsmSSL.tpp for setting the
  // certificate name and the SSL connection type so those can be called at
  // connection time, but this library does **NOT** currently support uploading,
  // deleting, or converting certificates on the modem.
  // Uploading the certificates is easy using the Digi XBee Studio program.

#undef TINY_GSM_MODEM_CAN_LOAD_CERTS

  /*
   * Wifi functions
   */
 protected:
  bool networkConnectImpl(const char* ssid, const char* pwd) {
    bool changesMade = false;
    bool retVal      = true;

    XBEE_COMMAND_START_DECORATOR(5, false)

    if (ssid == nullptr) retVal = false;

    changesMade |= changeSettingIfNeeded(GF("ID"), ssid);

    if (pwd && strlen(pwd) > 0) {
      // Set security to WPA2
      changesMade |= changeSettingIfNeeded(GF("EE"), 0x2);
      // set the password
      // the wifi bee will NOT return the previously set password,
      // so we have no way of knowing if the passwords has changed
      // and must re-write to flash each time
      sendAT(GF("PK"), pwd);
      if (waitResponse() != 1) {
        retVal = false;
      } else {
        changesMade = true;
      }
    } else {
      changesMade |= changeSettingIfNeeded(GF("EE"), 0x0);  // Set No security
    }

    if (changesMade) { retVal &= writeChanges(); }

    XBEE_COMMAND_END_DECORATOR

    return retVal;
  }

  bool networkDisconnectImpl() {
    XBEE_COMMAND_START_DECORATOR(5, false)
    sendAT(GF("NR0"));  // Do a network reset in order to disconnect
    // WARNING:  On wifi modules, using a network reset will not
    // allow the same ssid to re-join without rebooting the module.
    int8_t res = (1 == waitResponse(5000));
    writeChanges();
    XBEE_COMMAND_END_DECORATOR
    return res;
  }

  /*
   * Cellular data functions
   */
 protected:
  bool dataConnectImpl(const char* apn, const char* user, const char* pwd) {
    bool success     = true;
    bool changesMade = false;
    XBEE_COMMAND_START_DECORATOR(5, false)

    // the cellular bees will NOT return the previously set username or
    // password, so we have no way of knowing if they have changed
    // and must re-write to flash each time
    if (user && strlen(user) > 0) {
      sendAT(GF("CU"), user);  // Set the user for the APN
      if (waitResponse() != 1) {
        success = false;
      } else {
        changesMade = true;
      }
    }
    if (pwd && strlen(pwd) > 0) {
      sendAT(GF("CW"), pwd);  // Set the password for the APN
      if (waitResponse() != 1) {
        success = false;
      } else {
        changesMade = true;
      }
    }
    changesMade |= changeSettingIfNeeded(GF("AN"), String(apn));  // Set the APN

    changesMade |= changeSettingIfNeeded(GF("AM"), 0x0,
                                         5000L);  // Airplane mode off

    if (changesMade) { success = writeChanges(); }
    XBEE_COMMAND_END_DECORATOR
    return success;
  }

  bool dataDisconnectImpl() {
    bool success = true;
    XBEE_COMMAND_START_DECORATOR(5, false)
    // Cheating and disconnecting by turning on airplane mode
    bool changesMade = changeSettingIfNeeded(GF("AM"), 0x1, 5000L);

    if (changesMade) { success = writeChanges(); }
    XBEE_COMMAND_END_DECORATOR
    return success;
  }

  bool isDataConnectedImpl() {
    return isNetworkConnected();
  }

  String getOperatorImpl() {
    return sendATGetString(GF("MN"));
  }

  /*
   * SIM card functions
   */
 protected:
  bool simUnlockImpl(const char* pin) {
    if (pin && strlen(pin) > 0) {
      sendAT(GF("PN"), pin);
      return waitResponse() == 1;
    }
    return false;
  }

  String getSimCCIDImpl() {
    return sendATGetString(GF("S#"));
  }

  String getIMEIImpl() {
    return sendATGetString(GF("IM"));
  }

  String getIMSIImpl() {
    return sendATGetString(GF("II"));
  }

  SimStatus getSimStatusImpl(uint32_t) {
    return SIM_READY;  // unsupported
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
  String sendUSSDImpl(const String& code) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool sendSMSImpl(const String& number, const String& text) {
    bool changesMade = false;
    if (!commandMode()) { return false; }  // Return immediately

    // Put in text messaging mode
    changesMade |= changeSettingIfNeeded(GF("IP"), 2);

    // Set the phone number
    changesMade |= changeSettingIfNeeded(GF("PH"), number);

    // Set the text delimeter to the standard 0x0D (carriage return)
    // SRGD: WHY??
    changesMade |= changeSettingIfNeeded(GF("TD"), 0x0D);

    if (changesMade) {
      if (!writeChanges()) return exitAndFail();
    }
    // Get out of command mode to actually send the text
    exitCommand();

    stream.print(text);
    stream.write(
        static_cast<char>(0x0D));  // close off with the carriage return

    return true;
  }

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
  // ATDT0 - as secs since 1/1/2000
  // ATDO1 - as ISO8601 format

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
  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  int16_t getBattVoltageImpl() {
    int16_t intRes = 0;
    XBEE_COMMAND_START_DECORATOR(5, false)
    if (beeType == XBeeType::XBEE_UNKNOWN) getSeries();
    if (beeType == XBeeType::XBEE_S6B_WIFI) {
      sendAT(GF("%V"));
      intRes = readResponseInt();
    }
    XBEE_COMMAND_END_DECORATOR
    return intRes;
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
  float getTemperatureImpl() {
    XBEE_COMMAND_START_DECORATOR(5, static_cast<float>(-9999))
    sendAT(GF("TP"));
    int16_t intRes = readResponseInt();
    XBEE_COMMAND_END_DECORATOR
    if (intRes < 0) { return -9999.0F; }
    return static_cast<float>(static_cast<int8_t>(intRes));
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
    if (context_id != 0) {
      DBG("XBee only supports 1 SSL context in transparent mode!");
      return false;
    }

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
    if (context_id != 0) {
      DBG("XBee only supports 1 SSL context in transparent mode!");
      return false;
    }

    bool success = true;

    // The $0 command sets transparent-mode TLS profile 0 using the format
    // <CA certificate>;<client certificate>;<client key>. The TL command sets
    // the TLS protocol version used for subsequently opened sockets. In the
    // XBee command set, TL = 0x3 enables TLS 1.2, while TL = 0x4 enables TLS
    // 1.3 or later. Only SSLVersion::TLS1_2 maps to 0x3; all other versions
    // (SSL3_0, TLS1_0, TLS1_1, TLS1_3, and ALL_SSL) map to 0x4.

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

    XBEE_COMMAND_START_DECORATOR(5, false)

    bool changesMadeSSL = false;

    // set the SSL version
    // The SSL/TLS versions supported by the XBee depend on the specific module,
    // but this library maps as follows: SSLVersion::TLS1_2 (3) -> TL=0x3,
    // and all other versions (SSL3_0, TLS1_0, TLS1_1, TLS1_3, ALL_SSL) ->
    // TL=0x4.
    changesMadeSSL |= changeSettingIfNeeded(
        GF("TL"), sslVersion == SSLVersion::TLS1_2 ? 0x3 : 0x4);

    // apply the correct certificates to the connection
    // NOTE: We supply the number of certs we expect to use!  The module selects
    // the authentication mode based on how many certificates are provided.
    switch (sslAuthMode) {
      case SSLAuthMode::NO_VALIDATION: {
        // set all certs to empty
        changesMadeSSL |= changeSettingIfNeeded("$0", GF(";;"));
        break;
      }
      case SSLAuthMode::CA_VALIDATION: {
        if (CAcertName != nullptr &&
            strnlen(CAcertName, TINY_GSM_CERT_NAME_LENGTH) != 0) {
          char newTLSProfile[TINY_GSM_CERT_NAME_LENGTH + 3] = {0};
          strncpy(newTLSProfile, CAcertName, strlen(CAcertName));
          strncat(newTLSProfile, ";;", 3);
          changesMadeSSL |= changeSettingIfNeeded("$0", newTLSProfile);
        } else {
          DBG("### The CA certificate name is not set or empty.");
          XBEE_COMMAND_END_DECORATOR
          return false;
        }
        break;
      }
      case SSLAuthMode::MUTUAL_AUTHENTICATION: {
        if (CAcertName != nullptr &&
            strnlen(CAcertName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
            clientCertName != nullptr &&
            strnlen(clientCertName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
            clientKeyName != nullptr &&
            strnlen(clientKeyName, TINY_GSM_CERT_NAME_LENGTH) != 0) {
          char newTLSProfile[3 * TINY_GSM_CERT_NAME_LENGTH + 3] = {0};
          strncpy(newTLSProfile, CAcertName, strlen(CAcertName));
          strncat(newTLSProfile, ";", 2);
          strncat(newTLSProfile, clientCertName, strlen(clientCertName));
          strncat(newTLSProfile, ";", 2);
          strncat(newTLSProfile, clientKeyName, strlen(clientKeyName));
          changesMadeSSL |= changeSettingIfNeeded("$0", newTLSProfile);
        } else {
          DBG("### One or more of the certificate names for mutual "
              "authentication are not set or empty.");
          XBEE_COMMAND_END_DECORATOR
          return false;
        }
        break;
      }
      case SSLAuthMode::PRE_SHARED_KEYS:
      case SSLAuthMode::CLIENT_VALIDATION:
      default: {
        DBG("### The XBee only supports SSL using no validation, server "
            "authentication, and mutual authentication.");
        XBEE_COMMAND_END_DECORATOR
        return false;
      }
    }

    if (changesMadeSSL) { success &= writeChanges(); }

    XBEE_COMMAND_END_DECORATOR
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
    // Nothing to do here but check that the mux and context IDs are valid.
    if (mux != 0) {
      DBG("XBee only supports 1 IP channel in transparent mode!");
      return false;
    }
    if (context_id != 0) {
      DBG("XBee only supports 1 SSL context in transparent mode!");
      return false;
    }
    return true;
  }

  /*
   * Client-related functions
   */

 protected:
  int16_t getConnectionIndicator() {
    XBEE_COMMAND_START_DECORATOR(5, false)
    sendAT(GF("CI"));
    int16_t intRes = readResponseInt();
    XBEE_COMMAND_END_DECORATOR
    return intRes;
  }

  IPAddress getOperatingIP() {
    String strIP;
    strIP.reserve(16);

    XBEE_COMMAND_START_DECORATOR(5, NULL_IP)
    sendAT(GF("OD"));
    strIP = stream.readStringUntil('\r');  // read result
    strIP.trim();
    XBEE_COMMAND_END_DECORATOR

    if (strIP != "" && strIP != GF("ERROR")) {
      return TinyGsmIpFromString(strIP);
    } else {
      return NULL_IP;
    }
  }

  IPAddress lookupHostIP(const char* host, int timeout_s = 45) {
    String strIP;
    strIP.reserve(16);
    uint32_t startMillis = millis();
    uint32_t timeout_ms  = ((uint32_t)timeout_s) * 1000;
    bool     gotIP       = false;
    XBEE_COMMAND_START_DECORATOR(5, NULL_IP)
    // XBee's require a numeric IP address for connection, but do provide the
    // functionality to look up the IP address from a fully qualified domain
    // name
    // NOTE: the lookup can take a while
    while ((millis() - startMillis) < timeout_ms) {
      DBG("Looking up IP address for host:", host);
      sendAT(GF("LA"), host);
      uint32_t elapsed = millis() - startMillis;
      strIP = readResponseString(elapsed < timeout_ms ? timeout_ms - elapsed
                                                      : 0);
      if (strIP != "" && strIP != GF("ERROR")) {
        gotIP                = true;
        lastHostLookupMillis = millis();
        break;
      }
      delay(2500);  // wait a bit before trying again
    }

    XBEE_COMMAND_END_DECORATOR

    if (gotIP) {
      return TinyGsmIpFromString(strIP);
    } else {
      return NULL_IP;
    }
  }

  bool configureConnection(const char* host_or_ip, uint16_t port,
                           uint8_t mux = 0) {
    bool success     = true;
    bool changesMade = false;
    bool ssl         = sockets[mux]->is_secure;

    if (ssl && beeType == XBeeType::XBEE_S6B_WIFI) {
      DBG("### The WiFi XBee S6B does not support SSL.");
      return false;
    }

    XBEE_COMMAND_START_DECORATOR(5, false)

    // Set the text delimiter for packetizing in transparent mode
    // line feed (\n) is 0x0A, carriage return (\r) is 0x0D
    // Set to zero to disable text delimiter checking. (default)
    changesMade |= changeSettingIfNeeded(GF("TD"), 0x0);

    // Set the packetization timeout - the number of character times of
    // inter-character silence required before transmission begins when
    // operating in Transparent mode.  Set RO to 0 to transmit characters as
    // they arrive instead of buffering them into one RF packet.
    // The default value is 0x03 (three character spaces)
    changesMade |= changeSettingIfNeeded(GF("RO"), 0x03);

    if (ssl) {
      // Put in SSL over TCP communication mode
      changesMade |= changeSettingIfNeeded(GF("IP"), 0x4);
    } else {
      // Put in unsecured TCP mode
      changesMade |= changeSettingIfNeeded(GF("IP"), 0x1);
    }

    // Set the "Destination Address Low"
    changesMade |= changeSettingIfNeeded(GF("DL"), String(host_or_ip), 2500L);

    // Set the destination port
    changesMade |= changeSettingIfNeeded(GF("DE"), port);

    if (changesMade) { success &= writeChanges(); }

    XBEE_COMMAND_END_DECORATOR
    return success;
  }

  bool modemConnect(const char* host, uint16_t port, uint8_t mux = 0,
                    int timeout_s = TcpConfig::kConnectTimeoutS) {
    if (mux != 0) {
      DBG("XBee only supports 1 IP channel in transparent mode!");
      return false;
    }

    // check if the host is an IP address already - if so, use that pathway
    IPAddress hostIP = TinyGsmIpFromString(String(host));
    if (hostIP != NULL_IP) {
      DBG("Host is already an IP address; connecting directly");
      return modemConnect(hostIP, port, mux, timeout_s);
    }

    // If this is a new host name, replace the saved host and wipe out the saved
    // host IP
    if (savedHost != String(host)) {
      // if this is a different FQDN than what was previously saved, set the
      // FQDN information
      savedHost = String(host);
      // and clear the saved IP and last lookup time
      savedIP              = NULL_IP;
      lastHostLookupMillis = 0;
    }

    bool success = false;

    XBEE_COMMAND_START_DECORATOR(5, false)

    // confirm the XBee type if needed so we know if we can know if connected
    if (beeType == XBeeType::XBEE_UNKNOWN) { getSeries(); }
    bool fqdnSupported = beeType != XBeeType::XBEE_S6B_WIFI &&
        beeType != XBeeType::XBEE_LTE1_VZN && beeType != XBeeType::XBEE_3G;

    if (fqdnSupported) {
      // The newer cellular modules can look up the address on the fly.
      // This is definitely the better option.
      // Call configureConnection to set up the connection parameters with the
      // FQDN and port
      // The configureConnection function will also set up SSL if needed
      success = configureConnection(host, port, mux);
      if (success) { savedIP = NULL_IP; }
    }

    // For the pre XBee3 Bees, we need to look up the IP address manually

    // Use an IP for pre-XBee3 modules or when FQDN configuration failed.
    bool useIP = !fqdnSupported || !success;
    // Only repeat the DNS lookup if there is no cached IP or it is more than 12
    // hours old.
    bool lookupNeeded = useIP &&
        (savedIP == NULL_IP || lastHostLookupMillis == 0 ||
         millis() - lastHostLookupMillis > 43200000LL);
    if (lookupNeeded) {
      // This will return NULL_IP if lookup fails
      savedIP = lookupHostIP(host, timeout_s);
    }

    char hostFromIP[16] = {};

    // If we now have a valid IP address, use it to connect
    if (useIP && savedIP != NULL_IP) {
      // Call configureConnection to set up the connection parameters with the
      // found IP address and port
      TinyGsmIptoBuffer(savedIP, hostFromIP);
      success = configureConnection(hostFromIP, port, mux);
    }

    if (success) { success &= modemConnectXBee(timeout_s); }

    XBEE_COMMAND_END_DECORATOR

    return success;
  }


  bool modemConnect(
      IPAddress ip, uint16_t port, uint8_t mux = 0,
      int __attribute__((unused)) timeout_s = TcpConfig::kConnectTimeoutS) {
    if (mux != 0) {
      DBG("XBee only supports 1 IP channel in transparent mode!");
      return false;
    }

    if (ip != savedIP) {
      // if this is a different IP than what was previously saved or looked up,
      // set the saved IP
      savedIP = ip;
      // clear the saved host since we're directly connecting to an IP address
      savedHost = "";
      // clear the last lookup time since we have a new IP
      lastHostLookupMillis = 0;
    }

    bool success = true;

    XBEE_COMMAND_START_DECORATOR(5, false)

    char hostFromIP[16] = {};
    TinyGsmIptoBuffer(ip, hostFromIP);
    success = configureConnection(hostFromIP, port, mux);

    if (success) { success &= modemConnectXBee(timeout_s); }

    XBEE_COMMAND_END_DECORATOR

    return success;
  }

  bool modemConnectXBee(int timeout_s = TcpConfig::kConnectTimeoutS) {
    // clear the address we were last 100% certain was connected
    lastConnectedIP = NULL_IP;

    // Attempt to ping the remote host to check connectivity
    String host_or_ip = sendATGetString(GF("DL"));
    DBG("Attempting to ping", host_or_ip);
    sendAT(GF("PG"), host_or_ip);
    // NOTE: We don't actually care what ping time is, just whether there's an
    // error
    // NOTE: The user guide says the ping should time out after 5s, but it
    // sometimes takes longer for me.
    // NOTE: The error response has to be the first option so the "ERROR\r" is
    // caught before the "value\r" response.
    /*int8_t res =*/
    waitResponse(15000L, GFP(ModemConfig::GSM_ERROR), GF("\r"));

#if 0
    // If we get an error on the ping, send new line to see what happens
    if (res != 2) {
      // stream.read();  // eat the carriage return after the "ERROR"
      DBG(GF("Ping failed, exiting command and sending blank lines"));
      // exit command mode so we're sending the line[s] to the server
      exitCommand();
      streamPrint(GF("\r\n"));
      stream.flush();
      delay(4);  // wait >3 charater spaces (at 9600) to ensure the line is
                 // packetized and sent.
    }
#endif

    // DBG(GF("Waiting for sock to connect"));
    // NOTE: modemGetConnected will set sockConnected
    uint32_t start = millis();
    do {
      modemGetConnected(0);
      if (sockets[0]->sock_connected == 0) { delay(250); }
    } while (sockets[0]->sock_connected == 0 &&
             millis() - start < timeout_s * 1000LL);
    bool success = (sockets[0]->sock_connected != 0);
    return success;
  }

  void resetSocketTimeout() {
    // Get the current socket timeout
    sendAT(GF("TM"));
    int16_t timeoutUsed = readResponseInt(10000L);
    if (timeoutUsed <= 0) {
      timeoutUsed =
          0xBB8;  // XBee default socket timeout (5 minutes = 3000 x 100 ms)
    }
    char timeoutStr[5] = {};
    itoa(timeoutUsed, timeoutStr, HEX);
    // For cellular models, per documentation: If you write the TM (socket
    // timeout) value while in Transparent Mode, the current connection is
    // immediately closed - this works even if the TM values is unchanged
    sendAT(GF("TM"), timeoutStr);  // Re-set socket timeout
    waitResponse(10000L);          // This response can be slow
  }

  bool modemStop(uint8_t /*mux*/, uint32_t /*maxWaitMs*/) {
    streamClear();  // Empty anything in the buffer
    // clear the address we were last 100% certain was connected
    lastConnectedIP = NULL_IP;

    XBEE_COMMAND_START_DECORATOR(5, false)

    // For WiFi models, there's no direct way to close the socket;
    // use DigiXBeeWifi::disconnectInternet(void)

    if (beeType != XBeeType::XBEE_S6B_WIFI) { resetSocketTimeout(); }

    XBEE_COMMAND_END_DECORATOR
    return true;
  }

  bool modemBeginSend(size_t, uint8_t mux) {
    if (mux != 0) {
      DBG("XBee only supports 1 IP channel in transparent mode!");
      return false;
    }
    return true;
  }
  size_t modemSend(const uint8_t* buff, size_t len, uint8_t mux) {
    if (mux != 0) {
      DBG("XBee only supports 1 IP channel in transparent mode!");
      return 0;
    }
    return stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  }
  size_t modemEndSend(size_t len, uint8_t) {
    if (beeType != XBeeType::XBEE_S6B_WIFI) {
      // After a send, verify the outgoing ip if it isn't set
      if (lastConnectedIP == NULL_IP) {
        modemGetConnected(0);
      } else if (len > 5) {
        // After sending several characters, also re-check
        // NOTE:  I'm intentionally not checking after every single character!
        modemGetConnected(0);
      }
    }

    return len;
  }

  // Helper function to handle cellular connection status for 0x28/0xFF cases
  bool isDestinationCorrect(IPAddress od) {
    if (od != NULL_IP && (savedIP != NULL_IP && od != savedIP)) {
      DBG("We're connected to the wrong endpoint", od, "not", savedIP);
      return false;
    }
    return true;
  }

  // Helper function to handle cellular connection status for 0x28/0xFF cases
  bool handleUnknownConnectionStatus(IPAddress od,
                                     bool      returnOnUnknown = true) {
    // If we previously had an operating destination and we no longer
    // do, the socket must have closed
    if (od == NULL_IP && lastConnectedIP != NULL_IP) {
      DBG("Operating IP changed from valid to invalid, we're not connected");
      return false;
    }
    // else if the operating destination exists and matches, we're good to go
    if (od != NULL_IP && (od == savedIP || lastConnectedIP == NULL_IP)) {
      // DBG("Got new IP of", od, "we should be good");
      return true;
    }
    // If we never had an operating destination, then sock may be
    // open but data never sent - this is the dreaded "we don't know"
    // DBG("We have no idea if we're connected");
    // return true;
    return returnOnUnknown;
  }

  // NOTE:  The CI command returns the status of the TCP connection as open only
  // after data has been sent on the socket.  If it returns 0xFF the socket may
  // really be open, but no data has yet been sent.  We return this unknown
  // value as true so there's a possibility it's wrong.
  bool modemGetConnected(uint8_t) {
    // If there's data available, assume we're connected and return right away
    if (stream.available() > 0) { return true; }

    XBEE_COMMAND_START_DECORATOR(5, false)
    bool connected = false;

    if (beeType == XBeeType::XBEE_UNKNOWN)
      getSeries();  // Need to know the bee type to interpret response

    if (beeType == XBeeType::XBEE_S6B_WIFI) {
      // The wifi bee can only say if it's connected to the network
      XBeeRegStatus s = this->getRegistrationStatus();
      XBEE_COMMAND_END_DECORATOR
      // if the network is connected, we hope the sockets are too
      connected                  = (s == XBeeRegStatus::REG_OK);
      sockets[0]->sock_connected = connected;  // no multiplex
      return connected;
    }

    // Cellular XBee's
    int16_t ci = getConnectionIndicator();
    // Get the operating destination address
    IPAddress od = getOperatingIP();

#if 0
    // Ask for information about any open sockets
    sendAT(GF("SI"));
    String open_socks = stream.readStringUntil('\r');
    open_socks.replace(String(GFP(ModemConfig::GSM_NL)), "");
    open_socks.trim();
    if (open_socks != "") {
      // In transparent mode, only socket 0 should be possible
      sendAT(GF("SI0"));
      String sockResponse;
      sockResponse.reserve(64);
      int8_t res = waitResponse(5000, sockResponse, GF("\r\r"),
                                GFP(ModemConfig::GSM_ERROR));
      if (res == 1) {
        int cr = sockResponse.indexOf('\r');
        // The socket ID
        String sock_id = sockResponse.substring(0, cr);
        sockResponse   = sockResponse.substring(cr + 1);
        // The state of the socket: ALLOCATED, CONNECTING, CONNECTED,
        // LISTENING, BOUND, CLOSING
        cr                = sockResponse.indexOf('\r', cr);
        String sock_state = sockResponse.substring(0, cr);
        sockResponse      = sockResponse.substring(cr + 1);
        // The protocol of the socket: (UDP, TCP, TLS
        cr                   = sockResponse.indexOf('\r');
        String sock_protocol = sockResponse.substring(0, cr);
        sockResponse         = sockResponse.substring(cr + 1);
        // The local port of the socket. This is 0 unless the socket is
        // explicitly bound to a port.
        cr                = sockResponse.indexOf('\r');
        String local_port = sockResponse.substring(0, cr);
        sockResponse      = sockResponse.substring(cr + 1);
        // The remote port of the socket.
        cr                 = sockResponse.indexOf('\r');
        String remote_port = sockResponse.substring(0, cr);
        sockResponse       = sockResponse.substring(cr + 1);
        // The remote IPv4 address for the given socket. This is 0.0.0.0 for
        // an unconnected socket.
        cr                     = sockResponse.indexOf('\r');
        String remoted_address = sockResponse.substring(0, cr);
        sockResponse           = sockResponse.substring(cr + 1);
        DBG("Socket ID: " + sock_id);
        DBG("Socket State: " + sock_state);
        DBG("Socket Protocol: " + sock_protocol);
        DBG("Local Port: " + local_port);
        DBG("Remote Port: " + remote_port);
        DBG("Remote Address: " + remoted_address);
      }
    }
#endif

    switch (ci) {
      // 0x00 = The socket is definitely open
      case 0x00: {
        // but it's possible the socket is set to the wrong place
        if (!isDestinationCorrect(od)) {
          // stop the socket since the destination is incorrect
          sockets[0]->stop();
          connected = false;
        } else {
          connected = true;
        }
        break;
      }

        // 0x28 = "Unknown."
        // 0xFF = No known status - always returned prior to sending data
      case 0x28:
      case 0xFF: {
        // DBG("Got 0x28 or 0xFF, we don't know if we're connected");
        // check if we have the right destination
        if (!isDestinationCorrect(od)) {
          // stop the socket since the destination is incorrect
          sockets[0]->stop();
          connected = false;
        } else {
          connected = handleUnknownConnectionStatus(od /*, ci == 0xFF*/);
        }
        break;
      }

      // 0x21 = User closed
      // 0x27 = Connection lost
      // If the connection is lost or timed out on our side,
      // we force close so it can reopen
      case 0x21:
      case 0x27: {
        resetSocketTimeout();
        connected = false;
        break;
      }

      // 0x02 = Invalid parameters (bad IP/host)
      // 0x12 = DNS query lookup failure
      // 0x25 = Unknown server - DNS lookup failed (0x22 for UDP socket!)
      case 0x02:
      case 0x12:
      case 0x25:
      case 0x22: {
        savedIP = NULL_IP;  // force a lookup next time!
        // also invalidate the cached host lookup time, or modemConnect will
        // keep reusing the stale address for up to 12 hours
        lastHostLookupMillis = 0;
        connected            = false;
        break;
      }

      default: {
        // If it's anything else (inc 0x02, 0x12, and 0x25)...
        // it's definitely NOT connected
        connected = false;
        break;
      }
    }

    // exit command mode
    XBEE_COMMAND_END_DECORATOR

    // set the last connected IP to what we got
    lastConnectedIP = od;

    // if we didn't have an IP before because we connected via FQDN (host
    // name), set the IP now
    if (connected && savedIP == NULL_IP && savedHost != "") { savedIP = od; }

    // set the sock connection status
    sockets[0]->sock_connected = connected;
    return connected;
  }

  /*
   * Utilities
   */
 protected:
  /**
   * @brief Handle unsolicited responses (URCs) from the XBee module.
   * @return Always false, as the XBee does not have unsolicited responses in
   * command mode.
   */
  bool handleURCs(String&) {
    return false;
  }

 public:
  /// Clear out the stream buffer
  void streamClear() {
    while (streamTimedAvailable(1, TINY_GSM_INTERCHAR_TIMEOUT_MS) > 0) {
      stream.read();
    }
  }

  /*
   * XBee specific utilities
   */
 public:
  /**
   * @brief Put the XBee into command mode.
   *
   * If it is already in command mode, this function will return true.  If it is
   * not in command mode, it will attempt to enter command mode.
   *
   * @param retries The number of times to retry entering command mode.
   * @return True if the XBee is in command mode, false otherwise.
   */
  bool commandMode(uint8_t retries = 5) {
    // If we're already in command mode, move on
    if (inCommandMode && (millis() - lastCommandModeMillis) < 10000L)
      return true;

    uint8_t triesMade = 0;
    int8_t  res;
    bool    success         = false;
    uint8_t triesUntilReset = 4;  // reset after number of tries
    if (beeType == XBeeType::XBEE_S6B_WIFI ||
        beeType == XBeeType::XBEE3_LTEM3) {
      triesUntilReset = 9;
    }
    streamClear();  // Empty everything in the buffer before starting

    while (!success && triesMade < retries) {
      // Cannot send anything for 1 "guard time" before entering command mode
      // Default guard time is 1s, but the init fxn decreases it to 100 ms
      delay(guardTime + 10);
      stream.print(GF("+++"));  // enter command mode

      if (beeType != XBeeType::XBEE_S6B_WIFI &&
          beeType != XBeeType::XBEE3_LTEM3) {
        res = waitResponse(guardTime * 2);
      } else {
        // S6B and LTE wait a full second for OK
        res = waitResponse();
      }

      success = (1 == res);
      if (0 == res) {
        triesUntilReset--;
        if (triesUntilReset == 0) {
          triesUntilReset = 4;
          pinReset();  // if it's unresponsive, reset
          delay(250);  // a short delay to allow it to come back up
          // TODO(SRGDamia1) optimize this
        }
        if (beeType == XBeeType::XBEE_S6B_WIFI) {
          delay(5000);  // WiFi module frozen, wait longer
        }
      }
      triesMade++;
    }

    if (success) {
      inCommandMode         = true;
      lastCommandModeMillis = millis();
    }
    return success;
  }

  /**
   * @brief Write changes to flash and apply them.
   * @return True if the changes were written and applied successfully, false
   * otherwise.
   */
  bool writeChanges(void) {
    sendAT(GF("WR"));  // Write changes to flash
    if (1 != waitResponse()) { return false; }
    sendAT(GF("AC"));  // Apply changes
    if (1 != waitResponse()) { return false; }
    return true;
  }

  /// Exit command mode
  void exitCommand(void) {
    // NOTE:  Here we explicitely try to exit command mode
    // even if the internal flag inCommandMode was already false
    sendAT(GF("CN"));  // Exit command mode
    waitResponse(2500L);
    inCommandMode = false;
    delay(5);  // short wait to ensure the exit command is processed
  }

  /**
   * @brief Exit command mode and return false.
   *
   * This is a convenience function for use in functions that need to exit
   * command mode and return false on failure.
   *
   * @return False, always.
   */
  bool exitAndFail(void) {
    exitCommand();  // Exit command mode
    return false;
  }

  /**
   * @brief Reads a response from the XBee module as a string, waiting up to
   * timeout_ms milliseconds for data to become available.
   *
   * @param timeout_ms The maximum time to wait for a response, in milliseconds.
   * @return The response from the XBee module as a string.
   */
  String readResponseString(uint32_t timeout_ms = 1000L) {
    String res;
    int    firstChar = streamTimedRead(timeout_ms);
    if (firstChar < 0 || firstChar == '\r') { return res; }
    res += static_cast<char>(firstChar);
    res += stream.readStringUntil('\r');  // lines end with carriage returns
    res.trim();
    return res;
  }

  /**
   * @brief Reads a response from the XBee module as an integer, waiting up to
   * timeout milliseconds for data to become available.
   *
   * @param timeout_ms The maximum time to wait for a response, in milliseconds.
   * @return The response from the XBee module as an integer, or -9999 if there
   * is no response or an invalid character is encountered.
   */
  int16_t readResponseInt(uint32_t timeout_ms = 1000L) {
    char buf[16];  // buffer to store the response as a C-string
    int  firstChar = streamTimedRead(timeout_ms);

    // Return -9999 for empty response
    if (firstChar < 0 || firstChar == '\r') { return -9999; }

    buf[0]           = static_cast<char>(firstChar);
    size_t bytesRead = 1 +
        stream.readBytesUntil('\r', buf + 1, sizeof(buf) - 1);

    uint16_t result = 0;

    for (uint16_t i = 0; i < bytesRead; ++i) {
      char    c        = buf[i];
      uint8_t hexDigit = 0;

      // Convert valid hexadecimal digit to its numeric value
      if (c >= '0' && c <= '9') {
        hexDigit = c - '0';
      } else if (c >= 'A' && c <= 'F') {
        hexDigit = c - 'A' + 10;
      } else if (c >= 'a' && c <= 'f') {
        hexDigit = c - 'a' + 10;
      } else {
        // Invalid character outside 0-9, A-F, a-f
        return -9999;
      }

      result <<= 4;
      result |= hexDigit;
    }

    return static_cast<int16_t>(result);
  }

  /**
   * @brief Send an AT command to the XBee module and read the response as a
   * string.
   *
   * @param cmd The AT command to send.
   * @return The response from the XBee module as a string.
   */
  String sendATGetString(GsmConstStr cmd) {
    XBEE_COMMAND_START_DECORATOR(5, "")
    sendAT(cmd);
    String res = readResponseString();
    XBEE_COMMAND_END_DECORATOR
    return res;
  }

  /**
   * @brief Sends an AT command to the XBee module and changes the setting if
   * needed.
   *
   * @param cmd The AT command to send.
   * @param newValue The new value to set.
   * @param timeout_ms The maximum time to wait for a response, in
   * milliseconds.
   * @return True if the setting was changed successfully, false otherwise.
   *
   * @tparam commandType The type of the AT command.
   */
  template <typename commandType>
  bool changeSettingIfNeeded(commandType cmd, uint16_t newValue,
                             uint32_t timeout_ms = 1000L) {
    sendAT(cmd);
    if (readResponseInt() != static_cast<int16_t>(newValue)) {
      // convert the new value to a hexadecimal string
      // all numeric commands are sent in hexadecimal format
      char buf[16] = {0};
      utoa(newValue, buf, HEX);
      sendAT(cmd, buf);
      // return false if we attempted to change but failed
      if (waitResponse(timeout_ms) != 1) { return false; }
      // check if we succeeded in staging a change and retry once
      sendAT(cmd);
      if (readResponseInt() != static_cast<int16_t>(newValue)) {
        sendAT(cmd, buf);
        if (waitResponse(timeout_ms) != 1) { return false; }
      }
      // return true if we succeeded in staging a change
      return true;
    }
    // return false if no change is needed
    return false;
  }

  /**
   * @brief Sends an AT command to the XBee module and changes the setting if
   * needed.
   *
   * @param cmd The AT command to send.
   * @param newValue The new value to set.
   * @param timeout_ms The maximum time to wait for a response, in
   * milliseconds.
   * @return True if the setting was changed successfully, false otherwise.
   *
   * @tparam commandType The type of the AT command.
   */
  template <typename commandType>
  bool changeSettingIfNeeded(commandType cmd, String newValue,
                             uint32_t timeout_ms = 1000L) {
    sendAT(cmd);
    if (readResponseString() != newValue) {
      sendAT(cmd, newValue);
      // return false if we attempted to change but failed
      if (waitResponse(timeout_ms) != 1) { return false; }
      // check if we succeeded in staging a change and retry once
      sendAT(cmd);
      if (readResponseString() != newValue) {
        sendAT(cmd, newValue);
        if (waitResponse(timeout_ms) != 1) { return false; }
      }
      return true;
    }
    // return false if no change is needed
    return false;
  }

 public:
  /// Stream used to communicate with the modem.
  Stream& stream;

 protected:
  GsmClientXBee* sockets[TcpConfig::kMuxCount];
  int16_t        guardTime;
  /// The type of XBee we're working with
  XBeeType beeType;
  // A physical pin used on the mcu to reset the XBee module
  int8_t resetPin;
  /// The IP address we last requested - this may have been directly requested
  /// or resolved from a host name.
  IPAddress savedIP;
  /// The text name of the host we last requested a connection to
  String savedHost;
  /// The last IP address we actually were connected to
  /// This is set in modemGetConnected() when we're 100% sure we're connected
  /// and cleared in modemConnectXBee() and stop()
  IPAddress lastConnectedIP;
  bool      inCommandMode;
  uint32_t  lastCommandModeMillis;
  uint32_t  lastHostLookupMillis;
};

// cspell:words LTEM

#endif  // SRC_TINYGSMCLIENTXBEE_H_
