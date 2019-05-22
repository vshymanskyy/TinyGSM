/**
 * @file       TinyGsmClientESP8266.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClientESP8266_h
#define TinyGsmClientESP8266_h
//#pragma message("TinyGSM:  TinyGsmClientESP8266")

//#define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 512
#endif

#define TINY_GSM_MUX_COUNT 5

#include <TinyGsmCommon.h>

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;
static unsigned TINY_GSM_TCP_KEEP_ALIVE = 120;

// <stat> status of ESP8266 station interface
// 2 : ESP8266 station connected to an AP and has obtained IP
// 3 : ESP8266 station created a TCP or UDP transmission
// 4 : the TCP or UDP transmission of ESP8266 station disconnected
// 5 : ESP8266 station did NOT connect to an AP
enum RegStatus {
  REG_OK_IP        = 2,
  REG_OK_TCP       = 3,
  REG_UNREGISTERED = 4,
  REG_DENIED       = 5,
  REG_UNKNOWN      = 6,
};



class TinyGsmESP8266
{

public:

class GsmClient : public Client
{
  friend class TinyGsmESP8266;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmESP8266& modem, uint8_t mux = 1) {
    init(&modem, mux);
  }

  bool init(TinyGsmESP8266* modem, uint8_t mux = 1) {
    this->at = modem;
    this->mux = mux;
    sock_connected = false;

    at->sockets[mux] = this;

    return true;
  }

public:
  virtual int connect(const char *host, uint16_t port, int timeout_s) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();
    sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
    return sock_connected;
  }

TINY_GSM_CLIENT_CONNECT_OVERLOADS()

  virtual void stop() {
    TINY_GSM_YIELD();
    at->sendAT(GF("+CIPCLOSE="), mux);
    sock_connected = false;
    at->waitResponse();
    rx.clear();
  }

TINY_GSM_CLIENT_WRITE()

TINY_GSM_CLIENT_AVAILABLE_NO_MODEM_FIFO()

TINY_GSM_CLIENT_READ_NO_MODEM_FIFO()

TINY_GSM_CLIENT_PEEK_FLUSH_CONNECTED()

  /*
   * Extended API
   */

  String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

private:
  TinyGsmESP8266* at;
  uint8_t         mux;
  bool            sock_connected;
  RxFifo          rx;
};


class GsmClientSecure : public GsmClient
{
public:
  GsmClientSecure() {}

  GsmClientSecure(TinyGsmESP8266& modem, uint8_t mux = 1)
    : GsmClient(modem, mux)
  {}

public:
  virtual int connect(const char *host, uint16_t port, int timeout_s) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();
    sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
    return sock_connected;
  }
};


public:

  TinyGsmESP8266(Stream& stream)
    : stream(stream)
  {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */

  bool begin(const char* pin = NULL) {
    return init(pin);
  }

  bool init(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    if (!testAT()) {
      return false;
    }
    sendAT(GF("E0"));   // Echo Off
    if (waitResponse() != 1) {
      return false;
    }
    sendAT(GF("+CIPMUX=1"));  // Enable Multiple Connections
    if (waitResponse() != 1) {
      return false;
    }
    sendAT(GF("+CWMODE_CUR=1"));  // Put into "station" mode
    if (waitResponse() != 1) {
      return false;
    }
    DBG(GF("### Modem:"), getModemName());
    return true;
  }

  String getModemName() {
    return "ESP8266";
  }

  void setBaud(unsigned long baud) {
    sendAT(GF("+UART_CUR="), baud, "8,1,0,0");
  }

TINY_GSM_MODEM_TEST_AT()

TINY_GSM_MODEM_MAINTAIN_LISTEN()

  bool factoryDefault() {
    sendAT(GF("+RESTORE"));
    return waitResponse() == 1;
  }

  String getModemInfo() {
    sendAT(GF("+GMR"));
    String res;
    if (waitResponse(1000L, res) != 1) {
      return "";
    }
    res.replace(GSM_NL "OK" GSM_NL, "");
    res.replace(GSM_NL, " ");
    res.trim();
    return res;
  }

  bool hasSSL() {
    return true;
  }

  bool hasWifi() {
    return true;
  }

  bool hasGPRS() {
    return false;
  }

  /*
   * Power functions
   */

  bool restart() {
    if (!testAT()) {
      return false;
    }
    sendAT(GF("+RST"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    if (waitResponse(10000L, GF(GSM_NL "ready" GSM_NL)) != 1) {
      return false;
    }
    delay(500);
    return init();
  }

  bool poweroff() {
    sendAT(GF("+GSLP=0"));  // Power down indefinitely - until manually reset!
    return waitResponse() == 1;
  }

  bool radioOff() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sleepEnable(bool enable = true) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * SIM card functions
   */

  RegStatus getRegistrationStatus() {
    sendAT(GF("+CIPSTATUS"));
    if (waitResponse(3000, GF("STATUS:")) != 1) return REG_UNKNOWN;
    int status = waitResponse(GFP(GSM_ERROR), GF("2"), GF("3"), GF("4"), GF("5"));
    waitResponse();  // Returns an OK after the status
    return (RegStatus)status;
  }

  /*
   * Generic network functions
   */

  int16_t getSignalQuality() {
    sendAT(GF("+CWJAP_CUR?"));
    int res1 = waitResponse(GF("No AP"), GF("+CWJAP_CUR:"));
    if (res1 != 2) {
      waitResponse();
      return 0;
    }
    streamSkipUntil(',');  // Skip SSID
    streamSkipUntil(',');  // Skip BSSID/MAC address
    streamSkipUntil(',');  // Skip Chanel number
    int res2 = stream.parseInt();  // Read RSSI
    waitResponse();  // Returns an OK after the value
    return res2;
  }

  bool isNetworkConnected()  {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_IP || s == REG_OK_TCP);
  }

  bool waitForNetwork(unsigned long timeout_ms = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
      sendAT(GF("+CIPSTATUS"));
      int res1 = waitResponse(3000, GF("busy p..."), GF("STATUS:"));
      if (res1 == 2) {
        int res2 = waitResponse(GFP(GSM_ERROR), GF("2"), GF("3"), GF("4"), GF("5"));
        if (res2 == 2 || res2 == 3) {
            waitResponse();
            return true;
         }
        }
      delay(250);
    }
    return false;
  }

  /*
   * WiFi functions
   */

  bool networkConnect(const char* ssid, const char* pwd) {
    sendAT(GF("+CWJAP_CUR=\""), ssid, GF("\",\""), pwd, GF("\""));
    if (waitResponse(30000L, GFP(GSM_OK), GF(GSM_NL "FAIL" GSM_NL)) != 1) {
      return false;
    }

    return true;
  }

  bool networkDisconnect() {
    sendAT(GF("+CWQAP"));
    bool retVal = waitResponse(10000L) == 1;
    waitResponse(GF("WIFI DISCONNECT"));
    return retVal;
  }

  /*
   * IP Address functions
   */

  String getLocalIP() {
    sendAT(GF("+CIPSTA_CUR??"));
    int res1 = waitResponse(GF("ERROR"), GF("+CWJAP_CUR:"));
    if (res1 != 2) {
      return "";
    }
    String res2 = stream.readStringUntil('"');
    waitResponse();
    return res2;
  }

  IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
  }

  /*
   * Battery & temperature functions
   */

  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  uint16_t getBattVoltage() TINY_GSM_ATTR_NOT_AVAILABLE;
  int8_t getBattPercent() TINY_GSM_ATTR_NOT_AVAILABLE;
  uint8_t getBattChargeState() TINY_GSM_ATTR_NOT_AVAILABLE;
  bool getBattStats(uint8_t &chargeState, int8_t &percent, uint16_t &milliVolts) TINY_GSM_ATTR_NOT_AVAILABLE;
  float getTemperature() TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Client related functions
   */

protected:

  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75)
 {
    uint32_t timeout_ms = ((uint32_t)timeout_s)*1000;
    if (ssl) {
      sendAT(GF("+CIPSSLSIZE=4096"));
      waitResponse();
    }
    sendAT(GF("+CIPSTART="), mux, ',', ssl ? GF("\"SSL") : GF("\"TCP"),
           GF("\",\""), host, GF("\","), port, GF(","), TINY_GSM_TCP_KEEP_ALIVE);
    // TODO: Check mux
    int rsp = waitResponse(timeout_ms,
                           GFP(GSM_OK),
                           GFP(GSM_ERROR),
                           GF("ALREADY CONNECT"));
    // if (rsp == 3) waitResponse();  // May return "ERROR" after the "ALREADY CONNECT"
    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', len);
    if (waitResponse(GF(">")) != 1) {
      return 0;
    }
    stream.write((uint8_t*)buff, len);
    stream.flush();
    if (waitResponse(10000L, GF(GSM_NL "SEND OK" GSM_NL)) != 1) {
      return 0;
    }
    return len;
  }

  bool modemGetConnected(uint8_t mux) {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_IP || s == REG_OK_TCP);
  }

public:

  /*
   Utilities
   */

TINY_GSM_MODEM_STREAM_UTILITIES()

  // TODO: Optimize this!
  uint8_t waitResponse(uint32_t timeout_ms, String& data,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    /*String r1s(r1); r1s.trim();
    String r2s(r2); r2s.trim();
    String r3s(r3); r3s.trim();
    String r4s(r4); r4s.trim();
    String r5s(r5); r5s.trim();
    DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s);*/
    data.reserve(64);
    int index = 0;
    unsigned long startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (stream.available() > 0) {
        int a = stream.read();
        if (a <= 0) continue; // Skip 0x00 bytes, just in case
        data += (char)a;
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
          index = 2;
          goto finish;
        } else if (r3 && data.endsWith(r3)) {
          index = 3;
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (data.endsWith(GF(GSM_NL "+IPD,"))) {
          int mux = stream.readStringUntil(',').toInt();
          int len = stream.readStringUntil(':').toInt();
          int len_orig = len;
          if (len > sockets[mux]->rx.free()) {
            DBG("### Buffer overflow: ", len, "->", sockets[mux]->rx.free());
          } else {
            DBG("### Got: ", len, "->", sockets[mux]->rx.free());
          }
          while (len--) {
            TINY_GSM_MODEM_STREAM_TO_MUX_FIFO_WITH_DOUBLE_TIMEOUT
          }
          if (len_orig > sockets[mux]->available()) { // TODO
            DBG("### Fewer characters received than expected: ", sockets[mux]->available(), " vs ", len_orig);
          }
          data = "";
        } else if (data.endsWith(GF("CLOSED"))) {
          int muxStart = max(0,data.lastIndexOf(GSM_NL, data.length()-8));
          int coma = data.indexOf(',', muxStart);
          int mux = data.substring(muxStart, coma).toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### Closed: ", mux);
        }
      }
    } while (millis() - startMillis < timeout_ms);
finish:
    if (!index) {
      data.trim();
      if (data.length()) {
        DBG("### Unhandled:", data);
      }
      data = "";
    }
    //DBG('<', index, '>');
    return index;
  }

  uint8_t waitResponse(uint32_t timeout_ms,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
  }

  uint8_t waitResponse(GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    return waitResponse(1000, r1, r2, r3, r4, r5);
  }

public:
  Stream&       stream;

protected:
  GsmClient*    sockets[TINY_GSM_MUX_COUNT];
};

#endif
