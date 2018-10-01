/**
 * @file       TinyGsmClientESP8266.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClientESP8266_h
#define TinyGsmClientESP8266_h

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


//============================================================================//
//============================================================================//
//                     Declaration of the TinyGsmESP8266 Class
//============================================================================//
//============================================================================//


class TinyGsmESP8266
{

  //============================================================================//
  //============================================================================//
  //                      The ESP8266 Internal Client Class
  //============================================================================//
  //============================================================================//

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
  virtual int connect(const char *host, uint16_t port) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();
    sock_connected = at->modemConnect(host, port, mux);
    return sock_connected;
  }

  virtual int connect(IPAddress ip, uint16_t port) {
    String host; host.reserve(16);
    host += ip[0];
    host += ".";
    host += ip[1];
    host += ".";
    host += ip[2];
    host += ".";
    host += ip[3];
    return connect(host.c_str(), port);
  }

  virtual void stop() {
    TINY_GSM_YIELD();
    at->sendAT(GF("+CIPCLOSE="), mux);
    sock_connected = false;
    at->waitResponse();
    rx.clear();
  }

  virtual size_t write(const uint8_t *buf, size_t size) {
    TINY_GSM_YIELD();
    //at->maintain();
    return at->modemSend(buf, size, mux);
  }

  virtual size_t write(uint8_t c) {
    return write(&c, 1);
  }

  virtual size_t write(const char *str) {
    if (str == NULL) return 0;
    return write((const uint8_t *)str, strlen(str));
  }

  virtual int available() {
    TINY_GSM_YIELD();
    if (!rx.size() && sock_connected) {
      at->maintain();
    }
    return rx.size();
  }

  virtual int read(uint8_t *buf, size_t size) {
    TINY_GSM_YIELD();
    size_t cnt = 0;
    while (cnt < size) {
      size_t chunk = TinyGsmMin(size-cnt, rx.size());
      if (chunk > 0) {
        rx.get(buf, chunk);
        buf += chunk;
        cnt += chunk;
        continue;
      }
      // TODO: Read directly into user buffer?
      if (!rx.size() && sock_connected) {
        at->maintain();
        //break;
      }
    }
    return cnt;
  }

  virtual int read() {
    uint8_t c;
    if (read(&c, 1) == 1) {
      return c;
    }
    return -1;
  }

  virtual int peek() { return -1; } //TODO
  virtual void flush() { at->stream.flush(); }

  virtual uint8_t connected() {
    if (available()) {
      return true;
    }
    return sock_connected;
  }
  virtual operator bool() { return connected(); }

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

//============================================================================//
//============================================================================//
//                          The Secure ESP8266 Client Class
//============================================================================//
//============================================================================//


class GsmClientSecure : public GsmClient
{
public:
  GsmClientSecure() {}

  GsmClientSecure(TinyGsmESP8266& modem, uint8_t mux = 1)
    : GsmClient(modem, mux)
  {}

public:
  virtual int connect(const char *host, uint16_t port) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();
    sock_connected = at->modemConnect(host, port, mux, true);
    return sock_connected;
  }
};


//============================================================================//
//============================================================================//
//                          The ESP8266 Modem Functions
//============================================================================//
//============================================================================//

public:

#ifdef GSM_DEFAULT_STREAM
  TinyGsmESP8266(Stream& stream = GSM_DEFAULT_STREAM)
#else
  TinyGsmESP8266(Stream& stream)
#endif
    : stream(stream)
  {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
  bool begin() {
    return init();
  }

  bool init() {
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
    return true;
  }

  void setBaud(unsigned long baud) {
    sendAT(GF("+IPR="), baud);
  }

  bool testAT(unsigned long timeout = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      sendAT(GF(""));
      if (waitResponse(200) == 1) {
        delay(100);
        return true;
      }
      delay(100);
    }
    return false;
  }

  void maintain() {
    waitResponse(10, NULL, NULL);
  }

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

  bool poweroff() TINY_GSM_ATTR_NOT_IMPLEMENTED;

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

  int getSignalQuality() {
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

  bool waitForNetwork(unsigned long timeout = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
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
   * GPRS functions
   */

  /*
   * Messaging functions
   */

  /*
   * Location functions
   */

  String getGsmLocation() TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Battery functions
   */

  uint16_t getBattVoltage() TINY_GSM_ATTR_NOT_AVAILABLE;

  int getBattPercent() TINY_GSM_ATTR_NOT_AVAILABLE;

protected:

  bool modemConnect(const char* host, uint16_t port, uint8_t mux, bool ssl = false) {
    if (ssl) {
      sendAT(GF("+CIPSSLSIZE=4096"));
      waitResponse();
    }
    sendAT(GF("+CIPSTART="), mux, ',', ssl ? GF("\"SSL") : GF("\"TCP"), GF("\",\""), host, GF("\","), port, GF(","), TINY_GSM_TCP_KEEP_ALIVE);
    // TODO: Check mux
    int rsp = waitResponse(75000L,
                           GFP(GSM_OK),
                           GFP(GSM_ERROR),
                           GF("ALREADY CONNECT"));
    // if (rsp == 3) waitResponse();  // May return "ERROR" after the "ALREADY CONNECT"
    return (1 == rsp);
  }

  int modemSend(const void* buff, size_t len, uint8_t mux) {
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

  /* Utilities */

  template<typename T>
  void streamWrite(T last) {
    stream.print(last);
  }

  template<typename T, typename... Args>
  void streamWrite(T head, Args... tail) {
    stream.print(head);
    streamWrite(tail...);
  }

  bool streamSkipUntil(char c) {
    const unsigned long timeout = 1000L;
    unsigned long startMillis = millis();
    while (millis() - startMillis < timeout) {
      while (millis() - startMillis < timeout && !stream.available()) {
        TINY_GSM_YIELD();
      }
      if (stream.read() == c)
        return true;
    }
    return false;
  }

  template<typename... Args>
  void sendAT(Args... cmd) {
    streamWrite("AT", cmd..., GSM_NL);
    stream.flush();
    TINY_GSM_YIELD();
    //DBG("### AT:", cmd...);
  }

  // TODO: Optimize this!
  uint8_t waitResponse(uint32_t timeout, String& data,
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
            while (!stream.available()) { TINY_GSM_YIELD(); }
            sockets[mux]->rx.put(stream.read());
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
    } while (millis() - startMillis < timeout);
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

  uint8_t waitResponse(uint32_t timeout,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    String data;
    return waitResponse(timeout, data, r1, r2, r3, r4, r5);
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
