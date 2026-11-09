/**
 * @file       TinyGsmClientSim7080.h
 * @brief      SIM7070/SIM7080/SIM7090 modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup simcom_sim7080 SIMCom SIM7070, SIM7080, SIM7090
 * @brief Manufacturer: SIMCom. Models: SIM7070, SIM7080, SIM7090.
 * @ingroup simcom_sim70xx
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
 * @todo In `configureSSLContext()`: Skip verifying the context number?
 */

#ifndef SRC_TINYGSMCLIENTSIM7080_H_
#define SRC_TINYGSMCLIENTSIM7080_H_
#pragma message("TinyGSM:  TinyGsmClientSim7080")


#include "TinyGsmClientSIM70xx.h"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"

/// Basic modem configurations for the SIMCom SIM7080 modem family
/// @ingroup simcom_sim7080
struct TinyGsmSim7080ModemConfig
    : public TinyGsmModemConfigPreset<Sim70xxRegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "SIMCom";
#if defined(TINY_GSM_MODEM_SIM7070)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM7070";
#elif defined(TINY_GSM_MODEM_SIM7090)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM7090";
#else
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM7080";
#endif
};

constexpr char TinyGsmSim7080ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmSim7080ModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for the SIMCom SIM7080 modem family.
 *
 * Up to 1460 bytes can be sent at a time with CASEND.
 * NOTE: The manual says 1460, but the actual value seems to be variable.  I
 * have modules P/N S2-108HB-Z3037 that never report more than 1360 available
 * and P/N S2-108HB-Z30GJ that top out at 1318.  I'm setting the default to
 * 1360, but you can change it in the TinyGsmSim7080TcpConfig struct if you need
 * to.
 *
 * In my testing, if the check for available space in the send buffer reports
 * anything less than full space available, the modem is on the edge of crashing
 * and you need to back off until it's fully cleared. Refilling a partially
 * emptied buffer doesn't go well.
 * @ingroup simcom_sim7080
 */
struct TinyGsmSim7080TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 12,
          /*sendMaxSize*/ 1360,
          /*connectTimeoutS*/ 75,  // default
          /*stopTimeoutS*/ 15,
          /*sendTimeoutS*/ 15,
          /*retransmissionTimeoutS*/ 75,
          /*minFreeTxBuffer*/ 1318> {};

/**
 * @brief SSL behavior and limits for the SIMCom SIM7080 modem family.
 * The module supports 6 SSL contexts (0-5).
 * @ingroup simcom_sim7080
 */
struct TinyGsmSim7080SslConfig : public TinyGsmSslConfigPreset<
                                     /*contextCount*/ 6> {};

/// Class for the SIMCOM SIM7070, SIM7080, and SIM7090
/// @ingroup simcom_sim7080
class TinyGsmSim7080
    : public TinyGsmSim70xx<TinyGsmSim7080, TinyGsmSim7080ModemConfig>,
      public TinyGsmTCP<TinyGsmSim7080, TinyGsmSim7080TcpConfig>,
      public TinyGsmSSL<TinyGsmSim7080, TinyGsmSim7080SslConfig>,
      public TinyGsmSMS<TinyGsmSim7080>,
      public TinyGsmGSMLocation<TinyGsmSim7080>,
      public TinyGsmTime<TinyGsmSim7080>,
      public TinyGsmNTP<TinyGsmSim7080>,
      public TinyGsmBattery<TinyGsmSim7080> {
  friend class TinyGsmSim70xx<TinyGsmSim7080, TinyGsmSim7080ModemConfig>;
  friend class TinyGsmModem<TinyGsmSim7080, TinyGsmSim7080ModemConfig>;
  friend class TinyGsmCellularData<TinyGsmSim7080>;
  friend class TinyGsmTCP<TinyGsmSim7080, TinyGsmSim7080TcpConfig>;
  friend class GsmClient<TinyGsmSim7080, TinyGsmSim7080TcpConfig>;
  friend class TinyGsmSSL<TinyGsmSim7080, TinyGsmSim7080SslConfig>;
  friend class TinyGsmSMS<TinyGsmSim7080>;
  friend class TinyGsmGSMLocation<TinyGsmSim7080>;
  friend class TinyGsmGPS<TinyGsmSim7080>;
  friend class TinyGsmTime<TinyGsmSim7080>;
  friend class TinyGsmNTP<TinyGsmSim7080>;
  friend class TinyGsmBattery<TinyGsmSim7080>;

 public:
  using ModemConfig = TinyGsmSim7080ModemConfig;
  using TcpConfig   = TinyGsmSim7080TcpConfig;
  using SslConfig   = TinyGsmSim7080SslConfig;
  using GsmSecureContext =
      TinyGsmSSL<TinyGsmSim7080, TinyGsmSim7080SslConfig>::gsmSecureContext;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup simcom_sim7080
  class GsmClientSim7080
      : public GsmClient<TinyGsmSim7080, TinyGsmSim7080TcpConfig> {
    friend class TinyGsmSim7080;

   public:
    using GsmClient<TinyGsmSim7080, TinyGsmSim7080TcpConfig>::init;
    using GsmClient<TinyGsmSim7080, TinyGsmSim7080TcpConfig>::connect;
    using GsmClient<TinyGsmSim7080, TinyGsmSim7080TcpConfig>::stop;
    using TcpConfig = TinyGsmSim7080TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSim7080() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is identical to the identifier
     * the modem uses internally to identify the socket for the SIM7080.
     *
     * @note The SIM7080 and similar variants allow you choose the multiplexing
     * channel number, but if the input mux channel number is already in use and
     * other mux channels are available, this library will select the next
     * available one.  Use the getMux() function to get the assigned
     * multiplexing channel number after a successful connection.
     */
    explicit GsmClientSim7080(TinyGsmSim7080& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmSim7080, TinyGsmSim7080TcpConfig>(modem, mux) {
      is_secure = false;
      init(&modem, mux);
    }

    /**
     * @brief Initialize this client with modem context and multiplexing
     * channel.
     *
     * @param modem Pointer to the modem instance.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.
     * @param strictAttach If true, require the requested mux for static mux
     * clients.
     * @return true if initialization completed.
     * @note The SIM7080 uses an init() override to set the real maximum send
     * size for the TCP client.
     */
    bool init(TinyGsmSim7080* modem, uint8_t mux = 0,
              bool strictAttach = false) override {
      if (!this->attachToModem(this, modem, mux, TcpConfig::kMuxCount,
                               strictAttach)) {
        return false;
      }
      realMaxSendSize = TcpConfig::kSendMaxSize;
      return true;
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

   protected:
    size_t realMaxSendSize = TcpConfig::kSendMaxSize;
  };

  /*
   * Inner Secure Client
   */
 public:
  /// Inner secure client
  /// @ingroup simcom_sim7080
  class GsmClientSecureSim7080
      : public GsmSecureClient<GsmClientSim7080, TinyGsmSim7080> {
    friend class TinyGsmSim7080;

   public:
    using GsmClientSim7080::connect;
    using GsmSecureClient<GsmClientSim7080, TinyGsmSim7080>::init;
    using GsmClientSim7080::stop;
    using GsmSecureClient::GsmSecureClient;
    using TcpConfig = TinyGsmSim7080TcpConfig;

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
        DBG("### The SIM7080 requires a PSK table; identity/key pairs are "
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
  using GsmClientSecureSIM7080 = GsmClientSecureSim7080;

  /*
   * Inner Secure Context
   */
 public:
  /// Inner secure context
  /// @ingroup simcom_sim7080
  class GsmSecureContextSim7080
      : public ::GsmSecureContext<TinyGsmSim7080, TinyGsmSim7080SslConfig> {
    friend class TinyGsmSim7080;

   public:
    using ::GsmSecureContext<TinyGsmSim7080,
                             TinyGsmSim7080SslConfig>::GsmSecureContext;

    /// Unregister this context from its modem.
    ~GsmSecureContextSim7080() {}
  };

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmSim7080(Stream& stream)
      : TinyGsmSim70xx<TinyGsmSim7080, TinyGsmSim7080ModemConfig>(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool testATImpl(uint32_t timeout_ms) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      sendAT(GF(""));
      int8_t resp = waitResponse(200L, GFP(ModemConfig::GSM_OK),
                                 GFP(ModemConfig::GSM_ERROR),
                                 GFP(ModemConfig::GSM_AT));
      if (resp == 1) {
        return true;
      } else if (resp == 3) {
        waitResponse(200L);  // get the OK
        DBG(GF("## Turning off echo!"));
        sendAT(GF("E0"));  // Echo Off
        waitResponse(2000L);
      }
      delay(100);
    }
    return false;
  }

  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM7080"));

    bool gotATOK = testAT();
    if (!gotATOK) { return false; }

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
      GsmClientSim7080* sock = sockets[mux];
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
 protected:
  bool restartImpl(const char* pin) {
    bool success = true;

    bool gotATOK = testAT();
    if (!gotATOK) { return false; }

    sendAT(GF("+CREBOOT"));  // Reboot
    success &= waitResponse() == 1;
    waitResponse(30000L, GF("SMS Ready"));
    success &= initImpl(pin);
    return success;
  }

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
    success &= waitResponse(10500L, GF("DOWNLOAD"), GFP(ModemConfig::GSM_OK),
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
    while (!stream.available() && millis() - start < 10000) {}

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
    // NOTE:  despite docs using caps, "convert" must be in lower case
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
    // NOTE:  despite docs using caps, "convert" must be in lower case
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
    // NOTE:  despite docs using caps, "convert" must be in lower case
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

    // Check the APN returned by the server
    // not sure why, but the connection is more consistent with this
    sendAT(GF("+CGNAPN"));
    waitResponse();

    // Bearer settings for applications based on IP
    // Set the user name and password
    // AT+CNCFG=<pdpidx>,<ip_type>,[<APN>,[<usename>,<password>,[<authentication>]]]
    // <pdpidx> PDP Context Identifier, 0-3 - for reasons not understood by me,
    //          use PDP context identifier of 0 for what we defined as 1 above
    // <ip_type> 0: Dual PDN Stack
    //           1: Internet Protocol Version 4
    //           2: Internet Protocol Version 6
    // <authentication> 0: NONE
    //                  1: PAP
    //                  2: CHAP
    //                  3: PAP or CHAP
    if (pwd && strlen(pwd) > 0 && user && strlen(user) > 0) {
      sendAT(GF("+CNCFG=0,1,\""), apn, GF("\",\""), user, GF("\",\""), pwd,
             GF("\",3"));
      waitResponse();
    } else if (user && strlen(user) > 0) {
      // Set the user name only
      sendAT(GF("+CNCFG=0,1,\""), apn, GF("\",\""), user, '"');
      waitResponse();
    } else {
      // Set the APN only
      sendAT(GF("+CNCFG=0,1,\""), apn, '"');
      waitResponse();
    }

    // Activate application network connection
    // AT+CNACT=<pdpidx>,<action>
    // <pdpidx> PDP Context Identifier - for reasons not understood by me,
    //          use PDP context identifier of 0 for what we defined as 1 above
    // <action> 0: Deactive
    //          1: Active
    //          2: Auto Active
    bool res    = false;
    int  ntries = 0;
    while (!res && ntries < 5) {
      sendAT(GF("+CNACT=0,1"));
      res = waitResponse(60000L, GF("+APP PDP: 0,ACTIVE"),
                         GF("+APP PDP: 0,DEACTIVE"),
                         GFP(ModemConfig::GSM_ERROR)) == 1;
      waitResponse();
      ntries++;
    }

    // Set the PDP context for DNS lookup
    sendAT(GF("+CDNSPDPID=0"));
    if (waitResponse() != 1) { return false; }

#if TINY_GSM_USE_PUBLIC_DNS
    // Override network-assigned DNS with Google Public DNS when requested.
    sendAT(GF("+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\""));
    // This optional override is not required for the packet data connection.
    waitResponse();
#endif

    return res;
  }

  bool dataDisconnectImpl() {
    // Shut down the general application TCP/IP connection
    // CNACT will close *all* open application connections
    sendAT(GF("+CNACT=0,0"));
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
 protected:
  bool NTPServerSyncImpl(const char* server, int TimeZone) {
    // Set bearer profile to associate with NTP sync
    // this may fail, it's not supported by all modules
    sendAT(GF("+CNTPCID=0"));  // CID must be 0. With 1 (like other modules)
                               // does not work!
    waitResponse(10000L);

    // Set NTP server and timezone - write command
    // AT+CNTP=<ntpserver>[,<time zone>][,<cid>][,<mode>]
    // <ntpserver> - NTP server’s url
    // <time zone> - Local time zone, the range is (-47 to 48), in fact, time
    // zone range (-12 to 12), but taking into account that some countries and
    // regions will use half time zone, or even fourth time zone, so the entire
    // extended four time zones X, so that when the time zone of the input
    // integers are used, without the need for decimal. Time zone in front of
    // the West if it is a negative number indicates the time zone.
    // <cid> - Bearer profile identifier, refer to <pdpidx> of AT+CNACT
    // <mode> - print UTC time on uart and set to local time
    //        - 0 Just set UTC to localtime
    //        - 1 Just output UTC time to AT port
    //        - 2 Set UTC to localtime and output UTC time to AT port
    sendAT(GF("+CNTP=\""), server, GF("\","), TimeZone, GF(",0,2"));
    if (waitResponse(10000L) != 1) { return false; }

    // TODO: Should we have a "quick fail" here? We don't want to wait for the
    // time sync to finish here, but if we don't wait up to the maximum possible
    // response time, will the "+CNTP:" end up mangled in the responses to a
    // later command?  The waitForTimeSync() function calls the exact command
    // repeatedly waiting for a response within 10s each time.  Are those going
    // to cause trouble?

    // Request network synchronization - execution command
    sendAT(GF("+CNTP"));
    if (waitResponse(10000L, GF("+CNTP:")) == 1) {
      // Return: +CNTP: <code>[,<time>]
      // <code> - Result code of the NTP synchronization
      //    - 1 UTC time synchronization is successful
      //    - 61 Network Error
      //    - 62 DNS resolution error
      //    - 63 Connection Error
      //    - 64 Service response error
      //    - 65 Service Response Timeout
      int code = stream.parseInt();
      // Throw away the rest of the line, since we don't care about it here
      streamSkipUntil('\n');
      return code == 1;
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
    //              6: QAPI_NET_SSL_PROTOCOL_TLS_1_3 (only supported with 2117
    //              firmware baseline)
    // NOTE:  despite docs using caps, "sslversion" must be in lower case
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
      const GsmClientSecureSim7080* thisClient =
          static_cast<const GsmClientSecureSim7080*>(sockets[mux]);
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
    // AT+CAOPEN=<cid>,<pdp_index>,<conn_type>,<server>,<port>[,<recv_mode>]
    // <cid> TCP/UDP identifier
    // <pdp_index> Index of PDP connection; we set up PCP context 1 above, but
    // must use 0 here
    // <conn_type> "TCP" or "UDP"
    // <recv_mode> 0: The received data can only be read manually using
    // AT+CARECV=<cid>
    //             1: After receiving the data, it will automatically report
    //             URC:
    //                +CAURC:
    //                "recv",<id>,<length>,<remoteIP>,<remote_port><CR><LF><data>
    // NOTE:  including the <recv_mode> fails
    sendAT(GF("+CAOPEN="), mux, GF(",0,\"TCP\",\""), host, GF("\","), port);
    if (waitResponse(timeout_ms, GF("+CAOPEN:")) != 1) { return 0; }
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
      return false;
    }

    // Immediately after connecting, before sending any data, we need to check
    // actual the send buffer size, so we can wait for it to be available. The
    // actual size is smaller than the size in the manual and seems to be
    // variable, so we need to account for that when waiting for the buffer to
    // be available.
    sockets[mux]->realMaxSendSize = modemGetSendLength(mux);
    // DBG(GF("### Real max send size for mux"), mux, GF("is"),
    //     sockets[mux]->realMaxSendSize);

    return true;
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
  size_t modemEndSend(size_t len, uint8_t) {
    // Nothing but an OK after posting data
    if (waitResponse() != 1) { return 0; }
    return len;
  }
  size_t modemGetSendLength(uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    // Sending only the mux number will return the number of bytes left in the
    // send buffer (that we can soon fill up with our next send attempt)
    sendAT(GF("+CASEND="), mux);
    if (waitResponse(GF("+CASEND:")) != 1) {
      // If this returns anything other than `+CASEND: <mux>,<leftsize>`, we
      // assume the send buffer is empty and we can send the maximum size.  If
      // the send buffer is empty, the module will definitely return an error.
      // If the wait times out or returns something else, we treat that
      // identically.
      return TcpConfig::kSendMaxSize;
    }
    size_t leftsize = streamGetIntBefore('\n');
    waitResponse();  // final ok
    if (leftsize > TcpConfig::kSendMaxSize) { return TcpConfig::kSendMaxSize; }
    return leftsize;
  }

  size_t modemWaitForSend(uint8_t mux, uint32_t timeout_ms) {
    if (!isValidMux(mux)) { return 0; }
    size_t sendLength = modemGetSendLength(mux);
#if defined(TINY_GSM_DEBUG)
    if (sendLength != sockets[mux]->realMaxSendSize) {
      DBG(GF("### Full send buffer not available! Expected it to have"),
          sockets[mux]->realMaxSendSize, GF("bytes, but it has"), sendLength);
    }
    if (sendLength < sockets[mux]->realMaxSendSize) {
      DBG(GF(
          "### Waiting up to 15s for sufficient available send buffer space"));
    }
#endif
    uint32_t start = millis();
    while (sendLength < sockets[mux]->realMaxSendSize &&
           millis() - start < timeout_ms && sockets[mux]->sock_connected) {
      delay(250);
      sendLength = modemGetSendLength(mux);
#if defined(TINY_GSM_DEBUG)
      if (sendLength >= sockets[mux]->realMaxSendSize) {
        DBG(GF("### Send buffer has"), sendLength, GF("available after"),
            millis() - start, GF("ms"));
      }
#endif
    }
#if defined(TINY_GSM_DEBUG)
    if (sendLength == 0) { DBG(GF("### No available send buffer!")); }
#endif
    return sendLength;
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
            GsmClientSim7080* isock = sockets[extra_mux];
            if (isock) { isock->sock_available = 0; }
          }
          muxNo = ret_mux;
        }
      } else if (res == 2) {
        // if we get an OK, we've reached the last socket with available data
        // so we set any we haven't gotten to yet to 0
        for (int16_t extra_mux = muxNo; extra_mux < TcpConfig::kMuxCount;
             extra_mux++) {
          GsmClientSim7080* isock = sockets[extra_mux];
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
    // NOTE: This function gets the status for all sockets at once, so we don't
    // check if the specific mux is valid until right before returning the value
    // for that mux.
    // NOTE:  This gets the state of all connections that have been opened
    // since the last connection
    sendAT(GF("+CASTATE?"));

    for (int16_t muxNo = 0; muxNo < TcpConfig::kMuxCount; muxNo++) {
      // after the last connection, there's an ok, so we try to catch it right
      // away
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
            GsmClientSim7080* isock = sockets[extra_mux];
            if (isock) { isock->sock_connected = false; }
          }
          muxNo = ret_mux;
        }
      } else if (res == 2) {
        // if we get an OK, we've reached the last socket with available data
        // so we set any we haven't gotten to yet to 0
        for (int16_t extra_mux = muxNo; extra_mux < TcpConfig::kMuxCount;
             extra_mux++) {
          GsmClientSim7080* isock = sockets[extra_mux];
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
  GsmClientSim7080* sockets[TcpConfig::kMuxCount];
};

// cspell:words CASEND gotATOK configureSSLSNI

#endif  // SRC_TINYGSMCLIENTSIM7080_H_
