/**
 * @file       TinyGsmClient.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClient_h
#define TinyGsmClient_h

#if defined(SPARK) || defined(PARTICLE)
    #include "Particle.h"
#elif defined(ARDUINO)
    #if ARDUINO >= 100
        #include "Arduino.h"
    #else
        #include "WProgram.h"
    #endif
#endif

#include <Client.h>
#include <TinyGsmFifo.h>

#if defined(__AVR__)
  #define GSM_PROGMEM PROGMEM
  typedef const __FlashStringHelper* GsmConstStr;
  #define FP(x) (reinterpret_cast<GsmConstStr>(x))
#else
  #define GSM_PROGMEM
  typedef const char* GsmConstStr;
  #define FP(x) x
  #undef F
  #define F(x) x
#endif

//#define GSM_USE_HEX

#if !defined(GSM_RX_BUFFER)
  #define GSM_RX_BUFFER 64
#endif

#define GSM_NL "\r\n"
static const char GSM_OK[] GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] GSM_PROGMEM = "ERROR" GSM_NL;

class TinyGsmClient
    : public Client
{
  typedef TinyGsmFifo<uint8_t, GSM_RX_BUFFER> RxFifo;

#ifdef GSM_DEBUG
  template<typename T>
  void DBG(T last) {
    GSM_DEBUG.println(last);
  }

  template<typename T, typename... Args>
  void DBG(T head, Args... tail) {
    GSM_DEBUG.print(head);
    GSM_DEBUG.print(' ');
    DBG(tail...);
  }
#else
  #define DBG(...)
#endif

public:
  TinyGsmClient(Stream& stream, uint8_t mux = 1)
    : stream(stream)
    , mux(mux)
    , sock_available(0)
    , sock_connected(false)
  {}

public:

  virtual int connect(const char *host, uint16_t port) {
    return modemConnect(host, port);
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
    return modemConnect(host.c_str(), port); //TODO: c_str may be missing
  }

  virtual void stop() {
    sendAT(F("+CIPCLOSE="), mux);
    sock_connected = false;
    waitResponse();
  }

  virtual size_t write(const uint8_t *buf, size_t size) {
    maintain();
    return modemSend(buf, size);
  }

  virtual size_t write(uint8_t c) {
    return write(&c, 1);
  }

  virtual int available() {
    maintain();
    return rx.size() + sock_available;
  }

  virtual int read(uint8_t *buf, size_t size) {
    maintain();
    size_t cnt = 0;
    while (cnt < size) {
      size_t chunk = min(size-cnt, rx.size());
      if (chunk > 0) {
        rx.get(buf, chunk);
        buf += chunk;
        cnt += chunk;
        continue;
      }
      // TODO: Read directly into user buffer?
      maintain();
      if (sock_available > 0) {
        modemRead(rx.free());
      } else {
        break;
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
  virtual void flush() { stream.flush(); }

  virtual uint8_t connected() {
    maintain();
    return sock_connected;
  }
  virtual operator bool() { return connected(); }

public:
  bool factoryDefault() {
    if (!autoBaud()) {
      return false;
    }
    sendAT(F("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(F("+IPR=0"));   // Auto-baud
    waitResponse();
    sendAT(F("+IFC=0,0")); // No Flow Control
    waitResponse();
    sendAT(F("+ICF=3,3")); // 8 data 0 parity 1 stop
    waitResponse();
    sendAT(F("+CSCLK=0")); // Disable Slow Clock
    waitResponse();
    sendAT(F("&W"));       // Write configuration
    return waitResponse() == 1;
  }

  bool restart() {
    if (!autoBaud()) {
      return false;
    }
    sendAT(F("+CFUN=0"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    sendAT(F("+CFUN=1,1"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);
    autoBaud();

    sendAT(F("E0"));
    if (waitResponse() != 1) {
      return false;
    }

    return waitResponse(60000L, F("Ready" GSM_NL)) == 1;
  }

  bool init() {
    if (!autoBaud()) {
      return false;
    }
    sendAT(F("&FZE0"));  // Factory + Reset + Echo Off
    return waitResponse() == 1;

    // +ICCID
    // AT+CPIN?
    // AT+CPIN=pin-code
    // AT+CREG?
  }

  bool networkConnect(const char* apn, const char* user, const char* pwd) {
    networkDisconnect();

    // AT+CGATT?
    // AT+CGATT=1

    sendAT(F("+CIPMUX=1"));
    if (waitResponse() != 1) {
      return false;
    }

    sendAT(F("+CIPQSEND=1"));
    if (waitResponse() != 1) {
      return false;
    }

    sendAT(F("+CIPRXGET=1"));
    if (waitResponse() != 1) {
      return false;
    }

    sendAT(F("+CSTT=\""), apn, F("\",\""), user, F("\",\""), pwd, F("\""));
    if (waitResponse(60000L) != 1) {
      return false;
    }

    sendAT(F("+CIICR"));
    if (waitResponse(60000L) != 1) {
      return false;
    }

    sendAT(F("+CIFSR;E0"));
    String data;
    if (waitResponse(10000L, data) != 1) {
      data.replace(GSM_NL, "");
      return false;
    }

    sendAT(F("+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\""));
    if (waitResponse() != 1) {
      return false;
    }

    // AT+CIPSTATUS

    return true;
  }

  bool networkDisconnect() {
    sendAT(F("+CIPSHUT"));
    return waitResponse(60000L) == 1;
  }

  bool autoBaud(unsigned long timeout = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      sendAT("");
      if (waitResponse() == 1) {
          delay(100);
          return true;
      }
      delay(100);
    }
    return false;
  }

  void maintain() {
    while (stream.available()) {
      waitResponse(10);
    }
  }

  bool simUnlock(const char *pin)
  {
    sendAT(F("+CPIN="), pin);
    return waitResponse() == 1;
  }

  bool simGetCCID() {
    sendAT(F("+CCID"));
    //TODO...
  }


private:
  int modemConnect(const char* host, uint16_t port) {
    sendAT(F("+CIPSTART="), mux, ',', F("\"TCP"), F("\",\""), host, F("\","), port);
    sock_connected = (1 == waitResponse(75000L, F("CONNECT OK" GSM_NL), F("CONNECT FAIL" GSM_NL), F("ALREADY CONNECT" GSM_NL)));
    return sock_connected;
  }

  int modemSend(const void* buff, size_t len) {
    sendAT(F("+CIPSEND="), mux, ',', len);
    if (waitResponse(F(">")) != 1) {
      return -1;
    }
    stream.write((uint8_t*)buff, len);
    if (waitResponse(F(GSM_NL "DATA ACCEPT:")) != 1) {
      return -1;
    }
    stream.readStringUntil(',');
    String data = stream.readStringUntil('\n');
    return data.toInt();
  }

  size_t modemRead(size_t size) {
#ifdef GSM_USE_HEX
    sendAT(F("+CIPRXGET=3,"), mux, ',', size);
    if (waitResponse(F("+CIPRXGET: 3,")) != 1) {
      return 0;
    }
#else
    sendAT(F("+CIPRXGET=2,"), mux, ',', size);
    if (waitResponse(F("+CIPRXGET: 2,")) != 1) {
      return 0;
    }
#endif
    stream.readStringUntil(','); // Skip mux
    size_t len = stream.readStringUntil(',').toInt();
    sock_available = stream.readStringUntil('\n').toInt();

    for (size_t i=0; i<len; i++) {
#ifdef GSM_USE_HEX
      while (stream.available() < 2) { delay(1); }
      char buf[4] = { 0, };
      buf[0] = stream.read();
      buf[1] = stream.read();
      char c = strtol(buf, NULL, 16);
#else
      while (stream.available() < 1) { delay(1); }
      char c = stream.read();
#endif
      rx.put(c);
    }
    waitResponse();
    return len;
  }

  size_t modemGetAvailable() {
    sendAT(F("+CIPRXGET=4,"), mux);
    size_t result = 0;
    for (byte i = 0; i < 2; i++) {
      int res = waitResponse(F("+CIPRXGET: 4"), FP(GSM_OK), FP(GSM_ERROR));
      if (res == 1) {
        stream.readStringUntil(',');
        stream.readStringUntil(',');
        result = stream.readStringUntil('\n').toInt();
      } else if (res == 2) {
      } else {
        return result;
      }
    }
    if (!result) {
      sock_connected = modemGetConnected();
    }
    return result;
  }

  bool modemGetConnected() {
    sendAT(F("+CIPSTATUS="), mux);
    int res = waitResponse(F(",\"CONNECTED\""), F(",\"CLOSED\""), F(",\"CLOSING\""), F(",\"INITIAL\""));
    waitResponse();
    return 1 == res;
  }

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

  int streamRead() { return stream.read(); }
  void streamReadAll() { while(stream.available()) { stream.read(); } }

  template<typename... Args>
  void sendAT(Args... cmd) {
    streamWrite("AT", cmd..., GSM_NL);
  }

  // TODO: Optimize this!
  uint8_t waitResponse(uint32_t timeout, String& data,
                       GsmConstStr r1=FP(GSM_OK), GsmConstStr r2=FP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    data.reserve(64);
    bool gotNewData = false;
    int index = 0;
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      while (stream.available() > 0) {
        int a = streamRead();
        if (a < 0) continue; //?
        data += (char)a;
        if (data.indexOf(r1) >= 0) {
          index = 1;
          goto finish;
        } else if (r2 && data.indexOf(r2) >= 0) {
          index = 2;
          goto finish;
        } else if (r3 && data.indexOf(r3) >= 0) {
          index = 3;
          goto finish;
        } else if (r4 && data.indexOf(r4) >= 0) {
          index = 4;
          goto finish;
        } else if (r5 && data.indexOf(r5) >= 0) {
          index = 5;
          goto finish;
        } else if (data.indexOf(F(GSM_NL "+CIPRXGET: 1,1" GSM_NL)) >= 0) { //TODO: use mux
          gotNewData = true;
          data = "";
        } else if (data.indexOf(F(GSM_NL "1, CLOSED" GSM_NL)) >= 0) { //TODO: use mux
          sock_connected = false;
          data = "";
        }
      }
    }
finish:
    if (!index) {
      if (data.length()) {
        DBG("### Unhandled:", data);
      }
      data = "";
    }
    if (gotNewData) {
      sock_available = modemGetAvailable();
    }
    return index;
  }

  uint8_t waitResponse(uint32_t timeout,
                       GsmConstStr r1=FP(GSM_OK), GsmConstStr r2=FP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    String data;
    return waitResponse(timeout, data, r1, r2, r3, r4, r5);
  }

  uint8_t waitResponse(GsmConstStr r1=FP(GSM_OK), GsmConstStr r2=FP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    return waitResponse(1000, r1, r2, r3, r4, r5);
  }

private:
  Stream&       stream;
  const uint8_t mux;

  RxFifo        rx;
  uint16_t      sock_available;
  bool          sock_connected;
};

#endif
