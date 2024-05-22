/**
 * @file       TinyGsmSSL.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMSSL_H_
#define SRC_TINYGSMSSL_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_SSL

template <class modemType, uint8_t muxCount>
class TinyGsmSSL {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * Secure socket layer (SSL) functions
   */
  bool addCertificate(const char* filename) {
    return thisModem().addCertificateImpl(filename);
  }
  bool deleteCertificate() {
    return thisModem().deleteCertificateImpl();
  }

  bool setCertificate(const String& certificateName, const uint8_t mux = 0) {
    if (mux >= muxCount) return false;
    certificates[mux] = certificateName;
    return true;
  }

  /*
   * CRTP Helper
   */
 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }
  ~TinyGsmSSL() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Secure socket layer (SSL) functions
   */
 protected:
  bool addCertificateImpl(const char* filename) TINY_GSM_ATTR_NOT_IMPLEMENTED;
  bool deleteCertificateImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  String certificates[muxCount];
};

#endif  // SRC_TINYGSMSSL_H_
