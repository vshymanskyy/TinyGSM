/**
 * @file       TinyGsmSSL.tpp
 * @brief      TLS/SSL connection helper mixin for secure clients.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMSSL_TPP_
#define SRC_TINYGSMSSL_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_SSL
/// flag to indicate that the modem has Secure Socket Layer (SSL) functions
#define TINY_GSM_MODEM_HAS_SSL
#endif
#ifndef TINY_GSM_MODEM_CAN_SPECIFY_CERTS
/// flag to indicate that the modem can specify certificates for SSL connections
#define TINY_GSM_MODEM_CAN_SPECIFY_CERTS
#endif
#ifndef TINY_GSM_MODEM_CAN_LOAD_CERTS
/// flag to indicate that the modem can load certificates for SSL connections
#define TINY_GSM_MODEM_CAN_LOAD_CERTS
#endif

#ifndef TINY_GSM_CERT_NAME_LENGTH
/**
 * @brief The maximum length of a certificate file name - 32 characters plus a
 * null terminator.
 *
 * This is the maximum length of a certificate file name used to refer to the
 * certificate in the modems flash memory, not the certificate itself.  The file
 * name length limit varies by modem; some list a limit of 64 characters others
 * don't list one.  Increase this value if you want to use longer file names.
 */
#define TINY_GSM_CERT_NAME_LENGTH 33
#endif

#ifndef TINY_GSM_PSK_IDENTITY_LENGTH
/**
 * @brief The maximum length of the pre-shared key (PSK) identity
 * (identifier/hint) - 32 characters plus a null terminator.
 *
 * This is the maximum length of the identity value itself, not a reference or
 * file name. There is not a standard for the maximum length of a PSK identity,
 * but 32 characters is a common limit.  The null terminator is added to the end
 * of the string to indicate the end of the string.  Increase this value if you
 * need a longer hint value
 */
#define TINY_GSM_PSK_IDENTITY_LENGTH 33
#endif

#ifndef TINY_GSM_PSK_LENGTH
/**
 * @brief The maximum length of the pre-shared key (PSK) - 32 characters plus a
 * null terminator.
 *
 * This is the maximum length of the key itself, not a reference or file name.
 * 32 characters is a common limit for the length of a PSK, though 64-255
 * characters can be used in IPsec PSK (Palo Alto).    Increase this value if
 * you need to use a longer key.
 */

#define TINY_GSM_PSK_LENGTH 33
#endif

#ifndef TINY_GSM_DEFAULT_SSL_CTX
/**
 * @brief The default SSL context to use for SSL connections.
 *
 * This applies to **all** modems using the GsmSecureClient class.  The SSL
 * context is a collection of SSL settings, not the connection identifier.  This
 * library uses SSL context 0 by default.  At this time, all modems that support
 * SSL via contexts show using context 0 in their documentation.  If you find a
 * modem that uses a different context, please let us know and we will add it to
 * the list of modems that use a different default context.
 */
#define TINY_GSM_DEFAULT_SSL_CTX 0
#endif

/// Enum for different types of SSL/TLS certificates.
enum class CertificateType : int8_t {
  /// The CA certificate is used to verify the server's certificate. This is the
  /// most common type of certificate used in SSL connections.
  CA_CERTIFICATE = 0,
  /// The client certificate is used to authenticate the client to the server.
  /// This is less common and is typically used in mutual authentication
  /// scenarios.
  CLIENT_CERTIFICATE = 1,
  /// The client key is used in conjunction with the client certificate for
  /// mutual authentication. It is a private key that should be kept secure and
  /// not shared.
  CLIENT_KEY = 2,
  /// The client pre-shared key is used for authentication in TLS-PSK
  /// connections. It is a secret key shared between the client and the server.
  CLIENT_PSK = 3,
  /// The client pre-shared key identity is used in conjunction with the client
  /// PSK for authentication in TLS-PSK connections.
  CLIENT_PSK_IDENTITY = 4,
  /// The UNKNOWN type is used to indicate that the certificate type is not
  /// recognized or is not specified. It is a placeholder for error handling or
  /// default cases.
  UNKNOWN = -1
};

/**
 * @enum SSLAuthMode
 * @brief Enum for different SSL/TLS authentication modes (auth_mode).
 *
 * @var SSLAuthMode::NO_VALIDATION
 * 0: no validation
 *    - SRGD Note: Very insecure! You do not need to load any certificates
 *    onto your device for this. Not all servers will accept it.
 * @var SSLAuthMode::CLIENT_VALIDATION
 * 1: the client provides the client certificate for the server to
 * verify.
 *    - SRGD Note: I do not believe this is commonly used. To use
 *    this, you must load a client certificate and a client key onto
 *    your device.
 * @var SSLAuthMode::CA_VALIDATION
 * 2: the client loads CA certificate to verify the server’s
 * certificate.
 *    - SRGD Note: This is a common authentication type used by
 *    browsers, where the browser verifies the server's certificate.
 *    For this to work, you must load either the server's intermediate or parent
 *    CA certificate onto your device.
 * @var SSLAuthMode::MUTUAL_AUTHENTICATION
 * 3: mutual authentication.
 *    - SRGD Note: This is used by AWS IoT Core and other IoT
 *    services. In this case you must load 3 certs to your device:
 *    The server's CA cert, the client cert, and the client key.
 * @var SSLAuthMode::PRE_SHARED_KEYS
 * 4: pre-shared key encryption
 */
enum class SSLAuthMode : int8_t {
  NO_VALIDATION         = 0,
  CLIENT_VALIDATION     = 1,
  CA_VALIDATION         = 2,
  MUTUAL_AUTHENTICATION = 3,
  PRE_SHARED_KEYS       = 4,
};

/// Enum for different SSL/TLS versions.
enum class SSLVersion : int8_t {
  /// No SSL/TLS, plain TCP connection
  NO_SSL = -1,
  /// SSL 3.0
  SSL3_0 = 0,
  /// TLS 1.0
  TLS1_0 = 1,
  /// TLS 1.1
  TLS1_1 = 2,
  /// TLS 1.2
  TLS1_2 = 3,
  /// Try all SSL/TLS versions, the modem will negotiate the best version
  ALL_SSL = 4,
  /// TLS 1.3
  TLS1_3 = 5
};

/**
 * @brief Template class for modem SSL context config traits.
 *
 * @tparam contextCount The number of hardware SSL contexts supported by the
 * modem.
 *
 * @todo handle:
 *  - the number of SSL sockets supported by the modem
 *  - whether the socket numbering is shared between SSL and TCP sockets or
 * separate
 *  - the SSL capabilities of the modem (e.g., whether it supports PSK, client
 * certs, etc.)
 */
template <uint8_t contextCount>
struct TinyGsmSslConfigPreset {
  /// The number of hardware SSL contexts supported by the modem.
  static constexpr uint8_t kContextCount = contextCount;
};

// Forward declaration so the matching context specialization can be friended.
template <class modemType, class sslConfig>
class GsmSecureContext;

/**
 * @brief The CRTP parent class for Secure Socket Layer (SSL) functions.
 * @tparam modemType The derived modem class.
 * @tparam sslConfig Trait type controlling SSL context limits.
 */
template <class modemType, class sslConfig>
class TinyGsmSSL {
 public:
  /// The SSL trait type used by this modem.
  using SslConfig = sslConfig;
  /// The SSL context type used by this modem.
  using gsmSecureContext = ::GsmSecureContext<modemType, sslConfig>;

 private:
  friend class ::GsmSecureContext<modemType, sslConfig>;

 public:
  /// Compile-time capability flag indicating SSL/TLS support
  static constexpr bool hasSSL = true;
  /// Compile-time capability flag indicating certificate specification support
  static constexpr bool canSpecifyCerts = true;
  /// Compile-time capability flag indicating certificate loading support
  static constexpr bool canLoadCerts = true;

  /* =========================================== */
  /* =========================================== */
 public:
  /*
   * Mixin Constructors
   */

  /**
   * @brief Create the SSL capability mixin.
   */
  TinyGsmSSL() {
    initializeSSLContexts();
  }

  /**
   * @brief Create the SSL capability mixin.
   */
  explicit TinyGsmSSL(modemType& /*modem*/, uint8_t /*mux*/ = 0) {
    initializeSSLContexts();
  }

  /*
   * Define the interface
   */

  /**
   * @anchor ssl_certificate_management_functions
   * @name Secure socket layer (SSL) certificate management functions
   */
  /**@{*/

  /**
   * @brief Load a new certificate onto the module.
   *
   * Load a new certificate onto the module, including the whole text of the
   * certificate. A file will be created (or updated) on the module's filesystem
   * with the content of the certificate.
   *
   * @param certificateName The name of the certificate.
   * @param cert The certificate content.
   * @param len The length of the certificate content.
   * @return True if the certificate was successfully loaded, false otherwise.
   */
  bool loadCertificate(const char* certificateName, const char* cert,
                       const uint16_t len) {
    return thisModem().loadCertificateImpl(certificateName, cert, len);
  }
#if defined(TINY_GSM_HAS_PROGMEM)
  /**
   * @brief Load a new certificate stored in program memory onto the module.
   *
   * @param certificateName The name of the certificate.
   * @param cert The certificate content wrapped in GFP().
   * @param len The length of the certificate content.
   * @return True if the certificate was successfully loaded, false otherwise.
   */
  bool loadCertificate(const char* certificateName, GsmConstStr cert,
                       const uint16_t len) {
    return thisModem().loadCertificateImpl(certificateName, cert, len);
  }
#endif
  /// @copydoc loadCertificate(const char*, const char*, const uint16_t)
  bool loadCertificate(const String& certificateName, const String& cert,
                       const uint16_t len) {
    return loadCertificate(certificateName.c_str(), cert.c_str(), len);
  }

  /**
   * @brief Delete a certificate by name from the module's filesystem.
   *
   * NOTE: The functions for deleting a certificate rarely depend on the
   * certificate type.
   *
   * @param filename The name of the certificate file to delete.
   * @return True if the certificate was successfully deleted, false otherwise.
   */
  bool deleteCertificate(const char* filename) {
    return thisModem().deleteCertificateImpl(filename);
  }
  /// @copydoc deleteCertificate(const char*)
  bool deleteCertificate(const String& filename) {
    return deleteCertificate(filename.c_str());
  }

  // print the contents of a certificate file to a stream
  /**
   * @brief Print the contents of a certificate file to a stream.
   *
   * @note For security reasons, this function may not be available for all
   * modems.
   *
   * @param filename The name of the certificate file.
   * @param print_stream The stream to print the certificate contents to.
   * @return True if the certificate was successfully printed, false otherwise.
   */
  bool printCertificate(const char* filename, Stream& print_stream) {
    return thisModem().printCertificateImpl(filename, print_stream);
  }
  /// @copydoc printCertificate(const char*, Stream&)
  bool printCertificate(const String& filename, Stream& print_stream) {
    return printCertificate(filename.c_str(), print_stream);
  }

  /**
   * @brief Converts a certificate file into a format usable by the modem for
   * SSL connections.
   *
   * For many (but not all!) modules, the certificate must be "converted" from a
   * file stored somewhere in the flash file system into a certificate ready to
   * be used by the connection.
   *
   * @param cert_type The type of the certificate.
   * @param filename The name of the certificate file.
   * @return True if the certificate was successfully converted, false
   * otherwise.
   */
  bool convertCertificate(CertificateType cert_type, const char* filename) {
    return thisModem().convertCertificateImpl(cert_type, filename);
  }
  /// @copydoc convertCertificate(CertificateType, const char*)
  bool convertCertificate(CertificateType cert_type, const String& filename) {
    return convertCertificate(cert_type, filename.c_str());
  }
  /**
   * @brief Convert a certificate authority (CA) certificate.
   *
   * @param ca_cert_name The name of the CA certificate file.
   * @return True if the CA certificate was successfully converted, false
   * otherwise.
   */
  bool convertCACertificate(const char* ca_cert_name) {
    return thisModem().convertCACertificateImpl(ca_cert_name);
  }
  /// @copydoc convertCACertificate(const char*)
  bool convertCACertificate(const String& ca_cert_name) {
    return convertCACertificate(ca_cert_name.c_str());
  }
  /**
   * @brief Convert a client certificate and its corresponding key.
   *
   * In some cases, the two parts of the client certificate (the certificate
   * itself and the key to it) need to be converted together.
   *
   * @param client_cert_name The name of the client certificate file.
   * @param client_cert_key The name of the client certificate key file.
   * @return True if the client certificate and key were successfully converted,
   * false otherwise.
   */
  bool convertClientCertificates(const char* client_cert_name,
                                 const char* client_cert_key) {
    return thisModem().convertClientCertificatesImpl(client_cert_name,
                                                     client_cert_key);
  }
  /// @copydoc convertClientCertificates(const char*, const char*)
  bool convertClientCertificates(const String& client_cert_name,
                                 const String& client_cert_key) {
    return convertClientCertificates(client_cert_name.c_str(),
                                     client_cert_key.c_str());
  }
  /**
   * @brief Convert a pre-shared key (PSK) and its corresponding identity.
   *
   * In some cases, the two parts of the pre-shared key (the PSK itself and the
   * identity assigned to it) need to be converted together.
   *
   * @param psk The pre-shared key.
   * @param pskIdent The identity assigned to the pre-shared key (also called
   * identifier hint or hint).
   * @return True if the PSK and identity were successfully converted, false
   * otherwise.
   */
  bool convertPSKandID(const char* psk, const char* pskIdent) {
    return thisModem().convertPSKandIDImpl(psk, pskIdent);
  }
  /// @copydoc convertPSKandID(const char*, const char*)
  bool convertPSKandID(const String& psk, const String& pskIdent) {
    return convertPSKandID(psk.c_str(), pskIdent.c_str());
  }
  /**
   * @brief Convert a pre-shared key (PSK) table file.
   *
   * In some cases, a PSK table file needs to be converted into a format usable
   * by the modem for SSL connections.
   *
   * @param psk_table_name The name of the PSK table file.
   * @return True if the PSK table was successfully converted, false otherwise.
   */
  bool convertPSKTable(const char* psk_table_name) {
    return thisModem().convertPSKTableImpl(psk_table_name);
  }
  /// @copydoc convertPSKTable(const char*)
  bool convertPSKTable(const String& psk_table_name) {
    return convertPSKTable(psk_table_name.c_str());
  }
  /**@}*/


 protected:
  /**
   * @anchor ssl_context_functions
   * @name SSL context functions
   */
  /**@{*/
  /**
   * @brief Look up a registered SSL context by hardware context index.
   *
   * @param contextIndex The zero-based hardware SSL context index.
   * @return The registered context, or `nullptr` if the index is invalid or no
   * context is registered there.
   */
  gsmSecureContext* getSSLContext(uint8_t contextIndex) {
    if (contextIndex >= SslConfig::kContextCount) { return nullptr; }
    return sslContexts[contextIndex];
  }

  /**
   * @brief Look up a registered SSL context by hardware context index.
   *
   * @param contextIndex The zero-based hardware SSL context index.
   * @return The registered context, or `nullptr` if the index is invalid or no
   * context is registered there.
   */
  const gsmSecureContext* getSSLContext(uint8_t contextIndex) const {
    if (contextIndex >= SslConfig::kContextCount) { return nullptr; }
    return sslContexts[contextIndex];
  }

  /**
   * @brief Check that a context contains the credentials required by its
   * authentication mode.
   * @param context The context to validate.
   * @return `true` when the context and its required credentials are present.
   */
  bool validateSSLContextCredentials(const gsmSecureContext* context) const {
    if (context == nullptr) { return false; }

    switch (context->sslAuthMode) {
      case SSLAuthMode::NO_VALIDATION: {
        return true;  // no credentials required
      }
      case SSLAuthMode::CLIENT_VALIDATION: {
        // both a client certificate and key are required
        return context->clientCertName[0] != '\0' &&
            context->clientKeyName[0] != '\0';
      }
      case SSLAuthMode::CA_VALIDATION: {
        // a CA certificate is required
        return context->CAcertName[0] != '\0';
      }
      case SSLAuthMode::MUTUAL_AUTHENTICATION: {
        // a CA certificate, client certificate, and client key are all required
        return context->CAcertName[0] != '\0' &&
            context->clientCertName[0] != '\0' &&
            context->clientKeyName[0] != '\0';
      }
      case SSLAuthMode::PRE_SHARED_KEYS: {
        // either a table that combines the key and identity is present, or both
        // the individual key and identity (identifier/hint) are provided
        return context->pskTableName[0] != '\0' ||
            (context->pskIdent[0] != '\0' && context->psKey[0] != '\0');
      }
      default: {
        return false;
      }
    }
  }
  /**@}*/

  // destructor (protected!)
  ~TinyGsmSSL() {}

  /*
   * CRTP Helper
   */
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */
 protected:
  /*
   * Secure socket layer (SSL) certificate management functions
   */

  bool loadCertificateImpl(const char* certificateName, const char* cert,
                           const uint16_t len) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool
  deleteCertificateImpl(const char* filename) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool printCertificateImpl(const char* filename,
                            Stream& print_stream) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool convertCertificateImpl(CertificateType cert_type, const char* filename)
      TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool convertCACertificateImpl(const char* ca_cert_name)
      TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool convertClientCertificatesImpl(const char* client_cert_name,
                                     const char* client_cert_key)
      TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool convertPSKandIDImpl(const char* psk,
                           const char* pskIdent) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool
  convertPSKTableImpl(const char* psk_table_name) TINY_GSM_ATTR_NOT_IMPLEMENTED;

 protected:
  /**
   * @brief Register an SSL context in a hardware context slot.
   * @param context The context object to register.
   * @param contextIndex The zero-based hardware SSL context index.
   * @return `true` if the slot is valid and available for this context;
   * otherwise `false`.
   */
  bool attachSSLContext(gsmSecureContext* context, uint8_t contextIndex) {
    if (context == nullptr || contextIndex >= SslConfig::kContextCount ||
        (sslContexts[contextIndex] != nullptr &&
         sslContexts[contextIndex] != context)) {
      return false;
    }
    sslContexts[contextIndex] = context;
    return true;
  }

  /**
   * @brief Remove an SSL context from a hardware context slot.
   * @param context The context object requesting removal.
   * @param contextIndex The zero-based hardware SSL context index.
   */
  void detachSSLContext(gsmSecureContext* context, uint8_t contextIndex) {
    if (contextIndex < SslConfig::kContextCount &&
        sslContexts[contextIndex] == context) {
      sslContexts[contextIndex] = nullptr;
    }
  }

 private:
  /// Initialize the array of SSL context pointers to nullptr.
  void initializeSSLContexts() {
    for (uint8_t i = 0; i < SslConfig::kContextCount; ++i) {
      sslContexts[i] = nullptr;
    }
  }

  /// Registered contexts indexed by hardware SSL context number.
  gsmSecureContext* sslContexts[SslConfig::kContextCount];
};


/**
 * @brief Modem-owned SSL context configuration with application-owned storage.
 *
 * A context registers itself with one hardware SSL context slot in a modem.
 * The application must keep the context alive while any secure client may use
 * its index. The modem must outlive every context and client attached to it.
 * Copying is disabled because each registered object has unique ownership of
 * its registry slot.
 *
 * @tparam modemType The modem class that owns the hardware SSL contexts.
 * @tparam sslConfig Trait type controlling SSL context limits.
 */
template <class modemType, class sslConfig>
class GsmSecureContext {
  friend class TinyGsmSSL<modemType, sslConfig>;
  friend modemType;

 private:
  modemType* modem;
  uint8_t    sslCtxIndex;

 public:
  /**
   * @anchor ssl_context_ctors
   * @name Secure context constructors and initialization
   */
  /**@{*/
  /**
   * @brief Create an unattached SSL context with default settings.
   */
  GsmSecureContext()
      : modem(nullptr),
        sslCtxIndex(TINY_GSM_DEFAULT_SSL_CTX),
        sslAuthMode(SSLAuthMode::NO_VALIDATION),
        sslVersion(SSLVersion::TLS1_2) {
    clearConfigurationStrings();
  }

  /**
   * @brief Create and attach an SSL context to a modem context slot.
   *
   * Use isAttached() to determine whether registration succeeded.
   *
   * @param modem The modem that owns the hardware SSL context.
   * @param contextIndex The zero-based hardware SSL context index.
   */
  GsmSecureContext(modemType& modem,
                   uint8_t    contextIndex = TINY_GSM_DEFAULT_SSL_CTX)
      : modem(nullptr),
        sslCtxIndex(TINY_GSM_DEFAULT_SSL_CTX),
        sslAuthMode(SSLAuthMode::NO_VALIDATION),
        sslVersion(SSLVersion::TLS1_2) {
    clearConfigurationStrings();
    init(&modem, contextIndex);
  }

  /**
   * @brief Create and attach an SSL context using an integer context index.
   * @param modem The modem that owns the hardware SSL context.
   * @param contextIndex The zero-based hardware SSL context index.
   */
  GsmSecureContext(modemType& modem, int contextIndex)
      : GsmSecureContext(modem, static_cast<uint8_t>(contextIndex)) {}

  /**
   * @brief Create an SSL context with certificate authentication settings.
   * @param modem The modem that owns the SSL contexts
   * @param sslAuthMode The SSL authentication mode
   * @param sslVersion The SSL/TLS version (default: TLS1_2)
   * @param CAcertName The name of the CA certificate file
   * @param clientCertName The name of the client certificate file
   * @param clientKeyName The name of the client key file
   */
  explicit GsmSecureContext(modemType& modem, SSLAuthMode sslAuthMode,
                            SSLVersion  sslVersion     = SSLVersion::TLS1_2,
                            const char* CAcertName     = nullptr,
                            const char* clientCertName = nullptr,
                            const char* clientKeyName  = nullptr)
      : modem(nullptr),
        sslCtxIndex(TINY_GSM_DEFAULT_SSL_CTX),
        sslAuthMode(SSLAuthMode::NO_VALIDATION),
        sslVersion(SSLVersion::TLS1_2) {
    clearConfigurationStrings();
    init(&modem, TINY_GSM_DEFAULT_SSL_CTX);
    this->setSSLAuthMode(sslAuthMode);
    this->setSSLVersion(sslVersion);
    this->setCACertName(CAcertName);
    this->setClientCredentials(clientCertName, clientKeyName);
  }

  /**
   * @brief Create an SSL context with certificate authentication and explicit
   * context index.
   * @param modem The modem that owns the SSL contexts
   * @param contextIndex The SSL context index
   * @param sslAuthMode The SSL authentication mode
   * @param sslVersion The SSL/TLS version (default: TLS1_2)
   * @param CAcertName The name of the CA certificate file
   * @param clientCertName The name of the client certificate file
   * @param clientKeyName The name of the client key file
   */
  explicit GsmSecureContext(modemType& modem, uint8_t contextIndex,
                            SSLAuthMode sslAuthMode,
                            SSLVersion  sslVersion     = SSLVersion::TLS1_2,
                            const char* CAcertName     = nullptr,
                            const char* clientCertName = nullptr,
                            const char* clientKeyName  = nullptr)
      : modem(nullptr),
        sslCtxIndex(TINY_GSM_DEFAULT_SSL_CTX),
        sslAuthMode(SSLAuthMode::NO_VALIDATION),
        sslVersion(SSLVersion::TLS1_2) {
    clearConfigurationStrings();
    init(&modem, contextIndex);
    this->setSSLAuthMode(sslAuthMode);
    this->setSSLVersion(sslVersion);
    this->setCACertName(CAcertName);
    this->setClientCredentials(clientCertName, clientKeyName);
  }

  /**
   * @brief Create an SSL context with pre-shared key (PSK) settings.
   * @param modem The modem that owns the SSL contexts
   * @param pskIdent The PSK identity/hint
   * @param psKey The pre-shared key
   * @param sslVersion The SSL/TLS version (default: TLS1_2)
   */
  explicit GsmSecureContext(modemType& modem, const char* pskIdent,
                            const char* psKey,
                            SSLVersion  sslVersion = SSLVersion::TLS1_2)
      : modem(nullptr),
        sslCtxIndex(TINY_GSM_DEFAULT_SSL_CTX),
        sslAuthMode(SSLAuthMode::NO_VALIDATION),
        sslVersion(SSLVersion::TLS1_2) {
    clearConfigurationStrings();
    init(&modem, TINY_GSM_DEFAULT_SSL_CTX);
    this->setSSLAuthMode(SSLAuthMode::PRE_SHARED_KEYS);
    this->setSSLVersion(sslVersion);
    this->setPreSharedKey(pskIdent, psKey);
  }

  /**
   * @brief Create an SSL context with PSK and explicit context index.
   * @param modem The modem that owns the SSL contexts
   * @param contextIndex The SSL context index
   * @param pskIdent The PSK identity/hint
   * @param psKey The pre-shared key
   * @param sslVersion The SSL/TLS version (default: TLS1_2)
   */
  explicit GsmSecureContext(modemType& modem, uint8_t contextIndex,
                            const char* pskIdent, const char* psKey,
                            SSLVersion sslVersion = SSLVersion::TLS1_2)
      : modem(nullptr),
        sslCtxIndex(TINY_GSM_DEFAULT_SSL_CTX),
        sslAuthMode(SSLAuthMode::NO_VALIDATION),
        sslVersion(SSLVersion::TLS1_2) {
    clearConfigurationStrings();
    init(&modem, contextIndex);
    this->setSSLAuthMode(SSLAuthMode::PRE_SHARED_KEYS);
    this->setSSLVersion(sslVersion);
    this->setPreSharedKey(pskIdent, psKey);
  }

  /**
   * @brief Create an SSL context with PSK table name.
   * @param modem The modem that owns the SSL contexts
   * @param pskTableName The name of the PSK table file
   * @param sslVersion The SSL/TLS version (default: TLS1_2)
   */
  explicit GsmSecureContext(modemType& modem, const char* pskTableName,
                            SSLVersion sslVersion = SSLVersion::TLS1_2)
      : modem(nullptr),
        sslCtxIndex(TINY_GSM_DEFAULT_SSL_CTX),
        sslAuthMode(SSLAuthMode::NO_VALIDATION),
        sslVersion(SSLVersion::TLS1_2) {
    clearConfigurationStrings();
    init(&modem, TINY_GSM_DEFAULT_SSL_CTX);
    this->setSSLAuthMode(SSLAuthMode::PRE_SHARED_KEYS);
    this->setSSLVersion(sslVersion);
    this->setPSKTableName(pskTableName);
  }

  /**
   * @brief Create an SSL context with PSK table and explicit context index.
   * @param modem The modem that owns the SSL contexts
   * @param contextIndex The SSL context index
   * @param pskTableName The name of the PSK table file
   * @param sslVersion The SSL/TLS version (default: TLS1_2)
   */
  explicit GsmSecureContext(modemType& modem, uint8_t contextIndex,
                            const char* pskTableName,
                            SSLVersion  sslVersion = SSLVersion::TLS1_2)
      : modem(nullptr),
        sslCtxIndex(TINY_GSM_DEFAULT_SSL_CTX),
        sslAuthMode(SSLAuthMode::NO_VALIDATION),
        sslVersion(SSLVersion::TLS1_2) {
    clearConfigurationStrings();
    init(&modem, contextIndex);
    this->setSSLAuthMode(SSLAuthMode::PRE_SHARED_KEYS);
    this->setSSLVersion(sslVersion);
    this->setPSKTableName(pskTableName);
  }

  /**
   * @brief Create an SSL context with a PSK table and integer context index.
   * @param modem The modem that owns the SSL contexts
   * @param contextIndex The SSL context index
   * @param pskTableName The name of the PSK table file
   * @param sslVersion The SSL/TLS version (default: TLS1_2)
   */
  explicit GsmSecureContext(modemType& modem, int contextIndex,
                            const char* pskTableName,
                            SSLVersion  sslVersion = SSLVersion::TLS1_2)
      : GsmSecureContext(modem, static_cast<uint8_t>(contextIndex),
                         pskTableName, sslVersion) {}

  /**
   * @brief Unregister this context from its modem.
   */
  ~GsmSecureContext() {
    detach();
  }

  /**
   * @brief Attach or reattach this object to a modem SSL context slot.
   *
   * Reattaching to the same modem and slot succeeds. An invalid index, null
   * modem, or slot registered to another context is rejected without changing
   * the current attachment. When changing slots, the old slot is detached only
   * after the requested slot has been validated.
   *
   * @param modem The modem that owns the hardware SSL context.
   * @param contextIndex The zero-based hardware SSL context index.
   * @return `true` if this object is registered in the requested slot;
   * otherwise `false`.
   */
  bool init(modemType* modem, uint8_t contextIndex = TINY_GSM_DEFAULT_SSL_CTX) {
    if (modem == nullptr || contextIndex >= sslConfig::kContextCount) {
      return false;
    }

    GsmSecureContext* registered = modem->getSSLContext(contextIndex);
    if (registered != nullptr && registered != this) { return false; }
    if (this->modem == modem && this->sslCtxIndex == contextIndex &&
        registered == this) {
      return true;
    }

    detach();
    if (!modem->attachSSLContext(this, contextIndex)) { return false; }
    this->modem       = modem;
    this->sslCtxIndex = contextIndex;
    return true;
  }
  /**@}*/

  /**
   * @brief Check whether this context is registered with a modem.
   * @return `true` when attached; otherwise `false`.
   */
  bool isAttached() const {
    return modem != nullptr;
  }

  /**
   * @brief Detach this context from its modem SSL context slot.
   *
   * This releases only the TinyGSM context registration. It does not erase
   * configuration already applied to the modem.
   */
  void detach() {
    if (modem != nullptr) {
      modem->detachSSLContext(this, sslCtxIndex);
      modem       = nullptr;
      sslCtxIndex = TINY_GSM_DEFAULT_SSL_CTX;
    }
  }

  /**
   * @brief Get the hardware SSL context index.
   * @return The registered index, or #TINY_GSM_DEFAULT_SSL_CTX when unattached.
   */
  uint8_t getSSLContextIndex() const {
    return sslCtxIndex;
  }


  /**
   * @anchor context_ssl_config
   * @name Secure context configuration functions
   */
  /**@{*/
  /**
   * @brief Set the SSL authentication mode.
   * @param mode The SSL authentication mode.
   */
  void setSSLAuthMode(SSLAuthMode mode) {
    sslAuthMode = mode;
  }

  /**
   * @brief Set the SSL/TLS protocol version.
   * @param version The SSL/TLS protocol version.
   */
  void setSSLVersion(SSLVersion version) {
    sslVersion = version;
  }
  /**@}*/

  /**
   * @anchor context_cert_assignment
   * @name Secure context certificate assignment functions
   */
  /**@{*/

  /**
   * @brief Set or clear the CA certificate file name.
   * @param CAcertName The CA certificate file name, or `nullptr` to clear it.
   */
  void setCACertName(const char* CAcertName) {
    memset(this->CAcertName, '\0', sizeof(this->CAcertName));
    copyString(this->CAcertName, CAcertName, sizeof(this->CAcertName));
  }
  /// @copydoc setCACertName(const char*)
  void setCACertName(const String& CAcertName) {
    setCACertName(CAcertName.c_str());
  }

  /**
   * @brief Set or clear the client certificate and private key.
   *
   * Both names are stored only when both are non-null, non-empty, and fit in
   * the configured buffers. Otherwise both stored names are cleared.
   *
   * @param clientCertName The client certificate file name.
   * @param clientKeyName The matching private-key file name.
   */
  void setClientCredentials(const char* clientCertName,
                            const char* clientKeyName) {
    memset(this->clientCertName, '\0', sizeof(this->clientCertName));
    memset(this->clientKeyName, '\0', sizeof(this->clientKeyName));
    if (hasText(clientCertName, TINY_GSM_CERT_NAME_LENGTH) &&
        hasText(clientKeyName, TINY_GSM_CERT_NAME_LENGTH)) {
      const bool certCopied = copyString(this->clientCertName, clientCertName,
                                         sizeof(this->clientCertName));
      const bool keyCopied  = copyString(this->clientKeyName, clientKeyName,
                                         sizeof(this->clientKeyName));
      if (!certCopied || !keyCopied) {
        DBG(GF("### Rejected oversized client credential name(s):"),
            clientCertName, clientKeyName);
        memset(this->clientCertName, '\0', sizeof(this->clientCertName));
        memset(this->clientKeyName, '\0', sizeof(this->clientKeyName));
      }
    }
  }
  /// @copydoc setClientCredentials(const char*, const char*)
  void setClientCredentials(const String& clientCertName,
                            const String& clientKeyName) {
    setClientCredentials(clientCertName.c_str(), clientKeyName.c_str());
  }
  /**@}*/


  /**
   * @anchor context_psk_assignment
   * @name Secure context PSK assignment functions
   */
  /**@{*/
  /**
   * @brief Set or clear the PSK table file name.
   * @param pskTableName The PSK table file name, or `nullptr` to clear it.
   */
  void setPSKTableName(const char* pskTableName) {
    memset(this->pskTableName, '\0', sizeof(this->pskTableName));
    copyString(this->pskTableName, pskTableName, sizeof(this->pskTableName));
  }
  /// @copydoc setPSKTableName(const char*)
  void setPSKTableName(const String& pskTableName) {
    setPSKTableName(pskTableName.c_str());
  }

  /**
   * @brief Set or clear a pre-shared key identity and key.
   *
   * Both values are stored only when both are non-null, non-empty, and fit in
   * the configured buffers. Otherwise both stored values are cleared.
   *
   * @param pskIdent The pre-shared key identity.
   * @param psKey The pre-shared key.
   */
  void setPreSharedKey(const char* pskIdent, const char* psKey) {
    memset(this->pskIdent, '\0', sizeof(this->pskIdent));
    memset(this->psKey, '\0', sizeof(this->psKey));
    if (hasText(pskIdent, TINY_GSM_PSK_IDENTITY_LENGTH) &&
        hasText(psKey, TINY_GSM_PSK_LENGTH)) {
      const bool identityCopied = copyString(this->pskIdent, pskIdent,
                                             sizeof(this->pskIdent));
      const bool keyCopied      = copyString(this->psKey, psKey,
                                             sizeof(this->psKey));
      if (!identityCopied || !keyCopied) {
        DBG(GF("### Rejected oversized pre-shared key value(s) for identity:"),
            pskIdent);
        memset(this->pskIdent, '\0', sizeof(this->pskIdent));
        memset(this->psKey, '\0', sizeof(this->psKey));
      }
    }
  }
  /// @copydoc setPreSharedKey(const char*, const char*)
  void setPreSharedKey(const String& pskIdent, const String& psKey) {
    setPreSharedKey(pskIdent.c_str(), psKey.c_str());
  }
  /**@}*/

 protected:
  /// The SSL authentication mode.
  SSLAuthMode sslAuthMode;
  /// The SSL/TLS protocol version.
  SSLVersion sslVersion;
  /// The CA certificate file name.
  char CAcertName[TINY_GSM_CERT_NAME_LENGTH];
  /// The client certificate file name.
  char clientCertName[TINY_GSM_CERT_NAME_LENGTH];
  /// The client private-key file name.
  char clientKeyName[TINY_GSM_CERT_NAME_LENGTH];
  /// The PSK table file name.
  char pskTableName[TINY_GSM_CERT_NAME_LENGTH];
  /// The pre-shared key identity.
  char pskIdent[TINY_GSM_PSK_IDENTITY_LENGTH];
  /// The pre-shared key.
  char psKey[TINY_GSM_PSK_LENGTH];

 private:
  GsmSecureContext(const GsmSecureContext&);
  GsmSecureContext& operator=(const GsmSecureContext&);

  static bool hasText(const char* value, size_t maximumLength) {
    return value != nullptr && strnlen(value, maximumLength) != 0;
  }

  static bool copyString(char* destination, const char* source,
                         size_t destinationSize) {
    if (source == nullptr) { return false; }
    const size_t sourceLength = strnlen(source, destinationSize);
    if (sourceLength == 0 || sourceLength >= destinationSize) { return false; }
    memcpy(destination, source, sourceLength + 1);
    return true;
  }

  void clearConfigurationStrings() {
    memset(CAcertName, '\0', sizeof(CAcertName));
    memset(clientCertName, '\0', sizeof(clientCertName));
    memset(clientKeyName, '\0', sizeof(clientKeyName));
    memset(pskTableName, '\0', sizeof(pskTableName));
    memset(pskIdent, '\0', sizeof(pskIdent));
    memset(psKey, '\0', sizeof(psKey));
  }
};


/**
 * @brief Template for secure clients with SSL context index management.
 *
 * This template provides constructors and SSL context index management for all
 * modem-specific secure client implementations. Derived classes provide only
 * protocol-specific functionality.
 *
 * @tparam ClientType The base client class type (e.g., GsmClientA7672X)
 * @tparam modemType The modem class type (e.g., TinyGsmA7672X)
 */
template <typename ClientType, typename modemType>
class GsmSecureClient : public ClientType {
 public:
  /**
   * @anchor ssl_client_ctors
   * @name Secure client constructors and initialization
   */
  /**@{*/
  /// Default constructor creating an unattached secure client
  GsmSecureClient() {
    this->is_secure = true;
  }

  /**
   * @brief Create a secure client attached to a modem socket.
   * @param modem The modem that owns the socket and SSL contexts
   * @param mux The socket identifier
   */
  explicit GsmSecureClient(modemType& modem, uint8_t mux = 0)
      : ClientType(modem, mux) {
    this->is_secure = true;
  }

  /**
   * @brief Create a secure client with an explicit SSL context.
   * @param modem The modem that owns the socket and SSL contexts
   * @param mux The socket identifier
   * @param sslCtxIndex The hardware SSL context index
   */
  explicit GsmSecureClient(modemType& modem, uint8_t mux, uint8_t sslCtxIndex)
      : ClientType(modem, mux) {
    this->is_secure = true;
    this->setSSLContextIndex(sslCtxIndex);
  }
  /**@}*/


  /**
   * @anchor ssl_client_config
   * @name Secure client configuration
   */
  /**@{*/
  /**
   * @brief Select the hardware SSL context for this client.
   * @param sslCtxIndex The hardware SSL context index.
   *
   * @important The SSL context index set here must be **exactly what the modem
   * will use as the context identifier within AT commands**.  Use whatever
   * indexing the modem uses. This library will **not** adjust or validate the
   * index for you.  If unspecified, this library will use
   * #TINY_GSM_DEFAULT_SSL_CTX.
   *
   * @note This is not the same as the position of the client in the modem's
   * socket array (the mux) or the connection identifier. The context index is a
   * separate identifier used by the modem to manage groups of SSL settings.
   */
  void setSSLContextIndex(uint8_t sslCtxIndex) {
    this->sslCtxIndex = sslCtxIndex;
  }

 protected:
  /// The SSL context index to use for this connection
  uint8_t sslCtxIndex = TINY_GSM_DEFAULT_SSL_CTX;
};

// cSpell:words Palo

#endif  // SRC_TINYGSMSSL_TPP_
