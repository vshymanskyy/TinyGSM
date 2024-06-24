/**
 * @file       TinyGsmClientM95.h
 * @author     Volodymyr Shymanskyy, Pacman Pereira, and Replicade Ltd.
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy, (c)2017 Replicade Ltd.
 * <http://www.replicade.com>
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTM95_H_
#define SRC_TINYGSMCLIENTM95_H_
// #pragma message("TinyGSM:  TinyGsmClientM95")

// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MUX_COUNT 6
#define TINY_GSM_BUFFER_READ_NO_CHECK
#ifdef AT_NL
#undef AT_NL
#endif
#define AT_NL "\r\n"

#ifdef MODEM_MANUFACTURER
#undef MODEM_MANUFACTURER
#endif
#define MODEM_MANUFACTURER "Quectel"

#ifdef MODEM_MODEL
#undef MODEM_MODEL
#endif
#define MODEM_MODEL "M95"

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmBattery.tpp"
#include "TinyGsmTemperature.tpp"

enum M95RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};

class TinyGsmM95 : public TinyGsmModem<TinyGsmM95>,
                   public TinyGsmGPRS<TinyGsmM95>,
                   public TinyGsmTCP<TinyGsmM95, TINY_GSM_MUX_COUNT>,
                   public TinyGsmCalling<TinyGsmM95>,
                   public TinyGsmSMS<TinyGsmM95>,
                   public TinyGsmTime<TinyGsmM95>,
                   public TinyGsmBattery<TinyGsmM95>,
                   public TinyGsmTemperature<TinyGsmM95> {
  friend class TinyGsmModem<TinyGsmM95>;
  friend class TinyGsmGPRS<TinyGsmM95>;
  friend class TinyGsmTCP<TinyGsmM95, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmCalling<TinyGsmM95>;
  friend class TinyGsmSMS<TinyGsmM95>;
  friend class TinyGsmTime<TinyGsmM95>;
  friend class TinyGsmBattery<TinyGsmM95>;
  friend class TinyGsmTemperature<TinyGsmM95>;

  /*
   * Inner Client
   */
 public:
  class GsmClientM95 : public GsmClient {
    friend class TinyGsmM95;

   public:
    GsmClientM95() {}

    explicit GsmClientM95(TinyGsmM95& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmM95* modem, uint8_t mux = 0) {
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
      at->waitResponse((maxWaitMs - (millis() - startMillis)), GF("CLOSED"),
                       GF("CLOSE OK"), GF("ERROR"));
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
  // NOT SUPPORTED

  /*
   * Constructor
   */
 public:
  explicit TinyGsmM95(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = nullptr) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientM95"));

    if (!testAT()) { return false; }

    // sendAT(GF("&FZ"));  // Factory + Reset
    // waitResponse();

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

    // Enable network time synchronization
    sendAT(GF("+QNITZ=1"));
    if (waitResponse(10000L) != 1) { return false; }

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
 protected:
  bool restartImpl(const char* pin = nullptr) {
    if (!testAT()) { return false; }
    sendAT(GF("+CFUN=0"));
    if (waitResponse(10000L, GF("NORMAL POWER DOWN"), GF("OK"), GF("FAIL")) ==
        3) {
      return false;
    }
    sendAT(GF("+CFUN=1"));
    if (waitResponse(10000L, GF("Call Ready"), GF("OK"), GF("FAIL")) == 3) {
      return false;
    }
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+QPOWD=1"));
    return waitResponse(300, GF("NORMAL POWER DOWN")) == 1;
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

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false)
      TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 public:
  M95RegStatus getRegistrationStatus() {
    return (M95RegStatus)getRegistrationStatusXREG("CREG");
  }

 protected:
  bool isNetworkConnectedImpl() {
    M95RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  void setHostFormat(bool useDottedQuad) {
    if (useDottedQuad) {
      sendAT(GF("+QIDNSIP=0"));
    } else {
      sendAT(GF("+QIDNSIP=1"));
    }
    waitResponse();
  }

  String getLocalIPImpl() {
    sendAT(GF("+QILOCIP"));
    streamSkipUntil('\n');
    String res = stream.readStringUntil('\n');
    res.trim();
    return res;
  }

  /*
   * Secure socket layer (SSL) functions
   */
  // No functions of this type supported

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

    // select foreground context 0 = VIRTUAL_UART_1
    sendAT(GF("+QIFGCNT=0"));
    if (waitResponse() != 1) { return false; }

    // Select GPRS (=1) as the Bearer
    sendAT(GF("+QICSGP=1,\""), apn, GF("\",\""), user, GF("\",\""), pwd,
           GF("\""));
    if (waitResponse() != 1) { return false; }

    // Select TCP/IP transfer mode - NOT transparent mode
    sendAT(GF("+QIMODE=0"));
    if (waitResponse() != 1) { return false; }

    // Enable multiple TCP/IP connections
    sendAT(GF("+QIMUX=1"));
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
    // Mode = 1 - Output a notification when data is received
    // +QIRDI: <id>,<sc>,<sid>
    sendAT(GF("+QINDI=1"));
    if (waitResponse() != 1) { return false; }

    // // Request an IP header for received data
    // // "IPD(data length):"
    // sendAT(GF("+QIHEAD=1"));
    // if (waitResponse() != 1) {
    //   return false;
    // }
    //
    // // Do NOT show the IP address of the sender when receiving data
    // // The format to show the address is: RECV FROM: <IP ADDRESS>:<PORT>
    // sendAT(GF("+QISHOWRA=0"));
    // if (waitResponse() != 1) {
    //   return false;
    // }
    //
    // // Do NOT show the protocol type at the end of the header for received
    // data
    // // IPD(data length)(TCP/UDP):
    // sendAT(GF("+QISHOWPT=0"));
    // if (waitResponse() != 1) {
    //   return false;
    // }
    //
    // // Do NOT show the destination address before receiving data
    // // The format to show the address is: TO:<IP ADDRESS>
    // sendAT(GF("+QISHOWLA=0"));
    // if (waitResponse() != 1) {
    //   return false;
    // }

    return true;
  }

  bool gprsDisconnectImpl() {
    sendAT(GF("+QIDEACT"));  // Deactivate the bearer context
    return waitResponse(60000L, GF("DEACT OK"), GF("ERROR")) == 1;
  }

  String getProviderImpl() {
    sendAT(GF("+QSPN?"));
    if (waitResponse(GF("+QSPN:")) != 1) { return ""; }
    streamSkipUntil('"');                      // Skip mode and format
    String res = stream.readStringUntil('"');  // read the provider
    waitResponse();                            // skip anything else
    return res;
  }

  /*
   * SIM card functions
   */
 protected:
  String getSimCCIDImpl() {
    sendAT(GF("+QCCID"));
    if (waitResponse(GF(AT_NL "+QCCID:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone Call functions
   */
  // Follows all phone call functions as inherited from TinyGsmCalling.tpp


  /*
   * Audio functions
   */
  // No functions of this type supported
  /*
   * Text messaging (SMS) functions
   */
  // Follows all text messaging (SMS) functions as inherited from TinyGsmSMS.tpp

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
   * GSM Location functions
   */
  // No functions of this type supported

  /*
   * GPS/GNSS/GLONASS location functions
   */
  // No functions of this type supported

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
  // Follows all battery functions as inherited from TinyGsmBattery.tpp

  /*
   * Temperature functions
   */
 protected:
  float getTemperatureImpl() {
    sendAT(GF("+QTEMP?"));
    if (waitResponse(GF(AT_NL "+QTEMP:")) != 1) {
      return static_cast<float>(-9999);
    }
    streamSkipUntil(',');  // Skip mode
    // Read charge of thermistor
    // milliVolts = streamGetIntBefore(',');
    streamSkipUntil(',');  // Skip thermistor charge
    float temp = streamGetFloatBefore('\n');
    // Wait for final OK
    waitResponse();
    return temp;
  }

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
    if (ssl) { DBG("SSL not yet supported on this module!"); }
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    sendAT(GF("+QIOPEN="), mux, GF(",\""), GF("TCP"), GF("\",\""), host,
           GF("\","), port);
    int8_t rsp = waitResponse(timeout_ms, GF("CONNECT OK" AT_NL),
                              GF("CONNECT FAIL" AT_NL),
                              GF("ALREADY CONNECT" AT_NL));
    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+QISEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(GF(AT_NL "SEND OK")) != 1) { return 0; }

    // bool allAcknowledged = false;
    // // bool failed = false;
    // while ( !allAcknowledged ) {
    //   sendAT( GF("+QISACK"));
    //   if (waitResponse(5000L, GF(AT_NL "+QISACK:")) != 1) {
    //     return -1;
    //   } else {
    //     streamSkipUntil(',');  // Skip total length sent on connection
    //     streamSkipUntil(',');  // Skip length already acknowledged by remote
    //     // Make sure the total length un-acknowledged is 0
    //     if ( streamGetIntBefore('\n') == 0 ) {
    //       allAcknowledged = true;
    //     }
    //   }
    // }
    // waitResponse(5000L);

    return len;  // TODO(?): get len/ack properly
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) return 0;
    // TODO(?):  Does this work????
    // AT+QIRD=<id>,<sc>,<sid>,<len>
    // id = GPRS context number = 0, set in GPRS connect
    // sc = role in connection = 1, client of connection
    // sid = index of connection = mux
    // len = maximum length of data to retrieve
    sendAT(GF("+QIRD=0,1,"), mux, ',', (uint16_t)size);
    // If it replies only OK for the write command, it means there is no
    // received data in the buffer of the connection.
    int8_t res = waitResponse(GF("+QIRD:"), GFP(GSM_OK), GFP(GSM_ERROR));
    if (res == 1) {
      streamSkipUntil(':');  // skip IP address
      streamSkipUntil(',');  // skip port
      streamSkipUntil(',');  // skip connection type (TCP/UDP)
      // read the real length of the retrieved data
      uint16_t len = streamGetIntBefore('\n');
      // We have no way of knowing in advance how much data will be in the
      // buffer so when data is received we always assume the buffer is
      // completely full. Chances are, this is not true and there's really not
      // that much there. In that case, make sure we make sure we re-set the
      // amount of data available.
      if (len < size) { sockets[mux]->sock_available = len; }
      for (uint16_t i = 0; i < len; i++) {
        moveCharFromStreamToFifo(mux);
        sockets[mux]->sock_available--;
        // ^^ One less character available after moving from modem's FIFO to our
        // FIFO
      }
      waitResponse();  // ends with an OK
      // DBG("### READ:", len, "from", mux);
      return len;
    } else {
      sockets[mux]->sock_available = 0;
      return 0;
    }
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
   * Utilities
   */
 public:
  bool handleURCs(String& data) {
    if (data.endsWith(GF(AT_NL "+QIRDI:"))) {
      streamSkipUntil(',');  // Skip the context
      streamSkipUntil(',');  // Skip the role
      int8_t mux = streamGetIntBefore('\n');
      // DBG("### Got Data:", mux);
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        // We have no way of knowing how much data actually came in, so
        // we set the value to 1500, the maximum possible size.
        sockets[mux]->sock_available = 1500;
      }
      data = "";
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
    } else if (data.endsWith(GF("+QNITZ:"))) {
      streamSkipUntil('\n');  // URC for time sync
      data = "";
      DBG("### Network time updated.");
      return true;
    }
    return false;
  }

 public:
  Stream& stream;

 protected:
  GsmClientM95* sockets[TINY_GSM_MUX_COUNT];
};

#endif  // SRC_TINYGSMCLIENTM95_H_
