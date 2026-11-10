/**
 * @file       TinyGsmClientSim7000SSL.h
 * @brief      SIM7000 SSL modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup simcom_sim7000ssl SIMCom SIM7000 (SSL toolkit)
 * @brief Manufacturer: SIMCom. Models: SIM7000 (SSL toolkit)
 * @ingroup simcom_sim70xx
 *
 * # Connection Information
 *
 * - TCP sockets:
 *   - 8
 * - SSL sockets:
 *   - 2
 * - SSL contexts:
 *   - 6
 * - Socket Buffering:
 *   - The modem has an internal buffer for incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 *
 * @todo In `configureSSLContext()`: Skip verifying the context number?
 */

#ifndef SRC_TINYGSMCLIENTSIM7000SSL_H_
#define SRC_TINYGSMCLIENTSIM7000SSL_H_
#pragma message("TinyGSM:  TinyGsmClientSim7000SSL")

#include "TinyGsmClientSIM70xx.h"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"

/// Basic modem configurations for the SIMCom SIM7000 (SSL toolkit)
/// @ingroup simcom_sim7000ssl
struct TinyGsmSim7000SSLModemConfig
    : public TinyGsmModemConfigPreset<Sim70xxRegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "SIMCom";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM7000";
};

constexpr char TinyGsmSim7000SSLModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmSim7000SSLModemConfig::MODEM_MODEL[]
    __attribute__((weak));

/**
 * @brief TCP behavior and limits for the SIMCom SIM7000 (SSL toolkit).
 *
 * @todo Handle the different number of sockets for TCP and SSL in the code.
 *
 * The SIM7000 manual doesn't specify the max size for CASEND, but the SIM7080
 * takes up to 1460, so we'll use that.
 * @ingroup simcom_sim7000ssl
 */
struct TinyGsmSim7000SSLTcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 8,
          /*sendMaxSize*/ 1460> {};

/**
 * @brief SSL behavior and limits for SIMCom SIM7000 (SSL toolkit).
 * The module supports 6 SSL contexts (0-5).
 * @ingroup simcom_sim7000ssl
 */
struct TinyGsmSim7000SSLSslConfig : public TinyGsmSslConfigPreset<
                                        /*contextCount*/ 6> {};

/// Class for the SIMCOM SIM7000 with SSL support using the SSL application
/// @ingroup simcom_sim7000ssl
class TinyGsmSim7000SSL
    : public TinyGsmSim70xx<TinyGsmSim7000SSL, TinyGsmSim7000SSLModemConfig>,
      public TinyGsmTCP<TinyGsmSim7000SSL, TinyGsmSim7000SSLTcpConfig>,
      public TinyGsmSSL<TinyGsmSim7000SSL, TinyGsmSim7000SSLSslConfig>,
      public TinyGsmSMS<TinyGsmSim7000SSL>,
      public TinyGsmGSMLocation<TinyGsmSim7000SSL>,
      public TinyGsmTime<TinyGsmSim7000SSL>,
      public TinyGsmNTP<TinyGsmSim7000SSL>,
      public TinyGsmBattery<TinyGsmSim7000SSL> {
  friend class TinyGsmSim70xx<TinyGsmSim7000SSL, TinyGsmSim7000SSLModemConfig>;
  friend class TinyGsmModem<TinyGsmSim7000SSL, TinyGsmSim7000SSLModemConfig>;
  friend class TinyGsmCellularData<TinyGsmSim7000SSL>;
  friend class TinyGsmTCP<TinyGsmSim7000SSL, TinyGsmSim7000SSLTcpConfig>;
  friend class GsmClient<TinyGsmSim7000SSL, TinyGsmSim7000SSLTcpConfig>;
  friend class TinyGsmSSL<TinyGsmSim7000SSL, TinyGsmSim7000SSLSslConfig>;
  friend class TinyGsmSMS<TinyGsmSim7000SSL>;
  friend class TinyGsmGSMLocation<TinyGsmSim7000SSL>;
  friend class TinyGsmGPS<TinyGsmSim7000SSL>;
  friend class TinyGsmTime<TinyGsmSim7000SSL>;
  friend class TinyGsmNTP<TinyGsmSim7000SSL>;
  friend class TinyGsmBattery<TinyGsmSim7000SSL>;

 public:
  using ModemConfig = TinyGsmSim7000SSLModemConfig;
  using TcpConfig   = TinyGsmSim7000SSLTcpConfig;
  using SslConfig   = TinyGsmSim7000SSLSslConfig;
  using GsmSecureContext =
      TinyGsmSSL<TinyGsmSim7000SSL,
                 TinyGsmSim7000SSLSslConfig>::gsmSecureContext;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup simcom_sim7000ssl
  class GsmClientSim7000SSL
      : public GsmClient<TinyGsmSim7000SSL, TinyGsmSim7000SSLTcpConfig> {
    friend class TinyGsmSim7000SSL;

   public:
    using GsmClient<TinyGsmSim7000SSL, TinyGsmSim7000SSLTcpConfig>::connect;
    using GsmClient<TinyGsmSim7000SSL, TinyGsmSim7000SSLTcpConfig>::stop;
    using TcpConfig = TinyGsmSim7000SSLTcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSim7000SSL() {
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
    explicit GsmClientSim7000SSL(TinyGsmSim7000SSL& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmSim7000SSL, TinyGsmSim7000SSLTcpConfig>(modem, mux) {
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
  /// @ingroup simcom_sim7000ssl
  class GsmClientSecureSim7000SSL
      : public GsmClientSecure<GsmClientSim7000SSL, TinyGsmSim7000SSL> {
    friend class TinyGsmSim7000SSL;

   public:
    using GsmClientSecure::GsmClientSecure;
    using GsmClientSim7000SSL::connect;
    using GsmClientSim7000SSL::stop;
    using TcpConfig = TinyGsmSim7000SSLTcpConfig;

    // Because we have the same potential range of mux numbers for secure and
    // insecure connections, we don't need to re-check for mux number
    // availability.

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
      // check that the context has the required credentials
      if (!at->validateSSLContextCredentials(sslContext)) {
        DBG("### The SSL context is missing required credentials!");
        return 0;
      }
      // this module accepts PSK credentials only as a converted PSK table
      if (sslContext->sslAuthMode == SSLAuthMode::PRE_SHARED_KEYS &&
          sslContext->pskTableName[0] == '\0') {
        DBG("### The SIM7000 requires a PSK table; identity/key pairs are "
            "not supported.");
        return 0;
      }
      // configure the context
      if (!at->configureSSLContext(sslCtxIndex)) {
        DBG("### Failed to configure the SSL context!");
        return 0;
      }
      // set the SNI
      if (!at->configureSSLSNI(sslCtxIndex, host)) {
        DBG("### Failed to configure SSL SNI!");
        return 0;
      }
      sock_connected = at->modemConnect(host, port, mux, timeout_s);
      return sock_connected;
    }
  };

  /// Typedef for backward compatibility
  using GsmClientSecureSIM7000SSL = GsmClientSecureSim7000SSL;

  /*
   * Inner Secure Context
   */
 public:
  /// Inner secure context
  /// @ingroup simcom_sim7000ssl
  class GsmSecureContextSim7000SSL
      : public ::GsmSecureContext<TinyGsmSim7000SSL,
                                  TinyGsmSim7000SSLSslConfig> {
    friend class TinyGsmSim7000SSL;

   public:
    using ::GsmSecureContext<TinyGsmSim7000SSL,
                             TinyGsmSim7000SSLSslConfig>::GsmSecureContext;

    /// Unregister this context from its modem.
    ~GsmSecureContextSim7000SSL() {}
  };

  /*
   * Public Aliases
   */
 public:
  /// Arduino Client type for this modem.
  using TinyGsmClient = GsmClientSim7000SSL;
  /// Secure Arduino Client type for this modem.
  using TinyGsmClientSecure = GsmClientSecureSim7000SSL;
  /// Security-context type for this modem.
  using TinyGsmSecureContext = GsmSecureContextSim7000SSL;

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmSim7000SSL(Stream& stream)
      : TinyGsmSim70xx<TinyGsmSim7000SSL, TinyGsmSim7000SSLModemConfig>(
            stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM7000SSL"));

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

  void maintainImpl() {
    // Keep listening for modem URC's and proactively iterate through
    // sockets asking if any data is available
    bool check_socks = false;
    for (uint8_t mux = 0; mux < TcpConfig::kMuxCount; mux++) {
      GsmClientSim7000SSL* sock = sockets[mux];
      if (sock && sock->got_data) {
        sock->got_data = false;
        check_socks    = true;
      }
    }
    // modemGetAvailable checks all socks, so we only want to do it once
    // modemGetAvailable calls modemGetConnected(), which also checks all socks
    if (check_socks) { modemGetAvailable(0); }
    while (stream.available()) { waitResponse(15, nullptr, nullptr); }
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
    sendAT(GF("+CNACT?"));
    if (waitResponse(GF("+CNACT:")) != 1) { return ""; }
    streamSkipUntil('\"');
    String res = stream.readStringUntil('\"');
    waitResponse();
    return res;
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */

  //  NOTE: These functions are copied from the SIM7080 implementation. Per the
  //  AT command manual, the commands are identical and **should** work for both
  //  modules. BUT I (SRGDamia1) have done no testing to confirm this.

  // The name of the certificate/key/password file. The file name must
  // have type like ".pem" or ".der".
  // NOTE: Uploading the certificate only happens by filename, the type of
  // certificate does not matter here
  template <typename TCert>
  bool loadCertificateImpl(const char* certificateName, TCert cert,
                           const uint16_t len) {
    bool success = true;
    // Initialize AT relate to file system functions
    sendAT(GF("+CFSINIT"));
    success &= waitResponse(5000L) == 1;
    if (!success) { return false; }

    // NOTE: It just works much better if we wait a little bit before asking to
    // write the certificate
    delay(100);

    // Write File to the Flash Buffer Allocated by CFSINIT
    // AT+CFSWFILE=<index>,<file name>,<mode>,<file size>,<input time>
    //<index> 3: "/customer/" (always use customer for certificates)
    //<file name> File name length should less or equal 230 characters
    // <mode> 0: If the file already existed, write the data at the beginning of
    //           the file. - We always do this
    //        1: If the file already existed, add the data at the end of the
    //        file.
    // <file size> File size should be less than 10240 bytes
    // <input time> Millisecond, should send file during this period or you
    // can’t send file when timeout. The value should be less than 10000 ms.
    // <len_filename> Integer type. Maximum length of parameter <file name>.
    sendAT(GF("+CFSWFILE=3,\""), certificateName, GF("\",0,"), len,
           GF(",10000"));
    // The module sends back a 'DOWNLOAD' prompt - sometimes preceded by an 'OK'
    // NOTE: If we don't get the "DOWNLOAD" response or somehow miss hearing it,
    // we don't write the file. If we don't write something within 10 seconds
    // (the <input time>), the terminal will timeout and send back an 'OK' at
    // the 10s mark.
    success &= waitResponse(10500L, GF("DOWNLOAD"),
                            GFP(ModemConfig::GSM_ERROR)) == 1;

    if (success) {
      this->streamWrite(cert, len);
      stream.flush();
      success &= waitResponse(5000L) == 1;
    } else {
      DBG(GF("### Failed to get download prompt!"));
    }

    // Verify the size of the uploaded file
    // AT+CFSGFIS=<index>,<filename>
    //<index> 3: "/customer/" (always use customer for certificates)
    //<file name> File name length should less or equal 230 characters
    sendAT(GF("+CFSGFIS=3,\""), certificateName, '"');
    // Response: +CFSGFIS: <file_size>
    success &= waitResponse(5000L, GF("+CFSGFIS:")) == 1;
    if (success) {
      int16_t len_confirmed = streamGetIntBefore('\n');
      success &= len_confirmed == len;
      success &= waitResponse(5000L) == 1;
    }

    // Release AT relates to file system functions.
    // NOTE: We need to do this even if we didn't successfully write the file
    sendAT(GF("+CFSTERM"));
    success &= waitResponse(5000L) == 1;

    return success;
  }

  // NOTE: Deleting the certificate only happens by filename, the type of
  // certificate does not matter here
  bool deleteCertificateImpl(const char* certificateName) {
    bool success = true;

    // Initialize AT relate to file system functions
    sendAT(GF("+CFSINIT"));
    if (waitResponse(5000L) != 1) { return false; }

    // Delete file
    sendAT(GF("+CFSDFILE=3,\""), certificateName, '"');
    success &= waitResponse(5000L) == 1;

    // Release AT relates to file system functions.
    // NOTE: We need to do this even if we didn't successfully delete the file
    sendAT(GF("+CFSTERM"));
    return success & (waitResponse(5000L) == 1);
  }

  bool printCertificateImpl(const char* filename, Stream& print_stream) {
    bool    success   = true;
    int16_t print_len = 0;

    // Initialize AT relate to file system functions
    sendAT(GF("+CFSINIT"));
    if (waitResponse(5000L) != 1) { return false; }

    // Read the file
    // AT+CFSRFILE=<index>,<filename>,<mode>,<filesize>,<position>
    // <index> 3: "/customer/" (always use customer for certificates)
    // <file name> File name length should less or equal 230 characters
    // <mode> 0:Read data at the beginning of the file
    // <filesize> File size should be less than 10240 bytes, we put 10240 here
    //    because we want to read the whole file.
    // <position> The starting position that will be read in the file.
    sendAT(GF("+CFSRFILE=3,\""), filename, GF("\",0,10240,0"));
    success &= waitResponse(5000L, GF("+CFSRFILE:")) == 1;
    if (success) { print_len = streamGetIntBefore('\n'); }

    // wait for some characters to be available
    uint32_t start = millis();
    while (!stream.available() && millis() - start < 10000) { yield(); }

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

    // wait for the ending OK
    success &= waitResponse(5000L) == 1;

    // Release AT relates to file system functions.
    // NOTE: We need to do this even if we didn't successfully delete the file
    sendAT(GF("+CFSTERM"));
    return success & (waitResponse(5000L) == 1);
  }


  bool convertCertificateImpl(CertificateType cert_type, const char* filename) {
    // Convert certificate into something the module will use and save it to
    // file
    switch (cert_type) {
      case CertificateType::CLIENT_PSK:
      case CertificateType::CLIENT_PSK_IDENTITY: {
        DBG("### WARNING: The PSK and PSK identity must be converted together "
            "on the SIM7080.  Please use the convertPSKandID(..) function.");
        return false;
      }
      case CertificateType::CLIENT_CERTIFICATE:
      case CertificateType::CLIENT_KEY: {
        DBG("### WARNING: The client certificate and matching key must be "
            "converted together on the SIM7080.  Please use the "
            "convertClientCertificates(..) function.");
        return false;
      }
      case CertificateType::CA_CERTIFICATE:
      default: {
        return convertCACertificateImpl(filename);
      }
    }
  }
  bool convertCACertificateImpl(const char* ca_cert_name) {
    //  AT+CSSLCFG="CONVERT",<ssltype>,<cname>[,<keyname>[,<passkey>]]
    // <ssltype> 2=QAPI_NET_SSL_CA_LIST_E
    // <cname> name of certificate file
    sendAT(GF("+CSSLCFG=\"convert\",2,\""), ca_cert_name, '"');
    return waitResponse(5000L) == 1;
    // After conversion, the AT manual suggests you delete the files!
  }

  bool convertClientCertificatesImpl(const char* client_cert_name,
                                     const char* client_cert_key) {
    // Convert certificate into something the module will use and save it to
    // file
    //  AT+CSSLCFG="CONVERT",<ssltype>,<cname>[,<keyname>[,<passkey>]]
    // <ssltype> 1=QAPI_NET_SSL_CERTIFICATE_E
    // <cname> name of certificate file
    // <keyname> name of key file (for client cert/key pairs)
    // <passkey> passkey for the client key file [NOT SUPPORTED BY TINYGSM]
    sendAT(GF("+CSSLCFG=\"convert\",1,\""), client_cert_name, GF("\",\""),
           client_cert_key, '"');
    return waitResponse(5000L) == 1;
    // After conversion, the AT manual suggests you delete the files!
  }

  bool convertPSKTableImpl(const char* psk_table_name) {
    // Convert certificate into something the module will use and save it to
    // file
    //  AT+CSSLCFG="CONVERT",<ssltype>,<cname>[,<keyname>[,<passkey>]]
    // <ssltype> 3=QAPI_NET_SSL_PSK_TABLE_E
    // <cname> name of PSK table file
    // <keyname> not used for PSK tables
    // <passkey> not used for PSK tables
    sendAT(GF("+CSSLCFG=\"convert\",3,\""), psk_table_name, '"');
    return waitResponse(5000L) == 1;
    // After conversion, the AT manual suggests you delete the files!
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

    // Define the PDP context
    // AT+CGDCONT=<cid>[,<PDP_type>[,<APN>[,<PDP_addr>[,<d_comp>[,<h_comp>][,<ipv4_ctrl>[,<emergency_flag>]]]]]]
    // NOTE: The minimum cid (PDP context identifier) is 1 (range 1-15)
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Attach to Packet Domain service
    // AT+CGATT={state}
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // NOTE:  **DO NOT** activate the PDP context
    // For who only knows what reason, doing so screws up the rest of the
    // process

    // Bearer settings for applications based on IP
    // Set the user name and password
    // AT+CNCFG=<ip_type>[,<APN>[,<usename>,<password>[,<authentication>]]]
    // <ip_type> 0: Dual PDN Stack
    //           1: Internet Protocol Version 4
    //           2: Internet Protocol Version 6
    // <authentication> 0: NONE
    //                  1: PAP
    //                  2: CHAP
    //                  3: PAP or CHAP
    if (pwd && strlen(pwd) > 0 && user && strlen(user) > 0) {
      sendAT(GF("+CNCFG=1,\""), apn, GF("\",\""), user, GF("\",\""), pwd,
             GF("\",3"));
      waitResponse();
    } else if (user && strlen(user) > 0) {
      // Set the user name only
      sendAT(GF("+CNCFG=1,\""), apn, GF("\",\""), user, '"');
      waitResponse();
    } else {
      // Set the APN only
      sendAT(GF("+CNCFG=1,\""), apn, '"');
      waitResponse();
    }

    // Activate application network connection
    // This is for most other supported applications outside of the
    // TCP application toolkit (ie, SSL)
    // AT+CNACT=<mode>,<action>
    // <mode> 0: Deactive
    //        1: Active
    //        2: Auto Active
    bool res    = false;
    int  ntries = 0;
    while (!res && ntries < 5) {
      sendAT(GF("+CNACT=1,\""), apn, '"');
      res = waitResponse(60000L, GF("+APP PDP: ACTIVE"),
                         GF("+APP PDP: DEACTIVE"),
                         GFP(ModemConfig::GSM_ERROR)) == 1;
      waitResponse();
      ntries++;
    }

    return res;
  }

  bool dataDisconnectImpl() {
    // Shut down the general application TCP/IP connection
    // CNACT will close *all* open application connections
    sendAT(GF("+CNACT=0"));
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
   * SSL context functions
   */
 public:
  /**
   * @brief Configure the SSL context for the modem.
   * @param context_id The SSL context ID.
   * @param sslAuthMode The SSL authentication mode.
   * @param sslVersion The SSL version.
   * @return True if the operation was successful, false otherwise.
   */
  bool configureSSLContext(uint8_t context_id, SSLAuthMode sslAuthMode,
                           SSLVersion sslVersion) {
    // store the configuration in the SSL context object
    GsmSecureContext* sslContext = getSSLContext(context_id);
    if (sslContext) {
      sslContext->sslAuthMode = sslAuthMode;
      sslContext->sslVersion  = sslVersion;
    }

    return configureSSLContext(context_id);
  }
  /**
   * @brief Configure the SSL context for the modem.
   * @param context_id The SSL context ID.
   * @return True if the operation was successful, false otherwise.
   */
  bool configureSSLContext(uint8_t context_id) {
    bool success = true;

    // NOTE: The SSL context (<ctxindex>) is not the same as the connection
    // identifier.  The SSL context is the grouping of SSL settings, the
    // connection identifier is the mux/socket number.
    // CSSLCFG commands reference the SSL context number;
    // C**A**SSLCFG commands reference the connection number (aka, the mux).

    // NOTE: If you want to tie the same SSL context to an MQTT application on
    // the module, you need to add one to the index in the SMSSL command (ie,
    // SMSSL=1,... will call settings configured for SSL context 0)

    // Retrieve the SSL context object for the given context ID.
    GsmSecureContext* sslContext  = getSSLContext(context_id);
    SSLAuthMode       sslAuthMode = sslContext ? sslContext->sslAuthMode
                                               : SSLAuthMode::NO_VALIDATION;
    SSLVersion        sslVersion  = sslContext ? sslContext->sslVersion
                                               : SSLVersion::ALL_SSL;

    DBG("### SSL context index:", context_id);
    DBG("### SSL auth mode:", (int)sslAuthMode);
    DBG("### SSL version:", (int)sslVersion);

    // set the ssl version
    // AT+CSSLCFG="SSLVERSION",<ctxindex>,<sslversion>
    // <ctxindex> SSL context identifier
    // <sslversion> 0: QAPI_NET_SSL_PROTOCOL_UNKNOWN
    //              1: QAPI_NET_SSL_PROTOCOL_TLS_1_0
    //              2: QAPI_NET_SSL_PROTOCOL_TLS_1_1
    //              3: QAPI_NET_SSL_PROTOCOL_TLS_1_2
    //              4: QAPI_NET_SSL_PROTOCOL_DTLS_1_0
    //              5: QAPI_NET_SSL_PROTOCOL_DTLS_1_2
    int8_t s70x_ssl_version = 3;
    // convert the ssl version into the format for this command
    switch (sslVersion) {
      case SSLVersion::TLS1_0: {
        s70x_ssl_version = 1;
        break;
      }
      case SSLVersion::TLS1_1: {
        s70x_ssl_version = 2;
        break;
      }
      case SSLVersion::TLS1_2: {
        s70x_ssl_version = 3;
        break;
      }
      case SSLVersion::TLS1_3: {
        s70x_ssl_version = 6;
        break;
      }
      default: {
        s70x_ssl_version = 0;
        break;
      }
    }
    sendAT(GF("+CSSLCFG=\"sslversion\","), context_id, ',', s70x_ssl_version);
    if (waitResponse(5000L) != 1) {
      if (sslVersion != SSLVersion::TLS1_3) {
        success = false;
      } else {
        // try lowering the SSL version to TLS 1.2 - not all firmwares
        // support 1.3
        sendAT(GF("+CSSLCFG=\"sslversion\","), context_id, GF(",3"));
        sslVersion = SSLVersion::TLS1_2;
        success &= waitResponse(5000L) == 1;
      }
    }

    // set the SSL protocol
    // AT+CSSLCFG="PROTOCOL",<ctxindex>,<protocol>
    // <ctxindex> SSL context identifier
    // <protocol> Sever name (we use the host)
    //            1 - QAPI_NET_SSL_TLS_E (TCP)
    //            2 - QAPI_NET_SSL_DTLS_E (UDP)
    // NOTE:  despite docs using caps, "protocol" must be in lower case
    sendAT(GF("+CSSLCFG=\"protocol\","), context_id, GF(",1"));
    success &= waitResponse() == 1;

    // set the SSL cipher suite(s)
    // AT+CSSLCFG="CIPHERSUITE",<ctxindex>,<cipher_index>,<ciphersuite>
    // <ctxindex> SSL context identifier
    // <cipher_index> 0-7
    // <ciphersuite> Hex code for the suite - there's a long list
    // NOTE:  despite docs using caps, "ciphersuite" must be in lower case

    if (sslAuthMode == SSLAuthMode::PRE_SHARED_KEYS) {
      const char* ciphersuites[8] = {
        "0xC0A9", "0xC0A8", "0xC0A5", "0xC0A4", "0xC095", "0xC094",
        "0x00B1", "0x00B0", /*"0x00AF", "0x00AE", "0x00A9", "0x00A8",
        "0x008D", "0x008C", "0x008B", "0x008A", "0x002C"*/};
      for (uint8_t i = 0; i < 8; i++) {
        sendAT(GF("+CSSLCFG=\"ciphersuite\","), context_id, ',', i, ',',
               ciphersuites[i]);
        success &= waitResponse() == 1;
      }
    } else if (sslVersion == SSLVersion::TLS1_3) {
      const char* ciphersuites[3] = {"0x1301", "0x1302", "0x1303"};
      for (uint8_t i = 0; i < 3; i++) {
        sendAT(GF("+CSSLCFG=\"ciphersuite\","), context_id, ',', i, ',',
               ciphersuites[i]);
        success &= waitResponse() == 1;
      }
    } else {
      // These are selected from this AWS list:
      // https://docs.aws.amazon.com/iot/latest/developerguide/transport-security.html
      const char* ciphersuites[] = {
          /*"0xC014", "0xC013", "0xC00A", "0xC009",*/  // not on my firmware
          "0xC02C",
          "0xC02B",
          "0xC030",
          "0xC02F",
          /*"0xC028", "0xC027", "0xC024", "0xC023",*/  // not on my firmware
          /*"0x009D", "0x009C",*/                      // not on my firmware
          /*"0x003D", "0x003C",*/                      // not on my firmware
          "0x0035",
          "0x002F",  // not for all SSL versions
      };
      for (uint8_t i = 0; i < sizeof(ciphersuites) / sizeof(ciphersuites[0]);
           i++) {
        sendAT(GF("+CSSLCFG=\"ciphersuite\","), context_id, ',', i, ',',
               ciphersuites[i]);
        success &= waitResponse() == 1;
      }
    }

    // Ignore the RTC time?
    // AT+CSSLCFG="IGNORERTCTIME",<ctxindex>,<ignorertctime>
    // <ctxindex> SSL context identifier
    // <ignorertctime> 0: Do not ignore the RTC time, 1: Ignore the RTC time
    sendAT(GF("+CSSLCFG=\"ignorertctime\","), context_id, GF(",0"));
    success &= waitResponse() == 1;

    // Query all the parameters that have been set for this SSL context
    // TODO(@SRGDamia1): Skip this?
    // AT+CSSLCFG="CTXINDEX" ,<ctxindex>
    // <ctxindex> SSL context identifier
    // NOTE:  despite docs using "CTXINDEX" in all caps, the module only
    // accepts the command "CTXINDEX" and it must be in lower case
    // +CSSLCFG:<ctxindex>,<sslversion>,<ciphersuite>,<ignorertctime>,<protocol>,<sni>
    sendAT(GF("+CSSLCFG=\"ctxindex\","), context_id);
    if (waitResponse(5000L, GF("+CSSLCFG:")) != 1) return false;
    streamSkipUntil('\n');  // read out the certificate information
    waitResponse();

    return success;
  }

  /**
   * @brief Apply hostname-dependent SNI to an SSL context.
   * @param context_id The SSL context ID.
   * @param sni The Server Name Indication for this connection.
   * @return True if the operation was successful, false otherwise.
   */
  bool configureSSLSNI(uint8_t context_id, const char* sni) {
    if (getSSLContext(context_id) == nullptr || sni == nullptr ||
        sni[0] == '\0') {
      return false;
    }
    // Despite the manual using caps, "sni" must be in lower case.
    sendAT(GF("+CSSLCFG=\"sni\","), context_id, GF(",\""), sni, '"');
    return waitResponse() == 1;
  }

  /**
   * @brief Apply SSL certificates to the specified connection.
   * @param mux The **zero-indexed** position of the client in the modem's
   * socket array.
   * @param context_id The SSL context ID.
   * @param sslAuthMode The SSL authentication mode.
   * @param CAcertName The CA certificate name.
   * @param clientCertName The client certificate name.
   * @param clientKeyName The client key name.
   * @return True if the operation was successful, false otherwise.
   */
  bool applySSLCertificates(uint8_t mux, uint8_t context_id,
                            SSLAuthMode sslAuthMode, const char* CAcertName,
                            const char* clientCertName,
                            const char* clientKeyName) {
    // store the configuration in the SSL context object
    GsmSecureContext* sslContext = getSSLContext(context_id);
    if (sslContext == nullptr) { return false; }
    sslContext->sslAuthMode = sslAuthMode;
    strncpy(sslContext->CAcertName, CAcertName ? CAcertName : "",
            sizeof(sslContext->CAcertName) - 1);
    sslContext->CAcertName[sizeof(sslContext->CAcertName) - 1] = '\0';
    strncpy(sslContext->clientCertName, clientCertName ? clientCertName : "",
            sizeof(sslContext->clientCertName) - 1);
    sslContext->clientCertName[sizeof(sslContext->clientCertName) - 1] = '\0';
    strncpy(sslContext->clientKeyName, clientKeyName ? clientKeyName : "",
            sizeof(sslContext->clientKeyName) - 1);
    sslContext->clientKeyName[sizeof(sslContext->clientKeyName) - 1] = '\0';

    return applySSLCertificates(mux, context_id);
  }
  /**
   * @brief Apply SSL certificates to the specified connection.
   * @param mux The **zero-indexed** position of the client in the modem's
   * socket array.
   * @param context_id The SSL context ID.
   * @return True if the operation was successful, false otherwise.
   */
  bool applySSLCertificates(uint8_t mux, uint8_t context_id) {
    bool success = true;

    // Retrieve the SSL context object for the given context ID.
    GsmSecureContext* sslContext  = getSSLContext(context_id);
    SSLAuthMode       sslAuthMode = sslContext ? sslContext->sslAuthMode
                                               : SSLAuthMode::NO_VALIDATION;
    const char* CAcertName     = sslContext ? sslContext->CAcertName : nullptr;
    const char* clientCertName = sslContext ? sslContext->clientCertName
                                            : nullptr;
    const char* clientKeyName  = sslContext ? sslContext->clientKeyName
                                            : nullptr;

    DBG("### SSL context index:", context_id);
    DBG("### SSL auth mode:", (int)sslAuthMode);
    DBG("### CA cert name:", CAcertName);
    DBG("### Client cert name:", clientCertName);
    DBG("### Client key name:", clientKeyName);

    // Re-convert the certificates, just in case
    if (CAcertName != nullptr &&
        strnlen(CAcertName, TINY_GSM_CERT_NAME_LENGTH) != 0) {
      convertCACertificate(CAcertName);
    }
    if (clientCertName != nullptr &&
        strnlen(clientCertName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
        clientKeyName != nullptr &&
        strnlen(clientKeyName, TINY_GSM_CERT_NAME_LENGTH) != 0) {
      convertClientCertificates(clientCertName, clientKeyName);
    }

    // apply the correct certificates to the connection
    if (CAcertName != nullptr &&
        strnlen(CAcertName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
        (sslAuthMode == SSLAuthMode::CA_VALIDATION ||
         sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION)) {
      // AT+CASSLCFG=<cid>,"CACERT",<caname>
      // <cid> Application connection ID (set with AT+CACID above)
      // <certname> certificate name
      sendAT(GF("+CASSLCFG="), mux, ",\"cacert\",\"", CAcertName, '"');
      success &= waitResponse() == 1;
    }
    if (clientCertName != nullptr &&
        strnlen(clientCertName, TINY_GSM_CERT_NAME_LENGTH) != 0 &&
        (sslAuthMode == SSLAuthMode::CLIENT_VALIDATION ||
         sslAuthMode == SSLAuthMode::MUTUAL_AUTHENTICATION)) {
      // AT+CASSLCFG=<cid>,"CERT",<certname>
      // <cid> Application connection ID (set with AT+CACID above)
      // <certname> Alphanumeric ASCII text string up to 64 characters.
      // Client certificate name that has been configured by AT+CSSLCFG.
      // NOTE: The AT+CSSLCFG convert function for the client cert combines
      // the certificate and the key in a single certificate name
      sendAT(GF("+CASSLCFG="), mux, GF(",\"cert\",\""), clientCertName, '"');
      success &= waitResponse() == 1;
    }

    return success;
  }

  /**
   * @brief Apply SSL pre-shared key (PSK) to the specified connection.
   * @param mux The **zero-indexed** position of the client in the modem's
   * socket array.
   * @param context_id The SSL context ID.
   * @param pskTableName The PSK table name.
   * @return True if the operation was successful, false otherwise.
   */
  bool applySSLPSK(uint8_t mux, uint8_t context_id, const char* pskTableName) {
    // store the configuration in the SSL context object
    GsmSecureContext* sslContext = getSSLContext(context_id);
    if (sslContext == nullptr) { return false; }
    strncpy(sslContext->pskTableName, pskTableName ? pskTableName : "",
            sizeof(sslContext->pskTableName) - 1);
    sslContext->pskTableName[sizeof(sslContext->pskTableName) - 1] = '\0';

    return applySSLPSK(mux, context_id);
  }

  /**
   * @brief Apply SSL pre-shared key (PSK) to the specified connection.
   * @param mux The **zero-indexed** position of the client in the modem's
   * socket array.
   * @param context_id The SSL context ID.
   * @return True if the operation was successful, false otherwise.
   */
  bool applySSLPSK(uint8_t mux, uint8_t context_id) {
    bool success = true;

    // Retrieve the SSL context object for the given context ID.
    GsmSecureContext* sslContext = getSSLContext(context_id);
    const char* pskTableName = sslContext ? sslContext->pskTableName : nullptr;

    DBG("### SSL context index:", context_id);
    DBG("### PSK table name:", pskTableName);

    // SRGD WARNING: UNTESTED!!
    if (pskTableName != nullptr &&
        strnlen(pskTableName, TINY_GSM_CERT_NAME_LENGTH) != 0) {
      // Re-convert the psk, just in case
      convertPSKTable(pskTableName);

      // AT+CASSLCFG=<cid>,"PSKTABLE",<pskTableName>
      // <cid> Application connection ID (set with AT+CACID above)
      // <pskTableName> Alphanumeric ASCII text string up to 64 characters.
      // PSK table name that has been configured by AT+CSSLCFG. File content
      // format is <identity>:<hex string>.
      sendAT(GF("+CASSLCFG="), mux, GF(",\"psktable\",\""), pskTableName, '"');
      success &= waitResponse() == 1;
    }

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
    // set the connection identifier that the above SSL context settings apply
    // to (ie, tie connection mux to SSL context)
    // AT+CASSLCFG=<cid>,"CRINDEX",<crindex>
    // <cid> Application connection ID (set with AT+CACID above)
    // <crindex> SSL context identifier (<ctxindex>)
    sendAT(GF("+CASSLCFG="), mux, ',', GF("\"crindex\","), context_id);
    return waitResponse() == 1;
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

    // set the connection (mux) identifier to use
    sendAT(GF("+CACID="), mux);
    if (waitResponse(timeout_ms) != 1) return false;

    // enable or disable ssl
    // AT+CASSLCFG=<cid>,"SSL",<sslFlag>
    // <cid> Application connection ID (set with AT+CACID above)
    // <sslFlag> 0: Not support SSL
    //           1: Support SSL
    sendAT(GF("+CASSLCFG="), mux, ',', GF("\"ssl\","), ssl);
    bool set_ssl = waitResponse(timeout_ms) == 1;
    if (!set_ssl && ssl) return false;  // allow to fail for non-ssl connections

    // If we have a secure socket, use a static cast to get the context index.
    // This isn't ideal; hopefully the compiler will save us from ourselves. We
    // cannot use a dynamic cast because Arduino compiles with -fno-rtti.
    if (ssl) {
      const GsmClientSecureSim7000SSL* thisClient =
          static_cast<const GsmClientSecureSim7000SSL*>(sockets[mux]);
      uint8_t sslCtxIndex = thisClient->sslCtxIndex;

      // NOTE: We cannot link the SSL context or set the certificates until
      // AFTER setting the connection id (ie, AT+CACID=mux)
      bool ssl_configured =
          linkSSLContext(mux,
                         sslCtxIndex);  // Must be before applying certs
      if (!ssl_configured) { return false; }
      if (getSSLContext(sslCtxIndex)->sslAuthMode ==
          SSLAuthMode::PRE_SHARED_KEYS) {
        ssl_configured &= applySSLPSK(mux, sslCtxIndex);
      } else {
        ssl_configured &= applySSLCertificates(mux, sslCtxIndex);
      }
      if (!ssl_configured) { return false; }
    }

    DBG("### host:", host);
    DBG("### port:", port);

    // actually open the connection
    // AT+CAOPEN=<cid>[,<conn_type>],<server>,<port>
    // <cid> TCP/UDP identifier
    // <conn_type> "TCP" or "UDP"
    // NOTE:  the "TCP" can't be included
    sendAT(GF("+CAOPEN="), mux, GF(",\""), host, GF("\","), port);
    if (waitResponse(timeout_ms, GF("+CAOPEN:")) != 1) { return false; }
    // returns OK/r/n/r/n+CAOPEN: <cid>,<result>
    // <result> 0: Success
    //          1: Socket error
    //          2: No memory
    //          3: Connection limit
    //          4: Parameter invalid
    //          6: Invalid IP address
    //          7: Not support the function
    //          12: Can’t bind the port
    //          13: Can’t listen the port
    //          20: Can’t resolve the host
    //          21: Network not active
    //          23: Remote refuse
    //          24: Certificate’s time expired
    //          25: Certificate’s common name does not match
    //          26: Certificate’s common name does not match and time expired
    //          27: Connect failed
    int16_t ret_mux = streamGetIntBefore(',');  // mux

    // make sure the connection really opened
    int8_t res = streamGetIntBefore('\n');
    waitResponse();

    if (!isExpectedMux(ret_mux, mux) || 0 != res) {
      DBG(GF("### Connection failed for mux"), mux);
      DBG(GF("Instead got"), ret_mux, GF("with result"), res);
    }

    return isExpectedMux(ret_mux, mux) && 0 == res;
  }

  bool modemStop(uint8_t mux, uint32_t maxWaitMs) {
    if (!isValidMux(mux)) { return false; }
    // Same command for both secure and non-secure sockets
    sendAT(GF("+CACLOSE="), mux);
    return waitResponse(TinyGsmMin(maxWaitMs, static_cast<uint32_t>(3000))) ==
        1;  // should return within 3s
  }

  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    // send data on prompt
    sendAT(GF("+CASEND="), mux, ',', (uint16_t)len);
    return waitResponse(GF(">")) == 1;
  }
  // Between the modemBeginSend and modemEndSend, modemSend calls:
  // stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  // stream.flush();
  size_t modemEndSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    // after posting data, module responds with:
    //+CASEND: <cid>,<result>,<sendlen>
    if (waitResponse(GF("+CASEND:")) != 1) { return 0; }
    int16_t  ret_mux = streamGetIntBefore(',');       // check mux
    bool     result  = streamGetIntBefore(',') == 0;  // check result
    uint16_t sent    = streamGetIntBefore('\n');      // check send length
    if (sent != len) { DBG("### Sent:", sent, "of", len, "on", mux); }
    if (isExpectedMux(ret_mux, mux) && result) { return sent; }
    return 0;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }

    sendAT(GF("+CARECV="), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CARECV:")) != 1) { return 0; }

    // int16_t ret_mux = streamGetIntBefore(',');
    // const int16_t len_reported = streamGetIntBefore('\n');

    // if (!isExpectedMux(ret_mux, mux)) {
    //   DBG("### Data from wrong mux! Got", ret_mux, "expected", mux);
    //   waitResponse();
    //   sockets[mux]->sock_available = modemGetAvailable(mux);
    //   return 0;
    // }

    // NOTE:  manual says the mux number is returned before the number of
    // characters available, but in tests only the number is returned

    int16_t len_reported = streamGetIntBefore(',');
    size_t  len_read     = moveCharsFromStreamToFifo(mux, len_reported);
    waitResponse();  // final ok

    // make sure the sock available number is accurate again
    // the module is **EXTREMELY** testy about being asked to read more from
    // the buffer than exists; it will freeze until a hard reset or power cycle!
    sockets[mux]->sock_available = modemGetAvailable(mux);
    return len_read;
  }

  size_t modemGetAvailable(uint8_t mux) {
    // NOTE: This function gets the number of available characters for all
    // sockets at once, so we don't check if the specific mux is valid until
    // right before returning the value for that mux.
    // NOTE: This gets how many characters are available on all connections that
    // have data.  It does not return all the connections, just those with data.
    sendAT(GF("+CARECV?"));
    for (int16_t muxNo = 0; muxNo < TcpConfig::kMuxCount; muxNo++) {
      // after the last connection, there's an ok, so we catch it right away
      int res = waitResponse(3000, GF("+CARECV:"), GFP(ModemConfig::GSM_OK),
                             GFP(ModemConfig::GSM_ERROR));
      // if we get the +CARECV: response, read the mux number and the number of
      // characters available
      if (res == 1) {
        int16_t ret_mux = streamGetIntBefore(',');
        size_t  result  = streamGetIntBefore('\n');
        if (isValidMux(ret_mux)) {
          // if the mux is valid, set the number of available characters for
          // that socket
          sockets[ret_mux]->sock_available = result;
        }
        // if the first returned mux isn't 0 (or is higher than expected)
        // we need to fill in the missing muxes
        if (ret_mux > muxNo) {
          for (int16_t extra_mux = muxNo; extra_mux <
               TinyGsmMin(ret_mux, static_cast<int16_t>(TcpConfig::kMuxCount));
               extra_mux++) {
            GsmClientSim7000SSL* isock = sockets[extra_mux];
            if (isock) { isock->sock_available = 0; }
          }
          muxNo = ret_mux;
        }
      } else if (res == 2) {
        // if we get an OK, we've reached the last socket with available data
        // so we set any we haven't gotten to yet to 0
        for (int16_t extra_mux = muxNo; extra_mux < TcpConfig::kMuxCount;
             extra_mux++) {
          GsmClientSim7000SSL* isock = sockets[extra_mux];
          if (isock) { isock->sock_available = 0; }
        }
        break;
      } else {
        // if we got an error, give up
        break;
      }
      // Should be a final OK at the end.
      // If every connection was returned, catch the OK here.
      // If only a portion were returned, catch it above.
      if (muxNo == TcpConfig::kMuxCount - 1) { waitResponse(); }
    }
    // check the state of all connections
    // like this function, modemGetConnected doesn't check if the specific mux
    // is valid until right before returning the value for that mux, so calling
    // this will check and update the connection status for all valid muxes even
    // if the supplied mux wasn't a valid one.
    modemGetConnected(mux);
    if (!isValidMux(mux)) { return 0; }
    if (!sockets[mux]) { return 0; }
    return sockets[mux]->sock_available;
  }

  bool modemGetConnected(uint8_t mux) {
    // NOTE: This function gets status for all sockets at once, so we don't
    // check if the specific mux is valid until right before returning the value
    // for that mux.
    // NOTE:  This gets the state of all connections that have been opened since
    // the last connection
    sendAT(GF("+CASTATE?"));

    for (int16_t muxNo = 0; muxNo < TcpConfig::kMuxCount; muxNo++) {
      // after the last connection, there's an ok, so we catch it right away
      int res = waitResponse(3000, GF("+CASTATE:"), GFP(ModemConfig::GSM_OK),
                             GFP(ModemConfig::GSM_ERROR));
      // if we get the +CASTATE: response, read the mux number and the status
      if (res == 1) {
        int16_t ret_mux = streamGetIntBefore(',');
        size_t  status  = streamGetIntBefore('\n');
        // 0: Closed by remote server or internal error
        // 1: Connected to remote server
        // 2: Listening (server mode)
        if (isValidMux(ret_mux)) {
          // if the mux is valid, set the number of available characters for
          // that socket
          sockets[ret_mux]->sock_connected = (status == 1);
        }
        // if the first returned mux isn't 0 (or is higher than expected)
        // we need to fill in the missing muxes
        if (ret_mux > muxNo) {
          for (int16_t extra_mux = muxNo; extra_mux <
               TinyGsmMin(ret_mux, static_cast<int16_t>(TcpConfig::kMuxCount));
               extra_mux++) {
            GsmClientSim7000SSL* isock = sockets[extra_mux];
            if (isock) { isock->sock_connected = false; }
          }
          muxNo = ret_mux;
        }
      } else if (res == 2) {
        // if we get an OK, we've reached the last socket with available data
        // so we set any we haven't gotten to yet to 0
        for (int16_t extra_mux = muxNo; extra_mux < TcpConfig::kMuxCount;
             extra_mux++) {
          GsmClientSim7000SSL* isock = sockets[extra_mux];
          if (isock) { isock->sock_connected = false; }
        }
        break;
      } else {
        // if we got an error, give up
        break;
      }
      // Should be a final OK at the end.
      // If every connection was returned, catch the OK here.
      // If only a portion were returned, catch it above.
      if (muxNo == TcpConfig::kMuxCount - 1) { waitResponse(); }
    }
    if (!isValidMux(mux)) { return false; }
    return sockets[mux]->sock_connected;
  }

  /*
   * Utilities
   */
 protected:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+CARECV:"))) {
      int16_t mux = streamGetIntBefore(',');
      int16_t len = streamGetIntBefore('\n');
      if (isValidMux(mux)) {
        sockets[mux]->got_data = true;
        if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
      }
      data = "";
      DBG("### Got Data:", len, "on", mux);
      return true;
    } else if (data.endsWith(GF("+CADATAIND:"))) {
      int16_t mux = streamGetIntBefore('\n');
      if (isValidMux(mux)) { sockets[mux]->got_data = true; }
      data = "";
      DBG("### Got Data:", mux);
      return true;
    } else if (data.endsWith(GF("+CASTATE:"))) {
      int16_t mux   = streamGetIntBefore(',');
      int16_t state = streamGetIntBefore('\n');
      if (isValidMux(mux)) {
        if (state != 1) {
          sockets[mux]->sock_connected = false;
          DBG("### Closed: ", mux);
        }
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
      data = "";
      return true;
    }
    return false;
  }

 protected:
  GsmClientSim7000SSL* sockets[TcpConfig::kMuxCount];
};

// cspell:words CASEND configureSSLSNI

#endif  // SRC_TINYGSMCLIENTSIM7000SSL_H_
