/**
 * @file       TinyGsmClientESP8266NonOS.h
 * @brief      ESP8266 Non-OS modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup espressif_esp8266_nonos Espressif ESP8266 (Non-OS)
 * @ingroup espressif_at
 * @brief Manufacturer: Espressif. Models: ESP8266 (Non-OS AT firmware, version
 * ~1.7.4).
 *
 * @warning This is an outdated modem firmware.  Please update your module to
 * the latest version.
 *
 * # Connection Information
 *
 * - Combined TCP/SSL sockets:
 *   - 5
 *   - Using more than 1 SSL socket at a time may cause the module to crash.
 * - SSL contexts: 2
 * - Socket Buffering:
 *   - The modem does **not** have an internal buffer for incoming data.
 *   - You must set the buffer size in this library to be larger than your
 * largest expected incoming packet, or you will lose incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 */

#ifndef SRC_TINYGSMCLIENTESP8266NONOS_H_
#define SRC_TINYGSMCLIENTESP8266NONOS_H_
#pragma message("TinyGSM:  TinyGsmClientESP8266NonOS")

#include "TinyGsmClientEspressif.h"
#include "mixins/TinyGsmTCP.tpp"

// NOTE: This module supports SSL, but we do not support any certificate
// management for the non-OS version of the ESP8266 firmware.  Define
// TINY_GSM_MODEM_HAS_SSL here and do not include the SSL module
#ifndef TINY_GSM_MODEM_HAS_SSL
/// flag to indicate that the modem has Secure Socket Layer (SSL) functions
#define TINY_GSM_MODEM_HAS_SSL
#endif

/// Status of ESP8266 station interface
/// @ingroup espressif_esp8266_nonos
enum class ESP8266NonOSRegStatus {
  /// ESP8266 station is not initialized.
  REG_UNINITIALIZED = 0,
  /// ESP8266 station is initialized, but not started a Wi-Fi connection yet.
  REG_UNREGISTERED = 1,
  /// ESP8266 station connected to an AP and has obtained IP
  REG_OK_IP = 2,
  /// ESP8266 station created a TCP or UDP transmission
  REG_OK_TCP = 3,
  /// the TCP or UDP transmission of ESP8266 station disconnected
  REG_OK_NO_TCP = 4,
  /// ESP8266 station did NOT connect to an AP
  REG_DENIED = 5,
  /// ESP8266 station is in an unknown state
  REG_UNKNOWN = 6,
};

/// Basic modem configurations for the ESP8266 (NonOS)
/// @ingroup espressif_esp8266_nonos
struct TinyGsmESP8266NonOSModemConfig
    : public TinyGsmModemConfigPreset<ESP8266NonOSRegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "Espressif";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "ESP8266";
};

constexpr char TinyGsmESP8266NonOSModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmESP8266NonOSModemConfig::MODEM_MODEL[]
    __attribute__((weak));

/**
 * @brief TCP behavior and limits for the ESP8266 (NonOS) AT version.
 *
 * NOTE: There's a total limit of 5 sockets, any of them can be SSL. BUT the
 * manual warns that module may not be able to handle more than 1 SSL socket at
 * a time.
 *
 * The ESP8266 devices can receive 2048 bytes and send 1460 bytes at most in a
 * single transmission.
 * @ingroup espressif_esp8266_nonos
 */
struct TinyGsmESP8266NonOSTcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::NoModemBuffer,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 5,
          /*sendMaxSize*/ 1460,
          /*connectTimeoutS*/ 75,  // default
          /*stopTimeoutS*/ 5> {};

#if 0
/**
 * @brief SSL behavior and limits for the ESP8266 (NonOS).
 *
 * These modules don't have "SSL Contexts" per-say, but they only support 2
 * certificate sets.  The certificates are loaded and referenced by number.
 *
 * @ingroup espressif_esp8266_nonos
 */
struct TinyGsmESP8266NonOSSslConfig : public TinyGsmSslConfigPreset<
                                   /*contextCount*/ 2> {};
#endif

/**
 * @brief Class for the Espressif ESP8266 modem, which is a Wi-Fi module with
 * SSL support.
 *
 * @warning This class is used to communicate with a module that has been
 * programmed with the non-OS based AT command firmware.  If you're using this,
 * please update your module.  It's quite outdated.
 * @ingroup espressif_esp8266_nonos
 */
class TinyGsmESP8266NonOS
    : public TinyGsmEspressif<TinyGsmESP8266NonOS,
                              TinyGsmESP8266NonOSModemConfig>,
      public TinyGsmTCP<TinyGsmESP8266NonOS, TinyGsmESP8266NonOSTcpConfig> {
  friend class TinyGsmEspressif<TinyGsmESP8266NonOS,
                                TinyGsmESP8266NonOSModemConfig>;
  friend class TinyGsmModem<TinyGsmESP8266NonOS,
                            TinyGsmESP8266NonOSModemConfig>;
  friend class TinyGsmWifi<TinyGsmESP8266NonOS>;
  friend class TinyGsmTCP<TinyGsmESP8266NonOS, TinyGsmESP8266NonOSTcpConfig>;
  friend class GsmClient<TinyGsmESP8266NonOS, TinyGsmESP8266NonOSTcpConfig>;

 public:
  using ModemConfig = TinyGsmESP8266NonOSModemConfig;
  using TcpConfig   = TinyGsmESP8266NonOSTcpConfig;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup espressif_esp8266_nonos
  class GsmClientESP8266NonOS
      : public GsmClient<TinyGsmESP8266NonOS, TinyGsmESP8266NonOSTcpConfig> {
    friend class TinyGsmESP8266NonOS;

   public:
    using GsmClient<TinyGsmESP8266NonOS, TinyGsmESP8266NonOSTcpConfig>::connect;
    using GsmClient<TinyGsmESP8266NonOS, TinyGsmESP8266NonOSTcpConfig>::stop;
    using TcpConfig = TinyGsmESP8266NonOSTcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientESP8266NonOS() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is identical to the identifier
     * the modem uses internally to identify the socket for the ESP8266.
     *
     * @note The ESP8266 allows you to choose the multiplexing channel number,
     * but if the input mux channel number is already in use and other mux
     * channels are available, this library will select the next available one.
     * Use the getMux() function to get the assigned multiplexing channel number
     * after a successful connection.
     */
    explicit GsmClientESP8266NonOS(TinyGsmESP8266NonOS& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmESP8266NonOS, TinyGsmESP8266NonOSTcpConfig>(modem,
                                                                       mux) {
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
  /// @ingroup espressif_esp8266_nonos
  class GsmClientSecureESP8266NonOS : public GsmClientESP8266NonOS {
    friend class TinyGsmESP8266NonOS;

   public:
    using GsmClientESP8266NonOS::connect;
    using GsmClientESP8266NonOS::stop;
    using TcpConfig = TinyGsmESP8266NonOSTcpConfig;

    /*
     * Secure client constructors and initialization
     */
    /**
     * @brief Create a new secured TCP (SSL) client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSecureESP8266NonOS() {
      is_secure = true;
    }
    /**
     * @brief Create a new secured TCP (SSL) client and bind it to a modem and
     * optionally a multiplexing channel.
     * @copydetails GsmClientESP8266NonOS::GsmClientESP8266NonOS(
     * TinyGsmESP8266NonOS&, uint8_t)
     */
    explicit GsmClientSecureESP8266NonOS(TinyGsmESP8266NonOS& modem,
                                         uint8_t              mux = 0)
        : GsmClientESP8266NonOS(modem, mux) {
      is_secure = true;
    }
  };

  /*
   * Modem-scoped Client Aliases
   */
 public:
  /// Arduino Client type for this modem.
  using TinyGsmClient = GsmClientESP8266NonOS;
  /// Secure Arduino Client type for this modem.
  using TinyGsmClientSecure = GsmClientSecureESP8266NonOS;

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmESP8266NonOS(Stream& stream)
      : TinyGsmEspressif<TinyGsmESP8266NonOS, TinyGsmESP8266NonOSModemConfig>(
            stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  // Follows functions inherited from Espressif

  /*
   * Power functions
   */
 protected:
  // Light sleep does not work properly on the old ESP8266 non-OS AT firmware,
  // so we don't support it here.
  bool sleepEnableImpl(bool enable) TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Generic network functions
   */
 protected:
  ESP8266NonOSRegStatus getRegistrationStatusImpl() {
    sendAT(GF("+CIPSTATUS"));
    if (waitResponse(3000, GF("STATUS:")) != 1) {
      return ESP8266NonOSRegStatus::REG_UNKNOWN;
    }
    // after "STATUS:" it should return the status number (0,1,2,3,4,5),
    // followed by an OK
    int16_t statusCode = streamGetIntBefore('\n');
    waitResponse();  // wait for the OK or ERROR
    return static_cast<ESP8266NonOSRegStatus>(statusCode);
  }

  bool isNetworkConnectedImpl() {
    ESP8266NonOSRegStatus s = this->getRegistrationStatus();
    if (s == ESP8266NonOSRegStatus::REG_OK_IP ||
        s == ESP8266NonOSRegStatus::REG_OK_TCP) {
      // with these, we're definitely connected
      return true;
    } else if (s == ESP8266NonOSRegStatus::REG_OK_NO_TCP) {
      // with this, we may or may not be connected
      if (getLocalIP() == "") {
        return false;
      } else {
        return true;
      }
    } else {
      return false;
    }
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
  // No SSL functions are supported on the ESP8266 using the non-OS AT firmware.


  /*
   * Wifi functions
   */
  // Follows functions inherited from Espressif

  /*
   * Cellular data functions
   */
  // No functions of this type supported

  /*
   * SIM card functions
   */
  // No functions of this type supported

  /*
   * Audio functions
   */
  // No functions of this type supported

  /*
   * Text messaging (SMS) functions
   */
  // No functions of this type supported

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
    bool     ssl        = sockets[mux]->is_secure;
    if (ssl) {
      sendAT(GF("+CIPSSLCCONF="), mux, ',', '0');
      if (waitResponse() != 1) {
        sendAT(GF("+CIPSSLSIZE=4096"));
        waitResponse();
      }
    }
    sendAT(GF("+CIPSTART="), mux, ',', ssl ? GF("\"SSL") : GF("\"TCP"),
           GF("\",\""), host, GF("\","), port
#if defined(TINY_GSM_TCP_KEEP_ALIVE)
           ,
           ',', TINY_GSM_TCP_KEEP_ALIVE
#endif
    );
    // TODO(?): Check mux
    int8_t rsp = waitResponse(timeout_ms, GFP(ModemConfig::GSM_OK),
                              GFP(ModemConfig::GSM_ERROR),
                              GF("ALREADY CONNECT"));
    if (rsp == 3) waitResponse();
    // May return "ERROR" after the "ALREADY CONNECT"
    return (1 == rsp || 3 == rsp);  // OK or ALREADY CONNECT
  }

  // Disambiguate modemStop by using the Espressif implementation
  using TinyGsmEspressif<TinyGsmESP8266NonOS,
                         TinyGsmESP8266NonOSModemConfig>::modemStop;

  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPSEND="), mux, ',', len);
    return waitResponse(GF(">")) == 1;
  }
  // Between the modemBeginSend and modemEndSend, modemSend calls:
  // stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  // stream.flush();
  size_t modemEndSend(size_t len, uint8_t) {
    if (waitResponse(30000L, GF("SEND OK\r\n"), GF("SEND FAIL\r\n"),
                     GFP(ModemConfig::GSM_ERROR)) != 1) {
      return 0;
    }
    return len;
  }

  bool modemGetConnected(uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPSTATUS"));
    if (waitResponse(3000, GF("STATUS:")) != 1) { return false; }
    // after "STATUS:" it should return the status number (0,1,2,3,4,5),
    // followed by an OK
    // Hopefully we'll catch the "3" here, but fall back to the OK or Error
    int8_t status = waitResponse(GF("3"), GFP(ModemConfig::GSM_OK),
                                 GFP(ModemConfig::GSM_ERROR));
    // if the status is anything but 3, there are no connections open
    if (status != 1) {
      for (uint8_t muxNo = 0; muxNo < TcpConfig::kMuxCount; muxNo++) {
        if (sockets[muxNo]) { sockets[muxNo]->sock_connected = false; }
      }
      return false;
    }
    // initialize the connection array assuming no connections are active
    bool verified_connections[TcpConfig::kMuxCount] = {0};
    for (uint8_t muxNo = 0; muxNo < TcpConfig::kMuxCount; muxNo++) {
      uint8_t has_status = waitResponse(GF("+CIPSTATUS:"),
                                        GFP(ModemConfig::GSM_OK),
                                        GFP(ModemConfig::GSM_ERROR));
      if (has_status == 1) {
        int16_t returned_mux = streamGetIntBefore(',');
        streamSkipUntil(',');   // Skip type
        streamSkipUntil(',');   // Skip remote IP
        streamSkipUntil(',');   // Skip remote port
        streamSkipUntil(',');   // Skip local port
        streamSkipUntil('\n');  // Skip client/server type
        if (returned_mux >= 0 && returned_mux < TcpConfig::kMuxCount) {
          verified_connections[returned_mux] = 1;
        }
      }
      if (has_status == 2) break;  // once we get to the ok, stop
    }
    for (uint8_t muxNo = 0; muxNo < TcpConfig::kMuxCount; muxNo++) {
      if (sockets[muxNo]) {
        sockets[muxNo]->sock_connected = verified_connections[muxNo];
      }
    }
    return verified_connections[mux];
  }

  /*
   * Utilities
   */
 protected:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+IPD,"))) {
      int16_t mux          = streamGetIntBefore(',');
      int16_t len_reported = streamGetIntBefore(':');
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
    } else if (data.endsWith(GF("CLOSED\r\n"))) {
      int16_t muxStart =
          TinyGsmMax(0,
                     data.lastIndexOf(String(GFP(ModemConfig::GSM_NL)),
                                      data.length() - 8));
      int16_t coma = data.indexOf(',', muxStart);
      if (coma > muxStart + 2) {
        int16_t mux = data.substring(muxStart + 2, coma).toInt();
        if (isValidMux(mux)) {
          sockets[static_cast<uint8_t>(mux)]->sock_connected = false;
        }
        DBG("### Closed: ", mux);
      }
      data = "";
      return true;
    } else if (data.endsWith(GF("busy p..."))) {
      streamSkipUntil('\n');
      data = "";
      // DBG("### Busy, please wait");
      return true;
    } else if (data.endsWith(GF("ready\r\n"))) {
      streamSkipUntil('\n');
      data = "";
      // DBG("### Module ready!");
      return true;
    } else if (data.endsWith(GF("WIFI GOT IP"))) {
      streamSkipUntil('\n');
      data = "";
      // DBG("### Wifi got IP");
      return true;
    } else if (data.endsWith(GF("WIFI CONNECTED"))) {
      streamSkipUntil('\n');
      data = "";
      // DBG("### Wifi connected");
      return true;
    } else if (data.endsWith(GF("WIFI DISCONNECT"))) {
      streamSkipUntil('\n');
      data = "";
      // DBG("### Wifi disconnected");
      return true;
    }
    return false;
  }

 protected:
  GsmClientESP8266NonOS* sockets[TcpConfig::kMuxCount];
};

#endif  // SRC_TINYGSMCLIENTESP8266NONOS_H_
