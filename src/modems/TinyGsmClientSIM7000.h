/**
 * @file       TinyGsmClientSIM7000.h
 * @brief      SIM7000 modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup simcom_sim7000 SIMCom SIM7000
 * @brief Manufacturer: SIMCom. Models: SIM7000.
 * @ingroup simcom_sim70xx
 *
 * # Connection Information
 *
 * - TCP sockets:
 *   - 8
 *   - SSL is not supported
 * - Socket Buffering:
 *   - The modem has an internal buffer for incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 */

#ifndef SRC_TINYGSMCLIENTSIM7000_H_
#define SRC_TINYGSMCLIENTSIM7000_H_
#pragma message("TinyGSM:  TinyGsmClientSIM7000")

#include "TinyGsmClientSIM70xx.h"
#include "mixins/TinyGsmTCP.tpp"
#include "mixins/TinyGsmSMS.tpp"
#include "mixins/TinyGsmGSMLocation.tpp"
#include "mixins/TinyGsmTime.tpp"
#include "mixins/TinyGsmNTP.tpp"
#include "mixins/TinyGsmBattery.tpp"

/// Basic modem configurations for the SIMCom SIM7000
/// @ingroup simcom_sim7000
struct TinyGsmSim7000ModemConfig
    : public TinyGsmModemConfigPreset<Sim70xxRegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "SIMCom";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM7000";
};

constexpr char TinyGsmSim7000ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmSim7000ModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for the SIMCom SIM7000.
 * @ingroup simcom_sim7000
 */
struct TinyGsmSim7000TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 8> {};

/// Class for the SIMCOM SIM7000 using the TCP-IP toolkit
/// @ingroup simcom_sim7000
class TinyGsmSim7000
    : public TinyGsmSim70xx<TinyGsmSim7000, TinyGsmSim7000ModemConfig>,
      public TinyGsmTCP<TinyGsmSim7000, TinyGsmSim7000TcpConfig>,
      public TinyGsmSMS<TinyGsmSim7000>,
      public TinyGsmTime<TinyGsmSim7000>,
      public TinyGsmNTP<TinyGsmSim7000>,
      public TinyGsmGSMLocation<TinyGsmSim7000>,
      public TinyGsmBattery<TinyGsmSim7000> {
  friend class TinyGsmSim70xx<TinyGsmSim7000, TinyGsmSim7000ModemConfig>;
  friend class TinyGsmModem<TinyGsmSim7000, TinyGsmSim7000ModemConfig>;
  friend class TinyGsmCellularData<TinyGsmSim7000>;
  friend class TinyGsmTCP<TinyGsmSim7000, TinyGsmSim7000TcpConfig>;
  friend class GsmClient<TinyGsmSim7000, TinyGsmSim7000TcpConfig>;
  friend class TinyGsmSMS<TinyGsmSim7000>;
  friend class TinyGsmGSMLocation<TinyGsmSim7000>;
  friend class TinyGsmGPS<TinyGsmSim7000>;
  friend class TinyGsmTime<TinyGsmSim7000>;
  friend class TinyGsmNTP<TinyGsmSim7000>;
  friend class TinyGsmBattery<TinyGsmSim7000>;

 public:
  using ModemConfig = TinyGsmSim7000ModemConfig;
  using TcpConfig   = TinyGsmSim7000TcpConfig;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup simcom_sim7000
  class GsmClientSim7000
      : public GsmClient<TinyGsmSim7000, TinyGsmSim7000TcpConfig> {
    friend class TinyGsmSim7000;

   public:
    using GsmClient<TinyGsmSim7000, TinyGsmSim7000TcpConfig>::connect;
    using GsmClient<TinyGsmSim7000, TinyGsmSim7000TcpConfig>::stop;
    using TcpConfig = TinyGsmSim7000TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSim7000() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is identical to the identifier
     * the modem uses internally to identify the socket for the SIM7000.
     *
     * @note The SIM7000 allows you choose the multiplexing channel number, but
     * if the input mux channel number is already in use and other mux channels
     * are available, this library will select the next available one.  Use the
     * getMux() function to get the assigned multiplexing channel number after a
     * successful connection.
     */
    explicit GsmClientSim7000(TinyGsmSim7000& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmSim7000, TinyGsmSim7000TcpConfig>(modem, mux) {
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
  // NOTE:  Use modem TinyGsmSim7000SSL for a secure client!

  /*
   * Public Aliases
   */
 public:
  /// Arduino Client type for this modem.
  using TinyGsmClient = GsmClientSim7000;

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmSim7000(Stream& stream)
      : TinyGsmSim70xx<TinyGsmSim7000, TinyGsmSim7000ModemConfig>(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM7000"));

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

    // Enable Local Time Stamp for getting network time
    sendAT(GF("+CLTS=1"));
    if (waitResponse(10000L) != 1) { return false; }

    // Enable battery checks
    sendAT(GF("+CBATCHK=1"));
    if (waitResponse() != 1) { return false; }

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
  // Follows functions as inherited from TinyGsmClientSIM70xx.h

  /*
   * Generic network functions
   */
 protected:
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
  // NOTE:  Use modem TinyGsmSim7000SSL for a secure client!

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

    // Attach to Packet Domain service
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // Activate the PDP context
    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    // Open the definied bearer context
    sendAT(GF("+SAPBR=1,1"));
    waitResponse(85000L);
    // Query the bearer context status
    sendAT(GF("+SAPBR=2,1"));
    if (waitResponse(30000L) != 1) { return false; }

    // Set the TCP application toolkit to multi-IP
    sendAT(GF("+CIPMUX=1"));
    if (waitResponse() != 1) { return false; }

    // Put the TCP application toolkit in "quick send" mode
    // (thus no extra "Send OK")
    sendAT(GF("+CIPQSEND=1"));
    if (waitResponse() != 1) { return false; }

    // Set the TCP application toolkit to get data manually
    sendAT(GF("+CIPRXGET=1"));
    if (waitResponse() != 1) { return false; }

    // Start the TCP application toolkit task and set APN, USER NAME, PASSWORD
    sendAT(GF("+CSTT=\""), apn, GF("\",\""), user, GF("\",\""), pwd, '"');
    if (waitResponse(60000L) != 1) { return false; }

    // Bring up the TCP application toolkit wireless connection with GPRS or CSD
    sendAT(GF("+CIICR"));
    if (waitResponse(60000L) != 1) { return false; }

    // Get local IP address for the TCP application toolkit
    // only assigned after connection
    sendAT(GF("+CIFSR;E0"));
    if (waitResponse(10000L) != 1) { return false; }

    return true;
  }

  bool dataDisconnectImpl() {
    // Shut the TCP application toolkit connection
    // CIPSHUT will close *all* open TCP application toolkit connections
    sendAT(GF("+CIPSHUT"));
    if (waitResponse(60000L) != 1) { return false; }

    sendAT(GF("+CGATT=0"));  // Deactivate the bearer context
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  /*
   * SIM card functions
   */
  // Follows functions as inherited from TinyGsmClientSIM70xx.h

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
  // Follows all text messaging (SMS) functions as inherited from TinyGsmSMS.tpp

  /*
   * GSM location functions
   */
  // Follows all GSM-based location functions as inherited from
  // TinyGsmGSMLocation.tpp

  /*
   * GPS/GNSS/GLONASS location functions
   */
  // Follows functions as inherited from TinyGsmClientSIM70xx.h

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
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;

    // when not using SSL, the TCP application toolkit is more stable
    sendAT(GF("+CIPSTART="), mux, ',', GF("\"TCP"), GF("\",\""), host,
           GF("\","), port);
    return (1 ==
            waitResponse(timeout_ms, GF("CONNECT OK\r\n"),
                         GF("CONNECT FAIL\r\n"), GF("ALREADY CONNECT\r\n"),
                         GFP(ModemConfig::GSM_ERROR), GF("CLOSE OK\r\n")));
  }

  bool modemStop(uint8_t mux, uint32_t maxWaitMs) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPCLOSE="), mux);
    return waitResponse(TinyGsmMin(maxWaitMs, static_cast<uint32_t>(3000))) ==
        1;  // should return within 3s
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
    if (waitResponse(GF("DATA ACCEPT:"), GF("SEND FAIL")) != 1) { return 0; }
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
#else
    sendAT(GF("+CIPRXGET=2,"), mux, ',', (uint16_t)size);
#endif
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }

    streamSkipUntil(',');  // Skip Rx mode 2/normal or 3/HEX
    int16_t ret_mux      = streamGetIntBefore(',');  // mux
    int16_t len_reported = streamGetIntBefore(',');
    //  ^^ Requested number of data bytes (1-1460 bytes) to be read
    int16_t len_remaining = streamGetIntBefore('\n');
    // ^^ Confirmed number of data bytes to be read, which may be less than
    // requested. 0 indicates that no data can be read.
    // SRGD NOTE:  Contrary to above (which is copied from AT command manual)
    // this is actually be the number of bytes that will be remaining in the
    // buffer after the read.
    if (isValidMux(ret_mux)) {
      // Move the data to the socket buffer of the returned mux as long as the
      // returned mux is valid, even if it doesn't match the expected mux.
#ifdef TINY_GSM_USE_HEX
      len_read = moveCharsFromStreamToFifo<true>(ret_mux, len_reported);
#else
      len_read = moveCharsFromStreamToFifo(ret_mux, len_reported);
#endif
      // update the amount remaining for the returned mux, even if it doesn't
      // match the expected mux.
      sockets[ret_mux]->sock_available = len_remaining >= 0 ? len_remaining : 0;
    }
    waitResponse();  // ending OK; the waitResponse function will toss all the
                     // characters before the OK if the mux was invalid

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
                              GF(",\"INITIAL\""), GFP(ModemConfig::GSM_ERROR));
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
    } else if (data.endsWith(GF("DST: "))) {
      streamSkipUntil('\n');  // Refresh Network Daylight Saving Time by network
      data = "";
      DBG("### Daylight savings time state updated.");
      return true;
    } else if (data.endsWith(GF("SMS Ready\r\n"))) {
      data = "";
      DBG("### Unexpected module reset!");
      init();
      return true;
    }
    return false;
  }

 protected:
  GsmClientSim7000* sockets[TcpConfig::kMuxCount];
};

#endif  // SRC_TINYGSMCLIENTSIM7000_H_
