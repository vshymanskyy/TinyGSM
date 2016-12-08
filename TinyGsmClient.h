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
  #define GFP(x) (reinterpret_cast<GsmConstStr>(x))
  #define GF(x)  F(x)
#else
  #define GSM_PROGMEM
  typedef const char* GsmConstStr;
  #define GFP(x) x
  #define GF(x)  x
#endif

//#define GSM_DEBUG Serial
//#define GSM_USE_HEX

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 64
#endif

#define GSM_NL "\r\n"
static const char GSM_OK[] GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] GSM_PROGMEM = "ERROR" GSM_NL;

enum SimStatus {
  SIM_ERROR = 0,
  SIM_READY = 1,
  SIM_LOCKED = 2,
};

enum RegStatus {
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};


class TinyGsm
{
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

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
  TinyGsm(Stream& stream)
    : stream(stream)
  {}

public:

class GsmClient : public Client
{
  friend class TinyGsm;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {
    init(NULL, -1);
  }

  GsmClient(TinyGsm& at, uint8_t mux = 1) {
    init(&at, mux);
  }

  bool init(TinyGsm* at, uint8_t mux = 1) {
    this->at = at;
    this->mux = mux;
    at->sockets[mux] = this;
    sock_available = 0;
    sock_connected = false;
    return true;
  }

public:
  virtual int connect(const char *host, uint16_t port) {
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
    at->sendAT(GF("+CIPCLOSE="), mux);
    sock_connected = false;
    at->waitResponse();
  }

  virtual size_t write(const uint8_t *buf, size_t size) {
    at->maintain();
    return at->modemSend(buf, size, mux);
  }

  virtual size_t write(uint8_t c) {
    return write(&c, 1);
  }

  virtual int available() {
    at->maintain();
    return rx.size() + sock_available;
  }

  virtual int read(uint8_t *buf, size_t size) {
    at->maintain();
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
      at->maintain();
      if (sock_available > 0) {
        at->modemRead(rx.free(), mux);
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
  virtual void flush() { at->stream.flush(); }

  virtual uint8_t connected() {
    at->maintain();
    return sock_connected;
  }
  virtual operator bool() { return connected(); }
private:
  TinyGsm*      at;
  uint8_t       mux;
  uint16_t      sock_available;
  bool          sock_connected;
  RxFifo        rx;
};

public:

  /*
   * Basic functions
   */
  bool begin() {
    if (!autoBaud()) {
      return false;
    }
    sendAT(GF("&FZE0"));  // Factory + Reset + Echo Off
    if (waitResponse() != 1) {
      return false;
    }
    getSimStatus();
    return true;
  }

  bool autoBaud(unsigned long timeout = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      sendAT("");
      if (waitResponse(200) == 1) {
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

  bool factoryDefault() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("+IPR=0"));   // Auto-baud
    waitResponse();
    sendAT(GF("+IFC=0,0")); // No Flow Control
    waitResponse();
    sendAT(GF("+ICF=3,3")); // 8 data 0 parity 1 stop
    waitResponse();
    sendAT(GF("+CSCLK=0")); // Disable Slow Clock
    waitResponse();
    sendAT(GF("&W"));       // Write configuration
    return waitResponse() == 1;
  }

  /*
   * Power functions
   */

  bool restart() {
    return resetSoft();
  }

  bool resetSoft() {
    if (!autoBaud()) {
      return false;
    }
    sendAT(GF("+CFUN=0"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    sendAT(GF("+CFUN=1,1"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);
    return begin();
  }

  // Reboot the module by setting the specified pin LOW, then HIGH.
  // (The pin should be connected to a P-MOSFET)
  bool resetHard(int pwrPin) {
    powerOff(pwrPin);
    delay(100);
    return powerOn(pwrPin);
  }

  void powerOff(int pwrPin) {
    pinMode(pwrPin, OUTPUT);
    digitalWrite(pwrPin, LOW);
  }

  bool powerOn(int pwrPin) {
    pinMode(pwrPin, OUTPUT);
    digitalWrite(pwrPin, HIGH);
    delay(3000);
    return begin();
  }

  /*
   * SIM card & Networ Operator functions
   */

  bool simUnlock(const char *pin) {
    sendAT(GF("+CPIN="), pin);
    return waitResponse() == 1;
  }

  String getSimCCID() {
    sendAT(GF("+ICCID"));
    if (waitResponse(GF(GSM_NL "+ICCID: ")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  SimStatus getSimStatus(unsigned long timeout = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      sendAT(GF("+CPIN?"));
      if (waitResponse(GF(GSM_NL "+CPIN: ")) != 1) {
        delay(1000);
        continue;
      }
      int status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"), GF("NOT INSERTED"));
      waitResponse();
      switch (status) {
      case 2:
      case 3:  return SIM_LOCKED;
      case 1:  return SIM_READY;
      default: return SIM_ERROR;
      }
    }
    return SIM_ERROR;
  }

  RegStatus getRegistrationStatus() {
    sendAT(GF("+CREG?"));
    if (waitResponse(GF(GSM_NL "+CREG: 0,")) != 1) {
      return REG_UNKNOWN;
    }
    int status = stream.readStringUntil('\n').toInt();
    waitResponse();
    return (RegStatus)status;
  }

  String getOperator() {
    sendAT(GF("+COPS?"));
    if (waitResponse(GF(GSM_NL "+COPS: ")) != 1) {
      return "";
    }
    stream.readStringUntil('"'); // Skip mode and format
    String res = stream.readStringUntil('"');
    waitResponse();
    return res;
  }

  bool waitForNetwork(unsigned long timeout = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      RegStatus s = getRegistrationStatus();
      if(s == REG_OK_HOME || s == REG_OK_ROAMING) {
        return true;
      }
      delay(1000);
    }
    return true;
  }

  /*
   * GPRS functions
   */
  bool gprsConnect(const char* apn, const char* user, const char* pwd) {
    gprsDisconnect();

    // AT+CGATT?
    // AT+CGATT=1

    sendAT(GF("+CIPMUX=1"));
    if (waitResponse() != 1) {
      return false;
    }

    sendAT(GF("+CIPQSEND=1"));
    if (waitResponse() != 1) {
      return false;
    }

    sendAT(GF("+CIPRXGET=1"));
    if (waitResponse() != 1) {
      return false;
    }

    sendAT(GF("+CSTT=\""), apn, GF("\",\""), user, GF("\",\""), pwd, GF("\""));
    if (waitResponse(60000L) != 1) {
      return false;
    }

    sendAT(GF("+CIICR"));
    if (waitResponse(60000L) != 1) {
      return false;
    }

    sendAT(GF("+CIFSR;E0"));
    String data;
    if (waitResponse(10000L, data) != 1) {
      data.replace(GSM_NL, "");
      return false;
    }

    sendAT(GF("+CDNSCFG=\"8.8.8.8\",\"8.8.4.4\""));
    if (waitResponse() != 1) {
      return false;
    }

    return true;
  }

  bool gprsDisconnect() {
    sendAT(GF("+CIPSHUT"));
    return waitResponse(60000L) == 1;
  }

  /*
   * Phone Call functions
   */

  /*
   * Messaging functions
   */

  void sendUSSD() {
  }

  void sendSMS() {
  }

  /*
   * Location functions
   */
  void getLocation() {
  }

  /*
   * Battery functions
   */

private:
  int modemConnect(const char* host, uint16_t port, uint8_t mux) {
    sendAT(GF("+CIPSTART="), mux, ',', GF("\"TCP"), GF("\",\""), host, GF("\","), port);
    int rsp = waitResponse(75000L,
                           GF("CONNECT OK" GSM_NL),
                           GF("CONNECT FAIL" GSM_NL),
                           GF("ALREADY CONNECT" GSM_NL));
    return (1 == rsp);
  }

  int modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CIPSEND="), mux, ',', len);
    if (waitResponse(GF(">")) != 1) {
      return -1;
    }
    stream.write((uint8_t*)buff, len);
    if (waitResponse(GF(GSM_NL "DATA ACCEPT:")) != 1) {
      return -1;
    }
    stream.readStringUntil(',');
    String data = stream.readStringUntil('\n');
    return data.toInt();
  }

  size_t modemRead(size_t size, uint8_t mux) {
#ifdef GSM_USE_HEX
    sendAT(GF("+CIPRXGET=3,"), mux, ',', size);
    if (waitResponse(GF("+CIPRXGET: 3,")) != 1) {
      return 0;
    }
#else
    sendAT(GF("+CIPRXGET=2,"), mux, ',', size);
    if (waitResponse(GF("+CIPRXGET: 2,")) != 1) {
      return 0;
    }
#endif
    stream.readStringUntil(','); // Skip mux
    size_t len = stream.readStringUntil(',').toInt();
    sockets[mux]->sock_available = stream.readStringUntil('\n').toInt();

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
      sockets[mux]->rx.put(c);
    }
    waitResponse();
    return len;
  }

  size_t modemGetAvailable(uint8_t mux) {
    sendAT(GF("+CIPRXGET=4,"), mux);
    size_t result = 0;
    for (byte i = 0; i < 2; i++) {
      int res = waitResponse(GF("+CIPRXGET: 4"), GFP(GSM_OK), GFP(GSM_ERROR));
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
      sockets[mux]->sock_connected = modemGetConnected(mux);
    }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+CIPSTATUS="), mux);
    int res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""), GF(",\"CLOSING\""), GF(",\"INITIAL\""));
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
    //DBG("### AT:", cmd...);
  }

  // TODO: Optimize this!
  uint8_t waitResponse(uint32_t timeout, String& data,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    data.reserve(64);
    bool gotNewData = false;
    int index = 0;
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      while (stream.available() > 0) {
        int a = streamRead();
        if (a <= 0) continue;
        data += (char)a;
        if (r1 && data.indexOf(r1) >= 0) {
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
        } else if (data.indexOf(GF(GSM_NL "+CIPRXGET: 1,1" GSM_NL)) >= 0) { //TODO: use mux
          gotNewData = true;
          data = "";
        } else if (data.indexOf(GF(GSM_NL "1, CLOSED" GSM_NL)) >= 0) { //TODO: use mux
          sockets[1]->sock_connected = false;
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
      sockets[1]->sock_available = modemGetAvailable(1);
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
