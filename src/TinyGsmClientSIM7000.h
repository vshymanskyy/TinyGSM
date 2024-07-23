/**
 * @file       TinyGsmClientSIM7000.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSIM7000_H_
#define SRC_TINYGSMCLIENTSIM7000_H_

// #define TINY_GSM_DEBUG Serial
// #define TINY_GSM_USE_HEX

#define TINY_GSM_MUX_COUNT 8
#define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE

#include "TinyGsmClientSIM70xx.h"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"

class TinyGsmSim7000 : public TinyGsmSim70xx<TinyGsmSim7000>,
                       public TinyGsmTCP<TinyGsmSim7000, TINY_GSM_MUX_COUNT>,
                       public TinyGsmSMS<TinyGsmSim7000>,
                       public TinyGsmTime<TinyGsmSim7000>,
                       public TinyGsmNTP<TinyGsmSim7000>,
                       public TinyGsmGSMLocation<TinyGsmSim7000>,
                       public TinyGsmBattery<TinyGsmSim7000> {
  friend class TinyGsmSim70xx<TinyGsmSim7000>;
  friend class TinyGsmModem<TinyGsmSim7000>;
  friend class TinyGsmGPRS<TinyGsmSim7000>;
  friend class TinyGsmTCP<TinyGsmSim7000, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSMS<TinyGsmSim7000>;
  friend class TinyGsmGSMLocation<TinyGsmSim7000>;
  friend class TinyGsmGPS<TinyGsmSim7000>;
  friend class TinyGsmTime<TinyGsmSim7000>;
  friend class TinyGsmNTP<TinyGsmSim7000>;
  friend class TinyGsmBattery<TinyGsmSim7000>;

  /*
   * Inner Client
   */
 public:
  class GsmClientSim7000 : public GsmClient {
    friend class TinyGsmSim7000;

   public:
    GsmClientSim7000() {}

    explicit GsmClientSim7000(TinyGsmSim7000& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmSim7000* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_available = 0;
      prev_check     = 0;
      sock_connected = false;
      got_data       = false;

      if (mux < TINY_GSM_MUX_COUNT) {
        this->mux = mux;
      } else {
        this->mux = (mux % TINY_GSM_MUX_COUNT);
      }
      at->sockets[this->mux] = this;

      return true;
    }

   public:
    virtual int connect(const char* host, uint16_t port, int timeout_s) {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void stop(uint32_t maxWaitMs) {
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+CIPCLOSE="), mux);
      sock_connected = false;
      at->waitResponse(3000);
    }
    void stop() override {
      stop(15000L);
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  };

  /*
   * Inner Secure Client
   */
  // NOTE:  Use modem TinyGsmSim7000SSL for a secure client!

  /*
   * Constructor
   */
 public:
  explicit TinyGsmSim7000(Stream& stream)
      : TinyGsmSim70xx<TinyGsmSim7000>(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = nullptr) {
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
   * Secure socket layer (SSL) functions
   */
  // NOTE:  Use modem TinyGsmSim7000SSL for a secure client!

  /*
   * WiFi functions
   */
  // No functions of this type supported

  /*
   * GPRS functions
   */
 protected:
  bool gprsConnectImpl(const char* apn, const char* user = nullptr,
                       const char* pwd = nullptr) {
    gprsDisconnect();

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

    // Attach to GPRS
    sendAT(GF("+CGATT=1"));
    if (waitResponse(60000L) != 1) { return false; }

    // Activate the PDP context
    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    // Open the definied GPRS bearer context
    sendAT(GF("+SAPBR=1,1"));
    waitResponse(85000L);
    // Query the GPRS bearer context status
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
    sendAT(GF("+CSTT=\""), apn, GF("\",\""), user, GF("\",\""), pwd, GF("\""));
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

  bool gprsDisconnectImpl() {
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
   * Phone Call functions
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
   * GSM Location functions
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
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
    if (ssl) { DBG("SSL only supported using application on SIM7000!"); }
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;

    // when not using SSL, the TCP application toolkit is more stable
    sendAT(GF("+CIPSTART="), mux, ',', GF("\"TCP"), GF("\",\""), host,
           GF("\","), port);
    return (1 ==
            waitResponse(timeout_ms, GF("CONNECT OK" AT_NL),
                         GF("CONNECT FAIL" AT_NL), GF("ALREADY CONNECT" AT_NL),
                         GF("ERROR" AT_NL), GF("CLOSE OK" AT_NL)));
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }

    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();

    if (waitResponse(GF(AT_NL "DATA ACCEPT:"), GF("SEND FAIL")) != 1) {
      return 0;
    }
    streamSkipUntil(',');  // Skip mux
    return streamGetIntBefore('\n');
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) return 0;

#ifdef TINY_GSM_USE_HEX
    sendAT(GF("+CIPRXGET=3,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#else
    sendAT(GF("+CIPRXGET=2,"), mux, ',', (uint16_t)size);
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }
#endif
    streamSkipUntil(',');  // Skip Rx mode 2/normal or 3/HEX
    streamSkipUntil(',');  // Skip mux
    int16_t len_requested = streamGetIntBefore(',');
    //  ^^ Requested number of data bytes (1-1460 bytes)to be read
    int16_t len_confirmed = streamGetIntBefore('\n');
    // ^^ Confirmed number of data bytes to be read, which may be less than
    // requested. 0 indicates that no data can be read.
    // SRGD NOTE:  Contrary to above (which is copied from AT command manual)
    // this is actually be the number of bytes that will be remaining in the
    // buffer after the read.
    for (int i = 0; i < len_requested; i++) {
      uint32_t startMillis = millis();
#ifdef TINY_GSM_USE_HEX
      while (stream.available() < 2 &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_GSM_YIELD();
      }
      char buf[4] = {
          0,
      };
      buf[0] = stream.read();
      buf[1] = stream.read();
      char c = strtol(buf, nullptr, 16);
#else
      while (!stream.available() &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_GSM_YIELD();
      }
      char c = stream.read();
#endif
      sockets[mux]->rx.put(c);
    }
    // DBG("### READ:", len_requested, "from", mux);
    // sockets[mux]->sock_available = modemGetAvailable(mux);
    sockets[mux]->sock_available = len_confirmed;
    waitResponse();
    return len_requested;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!sockets[mux]) return 0;

    sendAT(GF("+CIPRXGET=4,"), mux);
    size_t result = 0;
    if (waitResponse(GF("+CIPRXGET:")) == 1) {
      streamSkipUntil(',');  // Skip mode 4
      streamSkipUntil(',');  // Skip mux
      result = streamGetIntBefore('\n');
      waitResponse();
    }
    // DBG("### Available:", result, "on", mux);
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+CIPSTATUS="), mux);
    waitResponse(GF("+CIPSTATUS"));
    int8_t res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""),
                              GF(",\"CLOSING\""), GF(",\"REMOTE CLOSING\""),
                              GF(",\"INITIAL\""));
    waitResponse();
    return 1 == res;
  }

  /*
   * Utilities
   */
 public:
  bool handleURCs(String& data) {
    if (data.endsWith(GF(AT_NL "+CIPRXGET:"))) {
      int8_t mode = streamGetIntBefore(',');
      if (mode == 1) {
        int8_t mux = streamGetIntBefore('\n');
        if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
          sockets[mux]->got_data = true;
        }
        data = "";
        // DBG("### Got Data:", mux);
        return true;
      } else {
        data += mode;
        return false;
      }
    } else if (data.endsWith(GF(AT_NL "+RECEIVE:"))) {
      int8_t  mux = streamGetIntBefore(',');
      int16_t len = streamGetIntBefore('\n');
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        sockets[mux]->got_data = true;
        if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
      }
      data = "";
      // DBG("### Got Data:", len, "on", mux);
      return true;
    } else if (data.endsWith(GF("CLOSED" AT_NL))) {
      int8_t nl   = data.lastIndexOf(AT_NL, data.length() - 8);
      int8_t coma = data.indexOf(',', nl + 2);
      int8_t mux  = data.substring(nl + 2, coma).toInt();
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        sockets[mux]->sock_connected = false;
      }
      data = "";
      DBG("### Closed: ", mux);
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
    } else if (data.endsWith(GF(AT_NL "SMS Ready" AT_NL))) {
      data = "";
      DBG("### Unexpected module reset!");
      init();
      return true;
    }
    return false;
  }

 protected:
  GsmClientSim7000* sockets[TINY_GSM_MUX_COUNT];
};

#endif  // SRC_TINYGSMCLIENTSIM7000_H_
