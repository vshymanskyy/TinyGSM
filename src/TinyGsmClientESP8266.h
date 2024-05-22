/**
 * @file       TinyGsmClientESP8266.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTESP8266_H_
#define SRC_TINYGSMCLIENTESP8266_H_
// #pragma message("TinyGSM:  TinyGsmClientESP8266")

// #define TINY_GSM_DEBUG Serial

#define TINY_GSM_MUX_COUNT 5
#define TINY_GSM_NO_MODEM_BUFFER
#ifdef AT_NL
#undef AT_NL
#endif
#define AT_NL "\r\n"

#ifdef MODEM_MANUFACTURER
#undef MODEM_MANUFACTURER
#endif
#define MODEM_MANUFACTURER "Espressif"

#ifdef MODEM_MODEL
#undef MODEM_MODEL
#endif
#if defined(TINY_GSM_MODEM_ESP8266)
#define MODEM_MODEL "ESP8266"
#else
#define MODEM_MODEL "ESP32"
#endif

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmWifi.tpp"

static uint8_t TINY_GSM_TCP_KEEP_ALIVE = 120;

// <stat> status of ESP8266 station interface
// 0: ESP8266 station is not initialized.
// 1: ESP8266 station is initialized, but not started a Wi-Fi connection yet.
// 2 : ESP8266 station connected to an AP and has obtained IP
// 3 : ESP8266 station created a TCP or UDP transmission
// 4 : the TCP or UDP transmission of ESP8266 station disconnected
// 5 : ESP8266 station did NOT connect to an AP
enum ESP8266RegStatus {
  REG_UNINITIALIZED = 0,
  REG_UNREGISTERED  = 1,
  REG_OK_IP         = 2,
  REG_OK_TCP        = 3,
  REG_OK_NO_TCP     = 4,
  REG_DENIED        = 5,
  REG_UNKNOWN       = 6,
};

class TinyGsmESP8266 : public TinyGsmModem<TinyGsmESP8266>,
                       public TinyGsmTCP<TinyGsmESP8266, TINY_GSM_MUX_COUNT>,
                       public TinyGsmSSL<TinyGsmESP8266, TINY_GSM_MUX_COUNT>,
                       public TinyGsmWifi<TinyGsmESP8266> {
  friend class TinyGsmModem<TinyGsmESP8266>;
  friend class TinyGsmTCP<TinyGsmESP8266, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmESP8266, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmWifi<TinyGsmESP8266>;

  /*
   * Inner Client
   */
 public:
  class GsmClientESP8266 : public GsmClient {
    friend class TinyGsmESP8266;

   public:
    GsmClientESP8266() {}

    explicit GsmClientESP8266(TinyGsmESP8266& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmESP8266* modem, uint8_t mux = 0) {
      this->at       = modem;
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
      TINY_GSM_YIELD();
      at->sendAT(GF("+CIPCLOSE="), mux);
      sock_connected = false;
      at->waitResponse(maxWaitMs);
      rx.clear();
    }
    void stop() override {
      stop(5000L);
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
  class GsmClientSecureESP8266 : public GsmClientESP8266 {
   public:
    GsmClientSecureESP8266() {}

    explicit GsmClientSecureESP8266(TinyGsmESP8266& modem, uint8_t mux = 0)
        : GsmClientESP8266(modem, mux) {}

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

  /*
   * Constructor
   */
 public:
  explicit TinyGsmESP8266(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = nullptr) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientESP8266"));

    if (!testAT()) { return false; }
    if (pin && strlen(pin) > 0) {
      DBG("ESP8266 modules do not use an unlock pin!");
    }
    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }
    sendAT(GF("+CIPMUX=1"));  // Enable Multiple Connections
    if (waitResponse() != 1) { return false; }
    sendAT(GF("+CWMODE=1"));  // Put into "station" mode
    if (waitResponse() != 1) {
      sendAT(GF("+CWMODE_CUR=1"));  // Attempt "current" station mode command
                                    // for some firmware variants if needed
      if (waitResponse() != 1) { return false; }
    }
    DBG(GF("### Modem:"), getModemName());
    return true;
  }

  bool setBaudImpl(uint32_t baud) {
    sendAT(GF("+UART_CUR="), baud, "8,1,0,0");
    if (waitResponse() != 1) {
      sendAT(GF("+UART="), baud,
             "8,1,0,0");  // Really old firmwares might need this
      // if (waitResponse() != 1) {
      //   sendAT(GF("+IPR="), baud);  // First release firmwares might need
      //   this
      return waitResponse() == 1;
      // }
    }
    return false;
  }

  String getModemInfoImpl() {
    sendAT(GF("+GMR"));
    String res;
    if (waitResponse(1000L, res) != 1) { return ""; }
    cleanResponseString(res);
    return res;
  }

  // Gets the modem hardware version
  String getModemManufacturerImpl() {
    return MODEM_MANUFACTURER;
  }

  // Gets the modem hardware version
  String getModemModelImpl() {
    String model = MODEM_MODEL;
    sendAT(GF("+GMR"));
    streamSkipUntil('\n');  // skip the AT version
    streamSkipUntil('\n');  // skip the SDK version
    streamSkipUntil('\n');  // skip the compile time
    // read the hardware from the Bin version
    streamSkipUntil('(');  // skip the text "Bin version"
    String wroom = stream.readStringUntil(
        ')');                        // read the WRoom version in the parethesis
    streamSkipUntil('(');            // skip the bin version itself
    if (waitResponse(1000L) == 1) {  // wait for the ending OK
      return wroom;
    }
    return model;
  }

  // Gets the modem firmware version
  String getModemRevisionImpl() {
    sendAT(GF("GMR"));  // GMR instead of CGMR
    String res;
    if (waitResponse(1000L, res) != 1) { return ""; }
    cleanResponseString(res);
    return res;
  }

  // Gets the modem serial number
  String getModemSerialNumberImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  bool factoryDefaultImpl() {
    sendAT(GF("+RESTORE"));
    return waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = nullptr) {
    if (!testAT()) { return false; }
    sendAT(GF("+RST"));
    if (waitResponse(10000L) != 1) { return false; }
    if (waitResponse(10000L, GF(AT_NL "ready" AT_NL)) != 1) { return false; }
    delay(500);
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+GSLP=0"));  // Power down indefinitely - until manually reset!
    return waitResponse() == 1;
  }

  bool radioOffImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sleepEnableImpl(bool enable = true) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false)
      TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 public:
  ESP8266RegStatus getRegistrationStatus() {
    sendAT(GF("+CIPSTATUS"));
    if (waitResponse(3000, GF("STATUS:")) != 1) return REG_UNKNOWN;
    // after "STATUS:" it should return the status number (0,1,2,3,4,5),
    // followed by an OK
    // Since there are more possible status number codes than the arguments for
    // waitResponse, we'll capture the response in a string and then parse it.
    String res;
    if (waitResponse(3000L, res) != 1) { return REG_UNKNOWN; }
    res.trim();
    int8_t status = res.toInt();
    return (ESP8266RegStatus)status;
  }

 protected:
  int8_t getSignalQualityImpl() {
    sendAT(GF("+CWJAP?"));
    int8_t res1 = waitResponse(GF("No AP"), GF("+CWJAP:"));
    if (res1 != 2) {
      waitResponse();
      sendAT(GF("+CWJAP_CUR?"));  // attempt "current" as used by some firmware
                                  // versions
      int8_t res1 = waitResponse(GF("No AP"), GF("+CWJAP_CUR:"));
      if (res1 != 2) {
        waitResponse();
        return 0;
      }
    }
    streamSkipUntil(',');             // Skip SSID
    streamSkipUntil(',');             // Skip BSSID/MAC address
    streamSkipUntil(',');             // Skip Chanel number
    int8_t res2 = stream.parseInt();  // Read RSSI
    waitResponse();                   // Returns an OK after the value
    return res2;
  }

  bool isNetworkConnectedImpl() {
    ESP8266RegStatus s = getRegistrationStatus();
    if (s == REG_OK_IP || s == REG_OK_TCP) {
      // with these, we're definitely connected
      return true;
    } else if (s == REG_OK_NO_TCP) {
      // with this, we may or may not be connected
      if (getLocalIP() == "") {
        return false;
      } else {
        return true;
      }
    } else {
      return false;
    }
  }

  String getLocalIPImpl() {
    // attempt with and without 'current' flag
    sendAT(GF("+CIPSTA?"));
    int8_t res1 = waitResponse(GF("ERROR"), GF("+CIPSTA:"));
    if (res1 != 2) {
      sendAT(GF("+CIPSTA_CUR?"));
      res1 = waitResponse(GF("ERROR"), GF("+CIPSTA_CUR:"));
      if (res1 != 2) { return ""; }
    }
    String res2 = stream.readStringUntil('\n');
    res2.replace("ip:", "");  // newer firmwares have this
    res2.replace("\"", "");
    res2.trim();
    waitResponse();
    return res2;
  }

  /*
   * Secure socket layer (SSL) functions
   */
  // Follows functions as inherited from TinyGsmSSL.tpp


  /*
   * WiFi functions
   */
 protected:
  bool networkConnectImpl(const char* ssid, const char* pwd) {
    // attempt first without than with the 'current' flag used in some firmware
    // versions
    sendAT(GF("+CWJAP=\""), ssid, GF("\",\""), pwd, GF("\""));
    if (waitResponse(30000L, GFP(GSM_OK), GF(AT_NL "FAIL" AT_NL)) != 1) {
      sendAT(GF("+CWJAP_CUR=\""), ssid, GF("\",\""), pwd, GF("\""));
      if (waitResponse(30000L, GFP(GSM_OK), GF(AT_NL "FAIL" AT_NL)) != 1) {
        return false;
      }
    }

    return true;
  }

  bool networkDisconnectImpl() {
    sendAT(GF("+CWQAP"));
    bool retVal = waitResponse(10000L) == 1;
    waitResponse(GF("WIFI DISCONNECT"));
    return retVal;
  }


  /*
   * GPRS functions
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
  // No functions of this type supported

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
    if (ssl) {
      sendAT(GF("+CIPSSLSIZE=4096"));
      waitResponse();
    }
    sendAT(GF("+CIPSTART="), mux, ',', ssl ? GF("\"SSL") : GF("\"TCP"),
           GF("\",\""), host, GF("\","), port, GF(","),
           TINY_GSM_TCP_KEEP_ALIVE);
    // TODO(?): Check mux
    int8_t rsp = waitResponse(timeout_ms, GFP(GSM_OK), GFP(GSM_ERROR),
                              GF("ALREADY CONNECT"));
    // if (rsp == 3) waitResponse();
    // May return "ERROR" after the "ALREADY CONNECT"
    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();
    if (waitResponse(10000L, GF(AT_NL "SEND OK" AT_NL)) != 1) { return 0; }
    return len;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+CIPSTATUS"));
    if (waitResponse(3000, GF("STATUS:")) != 1) { return false; }
    // after "STATUS:" it should return the status number (0,1,2,3,4,5),
    // followed by an OK
    // Hopefully we'll catch the "3" here, but fall back to the OK or Error
    int8_t status = waitResponse(GF("3"), GFP(GSM_OK), GFP(GSM_ERROR));
    // if the status is anything but 3, there are no connections open
    if (status != 1) {
      for (int muxNo = 0; muxNo < TINY_GSM_MUX_COUNT; muxNo++) {
        if (sockets[muxNo]) { sockets[muxNo]->sock_connected = false; }
      }
      return false;
    }
    bool verified_connections[TINY_GSM_MUX_COUNT] = {0, 0, 0, 0, 0};
    for (int muxNo = 0; muxNo < TINY_GSM_MUX_COUNT; muxNo++) {
      uint8_t has_status = waitResponse(GF("+CIPSTATUS:"), GFP(GSM_OK),
                                        GFP(GSM_ERROR));
      if (has_status == 1) {
        int8_t returned_mux = streamGetIntBefore(',');
        streamSkipUntil(',');   // Skip mux
        streamSkipUntil(',');   // Skip type
        streamSkipUntil(',');   // Skip remote IP
        streamSkipUntil(',');   // Skip remote port
        streamSkipUntil(',');   // Skip local port
        streamSkipUntil('\n');  // Skip client/server type
        verified_connections[returned_mux] = 1;
      }
      if (has_status == 2) break;  // once we get to the ok, stop
    }
    for (int muxNo = 0; muxNo < TINY_GSM_MUX_COUNT; muxNo++) {
      if (sockets[muxNo]) {
        sockets[muxNo]->sock_connected = verified_connections[muxNo];
      }
    }
    return verified_connections[mux];
  }

  /*
   * Utilities
   */
 public:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+IPD,"))) {
      int8_t  mux      = streamGetIntBefore(',');
      int16_t len      = streamGetIntBefore(':');
      int16_t len_orig = len;
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        if (len > sockets[mux]->rx.free()) {
          DBG("### Buffer overflow: ", len, "->", sockets[mux]->rx.free());
          // reset the len to read to the amount free
          len = sockets[mux]->rx.free();
        }
        while (len--) { moveCharFromStreamToFifo(mux); }
        // TODO(SRGDamia1): deal with buffer overflow/missed characters
        if (len_orig != sockets[mux]->available()) {
          DBG("### Different number of characters received than expected: ",
              sockets[mux]->available(), " vs ", len_orig);
        }
      }
      data = "";
      DBG("### Got Data: ", len_orig, "on", mux);
      return true;
    } else if (data.endsWith(GF("CLOSED"))) {
      int8_t muxStart = TinyGsmMax(0,
                                   data.lastIndexOf(AT_NL, data.length() - 8));
      int8_t coma     = data.indexOf(',', muxStart);
      int8_t mux      = data.substring(muxStart, coma).toInt();
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        sockets[mux]->sock_connected = false;
      }
      streamSkipUntil('\n');  // throw away the new line
      data = "";
      DBG("### Closed: ", mux);
      return true;
    }
    return false;
  }

 public:
  Stream& stream;

 protected:
  GsmClientESP8266* sockets[TINY_GSM_MUX_COUNT];
};

#endif  // SRC_TINYGSMCLIENTESP8266_H_
