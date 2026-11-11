/**
 * @file       TinyGsmClientA6.h
 * @brief      A6 modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup aithinker_a6 Ai-Thinker A6 and A7
 * @ingroup group_ai_thinker
 * @brief Manufacturer: Ai-Thinker. Models: A6, A7.
 *
 * # Connection Information
 *
 * - TCP sockets:
 *   - 8
 *   - SSL client not supported *
 * - Socket Buffering:
 *   - The modem does **not** have an internal buffer for incoming data.
 *   - You must set the buffer size in this library to be larger than your
 * largest expected incoming packet, or you will lose incoming data.
 * - Socket Numbering:
 *   - The modem will automatically assign a channel number when the client
 * connects to a server.
 *
 * @todo In `dataConnectImpl()`: wait AT+CGATT?
 * @todo In `dtmfSendImpl()`: correctly handle the duration parameter
 * @todo In `modemGetConnected()`: correctly read the mux number
 */

#ifndef SRC_TINYGSMCLIENTA6_H_
#define SRC_TINYGSMCLIENTA6_H_
#pragma message("TinyGSM:  TinyGsmClientA6")

#include "mixins/TinyGsmModem.tpp"
#include "mixins/TinyGsmTCP.tpp"
#include "mixins/TinyGsmCellularData.tpp"
#include "mixins/TinyGsmCalling.tpp"
#include "mixins/TinyGsmSMS.tpp"
#include "mixins/TinyGsmTime.tpp"
#include "mixins/TinyGsmBattery.tpp"

/// Registration status
/// @ingroup aithinker_a6
enum class A6RegStatus {
  REG_NO_RESULT    = -1,  ///< No registration result
  REG_UNREGISTERED = 0,   ///< Not registered on the network
  REG_SEARCHING    = 2,   ///< Searching for network
  REG_DENIED       = 3,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered on the home network
  REG_OK_ROAMING   = 5,   ///< Registered on a roaming network
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/// Basic modem configurations for the A6 and A7 modems
/// @ingroup aithinker_a6
struct TinyGsmA6ModemConfig : public TinyGsmModemConfigPreset<A6RegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "Ai-Thinker";
#if defined(TINY_GSM_MODEM_A7)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "A7";
#else
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "A6";
#endif
};

constexpr char TinyGsmA6ModemConfig::MODEM_MANUFACTURER[] __attribute__((weak));
constexpr char TinyGsmA6ModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for the A6 and A7 modems.
 * @ingroup aithinker_a6
 */
struct TinyGsmA6TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::NoModemBuffer,
          /*muxMode*/ TinyGsmTcpMuxMode::Dynamic,
          /*muxCount*/ 8,
          /*sendMaxSize*/ 1024,
          /*connectTimeoutS*/ 75,  // default
          /*stopTimeoutS*/ 1> {};

/**
 * @brief TinyGsmA6 is a class for controlling the Ai-Thinker A6 and A7 GSM/GPRS
 * module.
 * @ingroup aithinker_a6
 */
class TinyGsmA6 : public TinyGsmModem<TinyGsmA6, TinyGsmA6ModemConfig>,
                  public TinyGsmCellularData<TinyGsmA6>,
                  public TinyGsmTCP<TinyGsmA6, TinyGsmA6TcpConfig>,
                  public TinyGsmCalling<TinyGsmA6>,
                  public TinyGsmSMS<TinyGsmA6>,
                  public TinyGsmTime<TinyGsmA6>,
                  public TinyGsmBattery<TinyGsmA6> {
  friend class TinyGsmModem<TinyGsmA6, TinyGsmA6ModemConfig>;
  friend class TinyGsmCellularData<TinyGsmA6>;
  friend class TinyGsmTCP<TinyGsmA6, TinyGsmA6TcpConfig>;
  friend class GsmClient<TinyGsmA6, TinyGsmA6TcpConfig>;
  friend class TinyGsmCalling<TinyGsmA6>;
  friend class TinyGsmSMS<TinyGsmA6>;
  friend class TinyGsmTime<TinyGsmA6>;
  friend class TinyGsmBattery<TinyGsmA6>;

 public:
  using ModemConfig = TinyGsmA6ModemConfig;
  using TcpConfig   = TinyGsmA6TcpConfig;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup aithinker_a6
  class GsmClientA6 : public GsmClient<TinyGsmA6, TinyGsmA6TcpConfig> {
    friend class TinyGsmA6;

   public:
    using GsmClient<TinyGsmA6, TinyGsmA6TcpConfig>::connect;
    using GsmClient<TinyGsmA6, TinyGsmA6TcpConfig>::stop;
    using TcpConfig = TinyGsmA6TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientA6() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem.
     * @param modem Modem instance used by this client.
     *
     * @note The A6 and A7 do not allow you to specify the multiplexing channel.
     * The modem will automatically assign a channel when the client connects to
     * a server.  Use the getMux() function to get the assigned multiplexing
     * channel number after a successful connection.
     */
    explicit GsmClientA6(TinyGsmA6& modem, uint8_t /*mux*/ = 0)
        : GsmClient<TinyGsmA6, TinyGsmA6TcpConfig>(modem /*, mux*/) {
      is_secure = false;
      init(&modem);
    }

    /*
     * Arduino Client interface
     */
   public:
    /**
     * @copydoc GsmClient::connect(const char*, uint16_t, int)
     * @warning The A6 and A7 modems automatically assign the multiplexing
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
#if 1
      // stop if and only if the mux number is valid, the socket pointer is not
      // null, and the socket is connected
      if (mux < TcpConfig::kMuxCount && at->sockets[mux] != nullptr &&
          sock_connected) {
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
      // if the connection succeeded, but we can't register the assigned mux,
      // even with a forced overwrite, stop the connection and return false
      if (sock_connected &&
          !this->registerAssignedMux(this, assignedMux, true)) {
        at->modemStop(assignedMux, TcpConfig::kStopTimeoutS * 1000L);
        sock_connected = false;
      }
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

  // Doesn't support SSL

  /*
   * Modem-scoped Client Aliases
   */
 public:
  /// Arduino Client type for this modem.
  using TinyGsmClient = GsmClientA6;

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmA6(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientA6"));

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
    sendAT(
        GF("+CMER=3,0,0,2"));  // Set unsolicited result code output destination
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

  // Gets the modem serial number
  String getModemSerialNumberImpl() {
    sendAT(GF("GSN"));  // Not CGSN
    String res;
    if (waitResponse(1000L, res) != 1) { return ""; }
    cleanResponseString(res);
    return res;
  }

  bool factoryDefaultImpl() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
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
    sendAT(GF("+RST=1"));
    delay(3000);
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOF"));
    // +CPOF: MS OFF OK
    return waitResponse() == 1;
  }

  bool sleepEnableImpl(bool enable) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool setPhoneFunctionalityImpl(uint8_t fun,
                                 bool    reset) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 protected:
  A6RegStatus getRegistrationStatusImpl() {
    return static_cast<A6RegStatus>(getRegistrationStatusXREG("CREG"));
  }

  bool isNetworkConnectedImpl() {
    A6RegStatus s = this->getRegistrationStatus();
    return (s == A6RegStatus::REG_OK_HOME || s == A6RegStatus::REG_OK_ROAMING);
  }

  String getLocalIPImpl() {
    sendAT(GF("+CIFSR"));
    String res;
    if (waitResponse(10000L, res) != 1) { return ""; }
    cleanResponseString(res);
    return res;
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
 protected:
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

    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // TODO(?): wait AT+CGATT?

    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    if (!user) user = "";
    if (!pwd) pwd = "";
    sendAT(GF("+CSTT=\""), apn, GF("\",\""), user, GF("\",\""), pwd, '"');
    if (waitResponse(60000L) != 1) { return false; }

    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    sendAT(GF("+CIPMUX=1"));
    if (waitResponse() != 1) { return false; }

    return true;
  }

  bool dataDisconnectImpl() {
    // Shut the TCP/IP connection
    sendAT(GF("+CIPSHUT"));
    if (waitResponse(60000L) != 1) { return false; }

    for (int i = 0; i < 3; i++) {
      sendAT(GF("+CGATT=0"));
      if (waitResponse(5000L) == 1) { return true; }
    }

    return false;
  }

  String getOperatorImpl() {
    sendAT(GF("+COPS=3,0"));  // Set format
    waitResponse();

    sendAT(GF("+COPS?"));
    if (waitResponse(GF("+COPS:")) != 1) { return ""; }
    streamSkipUntil('"');  // Skip mode and format
    String res = stream.readStringUntil('"');
    waitResponse();
    return res;
  }

  /*
   * SIM card functions
   */
 protected:
  String getSimCCIDImpl() {
    sendAT(GF("+CCID"));
    if (waitResponse(GF("+SCID: SIM Card ID:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone call functions
   */
 protected:
  // Returns true on pick-up, false on error/busy
  bool callNumberImpl(const String& number) {
    if (number == GF("last")) {
      sendAT(GF("DLST"));
    } else {
      sendAT(GF("D\""), number, GF("\";"));
    }

    if (waitResponse(5000L) != 1) { return false; }

    if (waitResponse(60000L, GF("+CIEV: \"CALL\",1"), GF("+CIEV: \"CALL\",0"),
                     GFP(ModemConfig::GSM_ERROR)) != 1) {
      return false;
    }

    int8_t rsp = waitResponse(60000L, GF("+CIEV: \"SOUNDER\",0"),
                              GF("+CIEV: \"CALL\",0"));

    int8_t rsp2 = waitResponse(300L, GF("BUSY\r\n"), GF("NO ANSWER\r\n"));

    return rsp == 1 && rsp2 == 0;
  }

  // 0-9,*,#,A,B,C,D
  bool dtmfSendImpl(char cmd, int duration_ms) {
    duration_ms = constrain(duration_ms, 100, 1000);

    // The duration parameter is not working, so we simulate it using delay..
    // TODO(?): Maybe there's another way...

    // sendAT(GF("+VTD="), duration_ms / 100);
    // waitResponse();

    sendAT(GF("+VTS="), cmd);
    if (waitResponse(10000L) == 1) {
      delay(duration_ms);
      return true;
    }
    return false;
  }

  /*
   * Audio functions
   */
 public:
  /**
   * @brief Set the audio output to headphones.
   * @return True if the command was successful, false otherwise.
   */
  bool audioSetHeadphones() {
    sendAT(GF("+SNFS=0"));
    return waitResponse() == 1;
  }
  /**
   * @brief Set the audio output to speaker.
   * @return True if the command was successful, false otherwise.
   */
  bool audioSetSpeaker() {
    sendAT(GF("+SNFS=1"));
    return waitResponse() == 1;
  }
  /**
   * @brief Mute or unmute the microphone.
   * @param mute True to mute, false to unmute.
   * @return True if the command was successful, false otherwise.
   */
  bool audioMuteMic(bool mute) {
    sendAT(GF("+CMUT="), mute);
    return waitResponse() == 1;
  }

  /*
   * Text messaging (SMS) functions
   */
 protected:
  String sendUSSDImpl(const String& code) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    sendAT(GF("+CSCS=\"HEX\""));
    waitResponse();
    sendAT(GF("+CUSD=1,\""), code, GF("\",15"));
    if (waitResponse(10000L) != 1) { return ""; }
    if (waitResponse(GF("+CUSD:")) != 1) { return ""; }
    streamSkipUntil('"');
    String hex = stream.readStringUntil('"');
    streamSkipUntil(',');
    int8_t dcs = streamGetIntBefore('\n');

    if (dcs == 15) {
      return TinyGsmDecodeHex7bit(hex);
    } else if (dcs == 72) {
      return TinyGsmDecodeHex16bit(hex);
    } else {
      return hex;
    }
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
  // Follows all clock functions as inherited from TinyGsmTime.tpp
  // Note - the clock probably has to be set manaually first

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

  // Needs a '?' after CBC, unlike most
  int8_t getBattPercentImpl() {
    sendAT(GF("+CBC?"));
    if (waitResponse(GF("+CBC:")) != 1) { return -99; }
    streamSkipUntil(',');  // Skip battery charge status
    // Read battery charge level
    int16_t res = streamGetIntBefore('\n');
    // Wait for final OK
    bool validResponse = waitResponse() == 1;
    if (!validResponse || res < 0 || res > 100) { return -99; }
    return static_cast<int8_t>(res);
  }

  // Needs a '?' after CBC, unlike most
  int8_t getBattChargeStateImpl() {
    sendAT(GF("+CBC?"));
    if (waitResponse(GF("+CBC:")) != 1) { return -99; }
    int16_t res = streamGetIntBefore(',');
    // Wait for final OK
    bool validResponse = waitResponse() == 1;
    if (!validResponse || res < 0 || res > 2) { return -99; }
    return static_cast<int8_t>(res);
  }

  // Needs a '?' after CBC, unlike most
  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    // set all output variables to default invalid values
    chargeState = -99;
    percent     = -99;
    milliVolts  = -9999;

    sendAT(GF("+CBC?"));  // <- The '?' is needed!
    // if there's no response to the request, return false immediately
    if (waitResponse(GF("+CBC:")) != 1) { return false; }
    // try to parse the responses
    int16_t parsedChargeState = streamGetIntBefore(',');
    int16_t parsedPercent     = streamGetIntBefore('\n');
    // no voltage reported by this modem
    // Wait for final OK
    if (waitResponse() != 1) { return false; }

    bool results_valid = true;
    // the charge state should be between 0 and 2
    if (parsedChargeState >= 0 && parsedChargeState <= 2) {
      chargeState = static_cast<int8_t>(parsedChargeState);
    } else {
      results_valid = false;
    }
    // the percent should be between 0 and 100
    if (parsedPercent >= 0 && parsedPercent <= 100) {
      percent = static_cast<int8_t>(parsedPercent);
    } else {
      results_valid = false;
    }

    // return true if everything is valid and within the expected ranges
    return results_valid;
  }

  /*
   * Temperature functions
   */
  // No functions of this type supported

  /*
   * Client-related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t* dynamicMux,
                    int timeout_s) {
    // NOTE: Don't validate mux!  It's not the real one yet and we don't need to
    // access it for any SSL configuration
    uint32_t startMillis = millis();
    uint32_t timeout_ms  = ((uint32_t)timeout_s) * 1000;

    sendAT(GF("+CIPSTART="), GF("\"TCP"), GF("\",\""), host, GF("\","), port);
    if (waitResponse(timeout_ms, GF("+CIPNUM:")) != 1) { return false; }
    int16_t connected_mux = streamGetIntBefore('\n');

    uint32_t elapsed = millis() - startMillis;
    if (elapsed >= timeout_ms) { return false; }
    int8_t connect_rsp =
        waitResponse(timeout_ms - elapsed, GF("CONNECT OK\r\n"),
                     GF("CONNECT FAIL\r\n"), GF("ALREADY CONNECT\r\n"));
    bool success = connect_rsp == 1 || connect_rsp == 3;
    success &= waitResponse() == 1;

    // Validate the returned mux
    if (connected_mux < 0 || connected_mux >= TcpConfig::kMuxCount ||
        !success) {
      DBG(GF("ERROR: Modem returned invalid mux or connection failed"));
      *dynamicMux = static_cast<uint8_t>(-1);  // Set mux to invalid value
      return false;  // Return failure when mux is out of range
    }

    // set the mux to the new mux number if we're successful
    *dynamicMux = connected_mux;
    return success;
  }

  bool modemStop(uint8_t mux, uint32_t maxWaitMs) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPCLOSE="), mux);
    return waitResponse(maxWaitMs) == 1;
  }

  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    return waitResponse(2000L, GF("\r\n>")) == 1;
  }
  // Between the begin and end, modem send calls:
  // stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  // stream.flush();
  size_t modemEndSend(size_t len, uint8_t) {
    if (waitResponse(10000L, GFP(ModemConfig::GSM_OK), GF("FAIL")) != 1) {
      return 0;
    }
    return len;
  }

  bool modemGetConnected(uint8_t) {
    sendAT(GF("+CIPSTATUS"));  // TODO(?) mux?
    int8_t res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""),
                              GF(",\"CLOSING\""), GF(",\"INITIAL\""));
    waitResponse();
    return 1 == res;
  }

  /*
   * Utilities
   */
 protected:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+CIPRCV:"))) {
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
      DBG("### Got Data: ", len_reported, "on", mux);
      return true;
    } else if (data.endsWith(GF("+TCPCLOSED:"))) {
      int16_t mux = streamGetIntBefore('\n');
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
  GsmClientA6* sockets[TcpConfig::kMuxCount];
};

// cSpell:words aithinker

#endif  // SRC_TINYGSMCLIENTA6_H_
