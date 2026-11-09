/**
 * @file       TinyGsmClientM590.h
 * @brief      M590 modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup neoway_m590 Neoway M590
 * @ingroup group_neoway
 * @brief Manufacturer: Neoway. Models: M590.
 *
 * # Connection Information
 *
 * - TCP sockets:
 *   - 2
 *   - SSL is not supported
 * - Socket Buffering:
 *   - The modem does **not** have an internal buffer for incoming data.
 *   - You must set the buffer size in this library to be larger than your
 * largest expected incoming packet, or you will lose incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 *
 * @todo In `dataConnectImpl()`: configure DNS settings if needed
 * @todo In `dataDisconnectImpl()`: There is no command in AT command set
 * @todo In `modemConnect()`: no need for loop?
 */

#ifndef SRC_TINYGSMCLIENTM590_H_
#define SRC_TINYGSMCLIENTM590_H_
#pragma message("TinyGSM:  TinyGsmClientM590")

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmCellularData.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmTime.tpp"

/// Neoway M590 Registration status
/// @remark The Neoway M590 uses different numbers for registration status than
/// most other modules.
/// @ingroup neoway_m590
enum class M590RegStatus {
  REG_NO_RESULT    = -1,  ///< No result yet
  REG_UNREGISTERED = 0,   ///< Not registered
  REG_SEARCHING    = 3,   ///< Searching for network
  REG_DENIED       = 2,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered, home network
  REG_OK_ROAMING   = 5,   ///< Registered, roaming
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/// Basic modem configurations for the Neoway M590
/// @ingroup neoway_m590
struct TinyGsmM590ModemConfig : public TinyGsmModemConfigPreset<M590RegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "Neoway";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "M590";
};

constexpr char TinyGsmM590ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmM590ModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for the Neoway M590
 *
 * The M590 can send up to 2000 bytes at a time with TCPSEND
 * @ingroup neoway_m590
 */
struct TinyGsmM590TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::NoModemBuffer,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 2,
          /*sendMaxSize*/ 2000,
          /*connectTimeoutS*/ 75,  // default
          /*stopTimeoutS*/ 1> {};

/// Class for the Neoway M590
/// @ingroup neoway_m590
class TinyGsmM590 : public TinyGsmModem<TinyGsmM590, TinyGsmM590ModemConfig>,
                    public TinyGsmCellularData<TinyGsmM590>,
                    public TinyGsmTCP<TinyGsmM590, TinyGsmM590TcpConfig>,
                    public TinyGsmSMS<TinyGsmM590>,
                    public TinyGsmTime<TinyGsmM590> {
  friend class TinyGsmModem<TinyGsmM590, TinyGsmM590ModemConfig>;
  friend class TinyGsmCellularData<TinyGsmM590>;
  friend class TinyGsmTCP<TinyGsmM590, TinyGsmM590TcpConfig>;
  friend class GsmClient<TinyGsmM590, TinyGsmM590TcpConfig>;
  friend class TinyGsmSMS<TinyGsmM590>;
  friend class TinyGsmTime<TinyGsmM590>;

 public:
  using ModemConfig = TinyGsmM590ModemConfig;
  using TcpConfig   = TinyGsmM590TcpConfig;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup neoway_m590
  class GsmClientM590 : public GsmClient<TinyGsmM590, TinyGsmM590TcpConfig> {
    friend class TinyGsmM590;

   public:
    using GsmClient<TinyGsmM590, TinyGsmM590TcpConfig>::connect;
    using GsmClient<TinyGsmM590, TinyGsmM590TcpConfig>::stop;
    using TcpConfig = TinyGsmM590TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientM590() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is identical to the identifier
     * the modem uses internally to identify the socket for the M590.
     *
     * @note The M590 allows you choose the multiplexing channel number, but if
     * the input mux channel number is already in use and other mux channels are
     * available, this library will select the next available one.  Use the
     * getMux() function to get the assigned multiplexing channel number after a
     * successful connection.
     */
    explicit GsmClientM590(TinyGsmM590& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmM590, TinyGsmM590TcpConfig>(modem, mux) {
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
  // NOT SUPPORTED

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   *
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmM590(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientM590"));

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

  // This is extracted from the modem info
  String getModemNameImpl() {
    sendAT('I');
    String factory = stream.readStringUntil('\n');  // read the factory
    factory.trim();
    String model = stream.readStringUntil('\n');  // read the model
    model.trim();
    streamSkipUntil('\n');  // skip the revision
    waitResponse();         // wait for the OK
    return factory + ' ' + model;
  }

  // This is extracted from the modem info
  String getModemManufacturerImpl() {
    sendAT('I');
    String factory = stream.readStringUntil('\n');  // read the factory
    factory.trim();
    streamSkipUntil('\n');  // skip the model
    streamSkipUntil('\n');  // skip the revision
    if (waitResponse() == 1) { return factory; }
    return String(GFP(ModemConfig::MODEM_MANUFACTURER));
  }

  // This is extracted from the modem info
  String getModemModelImpl() {
    sendAT('I');
    streamSkipUntil('\n');                        // skip the factory
    String model = stream.readStringUntil('\n');  // read the model
    model.trim();
    streamSkipUntil('\n');  // skip the revision
    if (waitResponse() == 1) { return model; }
    return String(GFP(ModemConfig::MODEM_MODEL));
  }

  // Gets the modem firmware version
  // This is extracted from the modem info
  String getModemRevisionImpl() {
    sendAT('I');
    streamSkipUntil('\n');                      // skip the factory
    streamSkipUntil('\n');                      // skip the model
    String res = stream.readStringUntil('\n');  // read the revision
    res.trim();
    waitResponse();  // wait for the OK
    return res;
  }

  // Extra stuff here - pwr save, internal stack
  bool factoryDefaultImpl() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("+ICF=3,1"));  // 8 data 0 parity 1 stop
    waitResponse();
    sendAT(GF("+ENPWRSAVE=0"));  // Disable PWR save
    waitResponse();
    sendAT(GF("+XISP=0"));  // Use internal stack
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
    if (!setPhoneFunctionality(15)) { return false; }
    // MODEM:STARTUP
    waitResponse(60000L, GF("+PBREADY\r\n"));
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPWROFF"));
    return waitResponse(3000L) == 1;
  }

  bool sleepEnableImpl(bool enable) {
    sendAT(GF("+ENPWRSAVE="), enable);
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
  M590RegStatus getRegistrationStatusImpl() {
    return static_cast<M590RegStatus>(getRegistrationStatusXREG("CREG"));
  }

  bool isNetworkConnectedImpl() {
    M590RegStatus s = this->getRegistrationStatus();
    return (s == M590RegStatus::REG_OK_HOME ||
            s == M590RegStatus::REG_OK_ROAMING);
  }

  String getLocalIPImpl() {
    sendAT(GF("+XIIC?"));
    if (waitResponse(GF("+XIIC:")) != 1) { return ""; }
    streamSkipUntil(',');
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
  // No functions of this type supported

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

    sendAT(GF("+XISP=0"));
    waitResponse();

    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    if (!user) user = "";
    if (!pwd) pwd = "";
    sendAT(GF("+XGAUTH=1,1,\""), user, GF("\",\""), pwd, '"');
    waitResponse();

    sendAT(GF("+XIIC=1"));
    waitResponse();

    const uint32_t timeout_ms = 60000L;
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      if (isDataConnected()) {
        // goto set_dns; // TODO
        return true;
      }
      delay(500);
    }
    return false;

#if TINY_GSM_USE_PUBLIC_DNS
    // set_dns:  // TODO
    //     sendAT(GF("+DNSSERVER=1,8.8.8.8"));
    //     waitResponse();
    //
    //     sendAT(GF("+DNSSERVER=2,8.8.4.4"));
    //     waitResponse();
#endif

    return true;
  }

  bool dataDisconnectImpl() {
    // TODO(?): There is no command in AT command set
    // XIIC=0 does not work
    return true;
  }

  bool isDataConnectedImpl() {
    sendAT(GF("+XIIC?"));
    if (waitResponse(GF("+XIIC:")) != 1) { return false; }
    int8_t res = streamGetIntBefore(',');
    waitResponse();
    return res == 1;
  }

  /*
   * SIM card functions
   */
 protected:
  // Able to follow all SIM card functions as inherited from
  // TinyGsmCellularData.tpp


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
  bool sendSMS_UTF16Impl(const String& number, const void* text,
                         size_t len) TINY_GSM_ATTR_NOT_AVAILABLE;

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
  // Follows all NTP server functions as inherited from TinyGsmNTP.tpp

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
  // No functions of this type supported

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
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    for (int i = 0; i < 3; i++) {  // TODO(?): no need for loop?
      String ip = dnsIpQuery(host);

      sendAT(GF("+TCPSETUP="), mux, ',', ip, ',', port);
      int8_t rsp = waitResponse(timeout_ms, GF(",OK\r\n"), GF(",FAIL\r\n"),
                                GF("+TCPSETUP:Error\r\n"));
      if (1 == rsp) {
        return true;
      } else if (3 == rsp) {
        sendAT(GF("+TCPCLOSE="), mux);
        waitResponse();
      }
      delay(1000);
    }
    return false;
  }

  // re-implement so we don't have an extra flush
  size_t modemSend(const uint8_t* buff, size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
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
        // start up a send command
        send_success = modemBeginSend(sendLength, mux);
        if (!send_success) {
          send_attempts++;
          continue;
        }
        // write out the number of bytes for this chunk
        int16_t attempted =
            stream.write(reinterpret_cast<const uint8_t*>(txPtr), sendLength);
        // NOTE: Don't flush here! Differ that to the modemEndSend() function
        // End this send command and check its responses
        // NOTE: In many cases, confirmed is just a passthrough of len
        int16_t confirmed = modemEndSend(sendLength, mux);
        size_t  processed = TinyGsmMin(attempted, confirmed);
        // If bytes were written to the stream, don't retry that chunk even if
        // the modem doesn't confirm any bytes.
        if (attempted > 0 && processed == 0) { processed = attempted; }
        bytesSent += processed;  // bump up number of bytes sent
        txPtr += processed;      // bump up the pointer
        send_success = processed > 0;
        send_attempts++;
      }
      // if we failed after 3 attempts at the same chunk, bail from the whole
      // thing
      if (!send_success) { break; }
    } while (bytesSent < len && sockets[mux]->sock_connected);
    return bytesSent;
  }

  bool modemStop(uint8_t mux, uint32_t maxWaitMs) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+TCPCLOSE="), mux);
    return waitResponse(maxWaitMs) == 1;
  }

  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+TCPSEND="), mux, ',', (uint16_t)len);
    return waitResponse(GF(">")) == 1;
  }
  size_t modemEndSend(size_t len, uint8_t) {
    stream.write(static_cast<char>(0x0D));
    stream.flush();
    if (waitResponse(30000L, GF("+TCPSEND:")) != 1) { return 0; }
    streamSkipUntil('\n');
    return len;
  }

  bool modemGetConnected(uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPSTATUS="), mux);
    int8_t res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""),
                              GF(",\"CLOSING\""), GF(",\"INITIAL\""));
    waitResponse();
    return 1 == res;
  }

  String dnsIpQuery(const char* host) {
    sendAT(GF("+DNS=\""), host, '"');
    if (waitResponse(10000L, GF("+DNS:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse(GF("+DNS:OK\r\n"));
    res.trim();
    return res;
  }

  /*
   * Utilities
   */
 protected:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+TCPRECV:"))) {
      int16_t mux          = streamGetIntBefore(',');
      int16_t len_reported = streamGetIntBefore(',');
      if (isValidMux(mux)) {
        // This will handle an invalid len_reported value and will drain the
        // stream if there's a buffer overflow, so we don't need to validate
        // len_reported or check for overflow here.  We just need to make sure
        // the mux is valid before we call it.
        moveCharsFromStreamToFifo(mux, len_reported);
      } else {
        // If the mux is invalid, throw away the reported stream data
        streamDump(len_reported);
      }
      data = "";
      return true;
    } else if (data.endsWith(GF("+TCPCLOSE:"))) {
      int16_t mux = streamGetIntBefore(',');
      streamSkipUntil('\n');
      if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
      data = "";
      DBG("### Closed: ", mux);
      return true;
    }
    return false;
  }

 public:
  /// Stream used to communicate with the modem.
  Stream& stream;

 protected:
  GsmClientM590* sockets[TcpConfig::kMuxCount];
};

#endif  // SRC_TINYGSMCLIENTM590_H_
