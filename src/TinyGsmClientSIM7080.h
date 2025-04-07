/**
 * @file       TinyGsmClientSim7080.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTSIM7080_H_
#define SRC_TINYGSMCLIENTSIM7080_H_

// #define TINY_GSM_DEBUG Serial
// #define TINY_GSM_USE_HEX

#define TINY_GSM_MUX_COUNT 12
#define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE

#include "TinyGsmClientSIM70xx.h"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"

class TinyGsmSim7080 : public TinyGsmSim70xx<TinyGsmSim7080>,
                       public TinyGsmTCP<TinyGsmSim7080, TINY_GSM_MUX_COUNT>,
                       public TinyGsmSSL<TinyGsmSim7080, TINY_GSM_MUX_COUNT>,
                       public TinyGsmSMS<TinyGsmSim7080>,
                       public TinyGsmGSMLocation<TinyGsmSim7080>,
                       public TinyGsmTime<TinyGsmSim7080>,
                       public TinyGsmNTP<TinyGsmSim7080>,
                       public TinyGsmBattery<TinyGsmSim7080> {
  friend class TinyGsmSim70xx<TinyGsmSim7080>;
  friend class TinyGsmModem<TinyGsmSim7080>;
  friend class TinyGsmGPRS<TinyGsmSim7080>;
  friend class TinyGsmTCP<TinyGsmSim7080, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmSim7080, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSMS<TinyGsmSim7080>;
  friend class TinyGsmGSMLocation<TinyGsmSim7080>;
  friend class TinyGsmGPS<TinyGsmSim7080>;
  friend class TinyGsmTime<TinyGsmSim7080>;
  friend class TinyGsmNTP<TinyGsmSim7080>;
  friend class TinyGsmBattery<TinyGsmSim7080>;

  /*
   * Inner Client
   */
 public:
  class GsmClientSim7080 : public GsmClient {
    friend class TinyGsmSim7080;

   public:
    GsmClientSim7080() {}

    explicit GsmClientSim7080(TinyGsmSim7080& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmSim7080* modem, uint8_t mux = 0) {
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
      at->sendAT(GF("+CACLOSE="), mux);
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
 public:
  class GsmClientSecureSIM7080 : public GsmClientSim7080 {
   public:
    GsmClientSecureSIM7080() {}

    explicit GsmClientSecureSIM7080(TinyGsmSim7080& modem, uint8_t mux = 0)
        : GsmClientSim7080(modem, mux) {}

    bool setCertificate(const String& certificateName) {
      return at->setCertificate(certificateName, mux);
    }

    int connect(const char* host, uint16_t port, int timeout_s) override {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES
  };

  /*
   * Constructor
   */
 public:
  explicit TinyGsmSim7080(Stream& stream)
      : TinyGsmSim70xx<TinyGsmSim7080>(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = nullptr) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM7080"));

    bool gotATOK = false;
    for (uint32_t start = millis(); millis() - start < 10000L;) {
      sendAT(GF(""));
      int8_t resp = waitResponse(200L, GFP(GSM_OK), GFP(GSM_ERROR), GF("AT"));
      if (resp == 1) {
        gotATOK = true;
        break;
      } else if (resp == 3) {
        waitResponse(200L);  // get the OK
        sendAT(GF("E0"));    // Echo Off
        DBG(GF("## Turning off echo!"));
        waitResponse(2000L);
      }
      delay(100);
    }
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
    // sockets asking if any data is avaiable
    bool check_socks = false;
    for (int mux = 0; mux < TINY_GSM_MUX_COUNT; mux++) {
      GsmClientSim7080* sock = sockets[mux];
      if (sock && sock->got_data) {
        sock->got_data = false;
        check_socks    = true;
      }
    }
    // modemGetAvailable checks all socks, so we only want to do it once
    // modemGetAvailable calls modemGetConnected(), which also checks allf
    if (check_socks) { modemGetAvailable(0); }
    while (stream.available()) { waitResponse(15, nullptr, nullptr); }
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = nullptr) {
    bool success = true;

    bool gotATOK = false;
    for (uint32_t start = millis(); millis() - start < 10000L;) {
      sendAT(GF(""));
      int8_t resp = waitResponse(200L, GFP(GSM_OK), GFP(GSM_ERROR), GF("AT"));
      if (resp == 1) {
        gotATOK = true;
        break;
      } else if (resp == 3) {
        waitResponse(200L);  // get the OK
        DBG(GF("## Turning off echo!"));
        sendAT(GF("E0"));  // Echo Off
        waitResponse(2000L);
      }
      delay(100);
    }
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
    if (waitResponse(GF(AT_NL "+CNACT:")) != 1) { return ""; }
    streamSkipUntil('\"');
    String res = stream.readStringUntil('\"');
    waitResponse();
    return res;
  }

  /*
   * Secure socket layer (SSL) functions
   */
  // Follows functions as inherited from TinyGsmSSL.tpp

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

    // Define the PDP context
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Attach to GPRS
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
    // <pdpidx> PDP Context Identifier - for reasons not understood by me,
    //          use PDP context identifier of 0 for what we defined as 1 above
    // <ip_type> 0: Dual PDN Stack
    //           1: Internet Protocol Version 4
    //           2: Internet Protocol Version 6
    // <authentication> 0: NONE
    //                  1: PAP
    //                  2: CHAP
    //                  3: PAP or CHAP
    if (pwd && strlen(pwd) > 0 && user && strlen(user) > 0) {
      sendAT(GF("+CNCFG=0,1,\""), apn, "\",\"", user, "\",\"", pwd, "\",3");
      waitResponse();
    } else if (user && strlen(user) > 0) {
      // Set the user name only
      sendAT(GF("+CNCFG=0,1,\""), apn, "\",\"", user, '"');
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
      res = waitResponse(60000L, GF(AT_NL "+APP PDP: 0,ACTIVE"),
                         GF(AT_NL "+APP PDP: 0,DEACTIVE"));
      waitResponse();
      ntries++;
    }

    return res;
  }

  bool gprsDisconnectImpl() {
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
 protected:
  byte NTPServerSyncImpl(String server = "pool.ntp.org", int TimeZone = 0) {
    // Set GPRS bearer profile to associate with NTP sync
    // this may fail, it's not supported by all modules
    sendAT(GF("+CNTPCID=0"));  // CID must be 0. With 1 (like other modules)
                               // does not work!
    waitResponse(10000L);

    // Set NTP server and timezone
    sendAT(GF("+CNTP=\""), server, "\",", String(TimeZone));
    if (waitResponse(10000L) != 1) { return -1; }

    // Request network synchronization
    sendAT(GF("+CNTP"));
    if (waitResponse(10000L, GF("+CNTP:"))) {
      String result = stream.readStringUntil('\n');
      // Check for ',' in case the module appends the time next to the return
      // code. Eg: +CNTP: <code>[,<time>]
      int index = result.indexOf(',');
      if (index > 0) { result.remove(index); }
      result.trim();
      if (TinyGsmIsValidNumber(result)) { return result.toInt(); }
    } else {
      return -1;
    }
    return -1;
  }

  String ShowNTPErrorImpl(byte error) {
    switch (error) {
      case 1: return "Network time synchronization is successful";
      case 61: return "Network error";
      case 62: return "DNS resolution error";
      case 63: return "Connection error";
      case 64: return "Service response error";
      case 65: return "Service response timeout";
      default: return "Unknown error: " + String(error);
    }
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
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;

    // set the connection (mux) identifier to use
    sendAT(GF("+CACID="), mux);
    if (waitResponse(timeout_ms) != 1) return false;


    if (ssl) {
      // set the ssl version
      // AT+CSSLCFG="SSLVERSION",<ctxindex>,<sslversion>
      // <ctxindex> PDP context identifier - for reasons not understood by me,
      //            use PDP context identifier of 0 for what we defined as 1 in
      //            the gprsConnect function
      // <sslversion> 0: QAPI_NET_SSL_PROTOCOL_UNKNOWN
      //              1: QAPI_NET_SSL_PROTOCOL_TLS_1_0
      //              2: QAPI_NET_SSL_PROTOCOL_TLS_1_1
      //              3: QAPI_NET_SSL_PROTOCOL_TLS_1_2
      //              4: QAPI_NET_SSL_PROTOCOL_DTLS_1_0
      //              5: QAPI_NET_SSL_PROTOCOL_DTLS_1_2
      // NOTE:  despite docs using caps, "sslversion" must be in lower case
      sendAT(GF("+CSSLCFG=\"sslversion\",0,3"));  // TLS 1.2
      if (waitResponse(5000L) != 1) return false;
    }

    // enable or disable ssl
    // AT+CASSLCFG=<cid>,"SSL",<sslFlag>
    // <cid> Application connection ID (set with AT+CACID above)
    // <sslFlag> 0: Not support SSL
    //           1: Support SSL
    sendAT(GF("+CASSLCFG="), mux, ',', GF("SSL,"), ssl);
    waitResponse();

    if (ssl) {
      // set the PDP context to apply SSL to
      // AT+CSSLCFG="CTXINDEX",<ctxindex>
      // <ctxindex> PDP context identifier - for reasons not understood by me,
      //            use PDP context identifier of 0 for what we defined as 1 in
      //            the gprsConnect function
      // NOTE:  despite docs using "CRINDEX" in all caps, the module only
      // accepts the command "ctxindex" and it must be in lower case
      sendAT(GF("+CSSLCFG=\"ctxindex\",0"));
      if (waitResponse(5000L, GF("+CSSLCFG:")) != 1) return false;
      streamSkipUntil('\n');  // read out the certificate information
      waitResponse();

      if (certificates[mux] != "") {
        // apply the correct certificate to the connection
        // AT+CASSLCFG=<cid>,"CACERT",<caname>
        // <cid> Application connection ID (set with AT+CACID above)
        // <certname> certificate name
        sendAT(GF("+CASSLCFG="), mux, ",CACERT,\"", certificates[mux].c_str(),
               "\"");
        if (waitResponse(5000L) != 1) return false;
      }

      // set the SSL SNI (server name indication)
      // AT+CSSLCFG="SNI",<ctxindex>,<servername>
      // <ctxindex> PDP context identifier - for reasons not understood by me,
      //            use PDP context identifier of 0 for what we defined as 1 in
      //            the gprsConnect function
      // NOTE:  despite docs using caps, "sni" must be in lower case
      sendAT(GF("+CSSLCFG=\"sni\",0,"), GF("\""), host, GF("\""));
      waitResponse();
    }

    // actually open the connection
    // AT+CAOPEN=<cid>,<pdp_index>,<conn_type>,<server>,<port>[,<recv_mode>]
    // <cid> TCP/UDP identifier
    // <pdp_index> Index of PDP connection; we set up PCP context 1 above
    // <conn_type> "TCP" or "UDP"
    // <recv_mode> 0: The received data can only be read manually using
    // AT+CARECV=<cid>
    //             1: After receiving the data, it will automatically report
    //             URC:
    //                +CAURC:
    //                "recv",<id>,<length>,<remoteIP>,<remote_port><CR><LF><data>
    // NOTE:  including the <recv_mode> fails
    sendAT(GF("+CAOPEN="), mux, GF(",0,\"TCP\",\""), host, GF("\","), port);
    if (waitResponse(timeout_ms, GF(AT_NL "+CAOPEN:")) != 1) { return 0; }
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
    streamSkipUntil(',');  // Skip mux

    // make sure the connection really opened
    int8_t res = streamGetIntBefore('\n');
    waitResponse();

    return 0 == res;
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    // send data on prompt
    sendAT(GF("+CASEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }

    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();

    // OK after posting data
    if (waitResponse() != 1) { return 0; }

    return len;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) { return 0; }

    sendAT(GF("+CARECV="), mux, ',', (uint16_t)size);

    if (waitResponse(GF("+CARECV:")) != 1) { return 0; }

    // uint8_t ret_mux = stream.parseInt();
    // streamSkipUntil(',');
    // const int16_t len_confirmed = streamGetIntBefore('\n');
    // DBG("### READING:", len_confirmed, "from", ret_mux);

    // if (ret_mux != mux) {
    //   DBG("### Data from wrong mux! Got", ret_mux, "expected", mux);
    //   waitResponse();
    //   sockets[mux]->sock_available = modemGetAvailable(mux);
    //   return 0;
    // }

    // NOTE:  manual says the mux number is returned before the number of
    // characters available, but in tests only the number is returned

    int16_t len_confirmed = stream.parseInt();
    streamSkipUntil(',');  // skip the comma
    if (len_confirmed <= 0) {
      waitResponse();
      sockets[mux]->sock_available = modemGetAvailable(mux);
      return 0;
    }

    for (int i = 0; i < len_confirmed; i++) {
      uint32_t startMillis = millis();
      while (!stream.available() &&
             (millis() - startMillis < sockets[mux]->_timeout)) {
        TINY_GSM_YIELD();
      }
      char c = stream.read();
      sockets[mux]->rx.put(c);
    }
    waitResponse();
    // make sure the sock available number is accurate again
    sockets[mux]->sock_available = modemGetAvailable(mux);
    return len_confirmed;
  }

  size_t modemGetAvailable(uint8_t mux) {
    // If the socket doesn't exist, just return
    if (!sockets[mux]) { return 0; }
    // NOTE: This gets how many characters are available on all connections that
    // have data.  It does not return all the connections, just those with data.
    sendAT(GF("+CARECV?"));
    for (int muxNo = 0; muxNo < TINY_GSM_MUX_COUNT; muxNo++) {
      // after the last connection, there's an ok, so we catch it right away
      int res = waitResponse(3000, GF("+CARECV:"), GFP(GSM_OK), GFP(GSM_ERROR));
      // if we get the +CARECV: response, read the mux number and the number of
      // characters available
      if (res == 1) {
        int               ret_mux = streamGetIntBefore(',');
        size_t            result  = streamGetIntBefore('\n');
        GsmClientSim7080* sock    = sockets[ret_mux];
        if (sock) { sock->sock_available = result; }
        // if the first returned mux isn't 0 (or is higher than expected)
        // we need to fill in the missing muxes
        if (ret_mux > muxNo) {
          for (int extra_mux = muxNo; extra_mux < ret_mux; extra_mux++) {
            GsmClientSim7080* isock = sockets[extra_mux];
            if (isock) { isock->sock_available = 0; }
          }
          muxNo = ret_mux;
        }
      } else if (res == 2) {
        // if we get an OK, we've reached the last socket with available data
        // so we set any we haven't gotten to yet to 0
        for (int extra_mux = muxNo; extra_mux < TINY_GSM_MUX_COUNT;
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
      if (muxNo == TINY_GSM_MUX_COUNT - 1) { waitResponse(); }
    }
    modemGetConnected(mux);  // check the state of all connections
    if (!sockets[mux]) { return 0; }
    return sockets[mux]->sock_available;
  }

  bool modemGetConnected(uint8_t mux) {
    // NOTE:  This gets the state of all connections that have been opened
    // since the last connection
    sendAT(GF("+CASTATE?"));

    for (int muxNo = 0; muxNo < TINY_GSM_MUX_COUNT; muxNo++) {
      // after the last connection, there's an ok, so we catch it right away
      int res = waitResponse(3000, GF("+CASTATE:"), GFP(GSM_OK),
                             GFP(GSM_ERROR));
      // if we get the +CASTATE: response, read the mux number and the status
      if (res == 1) {
        int    ret_mux = streamGetIntBefore(',');
        size_t status  = streamGetIntBefore('\n');
        // 0: Closed by remote server or internal error
        // 1: Connected to remote server
        // 2: Listening (server mode)
        GsmClientSim7080* sock = sockets[ret_mux];
        if (sock) { sock->sock_connected = (status == 1); }
        // if the first returned mux isn't 0 (or is higher than expected)
        // we need to fill in the missing muxes
        if (ret_mux > muxNo) {
          for (int extra_mux = muxNo; extra_mux < ret_mux; extra_mux++) {
            GsmClientSim7080* isock = sockets[extra_mux];
            if (isock) { isock->sock_connected = false; }
          }
          muxNo = ret_mux;
        }
      } else if (res == 2) {
        // if we get an OK, we've reached the last socket with available data
        // so we set any we haven't gotten to yet to 0
        for (int extra_mux = muxNo; extra_mux < TINY_GSM_MUX_COUNT;
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
      if (muxNo == TINY_GSM_MUX_COUNT - 1) { waitResponse(); }
    }
    return sockets[mux]->sock_connected;
  }

  /*
   * Utilities
   */
 public:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+CARECV:"))) {
      int8_t  mux = streamGetIntBefore(',');
      int16_t len = streamGetIntBefore('\n');
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        sockets[mux]->got_data = true;
        if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
      }
      data = "";
      DBG("### Got Data:", len, "on", mux);
      return true;
    } else if (data.endsWith(GF("+CADATAIND:"))) {
      int8_t mux = streamGetIntBefore('\n');
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        sockets[mux]->got_data = true;
      }
      data = "";
      DBG("### Got Data:", mux);
      return true;
    } else if (data.endsWith(GF("+CASTATE:"))) {
      int8_t mux   = streamGetIntBefore(',');
      int8_t state = streamGetIntBefore('\n');
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
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
    } else if (data.endsWith(GF(AT_NL "SMS Ready" AT_NL))) {
      data = "";
      DBG("### Unexpected module reset!");
      init();
      data = "";
      return true;
    }
    return false;
  }

 protected:
  GsmClientSim7080* sockets[TINY_GSM_MUX_COUNT];
};

#endif  // SRC_TINYGSMCLIENTSIM7080_H_
