/**
 * @file       TinyWiFiClientESP8266.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyWiFiClientESP8266_h
#define TinyWiFiClientESP8266_h

//#define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 256
#endif

#include <TinyGsmCommon.h>

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;
static unsigned int TCP_KEEP_ALIVE = 120;

class TinyGsm
{

public:
  TinyGsm(Stream& stream)
    : stream(stream)
  {}

public:

class GsmClient : public Client
{
  friend class TinyGsm;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsm& modem, uint8_t mux = 1) {
    init(&modem, mux);
  }

  bool init(TinyGsm* modem, uint8_t mux = 1) {
    this->at = modem;
    this->mux = mux;
    sock_connected = false;

    at->sockets[mux] = this;

    return true;
  }

public:
  virtual int connect(const char *host, uint16_t port) {
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
    at->waitResponse();
  }

  virtual size_t write(const uint8_t *buf, size_t size) {
    TINY_GSM_YIELD();
    //at->maintain();
    return at->modemSend(buf, size, mux);
  }

  virtual size_t write(uint8_t c) {
    return write(&c, 1);
  }

  virtual int available() {
    TINY_GSM_YIELD();
    if (!rx.size()) {
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
      if (!rx.size()) {
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
private:
  TinyGsm*      at;
  uint8_t       mux;
  bool          sock_connected;
  RxFifo        rx;
};

public:

  /*
   * Basic functions
   */
  bool begin() {
    return init();
  }

  bool init() {
    if (!autoBaud()) {
      return false;
    }
    return true;
  }

  bool autoBaud(unsigned long timeout = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      sendAT(GF("E0"));
      if (waitResponse(200) == 1) {
          delay(100);
          return true;
      }
      delay(100);
    }
    return false;
  }

  void maintain() {
    //while (stream.available()) {
      waitResponse(10, NULL, NULL);
    //}
  }

  bool factoryDefault() {
    sendAT(GF("+RESTORE"));
    return waitResponse() == 1;
  }

  /*
   * Power functions
   */

  bool restart() {
    if (!autoBaud()) {
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
    return autoBaud();
  }

  /*
   * SIM card functions
   */

  /*
   * Generic network functions
   */

  int getSignalQuality() {
    sendAT(GF("+CWJAP_CUR?"));
    int res1 = waitResponse(GF("No AP"), GF("+CWJAP_CUR:"));
    if (res1 != 2){
      waitResponse();
      return 0;
    }
    streamSkipUntil(',');  // Skip SSID
    streamSkipUntil(',');  // Skip BSSID/MAC address
    streamSkipUntil(',');  // Skip Chanel number
    int res2 = stream.parseInt();  // Read RSSI
    DBG(res2);
    waitResponse();
    return res2;
  }

  bool waitForNetwork(unsigned long timeout = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      sendAT(GF("+CIPSTATUS"));
      int res1 = waitResponse(3000, GF("busy p..."), GF("STATUS:"));
      if (res1 == 2){
        int res2 = waitResponse(GFP(GSM_ERROR), GF("2"), GF("3"), GF("4"), GF("5"));
        if (res2 == 2 || res2 == 3 || res2 == 4) return true;
      }
      // <stat> status of ESP8266 station interface
      // 2 : ESP8266 station connected to an AP and has obtained IP
      // 3 : ESP8266 station created a TCP or UDP transmission
      // 4 : the TCP or UDP transmission of ESP8266 station disconnected (but AP is connected)
      // 5 : ESP8266 station did NOT connect to an AP
      delay(1000);
    }
    return false;
  }

  /*
   * WiFi functions
   */
  bool networkConnect(const char* ssid, const char* pwd) {

    sendAT(GF("+CIPMUX=1"));
    if (waitResponse() != 1) {
      return false;
    }

    sendAT(GF("+CWMODE_CUR=1"));
    if (waitResponse() != 1) {
      return false;
    }

    sendAT(GF("+CWJAP_CUR=\""), ssid, GF("\",\""), pwd, GF("\""));
    if (waitResponse(30000L, GFP(GSM_OK), GF(GSM_NL "FAIL" GSM_NL)) != 1) {
      return false;
    }

    return true;
  }

  bool networkDisconnect() {
    sendAT(GF("+CWQAP"));
    return waitResponse(10000L) == 1;
  }

  /*
   * GPRS functions
   */
  bool gprsConnect(const char* apn, const char* user, const char* pwd) {
    return false;
  }

  bool gprsDisconnect() {
    return false;
  }

private:
  int modemConnect(const char* host, uint16_t port, uint8_t mux) {
    sendAT(GF("+CIPSTART="), mux, ',', GF("\"TCP"), GF("\",\""), host, GF("\","), port, GF(","), TCP_KEEP_ALIVE);
    int rsp = waitResponse(75000L,
                           GFP(GSM_OK),
                           GFP(GSM_ERROR),
                           GF(GSM_NL "ALREADY CONNECT" GSM_NL));
    waitResponse(100, GF("1,CONNECT"));
    return (1 == rsp);
  }

  int modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', len);
    if (waitResponse(GF(">")) != 1) {
      return -1;
    }
    stream.write((uint8_t*)buff, len);
    if (waitResponse(GF(GSM_NL "SEND OK" GSM_NL)) != 1) {
      return -1;
    }
    return len;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+CIPSTATUS="), mux);
    int res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""), GF(",\"CLOSING\""), GF(",\"INITIAL\""));
    waitResponse();
    return 1 == res;
  }

  /* Private Utilities */
  template<typename T>
  void streamWrite(T last) {
    stream.print(last);
  }

  template<typename T, typename... Args>
  void streamWrite(T head, Args... tail) {
    stream.print(head);
    streamWrite(tail...);
  }

  int streamRead() { return stream.read(); }

  String streamReadUntil(char c) {
    String return_string = stream.readStringUntil(c);
    return_string.trim();
    if (String(c) == GSM_NL || String(c) == "\n"){
      DBG(return_string, c, "    ");
    } else DBG(return_string, c);
    return return_string;
  }

  bool streamSkipUntil(char c) {
    String skipped = stream.readStringUntil(c);
    skipped.trim();
    if (skipped.length()) {
      if (String(c) == GSM_NL || String(c) == "\n"){
        DBG(skipped, c, "    ");
      } else DBG(skipped, c);
      return true;
    } else return false;
  }

  template<typename... Args>
  void sendAT(Args... cmd) {
    streamWrite("AT", cmd..., GSM_NL);
    stream.flush();
    TINY_GSM_YIELD();
    DBG(GSM_NL, ">>> AT", cmd...);
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
    bool gotData = false;
    int mux = -1;
    int len = 0;
    int index = 0;
    unsigned long startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (stream.available() > 0) {
        int a = streamRead();
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
          mux = stream.readStringUntil(',').toInt();
          data += mux;
          data += (',');
          len = stream.readStringUntil(':').toInt();
          data += len;
          data += (':');
          gotData = true;
          index = 6;
          goto finish;
        } else if (data.endsWith(GF("1,CLOSED" GSM_NL))) { //TODO: use mux
          sockets[1]->sock_connected = false;
          index = 7;
          goto finish;
        }
      }
    } while (millis() - startMillis < timeout);
  finish:
    if (!index) {
      data.trim();
      if (data.length()) {
        DBG(GSM_NL, "### Unhandled:", data);
      }
    }
    else {
      data.trim();
      data.replace(GSM_NL GSM_NL, GSM_NL);
      data.replace(GSM_NL, GSM_NL "    ");
      if (data.length()) {
        DBG(GSM_NL, "<<< ", data);
      }
    }
      if (gotData) {
        int len_orig = len;
        if (len > sockets[mux]->rx.free()) {
          DBG(GSM_NL, "### Buffer overflow: ", len, "->", sockets[mux]->rx.free());
        } else {
          DBG(GSM_NL, "### Got: ", len, "->", sockets[mux]->rx.free());
        }
        while (len--) {
          TINY_GSM_YIELD();
          int r = stream.read();
          if (r <= 0) continue; // Skip 0x00 bytes, just in case
          sockets[mux]->rx.put((char)r);
        }
        if (len_orig > sockets[mux]->available()) {
          DBG(GSM_NL, "### Fewer characters received than expected: ", sockets[mux]->available(), " vs ", len_orig);
        }
      }
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

private:
  Stream&       stream;
  GsmClient*    sockets[5];
};

typedef TinyGsm::GsmClient TinyGsmClient;

#endif
