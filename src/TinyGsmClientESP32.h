/**
 * @file       TinyGsmClientESP32.h
 * @brief      ESP32 AT modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup espressif_esp32 Espressif ESP32 (AT)
 * @ingroup espressif_at
 * @brief Manufacturer: Espressif. Models: ESP32 (AT firmware, version
 * >= 3.2.0.0).
 *
 * # Connection Information
 *
 * - Combined TCP/SSL sockets:
 *   - 5
 *   - Using more than 1 SSL socket at a time may cause the module to crash.
 * - SSL contexts:
 *   - 2
 * - Socket Buffering:
 *   - The modem has an internal buffer for incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 *
 * @todo In `handleURCs()`: I'm not sure if each +IPD URC reports the amount
 * newly received or the total now in the buffer. It appears to be the latter.
 */

#ifndef SRC_TINYGSMCLIENTESP32_H_
#define SRC_TINYGSMCLIENTESP32_H_
#pragma message("TinyGSM:  TinyGsmClientESP32")

#include "TinyGsmClientEspressif.h"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"

/// State: current Wi-Fi state.
/// @ingroup espressif_esp32
enum class ESP32RegStatus {
  /// ESP32 station has not started any Wi-Fi connection.
  REG_UNINITIALIZED = 0,
  /// ESP32 station has connected to an AP, but does not get an IPv4 address
  /// yet.
  REG_UNREGISTERED = 1,
  /// ESP32 station has connected to an AP, and got an IPv4 address.
  REG_OK = 2,
  /// ESP32 station is in Wi-Fi connecting or reconnecting state.
  REG_CONNECTING = 3,
  /// ESP32 station is in Wi-Fi disconnected state.
  REG_DISCONNECTING = 4,
  /// ESP32 station is in an unknown state.
  REG_UNKNOWN = 5,

};

/// Basic modem configurations for the ESP32
/// @ingroup espressif_esp32
struct TinyGsmESP32ModemConfig
    : public TinyGsmModemConfigPreset<ESP32RegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "Espressif";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "ESP32";

  /// The set namespace for the certificate authority (CA) certificate
  static constexpr char CA_CERT_NAMESPACE[] TINY_GSM_PROGMEM = "client_ca";
  /// The set namespace for the client certificate
  static constexpr char CLIENT_CERT_NAMESPACE[] TINY_GSM_PROGMEM =
      "client_cert";
  /// The set namespace for the client key
  static constexpr char CLIENT_KEY_NAMESPACE[] TINY_GSM_PROGMEM = "client_key";
};

constexpr char TinyGsmESP32ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmESP32ModemConfig::MODEM_MODEL[] __attribute__((weak));
constexpr char TinyGsmESP32ModemConfig::CA_CERT_NAMESPACE[]
    __attribute__((weak));
constexpr char TinyGsmESP32ModemConfig::CLIENT_CERT_NAMESPACE[]
    __attribute__((weak));
constexpr char TinyGsmESP32ModemConfig::CLIENT_KEY_NAMESPACE[]
    __attribute__((weak));

/**
 * @brief TCP behavior and limits for the ESP32.
 *
 * NOTE: There's a total limit of 5 sockets, any of them can be SSL. BUT the
 * manual warns that module may not be able to handle more than 1 SSL socket at
 * a time.
 *
 * The ESP32 devices can receive 8192 bytes and send 2920 bytes at most in one
 * single transmission.
 * @ingroup espressif_esp32
 */
struct TinyGsmESP32TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 5,
          /*sendMaxSize*/ 2920,
          /*connectTimeoutS*/ 75,  // default
          /*stopTimeoutS*/ 5> {};

/**
 * @brief SSL behavior and limits for the ESP32.
 *
 * These modules don't have "SSL Contexts" per se.  They support 2 each of CA
 * certificates and client certificate/key pairs.  The certificates are
 * loaded and referenced by number.  The number of the CA certificate and the
 * client certificate/key pair does not have to match for a connection.
 *
 * @ingroup espressif_esp32
 */
struct TinyGsmESP32SslConfig : public TinyGsmSslConfigPreset<
                                   /*contextCount*/ 2> {};


/**
 * @brief Class for the Espressif ESP32 modem, which is a Wi-Fi module with SSL
 * support.
 *
 * @warning This class is used to communicate with a module that has been
 * programmed with the AT command firmware.
 * @ingroup espressif_esp32
 */
class TinyGsmESP32
    : public TinyGsmEspressif<TinyGsmESP32, TinyGsmESP32ModemConfig>,
      public TinyGsmTCP<TinyGsmESP32, TinyGsmESP32TcpConfig>,
      public TinyGsmSSL<TinyGsmESP32, TinyGsmESP32SslConfig>,
      public TinyGsmTime<TinyGsmESP32>,
      public TinyGsmNTP<TinyGsmESP32> {
  friend class TinyGsmEspressif<TinyGsmESP32, TinyGsmESP32ModemConfig>;
  friend class TinyGsmModem<TinyGsmESP32, TinyGsmESP32ModemConfig>;
  friend class TinyGsmWifi<TinyGsmESP32>;
  friend class TinyGsmTCP<TinyGsmESP32, TinyGsmESP32TcpConfig>;
  friend class GsmClient<TinyGsmESP32, TinyGsmESP32TcpConfig>;
  friend class TinyGsmSSL<TinyGsmESP32, TinyGsmESP32SslConfig>;
  friend class TinyGsmTime<TinyGsmESP32>;
  friend class TinyGsmNTP<TinyGsmESP32>;

 public:
  using ModemConfig = TinyGsmESP32ModemConfig;
  using TcpConfig   = TinyGsmESP32TcpConfig;
  using SslConfig   = TinyGsmESP32SslConfig;
  class GsmSecureContextESP32;
  using GsmSecureContext = GsmSecureContextESP32;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup espressif_esp32
  class GsmClientESP32 : public GsmClient<TinyGsmESP32, TinyGsmESP32TcpConfig> {
    friend class TinyGsmESP32;

   public:
    using GsmClient<TinyGsmESP32, TinyGsmESP32TcpConfig>::connect;
    using GsmClient<TinyGsmESP32, TinyGsmESP32TcpConfig>::stop;
    using TcpConfig = TinyGsmESP32TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientESP32() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is identical to the identifier
     * the modem uses internally to identify the socket for the ESP32.
     *
     * @note The requested mux is used when available. If it is already in use,
     * another available mux may be selected. Use getMux() to get the mux
     * assigned to the connection.
     */
    explicit GsmClientESP32(TinyGsmESP32& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmESP32, TinyGsmESP32TcpConfig>(modem, mux) {
      is_secure = false;
      init(&modem, mux);
    }

    /**
     * @brief Initialize the TCP client with a modem and optionally a
     * multiplexing channel.
     * @return true if initialization was successful, false otherwise.
     * @copydetails GsmClientESP32::GsmClientESP32(TinyGsmESP32&, uint8_t)
     */
    bool init(TinyGsmESP32* modem, uint8_t mux = 0) override {
      return this->attachToModem(this, modem, mux);
    }

    /*
     * Arduino Client interface
     */
   public:
    TINY_GSM_STATIC_TCP_CONNECT

    void stop(uint32_t maxWaitMs) override {
      if (at == nullptr) { return; }
      is_mid_send          = false;
      uint32_t startMillis = millis();
      if (sock_connected || sock_available) {
        // Update available data first, because if the socket was closed
        // externally, the module may have thrown away the data (though the
        // documentation says it should not).
        at->modemGetAvailable(mux);
        // Now we throw away any remaining data in the modem buffer
        // We explicitly toss it here because the socket will appear open in
        // response to connected() even after it closes until all data is read
        // to give the user a chance to recover the data if they want it.
        dumpModemBuffer(maxWaitMs);
      }
      uint32_t elapsed = millis() - startMillis;
      // NOTE: It should be safe to only send the close here if sock_connected
      // reads true because the above will have updated sock_connected
      // (dumpModemBuffer calls modemRead until sock_available=0, modemRead
      // calls modemGetAvailable on every read to update sock_available, once
      // sock_available=0 modemGetAvailable calls modemGetConnected, and
      // modemGetConnected updates sock_connected for all sockets.)
      // NOTE: Always give the modem at least 1 second to close the connection,
      // even if the maxWaitMs has already elapsed.
      if (sock_connected) {
        at->modemStop(mux, elapsed >= maxWaitMs ? 1000L : maxWaitMs - elapsed);
      }
      sock_connected = false;
    }

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
  /// @ingroup espressif_esp32
  class GsmClientSecureESP32
      : public GsmSecureClient<GsmClientESP32, TinyGsmESP32> {
    friend class TinyGsmESP32;

   public:
    using GsmClientESP32::connect;
    using GsmClientESP32::stop;
    using GsmSecureClient::GsmSecureClient;
    using TcpConfig = TinyGsmESP32TcpConfig;

    // Because we have the same potential range of mux numbers for secure and
    // insecure connections, we don't need to re-check for mux number
    // availability.
  };

  /*
   * Inner Secure Context
   */
 public:
  /// Inner secure context
  /// @ingroup espressif_esp32
  class GsmSecureContextESP32
      : public ::GsmSecureContext<TinyGsmESP32, TinyGsmESP32SslConfig> {
    friend class TinyGsmESP32;

   public:
    using ::GsmSecureContext<TinyGsmESP32,
                             TinyGsmESP32SslConfig>::GsmSecureContext;

    /// Unregister this context from its modem.
    ~GsmSecureContextESP32() {}

    /*
     * Secure context certificate assignment functions
     */

    /// @brief Set or clear the CA certificate name.
    /// @param CAcertName The CA certificate file name.
    /// @warning The CA certificate name must be either "client_ca.0" or
    /// "client_ca.1".
    void setCACertName(const char* CAcertName) {
      // handle null or empty certificate name
      if (CAcertName == nullptr ||
          strnlen(CAcertName, TINY_GSM_CERT_NAME_LENGTH) == 0) {
        // clear the certificate name buffer
        memset(this->CAcertName, '\0', sizeof(this->CAcertName));
        return;
      }
      // parse the certificate name
      uint8_t certNumber = 0;
      if (!TinyGsmESP32::parseCACertificateNumber(CAcertName, certNumber)) {
        // invalid certificate name: clear the buffer
        memset(this->CAcertName, '\0', sizeof(this->CAcertName));
        return;
      }
      // copy the certificate name into owned buffer
      strncpy(this->CAcertName, CAcertName, sizeof(this->CAcertName) - 1);
      this->CAcertName[sizeof(this->CAcertName) - 1] = '\0';
    }
    /// @brief Set or clear the CA certificate name.
    /// @param CAcertName The CA certificate file name.
    void setCACertName(const String& CAcertName) {
      setCACertName(CAcertName.c_str());
    }

    /**
     * @brief Set or clear the client certificate and private key names.
     * @param clientCertName The client certificate file name.
     * @param clientKeyName The matching private-key file name.
     *
     * @warning The client certificate name must be either "client_cert.0" or
     * "client_cert.1", and the private key name must be either "client_key.0"
     * or "client_key.1".
     * @note The client certificate number and the client private key number
     * must be the same for a given connection.
     */
    void setClientCredentials(const char* clientCertName,
                              const char* clientKeyName) {
      // handle null or empty certificate or key name
      if (clientCertName == nullptr || clientKeyName == nullptr ||
          strnlen(clientCertName, TINY_GSM_CERT_NAME_LENGTH) == 0 ||
          strnlen(clientKeyName, TINY_GSM_CERT_NAME_LENGTH) == 0) {
        // clear the certificate name buffers
        memset(this->clientCertName, '\0', sizeof(this->clientCertName));
        memset(this->clientKeyName, '\0', sizeof(this->clientKeyName));
        return;
      }
      // parse the certificate name
      uint8_t certNumber = 0;
      if (!TinyGsmESP32::parseClientCredentialNumber(
              clientCertName, clientKeyName, certNumber)) {
        // invalid credential names: clear the buffers
        memset(this->clientCertName, '\0', sizeof(this->clientCertName));
        memset(this->clientKeyName, '\0', sizeof(this->clientKeyName));
        return;
      }
      // copy the certificate name into owned buffer
      strncpy(this->clientCertName, clientCertName,
              sizeof(this->clientCertName) - 1);
      this->clientCertName[sizeof(this->clientCertName) - 1] = '\0';
      // set the client key name in the owned buffer
      strncpy(this->clientKeyName, clientKeyName,
              sizeof(this->clientKeyName) - 1);
      this->clientKeyName[sizeof(this->clientKeyName) - 1] = '\0';
    }
    /// @copydoc setClientCredentials(const char*, const char*)
    void setClientCredentials(const String& clientCertName,
                              const String& clientKeyName) {
      setClientCredentials(clientCertName.c_str(), clientKeyName.c_str());
    }

    /**
     * @brief Set the CA certificate number to use for this connection
     * @param certNumber The CA certificate number, must be 0 or 1.
     */
    void setCACertificateNumber(uint8_t certNumber) {
      if (certNumber > 1) {
        setCACertName(nullptr);
        return;
      }
      // convert the certificate number and type into the proper certificate
      // names for the ESP32
      char cert_name[16] = {};
      TinyGsmESP32::getCertificateName(CertificateType::CA_CERTIFICATE,
                                       certNumber, cert_name);
      memcpy(this->CAcertName, cert_name,
             TinyGsmMin(sizeof(this->CAcertName), sizeof(cert_name)));
      this->CAcertName[TinyGsmMin(sizeof(this->CAcertName), sizeof(cert_name)) -
                       1] = '\0';
    }
    /**
     * @brief Set the client certificate number to use for this connection
     * @param certNumber The client certificate number, must be 0 or 1.
     * @note The client certificate number and the client private key number
     * must be the same for a given connection.
     */
    void setClientCredentialNumber(uint8_t certNumber) {
      if (certNumber > 1) {
        setClientCredentials(nullptr, nullptr);
        return;
      }
      // generate and set the name for the client certificate from the number
      char cert_name[16] = {};
      TinyGsmESP32::getCertificateName(CertificateType::CLIENT_CERTIFICATE,
                                       certNumber, cert_name);
      memcpy(this->clientCertName, cert_name,
             TinyGsmMin(sizeof(this->clientCertName), sizeof(cert_name)));
      this->clientCertName[TinyGsmMin(sizeof(this->clientCertName),
                                      sizeof(cert_name)) -
                           1] = '\0';
      // generate and set the name for the client private key from the number
      TinyGsmESP32::getCertificateName(CertificateType::CLIENT_KEY, certNumber,
                                       cert_name);
      memcpy(this->clientKeyName, cert_name,
             TinyGsmMin(sizeof(this->clientKeyName), sizeof(cert_name)));
      this->clientKeyName[TinyGsmMin(sizeof(this->clientKeyName),
                                     sizeof(cert_name)) -
                          1] = '\0';
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
  explicit TinyGsmESP32(Stream& stream)
      : TinyGsmEspressif<TinyGsmESP32, TinyGsmESP32ModemConfig>(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientESP32"));
    bool success = true;

    if (!testAT()) { return false; }
    if (pin && strlen(pin) > 0) {
      DBG("Espressif modules do not use an unlock pin!");
    }
    sendAT(GF("E0"));  // Echo Off
    success &= waitResponse() == 1;

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+SYSLOG=1"));  // turn on verbose error codes
#else
    sendAT(GF("+SYSLOG=0"));  // turn off error codes
#endif
    waitResponse();

    sendAT(GF("+CIPMUX=1"));  // Enable Multiple Connections
    success &= waitResponse() == 1;
    sendAT(GF("+CWMODE=1"));  // Put into "station" mode
    if (waitResponse() != 1) {
      sendAT(GF("+CWMODE_CUR=1"));  // Attempt "current" station mode command
                                    // for some firmware variants if needed
      success &= waitResponse() == 1;
    }
    sendAT(GF("+CIPDINFO=0"));  // do not show the remote host and port in
                                // “+IPD” and “+CIPRECVDATA” messages.
    success &= waitResponse() == 1;

    // Set the data receive mode to have the module buffer data for all
    // connections AT+CIPRECVTYPE=<link ID>,<mode>[,<data_len_report_mode>]
    // <link ID>: ID of the connection (0 ~ max). For a single connection, <link
    //   ID> is 0. For multiple connections, if the value is max, it means all
    //   connections. Max is 5 by default.
    // <mode>: the receive mode of socket data. Default: 0.
    //  0: active mode. ESP-AT will send all the received socket data instantly
    //    to the host MCU with the header “+IPD”. (The socket receive window is
    //    5760 bytes by default. The maximum valid bytes sent to MCU is 2920
    //    bytes each time.)
    //  1: passive mode. ESP-AT will keep the received socket data in an
    //    internal buffer (socket receive window, 5760 bytes by default), and
    //    wait for the host MCU to read. If the buffer is full, the socket
    //    transmission will be blocked for TCP/SSL connections, or data will be
    //    lost for UDP connections.
    // <data_len_report_mode>: data length reporting mode. Default: 0.
    //  0: The length reported in the +IPD and +CIPRECVLEN messages is the total
    //    length of socket data currently in the receive buffer.
    //  1: The length reported in the +IPD and +CIPRECVLEN messages is the
    //    length of the next datagram. In UDP transmission, setting this to 1
    //    ensures that the reported length always corresponds to a single
    //    complete datagram.
    sendAT(GF("+CIPRECVTYPE=5,1"));
    success &= waitResponse() == 1;

    DBG(GF("### Modem:"), getModemName());
    return success;
  }

  void maintainImpl() {
    // Keep listening for modem URC's and proactively iterate through
    // sockets asking if any data is available
    uint8_t check_mux = static_cast<uint8_t>(-1);
    for (uint8_t mux = 0; mux < TcpConfig::kMuxCount; mux++) {
      GsmClientESP32* sock = sockets[mux];
      if (sock && sock->got_data) {
        sock->got_data = false;
        if (sock->sock_available == 0 &&
            check_mux == static_cast<uint8_t>(-1)) {
          // NOTE: Only check the socket if says it got new data by the amount
          // available is 0. This avoids extra un-needed checks.
          check_mux = mux;
        }
      }
    }
    // modemGetAvailable checks all socks, so we only want to do it once
    // modemGetAvailable calls modemGetConnected(), which also checks all
    // socks
    if (check_mux != static_cast<uint8_t>(-1)) { modemGetAvailable(check_mux); }
    while (stream.available()) { waitResponse(15, nullptr, nullptr); }
  }

  /*
   * Power functions
   */
 protected:
  // NOTE: The ESP32 series only supports deep sleep with a wake-up timer.  It
  // cannot be put to deep sleep indefinitely. This function is not implemented.
  bool powerOffImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool radioOffImpl() {
    bool success = true;

    // AT+CWINIT=<init>
    // <init>:
    //   0: Deinitialize Wi-Fi driver of ESP32 device.
    //   1: Initialize Wi-Fi driver of ESP32 device. (Default value)
    sendAT(GF("+CWINIT=0"));
    success &= waitResponse() == 1;

    // AT+BTINIT=<init>
    //  <init>:
    //    0: Deinitialize Classic Bluetooth.
    //    1: Initialize Classic Bluetooth.
    sendAT(GF("+BTINIT=0"));
    success &= waitResponse() == 1;

    // AT+BLEINIT=<init>
    // <init>:
    //   0: deinit Bluetooth LE
    //   1: client role
    //   2: server role
    //   3: dual role (client and server)
    sendAT(GF("+BLEINIT=0"));
    success &= waitResponse() == 1;

    return success;
  };

  /*
   * Generic network functions
   */
 protected:
  ESP32RegStatus getRegistrationStatusImpl() {
    sendAT(GF("+CWSTATE?"));
    if (waitResponse(3000, GF("+CWSTATE:")) != 1)
      return ESP32RegStatus::REG_UNKNOWN;
    // +CWSTATE:{state},{"ssid"}
    // followed by an OK
    int8_t status = streamGetIntBefore(',');
    streamSkipUntil('\n');  // throw away the ssid
    waitResponse();         // wait for trailing OK
    return static_cast<ESP32RegStatus>(status);
  }

  bool isNetworkConnectedImpl() {
    ESP32RegStatus s = this->getRegistrationStatus();
    return (s == ESP32RegStatus::REG_OK);
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
  // NOTE: You must be running AT firmware >= 3.2.0.0 for these functions to
  // work. If you are running a lower level firmware, you must update. You
  // almost certainly will need to flash your board with the new firmware using
  // esptools/ESP flash download tools instead of using the AT+CIUPDATE function
  // because the structure of the NVM space changed and the newer structure is
  // needed for the SYSMFG command used here. The CIUPDATE function does not
  // update the NVM.

  // NOTE: In firmware release notes, it says that more than 5 sets of CA
  // certificates are supported, but all other command examples and descriptions
  // of the flash memory storage for the certificates mention only 2 possible
  // sets (0 and 1).

 public:
  // This adds the server's CA certificate that the client connects to, used
  // in auth mode 2 and 3
  // This is the value client_ca_0x.crt in the AT firmware
  /**
   * @brief Load a CA certificate into the modem's flash memory for SSL
   * connections.
   *
   * @param certNumber The certificate number to load.
   * @param cert The certificate data.
   * @param len The length of the certificate data.
   * @return True if the certificate was successfully loaded, false otherwise.
   */
  bool loadCACert(uint8_t certNumber, const char* cert, const uint16_t len) {
    if (certNumber > 1) { return false; }
    return loadCertificateByNumber(CertificateType::CA_CERTIFICATE, certNumber,
                                   cert, len);
  }

  /**
   * @brief Load a client certificate into the modem's flash memory for SSL
   * connections.
   *
   * @param certNumber The certificate number to load.
   * @param cert The certificate data.
   * @param len The length of the certificate data.
   * @return True if the certificate was successfully loaded, false otherwise.
   */
  bool loadClientCert(uint8_t certNumber, const char* cert,
                      const uint16_t len) {
    if (certNumber > 1) { return false; }
    return loadCertificateByNumber(CertificateType::CLIENT_CERTIFICATE,
                                   certNumber, cert, len);
  }

  /**
   * @brief Load a private key into the modem's flash memory for SSL
   * connections.
   *
   * @param keyNumber The key number to load.
   * @param key The key data.
   * @param len The length of the key data.
   * @return True if the key was successfully loaded, false otherwise.
   */
  bool loadPrivateKey(uint8_t keyNumber, const char* key, const uint16_t len) {
    if (keyNumber > 1) { return false; }
    return loadCertificateByNumber(CertificateType::CLIENT_KEY, keyNumber, key,
                                   len);
  }

  /**
   * @brief Load a certificate into the modem's flash memory for SSL connections
   * by its number.
   *
   * @param cert_type The type of certificate to load.
   * @param certNumber The certificate number to load.
   * @param cert The certificate data.
   * @param len The length of the certificate data.
   * @return True if the certificate was successfully loaded, false otherwise.
   */
  bool loadCertificateByNumber(CertificateType cert_type, uint8_t certNumber,
                               const char* cert, const uint16_t len) {
    return loadCertificateByNumberImpl(cert_type, certNumber, cert, len);
  }
#if defined(TINY_GSM_HAS_PROGMEM)
  /**
   * @brief Load a certificate stored in program memory into the modem's flash
   * memory by its number.
   *
   * @param cert_type The type of certificate to load.
   * @param certNumber The certificate number to load.
   * @param cert The certificate data wrapped in GFP().
   * @param len The length of the certificate data.
   * @return True if the certificate was successfully loaded, false otherwise.
   */
  bool loadCertificateByNumber(CertificateType cert_type, uint8_t certNumber,
                               GsmConstStr cert, const uint16_t len) {
    return loadCertificateByNumberImpl(cert_type, certNumber, cert, len);
  }
#endif

 protected:
  template <typename TCert>
  bool loadCertificateByNumberImpl(CertificateType cert_type,
                                   uint8_t certNumber, TCert cert,
                                   const uint16_t len) {
    if (certNumber > 1) { return false; }
    char cert_number_char = certNumber + '0';

    // delete any old text in the cert first
    deleteCertificateByNumber(cert_type, certNumber);

    // AT+SYSMFG=<operation>,<"namespace">,<"key">,<type>,<value>
    // operation = 2 for write
    // type = 8 for binary (ie, the certificates must be stored in binary,
    // though you can enter them as strings)
    // Write a new value for client_cert.0 key into client_cert namespace
    // (That is, update the 0th client certificate)
    // AT+SYSMFG=2,"client_cert","client_cert.0",8,1164
    // Wait until AT command port returns ``>``, and then write 1164 bytes

    // data = at_update_param('client_ca.0', 'B', args.client_ca0, data)
    //        at_update_param(key,          type, value,           data):

    switch (cert_type) {
      case CertificateType::CLIENT_PSK:
      case CertificateType::CLIENT_PSK_IDENTITY: {
        return false;
      }
      case CertificateType::CA_CERTIFICATE: {
        sendAT(GF("+SYSMFG=2,\""), GFP(ModemConfig::CA_CERT_NAMESPACE),
               GF("\",\""), GFP(ModemConfig::CA_CERT_NAMESPACE), '.',
               cert_number_char, GF("\",8,"), len);
        break;
      }
      case CertificateType::CLIENT_CERTIFICATE: {
        sendAT(GF("+SYSMFG=2,\""), GFP(ModemConfig::CLIENT_CERT_NAMESPACE),
               GF("\",\""), GFP(ModemConfig::CLIENT_CERT_NAMESPACE), '.',
               cert_number_char, GF("\",8,"), len);
        break;
      }
      case CertificateType::CLIENT_KEY: {
        sendAT(GF("+SYSMFG=2,\""), GFP(ModemConfig::CLIENT_KEY_NAMESPACE),
               GF("\",\""), GFP(ModemConfig::CLIENT_KEY_NAMESPACE), '.',
               cert_number_char, GF("\",8,"), len);
        break;
      }
      default: {
        return false;
      }
    }

    if (waitResponse(GF(">")) != 1) { return false; }
    this->streamWrite(cert, len);
    stream.flush();
    if (waitResponse(10000L) != 1) { return false; }
    return true;
  }

 public:
  /**
   * @brief Delete a certificate from the modem's flash memory for SSL
   * connections by its number.
   *
   * @param cert_type The type of certificate to delete.
   * @param certNumber The certificate number to delete.
   * @return True if the certificate was successfully deleted, false otherwise.
   */
  bool deleteCertificateByNumber(CertificateType cert_type,
                                 uint8_t         certNumber) {
    if (certNumber > 1) { return false; }
    char cert_number_char = certNumber + '0';

    // AT+SYSMFG=<operation>,<"namespace">[,<"key">]
    // operation = 0 for erase
    // AT+SYSMFG=0,"client_cert","client_cert.0"

    switch (cert_type) {
      case CertificateType::CLIENT_PSK:
      case CertificateType::CLIENT_PSK_IDENTITY: {
        return false;
      }
      case CertificateType::CA_CERTIFICATE: {
        sendAT(GF("+SYSMFG=0,\""), GFP(ModemConfig::CA_CERT_NAMESPACE),
               GF("\",\""), GFP(ModemConfig::CA_CERT_NAMESPACE), '.',
               cert_number_char, '"');
        break;
      }
      case CertificateType::CLIENT_CERTIFICATE: {
        sendAT(GF("+SYSMFG=0,\""), GFP(ModemConfig::CLIENT_CERT_NAMESPACE),
               GF("\",\""), GFP(ModemConfig::CLIENT_CERT_NAMESPACE), '.',
               cert_number_char, '"');
        break;
      }
      case CertificateType::CLIENT_KEY: {
        sendAT(GF("+SYSMFG=0,\""), GFP(ModemConfig::CLIENT_KEY_NAMESPACE),
               GF("\",\""), GFP(ModemConfig::CLIENT_KEY_NAMESPACE), '.',
               cert_number_char, '"');
        break;
      }
      default: {
        return false;
      }
    }

    return waitResponse() == 1;
  }

  /**
   * @brief Print a certificate from the modem's flash memory for SSL
   * connections by its number to a stream.
   *
   * @param cert_type The type of certificate to print.
   * @param certNumber The certificate number to print.
   * @param print_stream The stream to print the certificate to.
   * @return True if the certificate was successfully printed, false otherwise.
   */
  bool printCertificateByNumber(CertificateType cert_type, uint8_t certNumber,
                                Stream& print_stream) {
    if (certNumber > 1) { return false; }
    char cert_number_char = certNumber + '0';

    // AT+SYSMFG=<operation>,<"namespace">,<"key">,<type>,<value>
    // operation = 1 for read
    // type = 8 for binary (ie, the certificates must be stored in binary,
    // though you can enter them as strings)
    // Read the value of client_cert.0 key from client_cert namespace
    // (That is, read the 0th client certificate)
    // AT+SYSMFG=1,"client_cert","client_cert.0",8

    switch (cert_type) {
      case CertificateType::CLIENT_PSK:
      case CertificateType::CLIENT_PSK_IDENTITY: {
        return false;
      }
      case CertificateType::CA_CERTIFICATE: {
        sendAT(GF("+SYSMFG=1,\""), GFP(ModemConfig::CA_CERT_NAMESPACE),
               GF("\",\""), GFP(ModemConfig::CA_CERT_NAMESPACE), '.',
               cert_number_char, '"');
        break;
      }
      case CertificateType::CLIENT_CERTIFICATE: {
        sendAT(GF("+SYSMFG=1,\""), GFP(ModemConfig::CLIENT_CERT_NAMESPACE),
               GF("\",\""), GFP(ModemConfig::CLIENT_CERT_NAMESPACE), '.',
               cert_number_char, '"');
        break;
      }
      case CertificateType::CLIENT_KEY: {
        sendAT(GF("+SYSMFG=1,\""), GFP(ModemConfig::CLIENT_KEY_NAMESPACE),
               GF("\",\""), GFP(ModemConfig::CLIENT_KEY_NAMESPACE), '.',
               cert_number_char, '"');
        break;
      }
      default: {
        return false;
      }
    }

    if (waitResponse(GF("+SYSMFG:")) != 1) { return false; }
    streamSkipUntil(',');  // skip the returned namespace
    streamSkipUntil(',');  // skip the returned name
    streamSkipUntil(',');  // skip the returned filetype (should be 8)
    // read the length of the certificate
    int print_len = streamGetIntBefore(',');

    for (int i = 0; i < print_len; i++) {
      int      c;
      uint32_t _startMillis = millis();
      do {
        c = stream.read();
        if (c >= 0) break;
      } while (millis() - _startMillis < 50);
      if (c < 0) { break; }  // if we run out of characters, stop
#ifndef DUMP_AT_COMMANDS
      // Print the file to the buffer
      // NOTE: Only do this if we're not dumping the all AT, or we'll double
      // print
      print_stream.write(c);
#endif
    }
    print_stream.flush();
    // Final OK
    return waitResponse() == 1;
  }

 protected:
  static void parseCertificateName(const char*      cert_name,
                                   CertificateType& parsed_type,
                                   uint8_t&         parsed_number) {
    uint8_t ns_length = 0;
    // look for the namespace in the name and determine the certificate type
    if (TINY_GSM_F_STRNCMP(cert_name, GFP(ModemConfig::CA_CERT_NAMESPACE),
                           sizeof(ModemConfig::CA_CERT_NAMESPACE) - 1) == 0) {
      parsed_type = CertificateType::CA_CERTIFICATE;
      ns_length   = sizeof(ModemConfig::CA_CERT_NAMESPACE) - 1;
    } else if (TINY_GSM_F_STRNCMP(
                   cert_name, GFP(ModemConfig::CLIENT_CERT_NAMESPACE),
                   sizeof(ModemConfig::CLIENT_CERT_NAMESPACE) - 1) == 0) {
      parsed_type = CertificateType::CLIENT_CERTIFICATE;
      ns_length   = sizeof(ModemConfig::CLIENT_CERT_NAMESPACE) - 1;
    } else if (TINY_GSM_F_STRNCMP(
                   cert_name, GFP(ModemConfig::CLIENT_KEY_NAMESPACE),
                   sizeof(ModemConfig::CLIENT_KEY_NAMESPACE) - 1) == 0) {
      parsed_type = CertificateType::CLIENT_KEY;
      ns_length   = sizeof(ModemConfig::CLIENT_KEY_NAMESPACE) - 1;
    } else {
      goto parsing_error;
    }

    if (cert_name[ns_length] != '.') { goto parsing_error; }
    if (cert_name[ns_length + 1] == '0') {
      parsed_number = 0;
      return;
    } else if (cert_name[ns_length + 1] == '1') {
      parsed_number = 1;
      return;
    } else {
      goto parsing_error;
    }
    return;

  parsing_error:
    parsed_type   = CertificateType::UNKNOWN;
    parsed_number = static_cast<uint8_t>(-1);
    return;
  }

  static bool parseCACertificateNumber(const char* cert_name,
                                       uint8_t&    cert_number) {
    CertificateType cert_type = CertificateType::UNKNOWN;
    parseCertificateName(cert_name, cert_type, cert_number);
    return cert_type == CertificateType::CA_CERTIFICATE && cert_number <= 1;
  }

  static bool parseClientCredentialNumber(const char* cert_name,
                                          const char* key_name,
                                          uint8_t&    cert_number) {
    CertificateType cert_type  = CertificateType::UNKNOWN;
    CertificateType key_type   = CertificateType::UNKNOWN;
    uint8_t         key_number = 0;
    parseCertificateName(cert_name, cert_type, cert_number);
    parseCertificateName(key_name, key_type, key_number);
    return cert_type == CertificateType::CLIENT_CERTIFICATE &&
        key_type == CertificateType::CLIENT_KEY && cert_number <= 1 &&
        key_number <= 1 && cert_number == key_number;
  }

  static void getCertificateName(CertificateType cert_type, uint8_t certNumber,
                                 char* cert_name) {
    // Validate certNumber is 0 or 1
    if (certNumber > 1) {
      if (cert_name != nullptr) { cert_name[0] = '\0'; }
      return;
    }
    char cert_number[2];  // Must be '0' or '1', so 2 bytes is enough
    itoa(certNumber, cert_number, 10);

    // put the set certificate namespace and name into the provided buffers
    // based on the certificate type
    switch (cert_type) {
      case CertificateType::CLIENT_PSK_IDENTITY:
      case CertificateType::CLIENT_PSK: {
        // The ESP32 does not support SSL using pre-shared keys with AT
        // firmware.
        strcpy(cert_name, "\0");
        return;
      }
      case CertificateType::CLIENT_KEY: {
        TINY_GSM_F_STRCPY(cert_name, GFP(ModemConfig::CLIENT_KEY_NAMESPACE));
        break;
      }
      case CertificateType::CLIENT_CERTIFICATE: {
        TINY_GSM_F_STRCPY(cert_name, GFP(ModemConfig::CLIENT_CERT_NAMESPACE));
        break;
      }
      case CertificateType::CA_CERTIFICATE:
      default: {
        TINY_GSM_F_STRCPY(cert_name, GFP(ModemConfig::CA_CERT_NAMESPACE));
        break;
      }
    }
    // append the certificate number to the name to create the full certificate
    // name (e.g., "client_cert.0")
    strcat(cert_name, ".");
    strcat(cert_name, cert_number);
    return;
  }


  template <typename TCert>
  bool loadCertificateImpl(const char* certificateName, TCert cert,
                           const uint16_t len) {
    // parse the certificate name
    CertificateType parsed_type = CertificateType::UNKNOWN;
    uint8_t         certNumber  = 0;
    parseCertificateName(certificateName, parsed_type, certNumber);
    // add the certificate by name
    return loadCertificateByNumber(parsed_type, certNumber, cert, len);
  }

  bool deleteCertificateImpl(const char* certificateName) {
    // parse the certificate name
    CertificateType parsed_type = CertificateType::UNKNOWN;
    uint8_t         certNumber  = 0;
    parseCertificateName(certificateName, parsed_type, certNumber);
    // add the certificate by name
    return deleteCertificateByNumber(parsed_type, certNumber);
  }


  bool printCertificateImpl(const char* filename, Stream& print_stream) {
    // parse the certificate name
    CertificateType parsed_type = CertificateType::UNKNOWN;
    uint8_t         certNumber  = 0;
    parseCertificateName(filename, parsed_type, certNumber);
    // add the certificate by name
    return printCertificateByNumber(parsed_type, certNumber, print_stream);
  }

  bool convertCertificateImpl(CertificateType cert_type, const char*) {
    if (cert_type == CertificateType::CLIENT_PSK ||
        cert_type == CertificateType::CLIENT_PSK_IDENTITY) {
      // The ESP32 does not support SSL using pre-shared keys with AT firmware.
      return false;
    }
    return true;  // no conversion needed on the ESP32
  }
  bool convertCACertificateImpl(const char*) {
    return true;
  }
  bool convertClientCertificatesImpl(const char*, const char*) {
    return true;  // no conversion needed on the ESP32
  }
  bool convertPSKandIDImpl(const char*, const char*) {
    // The ESP32 does not support SSL using pre-shared keys with AT firmware.
    return false;
  }
  bool convertPSKTableImpl(const char*) {
    return true;
  }

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
 public:
  /**
   * @brief Set the time zone for the modem and optionally enable SNTP time
   * synchronization.
   * @param timezone The time zone offset in hours from UTC (e.g., -5 for EST, 1
   * for CET).
   * @param enable_sync Optional parameter to enable or disable SNTP time
   * synchronization. Defaults to true (enabled).
   */
  void setTimeZone(int8_t timezone, bool enable_sync = true) {
    // configure the NTP settings for the modem
    sendAT(GF("+CIPSNTPCFG="), enable_sync ? 1 : 0, ',', timezone);
    waitResponse();
  }

  /**
   * @brief Set the time synchronization interval for the modem.
   * @param seconds The number of seconds between time synchronization attempts.
   */
  void setTimeSyncInterval(uint16_t seconds) {
    // configure the NTP settings for the modem
    sendAT(GF("+CIPSNTPINTV="), seconds);
    waitResponse();
  }

 protected:
  bool waitForTimeSyncImpl(uint16_t timeout_s) {
    // if we're not connected, we'll never get the time
    if (!isNetworkConnected()) {
      DBG(GF("### Not connected to network; cannot sync time!"));
      return false;
    }
    // if SNTP sync isn't enabled, we won't have the time
    // NOTE: We don't actually enable the time here, because doing so would
    // change any user settings for the timezone and time servers.
    sendAT(GF("+CIPSNTPCFG?"));
    bool is_enabled = waitResponse(2000L, GF("+CIPSNTPCFG:")) == 1;
    is_enabled &= streamGetIntBefore(',') == 1;
    waitResponse(5000L);  // returns OK at the end
    if (!is_enabled) {
      DBG(GF("### SNTP sync not enabled; cannot sync time!"));
      return false;
    }
    // if we're sure we should be able to get the time, wait for it
    uint32_t start_millis = millis();
    while (millis() - start_millis < static_cast<uint32_t>(timeout_s) * 1000) {
      uint32_t modem_time = getNetworkEpoch();
      // If we get a time between January 1, 2020 and January 1, 2035, we're
      // (hopefully) good
      if (modem_time > 1577836800 && modem_time < 2051222400) { return true; }
      delay(250);
    }
    return false;
  }

  // NOTE: Only returning full date time in Espressif's format
  String getGSMDateTimeImpl(TinyGSMDateTimeFormat) {
    sendAT(GF("+CIPSNTPTIME?"));
    if (waitResponse(2000L, GF("+CIPSNTPTIME:")) != 1) { return ""; }

    String res = stream.readStringUntil('\r');
    waitResponse();  // Ends with OK
    return res;
  }

  bool getNetworkTimeImpl(int* year, int* month, int* day, int* hour,
                          int* minute, int* second, float* timezone) {
    // get the time as a string
    sendAT(GF("+CIPSNTPTIME?"));
    if (waitResponse(2000L, GF("+CIPSNTPTIME:")) != 1) { return false; }

    int16_t iyear              = 0;
    int16_t imonth             = 0;
    int16_t iday               = 0;
    int16_t ihour              = 0;
    int16_t imin               = 0;
    int16_t isec               = 0;
    float   itimezone          = 0;
    int16_t itimezone_quarters = 0;

    // Date & Time
    streamSkipUntil(' ');  // skip the day of the week
    String mon_abbrev = stream.readStringUntil(' ');
    imonth            = getMonthFromAbbrev(mon_abbrev);
    iday              = streamGetIntBefore(' ');
    if (iday == -9999) {
      // there may be multiple spaces between the month and day, so try a second
      // time
      iday = streamGetIntBefore(' ');
    }
    ihour = streamGetIntBefore(':');
    imin  = streamGetIntBefore(':');
    isec  = streamGetIntBefore(' ');
    iyear = streamGetIntLength(4);

    // Final OK
    waitResponse();

    // get the timezone
    sendAT(GF("+CIPSNTPCFG?"));
    if (waitResponse(2000L, GF("+CIPSNTPCFG:")) != 1) { return false; }

    streamSkipUntil(',');  // skip if sync is enabled
    const int16_t itimezone_cfg = streamGetIntBefore(',');
    // AT+CIPSNTPCFG may report timezone as signed whole hours or signed HHMM.
    if (itimezone_cfg >= -14 && itimezone_cfg <= 14) {
      itimezone          = static_cast<float>(itimezone_cfg);
      itimezone_quarters = static_cast<int16_t>(itimezone_cfg * 4);
    } else {
      const int16_t  timezone_sign    = (itimezone_cfg < 0) ? -1 : 1;
      const uint16_t timezone_abs     = abs(itimezone_cfg);
      const uint8_t  timezone_hours   = timezone_abs / 100;
      const uint8_t  timezone_minutes = timezone_abs % 100;
      itimezone                       = timezone_sign *
          (static_cast<float>(timezone_hours) +
           static_cast<float>(timezone_minutes) / 60.0f);
      if (timezone_minutes % 15 == 0) {
        itimezone_quarters = timezone_sign *
            static_cast<int16_t>(timezone_hours * 4 + timezone_minutes / 15);
      } else {
        // force validation failure for unsupported minute precision
        itimezone_quarters = 127;
      }
    }
    // Final OK
    waitResponse(5000L);

    // Set pointers
    if (iyear < 2000) iyear += 2000;
    if (year != nullptr) *year = iyear;
    if (month != nullptr) *month = imonth;
    if (day != nullptr) *day = iday;
    if (hour != nullptr) *hour = ihour;
    if (minute != nullptr) *minute = imin;
    if (second != nullptr) *second = isec;
    if (timezone != nullptr) *timezone = itimezone;

#if 0
    DBG(GF("Year:"), iyear, GF("	Month:"), imonth, GF("	Day:"), iday);
    DBG(GF("Hour:"), ihour, GF("	Minute:"), imin, GF("	Second:"), isec);
    DBG(GF("Timezone:"), static_cast<float>(itimezone) / 4.0);
#endif

    // Validate parsed values
    if (!TinyGsmTime<TinyGsmESP32>::isValidDateTimeValues(
            iyear, imonth, iday, ihour, imin, isec, itimezone_quarters)) {
      return false;
    }

    return true;
  }

  uint8_t getMonthFromAbbrev(String month_abbrev) {
    if (month_abbrev == "Jan") { return 1; }
    if (month_abbrev == "Feb") { return 2; }
    if (month_abbrev == "Mar") { return 3; }
    if (month_abbrev == "Apr") { return 4; }
    if (month_abbrev == "May") { return 5; }
    if (month_abbrev == "Jun") { return 6; }
    if (month_abbrev == "Jul") { return 7; }
    if (month_abbrev == "Aug") { return 8; }
    if (month_abbrev == "Sep") { return 9; }
    if (month_abbrev == "Oct") { return 10; }
    if (month_abbrev == "Nov") { return 11; }
    if (month_abbrev == "Dec") { return 12; }
    return 0;
  }

  uint32_t getNetworkEpochImpl(TinyGSM_EpochStart epoch) {
    // Returns unix timestamp. Will match SNTP after SNTP syncs.
    sendAT(GF("+SYSTIMESTAMP?"));
    if (waitResponse(2000L, GF("+SYSTIMESTAMP:")) != 1) { return 0; }

    uint32_t start = millis();
    while (stream.available() < 9 && millis() - start < 10000L) {}

    uint32_t modem_time = streamGetULBefore('\r');

    waitResponse();

    if (modem_time != static_cast<uint32_t>(-1)) {
      switch (epoch) {
        case TinyGSM_EpochStart::UNIX: modem_time += 0; break;
        case TinyGSM_EpochStart::Y2K: modem_time -= 946684800; break;
        case TinyGSM_EpochStart::GPS: modem_time -= 315964800; break;
      }
    } else {
      // return 0 if we couldn't get a valid time from the modem
      modem_time = 0;
    }

    return modem_time;
  }

  /*
   * NTP server functions
   */
 protected:
  // NOTE: I don't think this forces an immediate sync
  bool NTPServerSyncImpl(const char* server, int TimeZone) {
    // AT+CIPSNTPCFG uses signed HHMM for non-whole-hour UTC offsets.
    const int timeZoneOffset = (TimeZone / 4) * 100 + (TimeZone % 4) * 15;
    // configure the NTP settings for the modem
    sendAT(GF("+CIPSNTPCFG="), 1, ',', timeZoneOffset, GF(",\""), server, '"');
    // returns a simple OK or ERROR
    return waitResponse() == 1;
  }

  /*
   * BLE functions
   */
  // No functions of this type implemented

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

    // Blank holders for the SSL auth mode and certificates
    SSLAuthMode sslAuthMode = SSLAuthMode::NO_VALIDATION;
    uint8_t     ca_number   = 0;
    uint8_t     pki_number  = 0;
    const char* pskIdent    = nullptr;
    const char* psKey       = nullptr;
    // If we actually have a secure socket populate the above with real values
    if (ssl) {
      // Use a static cast to get the context index.
      // This isn't ideal; hopefully the compiler will save us from ourselves.
      // We cannot use a dynamic cast because Arduino compiles with -fno-rtti.
      const GsmClientSecureESP32* thisClient =
          static_cast<const GsmClientSecureESP32*>(sockets[mux]);
      uint8_t sslCtxIndex = thisClient->sslCtxIndex;
      // Retrieve the SSL context object for the given context ID.
      ::GsmSecureContext<TinyGsmESP32, TinyGsmESP32SslConfig>* sslContext =
          getSSLContext(sslCtxIndex);
      // check for a valid SSL context
      if (sslContext == nullptr) {
        DBG("### No SSL context is registered at index", sslCtxIndex);
        return false;
      }
      sslAuthMode = sslContext->sslAuthMode;
      pskIdent    = sslContext->pskIdent;
      psKey       = sslContext->psKey;

      // check that the context has the required credentials
      if (!validateSSLContextCredentials(sslContext)) {
        DBG("### The SSL context is missing required credentials!");
        return false;
      }

      if (sslAuthMode == SSLAuthMode::CA_VALIDATION ||
          sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION) {
        if (!parseCACertificateNumber(sslContext->CAcertName, ca_number)) {
          DBG("### Invalid CA certificate name for ESP32 SSL context!");
          return false;
        }
      }

      if (sslAuthMode == SSLAuthMode::CLIENT_VALIDATION ||
          sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION) {
        if (!parseClientCredentialNumber(sslContext->clientCertName,
                                         sslContext->clientKeyName,
                                         pki_number)) {
          DBG("### Invalid client credentials for ESP32 SSL context!");
          return false;
        }
      }

      DBG("### SSL context index:", sslCtxIndex);
      DBG("### SSL auth mode:", (int)sslAuthMode);
      DBG("### CA cert number:", ca_number);
      DBG("### PKI number:", pki_number);
      DBG("### PSK identity:", pskIdent);
      DBG("### PSK key:", psKey);

      // SSL certificate checking will not work without a valid timestamp!
      if ((sslAuthMode != SSLAuthMode::NO_VALIDATION) &&
          !waitForTimeSync(timeout_s)) {
        DBG("### WARNING: The module timestamp must be valid for SSL auth. "
            "Please use setTimeZone(...) or NTPServerSync(...) to enable "
            "time syncing before attempting an SSL connection!");
        return false;
      }

      // configure SSL authentication type and in-use certificates
      // AT+CIPSSLCCONF=<link ID>,{auth_mode}[,{pki_number}][,{ca_number}]
      // <link ID>: ID of the connection (0 ~ max). For multiple connections, if
      // the value is max, it means all connections. By default, max is 5.
      // auth_mode:
      //     0: no authentication. In this case <pki_number> and <ca_number> are
      //     not required.
      //     1: the client provides the client certificate for the server to
      //     verify.
      //     2: the client loads CA certificate to verify the server’s
      //     certificate.
      //     3: mutual authentication.
      // pki_number: the index of certificate and private key. If there is
      // only one certificate and private key, the value should be 0.
      //    PKI - A public key infrastructure (PKI) is a set of roles, policies,
      //    hardware, software and procedures needed to create, manage,
      //    distribute, use, store and revoke digital certificates and manage
      //    public-key encryption.
      // ca_number: the index of CA (certificate authority certificate =
      // server's certificate). If there is only one CA, the value should be 0.
      // The PKI number and CA number to use are based on what certificates were
      // (or were not) put into the customized certificate partitions.
      // The default firmware comes with espressif certificates in slots 0
      // and 1.
      if ((sslAuthMode == SSLAuthMode::NO_VALIDATION)) {
        sendAT(GF("+CIPSSLCCONF="), mux, GF(",0"));
      } else if (sslAuthMode == SSLAuthMode::PRE_SHARED_KEYS) {
        // NOTE: Support for this is firmware dependent!
        // AT+CIPSSLCPSK=<link ID>,<"psk">,<"hint">
        if (psKey == nullptr || strnlen(psKey, TINY_GSM_PSK_LENGTH) == 0 ||
            pskIdent == nullptr ||
            strnlen(pskIdent, TINY_GSM_PSK_IDENTITY_LENGTH) == 0) {
          DBG("### PSK authentication requires both a PSK and a PSK identity "
              "(hint)!");
          return false;
        }
        sendAT(GF("+CIPSSLCPSK="), mux, GF(",\""), psKey, GF("\",\""), pskIdent,
               '"');
      } else if (sslAuthMode == SSLAuthMode::CLIENT_VALIDATION ||
                 sslAuthMode == SSLAuthMode::CA_VALIDATION ||
                 sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION) {
        // For auth modes 1, 2, and 3, we need to specify the PKI and CA numbers
        const uint8_t espAtAuthMode =
            sslAuthMode == SSLAuthMode::CLIENT_VALIDATION ? 1
            : sslAuthMode == SSLAuthMode::CA_VALIDATION   ? 2
                                                          : 3;
        sendAT(GF("+CIPSSLCCONF="), mux, ',', espAtAuthMode, ',', pki_number,
               ',', ca_number);
      } else {
        DBG("### Unsupported ESP32 SSL authentication mode!");
        return false;
      }
      if (waitResponse() != 1) { return false; }

      // set the SSL SNI (server name indication)
      // Multiple connections: (AT+CIPMUX=1)
      // AT+CIPSSLCSNI=<link ID>,<"sni">
      // NOTE: On firmware versions above 0.4.2 this happens automatically, but
      // on older versions it must be done manually.
      sendAT(GF("+CIPSSLCSNI="), mux, GF(",\""), host, '"');
      if (waitResponse() != 1) { return false; }
    }

    String resolved_ip;
    // If you need to use a domain name and the length of the domain name
    // exceeds 64 bytes, use the AT+CIPDOMAIN command to obtain the IP address
    // corresponding to the domain name, and then use the IP address to
    // establish a connection.
    if (strlen(host) > 64) {
      // AT+CIPDOMAIN=<"domain name">[,<ip network>][,<timeout>]
      sendAT(GF("+CIPDOMAIN=\""), host, '"');
      // +CIPDOMAIN:<"IP address"> then OK
      if (waitResponse(GF("+CIPDOMAIN:\"")) != 1) { return false; }
      resolved_ip = stream.readStringUntil('"');
      streamSkipUntil('\n');  // skip the rest of the line
      waitResponse();         // ends with OK
      if (resolved_ip.length() > 0) {
        host = resolved_ip.c_str();
      } else {
        return false;
      }
    }

    // Select TCP/IP transmission mode (normal mode)
    sendAT(GF("+CIPMODE=0"));
    waitResponse();

    // Make the connection
    sendAT(GF("+CIPSTART="), mux, ',', ssl ? GF("\"SSL") : GF("\"TCP"),
           GF("\",\""), host, GF("\","), port
#if defined(TINY_GSM_TCP_KEEP_ALIVE)
           ,
           ',', TINY_GSM_TCP_KEEP_ALIVE
#endif
    );

    String data;
    int8_t rsp     = waitResponse(timeout_ms, data, GFP(ModemConfig::GSM_OK),
                                  GFP(ModemConfig::GSM_ERROR),
                                  GF("ALREADY CONNECT"));
    bool   success = rsp == 1 || rsp == 3;  // OK or ALREADY CONNECT
    if (rsp == 1 && data.length() > 8) {
      int16_t coma          = data.indexOf(',');
      int16_t connected_mux = data.substring(0, coma).toInt();
      if (!isExpectedMux(connected_mux, mux)) {
        DBG("WARNING:  Unexpected mux number returned:", connected_mux, "not",
            mux);
      }
    }
    return success;
  }

  // Disambiguate modemStop by using the Espressif implementation
  using TinyGsmEspressif<TinyGsmESP32, TinyGsmESP32ModemConfig>::modemStop;

  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPSEND="), mux, ',', len);
    return waitResponse(GF(">")) == 1;
  }
  // Between the modemBeginSend and modemEndSend, modemSend calls:
  // stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  // stream.flush();
  size_t modemEndSend(size_t len, uint8_t) {
    uint16_t received = 0;
    if (waitResponse(10000L, GF("Recv ")) == 1) {
      received = streamGetIntBefore(' ');  // check received length
    }
    if (waitResponse(30000L, GF("SEND OK\r\n"), GF("SEND FAIL\r\n"),
                     GFP(ModemConfig::GSM_ERROR)) != 1) {
      return 0;
    }
    if (received != len) { DBG("### Sent:", received, "of", len); }
    return len;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }

    // AT+CIPRECVDATA=<link_id>,<len>
    sendAT(GF("+CIPRECVDATA="), mux, ',', (uint16_t)size);
    // +CIPRECVDATA:<actual_len>,<"remote IP">,<remote port>,<data>
    if (waitResponse(GF("+CIPRECVDATA:")) != 1) { return 0; }
    int16_t len_reported = streamGetIntBefore(',');
    size_t  len_read     = moveCharsFromStreamToFifo(mux, len_reported);
    waitResponse();  // final ok

    // Check how much is left in the buffer after reading.
    sockets[mux]->sock_available = modemGetAvailable(mux);
    return len_read;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    size_t result = 0;
    sendAT(GF("+CIPRECVLEN?"));
    // +CIPRECVLEN:<data length of link>[...][,<data length of link>]
    if (waitResponse(GF("+CIPRECVLEN:")) != 1) { return result; }
    for (uint8_t muxNo = 0; muxNo < TcpConfig::kMuxCount; muxNo++) {
      long mux_avail = stream.parseInt();
      if (sockets[muxNo]) { sockets[muxNo]->sock_available = mux_avail; }
    }
    waitResponse();  // ends with OK
    result = sockets[mux]->sock_available;
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPSTATE?"));
    // initialize the connection array assuming no connections are active
    bool verified_connections[TcpConfig::kMuxCount] = {0};
    for (uint8_t muxNo = 0; muxNo < TcpConfig::kMuxCount; muxNo++) {
      uint8_t has_status = waitResponse(GF("+CIPSTATE:"),
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
      } else {
        break;
      };  // once we get to the ok or error, stop
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
      int16_t len_reported = streamGetIntBefore('\n');
      if (isValidMux(mux)) {
        sockets[static_cast<uint8_t>(mux)]->got_data = true;
        // TODO: I'm not sure if each +IPD URC reports the amount newly received
        // or the total now in the buffer. It appears to be the latter.
        // sockets[mux]->sock_available = sockets[mux]->sock_available + len;
        sockets[static_cast<uint8_t>(mux)]->sock_available = len_reported;
      }
      data = "";
      DBG("### Got Data:", len_reported, "on", mux);
      return true;
    } else if (data.endsWith(GF("CLOSED\r\n"))) {
      int16_t muxStart =
          TinyGsmMax(0,
                     data.lastIndexOf(String(GFP(ModemConfig::GSM_NL)),
                                      data.length() - 8));
      int16_t coma = data.indexOf(',', muxStart);
      int16_t mux  = data.substring(muxStart, coma).toInt();
      if (isValidMux(mux)) {
        sockets[static_cast<uint8_t>(mux)]->sock_connected = false;
      }
      data = "";
      DBG("### Closed: ", mux);
      return true;
    } else if (data.endsWith(GF("ERR CODE:"))) {
#if defined(TINY_GSM_DEBUG) && !defined(DUMP_AT_COMMANDS)
      DBG("### ERR CODE: ", stream.readStringUntil('\n'));
#else
      streamSkipUntil('\n');  // Read out the ERR CODE
#endif
      data = "";
      return true;
    } else if (data.endsWith(GF("+TIME_UPDATED"))) {
      streamSkipUntil('\n');  // Refresh time and time zone by network
      data = "";
      DBG("### Network time updated.");
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
      // WIFI GOT IP; WIFI GOT IPv6 LL; WIFI GOT IPv6 GL
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
  GsmClientESP32* sockets[TcpConfig::kMuxCount];
};

#endif  // SRC_TINYGSMCLIENTESP32_H_
