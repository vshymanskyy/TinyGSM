/**
 * @file       TinyGsmClientESP8266.h
 * @brief      ESP8266 AT modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup espressif_esp8266 Espressif ESP8266 (AT)
 * @ingroup espressif_at
 * @brief Manufacturer: Espressif. Models: ESP8266 (AT firmware version
 * ~2.3.0.0).
 *
 * Documentation for the ESP8266 AT firmware can be found here:
 * https://docs.espressif.com/projects/esp-at/en/release-v2.3.0.0_esp8266/Get_Started/index.html
 *
 * # Connection Information
 *
 * - Combined TCP/SSL sockets:
 *   - 5
 *   - Using more than 1 SSL socket at a time may cause the module to crash.
 * - SSL contexts:
 *   - 2
 * - Socket Buffering:
 *   - The modem does **not** have an internal buffer for incoming data.
 *   - You must set the buffer size in this library to be larger than your
 * largest expected incoming packet, or you will lose incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 */

#ifndef SRC_TINYGSMCLIENTESP8266_H_
#define SRC_TINYGSMCLIENTESP8266_H_
#pragma message("TinyGSM:  TinyGsmClientESP8266")

#include "TinyGsmClientEspressif.h"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"

/// State: current Wi-Fi state.
/// @ingroup espressif_esp8266
enum class ESP8266RegStatus {
  /// ESP8266 station has not started any Wi-Fi connection.
  REG_UNINITIALIZED = 0,
  /// ESP8266 station has connected to an AP, but does not get an IPv4 address
  /// yet.
  REG_UNREGISTERED = 1,
  /// ESP8266 station has connected to an AP, and got an IPv4 address.
  REG_OK = 2,
  /// ESP8266 station is in Wi-Fi connecting or reconnecting state.
  REG_CONNECTING = 3,
  /// ESP8266 station is in Wi-Fi disconnected state.
  REG_DISCONNECTING = 4,
  /// ESP8266 station is in an unknown state.
  REG_UNKNOWN = 5,
};

/// Basic modem configurations for the ESP8266
/// @ingroup espressif_esp8266
struct TinyGsmESP8266ModemConfig
    : public TinyGsmModemConfigPreset<ESP8266RegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "Espressif";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "ESP8266";

  /// The set namespace for the certificate authority (CA) certificate
  static constexpr char CA_CERT_NAMESPACE[] TINY_GSM_PROGMEM = "client_ca";
  /// The set namespace for the client certificate
  static constexpr char CLIENT_CERT_NAMESPACE[] TINY_GSM_PROGMEM =
      "client_cert";
  /// The set namespace for the client key
  static constexpr char CLIENT_KEY_NAMESPACE[] TINY_GSM_PROGMEM = "client_key";
};

constexpr char TinyGsmESP8266ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmESP8266ModemConfig::MODEM_MODEL[] __attribute__((weak));
constexpr char TinyGsmESP8266ModemConfig::CA_CERT_NAMESPACE[]
    __attribute__((weak));
constexpr char TinyGsmESP8266ModemConfig::CLIENT_CERT_NAMESPACE[]
    __attribute__((weak));
constexpr char TinyGsmESP8266ModemConfig::CLIENT_KEY_NAMESPACE[]
    __attribute__((weak));

/**
 * @brief TCP behavior and limits for the ESP8266.
 *
 * NOTE: There's a total limit of 5 sockets, any of them can be SSL. BUT the
 * manual warns that module may not be able to handle more than 1 SSL socket at
 * a time.
 *
 * The ESP8266 devices can receive 2048 bytes and send 1460 bytes at most in a
 * single transmission.
 * @ingroup espressif_esp8266
 */
struct TinyGsmESP8266TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::NoModemBuffer,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 5,
          /*sendMaxSize*/ 1460,
          /*connectTimeoutS*/ 75,  // default
          /*stopTimeoutS*/ 5> {};

/**
 * @brief SSL behavior and limits for the ESP8266.
 *
 * These modules don't have "SSL Contexts" per se.  They support 2 each of CA
 * certificates and client certificate/key pairs.  The certificates are
 * loaded and referenced by number.  The number of the CA certificate and the
 * client certificate/key pair does not have to match for a connection.
 *
 * @ingroup espressif_esp8266
 */
struct TinyGsmESP8266SslConfig : public TinyGsmSslConfigPreset<
                                     /*contextCount*/ 2> {};

/**
 * @brief Class for the Espressif ESP8266 modem, which is a Wi-Fi module with
 * SSL support.
 *
 * @warning This class is used to communicate with a module that has been
 * programmed with the AT command firmware.
 * @ingroup espressif_esp8266
 */
class TinyGsmESP8266
    : public TinyGsmEspressif<TinyGsmESP8266, TinyGsmESP8266ModemConfig>,
      public TinyGsmTCP<TinyGsmESP8266, TinyGsmESP8266TcpConfig>,
      public TinyGsmSSL<TinyGsmESP8266, TinyGsmESP8266SslConfig>,
      public TinyGsmTime<TinyGsmESP8266>,
      public TinyGsmNTP<TinyGsmESP8266> {
  friend class TinyGsmEspressif<TinyGsmESP8266, TinyGsmESP8266ModemConfig>;
  friend class TinyGsmModem<TinyGsmESP8266, TinyGsmESP8266ModemConfig>;
  friend class TinyGsmWifi<TinyGsmESP8266>;
  friend class TinyGsmTCP<TinyGsmESP8266, TinyGsmESP8266TcpConfig>;
  friend class GsmClient<TinyGsmESP8266, TinyGsmESP8266TcpConfig>;
  friend class TinyGsmSSL<TinyGsmESP8266, TinyGsmESP8266SslConfig>;
  friend class TinyGsmTime<TinyGsmESP8266>;
  friend class TinyGsmNTP<TinyGsmESP8266>;

 public:
  using ModemConfig = TinyGsmESP8266ModemConfig;
  using TcpConfig   = TinyGsmESP8266TcpConfig;
  using SslConfig   = TinyGsmESP8266SslConfig;
  class GsmSecureContextESP8266;
  using GsmSecureContext = GsmSecureContextESP8266;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup espressif_esp8266
  class GsmClientESP8266
      : public GsmClient<TinyGsmESP8266, TinyGsmESP8266TcpConfig> {
    friend class TinyGsmESP8266;

   public:
    using GsmClient<TinyGsmESP8266, TinyGsmESP8266TcpConfig>::connect;
    using GsmClient<TinyGsmESP8266, TinyGsmESP8266TcpConfig>::stop;
    using TcpConfig = TinyGsmESP8266TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientESP8266() {
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
    explicit GsmClientESP8266(TinyGsmESP8266& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmESP8266, TinyGsmESP8266TcpConfig>(modem, mux) {
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
  /// @ingroup espressif_esp8266
  class GsmClientSecureESP8266
      : public GsmSecureClient<GsmClientESP8266, TinyGsmESP8266> {
    friend class TinyGsmESP8266;

   public:
    using GsmClientESP8266::connect;
    using GsmClientESP8266::stop;
    using GsmSecureClient::GsmSecureClient;
    using TcpConfig = TinyGsmESP8266TcpConfig;

    // Because we have the same potential range of mux numbers for secure and
    // insecure connections, we don't need to re-check for mux number
    // availability.
  };

  /*
   * Inner Secure Context
   */
 public:
  /// Inner secure context
  /// @ingroup espressif_esp8266
  class GsmSecureContextESP8266
      : public ::GsmSecureContext<TinyGsmESP8266, TinyGsmESP8266SslConfig> {
    friend class TinyGsmESP8266;

   public:
    using ::GsmSecureContext<TinyGsmESP8266,
                             TinyGsmESP8266SslConfig>::GsmSecureContext;

    /// Unregister this context from its modem.
    ~GsmSecureContextESP8266() {}

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
      if (!TinyGsmESP8266::parseCACertificateNumber(CAcertName, certNumber)) {
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
      if (!TinyGsmESP8266::parseClientCredentialNumber(
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
      TinyGsmESP8266::getCertificateName(CertificateType::CA_CERTIFICATE,
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
      TinyGsmESP8266::getCertificateName(CertificateType::CLIENT_CERTIFICATE,
                                         certNumber, cert_name);
      memcpy(this->clientCertName, cert_name,
             TinyGsmMin(sizeof(this->clientCertName), sizeof(cert_name)));
      this->clientCertName[TinyGsmMin(sizeof(this->clientCertName),
                                      sizeof(cert_name)) -
                           1] = '\0';
      // generate and set the name for the client private key from the number
      TinyGsmESP8266::getCertificateName(CertificateType::CLIENT_KEY,
                                         certNumber, cert_name);
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
  explicit TinyGsmESP8266(Stream& stream)
      : TinyGsmEspressif<TinyGsmESP8266, TinyGsmESP8266ModemConfig>(stream) {
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
  // Follows functions inherited from Espressif

  /*
   * Generic network functions
   */
 protected:
  ESP8266RegStatus getRegistrationStatusImpl() {
    sendAT(GF("+CWSTATE?"));
    if (waitResponse(3000, GF("+CWSTATE:")) != 1)
      return ESP8266RegStatus::REG_UNKNOWN;
    // +CWSTATE:{state},{"ssid"}
    // followed by an OK
    int8_t status = streamGetIntBefore(',');
    streamSkipUntil('\n');  // throw away the ssid
    waitResponse();         // wait for trailing OK
    return static_cast<ESP8266RegStatus>(status);
  }

  bool isNetworkConnectedImpl() {
    ESP8266RegStatus s = this->getRegistrationStatus();
    return (s == ESP8266RegStatus::REG_OK);
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
  // Uses the secure client inherited from TinyGsmSSL.tpp for setting the
  // certificate name and the SSL connection type so those can be called at
  // connection time, but this library does **NOT** currently support uploading,
  // deleting, or converting certificates on the modem.
  // On the ESP8266 the only way to update your certificates is to use a
  // combination of python scripts, AT commands, and flash tools or to fully
  // build your own version of the AT firmware.

#undef TINY_GSM_MODEM_CAN_LOAD_CERTS

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
    if (!TinyGsmTime<TinyGsmESP8266>::isValidDateTimeValues(
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

    // Blank holders for the SSL auth mode and certificates
    SSLAuthMode sslAuthMode = SSLAuthMode::NO_VALIDATION;
    uint8_t     ca_number   = 0;
    uint8_t     pki_number  = 0;
    // If we actually have a secure socket populate the above with real values
    if (ssl) {
      // Use a static cast to get the context index.
      // This isn't ideal; hopefully the compiler will save us from ourselves.
      // We cannot use a dynamic cast because Arduino compiles with -fno-rtti.
      const GsmClientSecureESP8266* thisClient =
          static_cast<const GsmClientSecureESP8266*>(sockets[mux]);
      uint8_t sslCtxIndex = thisClient->sslCtxIndex;
      // Retrieve the SSL context object for the given context ID.
      ::GsmSecureContext<TinyGsmESP8266, TinyGsmESP8266SslConfig>* sslContext =
          getSSLContext(sslCtxIndex);
      // check for a valid SSL context
      if (sslContext == nullptr) {
        DBG(GF("### No SSL context is registered at index"), sslCtxIndex);
        return false;
      }
      sslAuthMode = sslContext->sslAuthMode;

      // check that the context has the required credentials
      if (!validateSSLContextCredentials(sslContext)) {
        DBG(GF("### The SSL context is missing required credentials!"));
        return false;
      }

      if (sslAuthMode == SSLAuthMode::CA_VALIDATION ||
          sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION) {
        if (!parseCACertificateNumber(sslContext->CAcertName, ca_number)) {
          DBG(GF("### Invalid CA certificate name for ESP8266 SSL context!"));
          return false;
        }
      }

      if (sslAuthMode == SSLAuthMode::CLIENT_VALIDATION ||
          sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION) {
        if (!parseClientCredentialNumber(sslContext->clientCertName,
                                         sslContext->clientKeyName,
                                         pki_number)) {
          DBG(GF("### Invalid client credentials for ESP8266 SSL context!"));
          return false;
        }
      }

      DBG(GF("### SSL context index:"), sslCtxIndex);
      DBG(GF("### SSL auth mode:"), (int)sslAuthMode);
      DBG(GF("### CA cert number:"), ca_number);
      DBG(GF("### PKI number:"), pki_number);

      // SSL certificate checking will not work without a valid timestamp!
      if ((sslAuthMode != SSLAuthMode::NO_VALIDATION) &&
          !waitForTimeSync(timeout_s)) {
        DBG(GF("### WARNING: The module timestamp must be valid for SSL auth. "
               "Please use setTimeZone(...) or NTPServerSync(...) to enable "
               "time syncing before attempting an SSL connection!"));
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
        DBG(GF("### The ESP8266 does not support SSL using pre-shared keys."));
        // The ESP8266 does not support SSL using pre-shared keys with AT
        // firmware.
        return false;
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
        DBG(GF("### Unsupported ESP8266 SSL authentication mode!"));
        return false;
      }
      if (waitResponse() != 1) { return false; }

      // set the SSL SNI (server name indication)
      // Multiple connections: (AT+CIPMUX=1)
      // AT+CIPSSLCSNI=<link ID>,<"sni">
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
  using TinyGsmEspressif<TinyGsmESP8266, TinyGsmESP8266ModemConfig>::modemStop;

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
  GsmClientESP8266* sockets[TcpConfig::kMuxCount];
};

#endif  // SRC_TINYGSMCLIENTESP8266_H_
