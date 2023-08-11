/**
 * @file       TinyGsmClientBC66.h
 * @author     Clemens Arth
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2021 Clemens Arth
 * @date       Jan 2021
 *
 */

#ifndef SRC_TINYGSMCLIENTBC66_H_
#define SRC_TINYGSMCLIENTBC66_H_
// #pragma message("TinyGSM:  TinyGsmClientBC66")

// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MUX_COUNT 5
#define TINY_GSM_BUFFER_READ_NO_CHECK

// #include "TinyGsmBattery.tpp"
// #include "TinyGsmCalling.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmModem.tpp"
#include "TinyGsmMqtt.tpp"
// #include "TinyGsmSMS.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmTime.tpp"

#include <Time.h>

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM    = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;
#if defined       TINY_GSM_DEBUG
static const char GSM_CME_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CME ERROR:";
static const char GSM_CMS_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CMS ERROR:";
#endif

enum RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};

class TinyGsmBC66 : public TinyGsmModem<TinyGsmBC66>,
                    public TinyGsmGPRS<TinyGsmBC66>,
                    public TinyGsmTCP<TinyGsmBC66, TINY_GSM_MUX_COUNT>,
                    public TinyGsmSSL<TinyGsmBC66>,
                    public TinyGsmMqtt<TinyGsmBC66>,
                    public TinyGsmTime<TinyGsmBC66>  {
  friend class TinyGsmModem<TinyGsmBC66>;
  friend class TinyGsmGPRS<TinyGsmBC66>;
  friend class TinyGsmTCP<TinyGsmBC66, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmBC66>;
  friend class TinyGsmMqtt<TinyGsmBC66>;
  friend class TinyGsmTime<TinyGsmBC66>;

  /*
   * Inner Client
   */
 public:
  class GsmClientBC66 : public GsmClient {
    friend class TinyGsmBC66;

   public:
    GsmClientBC66() {}

    explicit GsmClientBC66(TinyGsmBC66& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmBC66* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_available = 0;
      sock_connected = false;

      if (mux < TINY_GSM_MUX_COUNT) {
        this->mux = mux;
      } else {
        this->mux = (mux % TINY_GSM_MUX_COUNT);
      }
      at->sockets[this->mux] = this;

      return true;
    }

   public:
   // CLEMENS: THIS NEEDS FULL OVERHAUL - UNTESTED!!!
    virtual int connect(const char* host, uint16_t port, int timeout_s) {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void stop(uint32_t maxWaitMs) {
      uint32_t startMillis = millis();
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+QICLOSE="), mux);
      sock_connected = false;
      //Serial.println("...1...");
      at->waitResponse((maxWaitMs - (millis() - startMillis)), GF("CLOSED"),
                       GF("CLOSE OK"), GF("ERROR"));
      //Serial.println("\n...2...");
    }
    void stop() override {
      stop(75000L);
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  };

  /*
   * Inner Secure Client
   */

  /*
    class GsmClientSecureMC60 : public GsmClientBC66
    {
    public:
      GsmClientSecure() {}

      GsmClientSecure(TinyGsmBC66& modem, uint8_t mux = 0)
       : GsmClient(modem, mux)
      {}


    public:
      int connect(const char* host, uint16_t port, int timeout_s) override {
        stop();
        TINY_GSM_YIELD();
        rx.clear();
        sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
        return sock_connected;
      }
      TINY_GSM_CLIENT_CONNECT_OVERRIDES
    };
 */

  /*
   * Constructor
   */
 public:
  explicit TinyGsmBC66(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientBC66"));

    if (!testAT()) { return false; }

    // sendAT(GF("&FZ"));  // Factory + Reset
    // waitResponse();

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1)
    {
      DBG("### E0 issue!");
      return false;
    }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

    // set show length
    sendAT(GF("+QICFG=\"showlength\",1"));
    if (waitResponse() != 1)
    {
      DBG("### showlength issue!");
      return false;
    }

    // CLEMENS UNSUPPORTED ON BC66
    // Enable network time synchronization - not available on BC66
    //sendAT(GF("+QNITZ=1"));
    //if (waitResponse(10000L) != 1) { return false; }

    SimStatus ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != NULL && strlen(pin) > 0) {
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
 protected:
  bool restartImpl() {
    if (!testAT()) { return false; }
    if (!setPhoneFunctionality(0)) { return false; }
    if (!setPhoneFunctionality(1, true)) { return false; }
    delay(3000);
    return init();
  }

  bool powerOffImpl() {
    sendAT(GF("+QPOWD=1"));
    return waitResponse(GF("NORMAL POWER DOWN")) == 1;
  }

  // When entering into sleep mode is enabled, DTR is pulled up, and WAKEUP_IN
  // is pulled up, the module can directly enter into sleep mode.If entering
  // into sleep mode is enabled, DTR is pulled down, and WAKEUP_IN is pulled
  // down, there is a need to pull the DTR pin and the WAKEUP_IN pin up first,
  // and then the module can enter into sleep mode.
  bool sleepEnableImpl(bool enable = true) {
    sendAT(GF("+QSCLK="), enable);
    return waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 public:
  // original MC60
  RegStatus getRegistrationStatus() {
    return (RegStatus)getRegistrationStatusXREG("CEREG");
  }

 protected:
  bool isNetworkConnectedImpl() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  String getLocalIPImpl() {
    return m_localIP.toString();
  }

  bool waitForNetworkImpl(uint32_t timeout_ms = 60000L) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      if (waitResponse(2000L, GF(GSM_NL "+IP:")) != 1) {
        continue;
      } else {
        // connection ID
        uint8_t aaa = streamGetIntBefore('.');
        uint8_t bbb = streamGetIntBefore('.');
        uint8_t ccc = streamGetIntBefore('.');
        uint8_t ddd = streamGetIntBefore('\n');
        m_localIP = IPAddress(aaa,bbb,ccc,ddd);
        return true;
      }
    }
    return false;
  }


  /*
   * GPRS functions
   */
 protected:
   // CLEMENS: THIS NEEDS FULL OVERHAUL
   /*
  bool gprsConnectImpl(const char* apn, const char* user = NULL,
                       const char* pwd = NULL) {
    gprsDisconnect();

    // select foreground context 0 = VIRTUAL_UART_1
    sendAT(GF("+QIFGCNT=0"));
    if (waitResponse() != 1) { return false; }

    // Select GPRS (=1) as the Bearer
    sendAT(GF("+QICSGP=1,\""), apn, GF("\",\""), user, GF("\",\""), pwd,
           GF("\""));
    if (waitResponse() != 1) { return false; }

    // Define PDP context - is this necessary?
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Activate PDP context - is this necessary?
    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    // Select TCP/IP transfer mode - NOT transparent mode
    sendAT(GF("+QIMODE=0"));
    if (waitResponse() != 1) { return false; }

    // Enable multiple TCP/IP connections
    sendAT(GF("+QIMUX=1"));
    if (waitResponse() != 1) { return false; }

    // Modem is used as a client
    sendAT(GF("+QISRVC=1"));
    if (waitResponse() != 1) { return false; }

    // Start TCPIP Task and Set APN, User Name and Password
    sendAT("+QIREGAPP=\"", apn, "\",\"", user, "\",\"", pwd, "\"");
    if (waitResponse() != 1) { return false; }

    // Activate GPRS/CSD Context
    sendAT(GF("+QIACT"));
    if (waitResponse(60000L) != 1) { return false; }

    // Check that we have a local IP address
    if (localIP() == IPAddress(0, 0, 0, 0)) { return false; }

    // Set Method to Handle Received TCP/IP Data
    // Mode=2 - Output a notification statement:
    // +QIRDI: <id>,<sc>,<sid>,<num>,<len>,< tlen>
    sendAT(GF("+QINDI=2"));
    if (waitResponse() != 1) { return false; }

    return true;
  }

  bool gprsDisconnectImpl() {
    sendAT(GF("+QIDEACT"));  // Deactivate the bearer context
    return waitResponse(60000L, GF("DEACT OK"), GF("ERROR")) == 1;
  }
    */
  /*
   * SIM card functions
   */
 protected:
  SimStatus getSimStatusImpl(uint32_t timeout_ms = 10000L) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      sendAT(GF("+CPIN?"));
      if (waitResponse(GF(GSM_NL "+CPIN:")) != 1) {
        delay(1000);
        continue;
      }
      int8_t status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"),
                                   GF("NOT INSERTED"), GF("PH_SIM PIN"),
                                   GF("PH_SIM PUK"));
      waitResponse();
      switch (status) {
        case 2:
        case 3: return SIM_LOCKED;
        case 5:
        case 6: return SIM_ANTITHEFT_LOCKED;
        case 1: return SIM_READY;
        default: return SIM_ERROR;
      }
    }
    return SIM_ERROR;
  }

  /*
   * Phone Call functions
   */
 protected:
  // Can follow all of the phone call functions from the template

  /*
   * Messaging functions
   */
 protected:
  // Can follow all template functions

 public:
  /** Delete all SMS */
  bool deleteAllSMS() {
    sendAT(GF("+QMGDA=6"));
    if (waitResponse(waitResponse(60000L, GF("OK"), GF("ERROR")) == 1)) {
      return true;
    }
    return false;
  }

  /*
   * Time functions
   */
 protected:
  // Can follow the standard CCLK function in the template

  /*
   * Battery functions
   */
  // Can follow battery functions as in the template

  /*
   * Client related functions
   */
 protected:
   // CLEMENS: THIS NEEDS FULL OVERHAUL
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {

    if (ssl) { DBG("SSL not yet supported on this module!"); }
    // <contextID> Integer type. Context ID. The range is 1-3. -> only 1 is supported!!!!


    // +QIOPEN=1,mux,"TCP","54.73.62.155",5566
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    sendAT(GF("+QIOPEN=1,"), mux, GF(",\""), GF("TCP"), GF("\",\""), host,
           GF("\","), port);

    if (waitResponse(5000L) != 1) { return false; }

    // +QIOPEN: mux,0
    for (uint32_t start = millis(); millis() - start < 75000L;) {
      if (waitResponse(2000L, GF(GSM_NL "+QIOPEN:")) != 1) {
        continue;
      } else {
        // connection ID
        streamSkipUntil(','); // instead of int8_t mux = streamGetIntBefore(','); // UNUSED
        // read the result (0 if ok, -1 if bad)
        int8_t result = streamGetIntBefore('\n');
        if (result == 0) {
          //Serial.println("\n=========== OK WITH QIOPEN!!! ===========");
          return true;
        } else {
          //Serial.println("\n=========== ERROR WITH QIOPEN!!! ===========");
          return false;
        }
      }
    }
    return false;
  }





    // CLEMENS: THIS NEEDS FULL OVERHAUL
  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {

    sendAT(GF("+QISEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.write(static_cast<char>(0x1A));
    stream.flush();
    if (waitResponse(GF(GSM_NL "SEND OK")) != 1)
    {
      //Serial.println("\n=========== ERROR WITH QISEND!!! ===========");
      return 0;
    }
    //Serial.println("\n=========== OK WITH QISEND!!! ===========");
/*
    bool allAcknowledged = false;
    // bool failed = false;
    while (!allAcknowledged) {
      sendAT(GF("+QISEND"));
      if (waitResponse(5000L, GF(GSM_NL "+QISEND:")) != 1) {
        return -1;
      } else {
        streamSkipUntil(','); // Skip total
        streamSkipUntil(','); // Skip acknowledged data size
        if (streamGetIntBefore('\n') == 0) { allAcknowledged = true; }
      }
    }
*/
    return len;
  }

    // CLEMENS: THIS NEEDS FULL OVERHAUL
  size_t modemRead(size_t size, uint8_t mux) {

    if (!sockets[mux]) return 0;
    // TODO(?):  Does this even work????
    // AT+QIRD=<id>,<sc>,<sid>,<len>
    // id = GPRS context number = 0, set in GPRS connect
    // sc = role in connection = 1, client of connection
    // sid = index of connection = mux
    // len = maximum length of data to retrieve
    // sendAT(GF("+QIRD=0,1,"), mux, ',', (uint16_t)size);
    //Serial.print("\nMODEM READ...\n");
    sendAT(GF("+QIRD="), mux, ',', (uint16_t)size);
    // If it replies only OK for the write command, it means there is no
    // received data in the buffer of the connection.
    int8_t res = waitResponse(GF("+QIRD:"), GFP(GSM_OK), GFP(GSM_ERROR));
    if (res == 1) {
      //streamSkipUntil(':');  // skip IP address
      //streamSkipUntil(',');  // skip port
      //streamSkipUntil(',');  // skip connection type (TCP/UDP)
      // read the real length of the retrieved data
      //uint16_t len = streamGetIntBefore(',');

      uint16_t len;
      String bla; streamReadUntil(bla,'\n');
      int com = bla.indexOf(',');
      if( com < 0 )
        len = atoi(bla.c_str());
      else
        len = atoi(bla.substring(0,com).c_str());

      //Serial.print("\nWILL TRY READING... ");
      //Serial.print(len);
      //Serial.print(" CHARS...\n");

      // streamSkipUntil('\n');
      // It's possible that the real length available is less than expected
      // This is quite likely if the buffer is broken into packets - which may
      // be different sizes.
      // If so, make sure we make sure we re-set the amount of data available.
      if (len < size) { sockets[mux]->sock_available = len; }
      for (uint16_t i = 0; i < len; i++) {
        moveCharFromStreamToFifo(mux);
        sockets[mux]->sock_available--;
        // ^^ One less character available after moving from modem's FIFO to our
        // FIFO
      }
      waitResponse();  // ends with an OK
      //DBG("### READ:", len, "from", mux);
      return len;
    } else {
      //DBG("### XIT:");
      sockets[mux]->sock_available = 0;
      return 0;
    }
    //DBG("### XIT2");
    return 0;
  }

  // Not possible to check the number of characters remaining in buffer
  size_t modemGetAvailable(uint8_t) {
    return 0;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+QISTATE=1,"), mux);
    // +QISTATE: 0,"TCP","151.139.237.11",80,5087,4,1,0,0,"uart1"

    if (waitResponse(GF("+QISTATE:")) != 1) { return false; }

    streamSkipUntil(',');                  // Skip mux
    streamSkipUntil(',');                  // Skip socket type
    streamSkipUntil(',');                  // Skip remote ip
    streamSkipUntil(',');                  // Skip remote port
    streamSkipUntil(',');                  // Skip local port
    int8_t res = streamGetIntBefore(',');  // socket state

    waitResponse();

    // 0 Initial, 1 Opening, 2 Connected, 3 Listening, 4 Closing
    return 2 == res;
  }

  /*
   * SSL related functions
   */
  protected:
  bool addCertificateAsStringImpl(String certificatestring)
  {
    if(certificatestring.length() > 0) {
      m_sslServerCertificate = certificatestring;
      return true;
    }
    return false;
  }

  bool configureSSLImpl(uint8_t contextId, uint8_t connectId, String cmd, uint8_t arg = 0)
  {
    // From SSL Docu of Bc66
    // <contextID> aka sslconectId - Integer type. SSL context index. The range is 1-3.
    // <connectID> aka sslconnectId - Integer type. SSL connect index. The range is 0-5.
    // <seclevel> Integer type. Authentication mode.
    // 0 No authentication
    // 1 Manage server authentication
    // 2 Manage server and client authentication if requested by the remote server
    uint8_t sslconnectId = connectId;
    uint8_t sslcontextId = contextId;
    if(sslcontextId < 1 || sslcontextId > 3 || sslconnectId > 5) {
      DBG("CONTEXTID/CONNECTID OUT OF RANGE!");
      return false;
    }
    if(cmd.compareTo("seclevel") == 0) {
      uint8_t seclvl = arg;
      if( seclvl > 2) {
        DBG("SECLEVEL OUT OF RANGE!");
        return false;
      }
      sendAT(GF("+QSSLCFG="), contextId, GF(","), connectId, GF(",\""), GF("seclevel"), GF("\","), seclvl);
      if (waitResponse() != 1) { return false; }
      return true;
    }
    else if(cmd.compareTo("cacert") == 0) {
      if(m_sslServerCertificate.length() == 0) {
        DBG("SSL CERTIFICATE EMPTY!");
        return false;
      }
      sendAT(GF("+QSSLCFG="), contextId, GF(","), connectId, GF(",\""), GF("cacert"), GF("\""));
      if (waitResponse(GF(">")) != 1) { return false; }

      // chunked in 64-byte packets...
      int sz = 0; int chnk = 64; int len = m_sslServerCertificate.length();
      while(sz < len)
      {
        int to = sz+chnk-1 > len ? len : sz+chnk-1;
        stream.print(m_sslServerCertificate.substring(sz,to).c_str());
        sz = to;
        delay(50);
      }
      // all at once
      // stream.print(m_sslServerCertificate.c_str());  // Actually send the message
      stream.write(static_cast<char>(0x1A));  // Terminate the message
      stream.flush();
      return waitResponse(5000L) == 1;
    }
    return false;
  }

  /*
   * MQTT related functions
   */
  bool configureMqttImpl(String cmd, uint8_t arg1, uint8_t arg2, uint8_t arg3 = 0, uint8_t arg4 = 0)
  {
    // From TCP Docu of Bc66
    // <contextID> aka tcpcontextId - Integer type. Context ID. The range is 1-3.
    // <connectID> aka tcpconnectId - Integer type. Socket service index. The range is 0-4.
    if(cmd.compareTo("ssl") == 0) {
      // modem.configureMqtt("ssl",tcpconnectId,enable,sslcontextId,sslconnectId)
      uint8_t tcpconnectId = arg1;
      uint8_t enable = arg2 == 1; // 0 or 1
      uint8_t sslcontextId = arg3;
      uint8_t sslconnectId = arg4;
      if(tcpconnectId > 4 || sslcontextId < 1 || sslcontextId > 3 || sslconnectId > 5)
      {
        DBG("TCPCONNECTID/SSLCONNECTID/SSLCONTEXTID OUT OF RANGE!");
        return false;
      }
      sendAT(GF("+QMTCFG="), GF("\""), GF("ssl"), GF("\","), tcpconnectId, GF(","), enable, GF(","), sslcontextId, GF(","), sslconnectId);
      if (waitResponse() != 1) { return false; }
      return true;
    }
    else if(cmd.compareTo("version") == 0) {
      // modem.configureMqtt("version",tcpconnectId,mqttVersion)
      uint8_t tcpconnectId = arg1;
      uint8_t mqttVersion = arg2;
      if(tcpconnectId > 4 || mqttVersion < 3 || mqttVersion > 4)
      {
        DBG("TCPCONNECTID/MQTTVERSION OUT OF RANGE!");
        return false;
      }
      sendAT(GF("+QMTCFG="), GF("\""), GF("version"), GF("\","), tcpconnectId, GF(","), mqttVersion);
      if (waitResponse() != 1) { return false; }
      return true;
    }
    /*
    else if (cmd.compareTo("timeout") == 0)
    {
      // AT+QMTCFG="timeout",<TCP_connectID>[,<pkt_timeout>,<retry_times>,<timeout_notice>]
      // <pkt_timeout> Integer type. Timeout of the packet delivery. The range is 1-60. The default value is 10. Unit: second.
      // <retry_times> Integer type. Retry times when packet delivery times out. The range is 0-10. The default value is 3.
      // <timeout_notice> Integer type. Whether to report timeout message when transmitting packet. 0 Not report, 1 Report
      uint8_t tcpconnectId = arg1;
    }
    */
    return false;
  }

  bool openMqttImpl(uint8_t connectId, String servername, uint16_t serverport) {
    // From TCP Docu of Bc66
    // <connectID> aka tcpconnectId - Integer type. Socket service index. The range is 0-4.
    uint8_t tcpconnectId = connectId;
    if(tcpconnectId > 4)
    {
      DBG("TCPCONNECTID OUT OF RANGE!");
      return false;
    }
    sendAT(GF("+QMTOPEN="), tcpconnectId, GF(",\""), servername.c_str(), GF("\","), serverport);
    if (waitResponse(5000L) != 1) { return false; }

    // +QMTOPEN: 3,0
    for (uint32_t start = millis(); millis() - start < 75000L;) {
      if (waitResponse(2000L, GF(GSM_NL "+QMTOPEN:")) != 1) {
        continue;
      } else {
        // connection ID
        streamSkipUntil(','); // instead of int8_t mux = streamGetIntBefore(','); // UNUSED
        // read the result (0 if ok, -1 if bad)
        int8_t result = streamGetIntBefore('\n');
        if (result == 0) {
          return true;
        } else {
          return false;
        }
      }
    }
    return false;
  }

  bool connectMqttImpl(uint8_t connectId, String clientname, String username = "", String password = "") {
    // From TCP Docu of Bc66
    // <connectID> aka tcpconnectId - Integer type. Socket service index. The range is 0-4.
    uint8_t tcpconnectId = connectId;
    if(tcpconnectId > 4)
    {
      DBG("TCPCONNECTID OUT OF RANGE!");
      return false;
    }
    if(username.length() > 0 || password.length() > 0)
    {
      DBG("USER/PASSWORD AUTH NOT IMPLEMENTED!");
      return false;
    }

    sendAT(GF("+QMTCONN="), tcpconnectId, GF(",\""), clientname.c_str(), GF("\""));
    if (waitResponse(5000L) != 1) { return false; }

    // +QMTCONN: 3,0,0
    for (uint32_t start = millis(); millis() - start < 60000L;) {
      if (waitResponse(2000L, GF(GSM_NL "+QMTCONN:")) != 1) {
        continue;
      } else {
        // connection ID
        streamSkipUntil(','); // instead of int8_t mux = streamGetIntBefore(',');
        // read the result (0 ok, 1 retransmit, 2 failed)
        int8_t result = streamGetIntBefore(',');
        // read the retcode (0 accepted, 1 protocol bad, 2 identifier reject, 3 refused (unavail), 4 bad user/pwd,  5 unauthorized
        int8_t retcode = streamGetIntBefore('\n');
        if (result == 0 && retcode == 0) {
          return true;
        } else {
          return false;
        }
      }
    }
    return false;
  }

  bool disconnectMqttImpl(uint8_t connectId)
  {
    // From TCP Docu of Bc66
    // <connectID> aka tcpconnectId - Integer type. Socket service index. The range is 0-4.
    uint8_t tcpconnectId = connectId;
    if(tcpconnectId > 4)
    {
      DBG("TCPCONNECTID OUT OF RANGE!");
      return false;
    }

    sendAT(GF("+QMTDISC="), tcpconnectId);
    for (uint32_t start = millis(); millis() - start < 75000L;) {
      if (waitResponse(2000L, GF(GSM_NL "+QMTDISC:")) != 1) {
        continue;
      } else {
        // connection ID
        streamSkipUntil(','); // instead of int8_t mux = streamGetIntBefore(','); // UNUSED
        // read the result (0 if ok, -1 if bad)
        int8_t result = streamGetIntBefore('\n');
        if (result == 0) {
          return true;
        } else {
          return false;
        }
      }
    }
    return false;
  }
  bool closeMqttImpl(uint8_t connectId)
  {
    // From TCP Docu of Bc66
    // <connectID> aka tcpconnectId - Integer type. Socket service index. The range is 0-4.
    uint8_t tcpconnectId = connectId;
    if(tcpconnectId > 4)
    {
      DBG("TCPCONNECTID OUT OF RANGE!");
      return false;
    }

    sendAT(GF("+QMTCLOSE="), tcpconnectId);
    for (uint32_t start = millis(); millis() - start < 75000L;) {
      if (waitResponse(2000L, GF(GSM_NL "+QMTCLOSE:")) != 1) {
        continue;
      } else {
        // connection ID
        streamSkipUntil(','); // instead of int8_t mux = streamGetIntBefore(','); // UNUSED
        // read the result (0 if ok, -1 if bad)
        int8_t result = streamGetIntBefore('\n');
        if (result == 0) {
          return true;
        } else {
          return false;
        }
      }
    }
    return false;
  }

  bool subscribeMqttImpl(uint8_t connectId, uint16_t msgId, String topic, uint8_t qos = 0)
  {
    // From TCP Docu of Bc66
    // <connectID> aka tcpconnectId - Integer type. Socket service index. The range is 0-4.
    // <msgID> Integer type. Message identifier of packet. The range is 1-65535
    // <qos> Integer type. The QoS level at which the client wants to publish the messages (0 at most once, 1 at least once, 2 exactly once)
    uint8_t tcpconnectId = connectId;
    if(tcpconnectId > 4 || msgId < 1 || qos > 2)
    {
      DBG("TCPCONNECTID/MSGID/QOS OUT OF RANGE!");
      return false;
    }
    sendAT(GF("+QMTSUB="), tcpconnectId, GF(","), msgId, GF(",\""), topic.c_str(), GF("\","), qos);
    if (waitResponse(5000L) != 1) { return false; }

    for (uint32_t start = millis(); millis() - start < 75000L;) {
      if (waitResponse(2000L, GF(GSM_NL "+QMTSUB:")) != 1) {
        continue;
      } else {
        // connection ID
        streamSkipUntil(','); // instead of int8_t mux = streamGetIntBefore(','); // UNUSED
        // msgId
        streamSkipUntil(','); // unused
        // read the result (0 if ok, 1 or 2 if bad)
        int8_t result = streamGetIntBefore('\n');
        if (result == 0) {
          return true;
        } else {
          return false;
        }
      }
    }

    return false;
  }

  bool unsubscribeMqttImpl(uint8_t connectId, uint16_t msgId, String topic)
  {
    // From TCP Docu of Bc66
    // <connectID> aka tcpconnectId - Integer type. Socket service index. The range is 0-4.
    // <msgID> Integer type. Message identifier of packet. The range is 1-65535
    uint8_t tcpconnectId = connectId;
    if(tcpconnectId > 4 || msgId < 1)
    {
      DBG("TCPCONNECTID/MSGID OUT OF RANGE!");
      return false;
    }
    sendAT(GF("+QMTUNS="), tcpconnectId, GF(","), msgId, GF(",\""), topic.c_str(), GF("\""));
    if (waitResponse(5000L) != 1) { return false; }

    for (uint32_t start = millis(); millis() - start < 75000L;) {
      if (waitResponse(2000L, GF(GSM_NL "+QMTUNS:")) != 1) {
        continue;
      } else {
        // connection ID
        streamSkipUntil(','); // instead of int8_t mux = streamGetIntBefore(','); // UNUSED
        // msgId
        streamSkipUntil(','); // unused
        // read the result (0 if ok, 1 or 2 if bad)
        int8_t result = streamGetIntBefore('\n');
        if (result == 0) {
          return true;
        } else {
          return false;
        }
      }
    }

    return false;
  }

  bool publishMqttImpl(uint8_t connectId, uint16_t msgId, String topic, String msg, uint8_t qos = 0, uint8_t retain = 0)
  {
    // From TCP Docu of Bc66
    // <connectID> aka tcpconnectId - Integer type. Socket service index. The range is 0-4.
    // <msgID> Integer type. Message identifier of packet. The range is 1-65535
    // <qos> Integer type. The QoS level at which the client wants to publish the messages (0 at most once, 1 at least once, 2 exactly once)
    // <retain> Integer type. Whether or not the server will retain the message after it has been delivered to the current subscribers. (0 server will not retain mgs, 1 will retain the message)
    uint8_t tcpconnectId = connectId;
    uint16_t tmsgId = msgId;
    uint8_t tqos = qos;
    if(tcpconnectId > 4 || tmsgId < 1 || tqos > 2 || retain > 1)
    {
      DBG("TCPCONNECTID/MSGID/QOS/RETAIN OUT OF RANGE!");
      return false;
    }

    // need to fix
    if(tqos == 0)
      tmsgId = 0;

    sendAT(GF("+QMTPUB="), tcpconnectId, GF(","), tmsgId, GF(","), tqos, GF(","), retain, GF(",\""), topic.c_str(), GF("\",\""),
      msg.c_str(), GF("\""));
    if (waitResponse(5000L) != 1) { return false; }

    for (uint32_t start = millis(); millis() - start < 75000L;) {
      if (waitResponse(2000L, GF(GSM_NL "+QMTPUB:")) != 1) {
        continue;
      } else {
        // connection ID
        streamSkipUntil(','); // instead of int8_t mux = streamGetIntBefore(','); // UNUSED
        // msgId
        streamSkipUntil(','); // unused
        // read the result (0 if ok, 1 or 2 if bad)
        int8_t result = streamGetIntBefore('\n');
        if (result == 0) {
          return true;
        } else {
          return false;
        }
      }
    }
    return false;
  }

  void setRecvCallbackMqttImpl(MQTT_CALLBACK_SIGNATURE_T callback)
  {
    m_cb = callback;
  }


  inline bool streamReadUntil(String& strRead, const char c, const uint32_t timeout_ms = 1000L) {
    uint32_t startMillis = millis();
    bool open = false;
    while (millis() - startMillis < timeout_ms) {
      while (millis() - startMillis < timeout_ms &&
        !stream.available()) {
        TINY_GSM_YIELD();
      }
      const char read = stream.read();

      // toggle exit
      if(read == '"') {
        open = !open;
        continue;
      }

      if (read == c && !open)
      {
        return true;
      }
      strRead += read;
    }
    return false;
  }

  void loopMqttImpl()
  {
    for (uint32_t start = millis(); millis() - start < 1000L;) {
      if (waitResponse(1000L, GF(GSM_NL "+QMTRECV:")) != 1) {
        continue;
      } else {
        // connection ID
        streamSkipUntil(','); // instead of int8_t mux = streamGetIntBefore(','); // UNUSED
        // msgId
        streamSkipUntil(','); // unused
        // topic
        String topic;
        streamReadUntil(topic,',');
        // message
        String msg;
        streamReadUntil(msg,'\n');

        if(m_cb) {
          int len = msg.length();
          uint8_t payload[1024];
          msg.getBytes(payload, len);
          m_cb(const_cast<char*>(topic.c_str()), reinterpret_cast<uint8_t*>(payload), len);
        }
      }
    }
  }

  /*
   * Utilities
   */
 public:
  // TODO(vshymanskyy): Optimize this!
  int8_t waitResponse(uint32_t timeout_ms, String& data,
                      GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL, GsmConstStr r6 = NULL) {
    /*String r1s(r1); r1s.trim();
    String r2s(r2); r2s.trim();
    String r3s(r3); r3s.trim();
    String r4s(r4); r4s.trim();
    String r5s(r5); r5s.trim();
    String r6s(r6); r6s.trim();
    DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s, ",", r6s);*/
    data.reserve(64);
    uint8_t  index       = 0;
    uint32_t startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (stream.available() > 0) {
        TINY_GSM_YIELD();
        int8_t a = stream.read();
        if (a <= 0) continue;  // Skip 0x00 bytes, just in case
        data += static_cast<char>(a);
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
          index = 2;
          goto finish;
        } else if (r3 && data.endsWith(r3)) {
#if defined TINY_GSM_DEBUG
          if (r3 == GFP(GSM_CME_ERROR)) {
            streamSkipUntil('\n');  // Read out the error
          }
#endif
          index = 3;
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (r6 && data.endsWith(r6)) {
          index = 6;
          goto finish;
          /*
        } else if (data.endsWith(
                       GF(GSM_NL "+QIRD:"))) {  // TODO(?):  QIRD? or QIRDI?
          // Serial.print("\nQUIRDI ANSWER...\n");
          // +QIRDI: <id>,<sc>,<sid>,<num>,<len>,< tlen>
          streamSkipUntil(',');  // Skip the context
          streamSkipUntil(',');  // Skip the role
          // read the connection id
          int8_t mux = streamGetIntBefore(',');
          // read the number of packets in the buffer
          int8_t num_packets = streamGetIntBefore(',');
          // read the length of the current packet
          streamSkipUntil(
              ',');  // Skip the length of the current package in the buffer
          int16_t len_total =
              streamGetIntBefore('\n');  // Total length of all packages
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux] &&
              num_packets >= 0 && len_total >= 0) {
            sockets[mux]->sock_available = len_total;
          }
          data = "";
          // DBG("### Got Data:", len_total, "on", mux);
        } else if (data.endsWith(GF("CLOSED" GSM_NL))) {
          int8_t nl   = data.lastIndexOf(GSM_NL, data.length() - 8);
          int8_t coma = data.indexOf(',', nl + 2);
          int8_t mux  = data.substring(nl + 2, coma).toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### Closed: ", mux);*/
        }
        else if (data.endsWith(GF("+QNTP:"))) {
          streamSkipUntil(',');
          // read time...
          String timestrng;
          streamReadUntil(timestrng,'\r');
          processTimeStrng(timestrng);
          DBG("### Network time updated.");
          data = "";
        }
        else if (data.endsWith(GF("+QIURC: "))) {
          String inf;
          streamReadUntil(inf,',');
          //Serial.println("INF = ");
          //Serial.println(inf.c_str());
          if(inf.compareTo("recv") == 0)
          {
            int8_t mux = streamGetIntBefore(',');
            int8_t szpkt = streamGetIntBefore('\n'); // this one is the mux
            // DBG("### DATA received.");
            //modemRead(szpkt, mux);
            if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
              // We have no way of knowing how much data actually came in, so
              // we set the value to 1500, the maximum possible size.
              sockets[mux]->sock_available = 1500;
            }
          } else if(inf.compareTo("closed") == 0)
          {
            int8_t mux = streamGetIntBefore('\n');
            // DBG("### CLOSED CONNECTION received.");
          }
          //sendAT(GF("+QIRD="), mux, GF(","), szpkt);
          //if (waitResponse() != 1) { return false; }
          data = "";
        }
      }
    } while (millis() - startMillis < timeout_ms);
  finish:
    if (!index) {
      data.trim();
      if (data.length()) { DBG("### Unhandled: ", data); }
      data = "";
    }
    // data.replace(GSM_NL, "/");
    // DBG('<', index, '>', data);
    return index;
  }

  int8_t waitResponse(uint32_t timeout_ms, GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL, GsmConstStr r6 = NULL) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5, r6);
  }

  int8_t waitResponse(GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL, GsmConstStr r6 = NULL) {
    return waitResponse(1000, r1, r2, r3, r4, r5, r6);
  }

  bool processTimeStrng(String timestrng)
  {
    //Serial.println("TIMESTRNG: ");
    // Serial.println(timestrng.c_str());
    struct tm mdt;
    int tz;
    sscanf(timestrng.c_str(),"%02d/%02d/%02d,%02d:%02d:%02d+%02d",
      &(mdt.tm_year), &(mdt.tm_mon), &(mdt.tm_mday), &(mdt.tm_hour), &(mdt.tm_min), &(mdt.tm_sec), &tz);
    mdt.tm_year += 100; mdt.tm_mon -= 1;
    // set chip date/tz

    //Serial.printf("--- %s ---\n",timestrng.c_str());

    sendAT(GF("+CCLK="), GF("\""), timestrng.c_str(), GF("\""));
    if(waitResponse(1000L) == 1) {
      m_dt = mktime ( &mdt );
      //Serial.printf("SETTING CLOCK: %d %d %d %d %d %d\n", mdt.tm_year, mdt.tm_mon, mdt.tm_mday, mdt.tm_hour, mdt.tm_min, mdt.tm_sec);
      return true;
    }
    return false;
  }

 public:
  Stream& stream;

 // BC66 specific internals
 private:
  IPAddress                   m_localIP;
  String                      m_sslServerCertificate;
  MQTT_CALLBACK_SIGNATURE_T   m_cb;
  time_t                      m_dt;

 protected:
  GsmClientBC66* sockets[TINY_GSM_MUX_COUNT];
  const char*    gsmNL = GSM_NL;
};

#endif  // SRC_TINYGSMCLIENTBC66_H_
