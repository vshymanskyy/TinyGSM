/**
 * @file       TinyGsmClientM590.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClientM590_h
#define TinyGsmClientM590_h

//#define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 256
#endif

#include <TinyGsmCommon.h>

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;

enum SimStatus {
  SIM_ERROR = 0,
  SIM_READY = 1,
  SIM_LOCKED = 2,
};

enum RegStatus {
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 3,
  REG_DENIED       = 2,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};


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
    at->sendAT(GF("+TCPCLOSE="), mux);
    sock_connected = false;
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
    sendAT(GF("&FZE0"));  // Factory + Reset + Echo Off
    if (waitResponse() != 1) {
      return false;
    }
#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));
    waitResponse();
#endif

    getSimStatus();
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
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("+ICF=3,1")); // 8 data 0 parity 1 stop
    waitResponse();
    sendAT(GF("+enpwrsave=0")); // Disable PWR save
    waitResponse();
    sendAT(GF("+XISP=0"));  // Use internal stack
    waitResponse();
    sendAT(GF("&W"));       // Write configuration
    return waitResponse() == 1;
  }

  /*
   * Power functions
   */

  bool restart() {
    if (!autoBaud()) {
      return false;
    }
    sendAT(GF("+CFUN=15"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    //MODEM:STARTUP
    waitResponse(60000L, GF(GSM_NL "+PBREADY" GSM_NL));
    return init();
  }

  /*
   * SIM card functions
   */

  bool simUnlock(const char *pin) {
    sendAT(GF("+CPIN=\""), pin, GF("\""));
    return waitResponse() == 1;
  }

  String getSimCCID() {
    sendAT(GF("+CCID"));
    if (waitResponse(GF(GSM_NL "+CCID:")) != 1) {
      return "";
    }
    String res = streamReadUntil('\n');
    waitResponse();
    return res;
  }

  String getIMEI() {
    sendAT(GF("+GSN"));
    if (waitResponse(GF(GSM_NL)) != 1) {
      return "";
    }
    String res = streamReadUntil('\n');
    waitResponse();
    return res;
  }

  SimStatus getSimStatus(unsigned long timeout = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      sendAT(GF("+CPIN?"));
      if (waitResponse(GF(GSM_NL "+CPIN:")) != 1) {
        delay(1000);
        continue;
      }
      int status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"));
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
    if (waitResponse(GF(GSM_NL "+CREG:")) != 1) {
      return REG_UNKNOWN;
    }
    streamSkipUntil(','); // Skip format (0)
    int status = streamReadUntil('\n').toInt();
    waitResponse();
    return (RegStatus)status;
  }

  String getOperator() {
    sendAT(GF("+COPS?"));
    if (waitResponse(GF(GSM_NL "+COPS:")) != 1) {
      return "";
    }
    streamSkipUntil('"'); // Skip mode and format
    String res = streamReadUntil('"');
    waitResponse();
    return res;
  }

 /*
  * Generic network functions
  */

  int getSignalQuality() {
    sendAT(GF("+CSQ"));
    if (waitResponse(GF(GSM_NL "+CSQ:")) != 1) {
      return 99;
    }
    int res = streamReadUntil(',').toInt();
    waitResponse();
    return res;
  }

  bool waitForNetwork(unsigned long timeout = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      RegStatus s = getRegistrationStatus();
      if (s == REG_OK_HOME || s == REG_OK_ROAMING) {
        return true;
      }
      delay(1000);
    }
    return false;
  }

  /*
   * WiFi functions
   */
  bool networkConnect(const char* ssid, const char* pwd) {
    return false;
  }

  bool networkDisconnect() {
    return false;
  }

  /*
   * GPRS functions
   */
  bool gprsConnect(const char* apn, const char* user, const char* pwd) {
    gprsDisconnect();

    sendAT(GF("+XISP=0"));
    waitResponse();

    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    if (!user) user = "";
    if (!pwd)  pwd = "";
    sendAT(GF("+XGAUTH=1,1,\""), user, GF("\",\""), pwd, GF("\""));
    waitResponse();

    sendAT(GF("+XIIC=1"));
    waitResponse();

    delay(10000L); // TODO

    sendAT(GF("+XIIC?"));
    waitResponse();

    /*sendAT(GF("+DNSSERVER=1,8.8.8.8"));
    waitResponse();

    sendAT(GF("+DNSSERVER=2,8.8.4.4"));
    if (waitResponse() != 1) {
      return false;
    }*/

    return true;
  }

  bool gprsDisconnect() {
    sendAT(GF("+XIIC=0"));
    return waitResponse(60000L) == 1;
  }

  /*
   * Phone Call functions
   */

  bool callAnswer() {
    sendAT(GF("A"));
    return waitResponse() == 1;
  }

  bool callNumber(const String& number) {
    sendAT(GF("D"), number);
    return waitResponse() == 1;
  }

  bool callHangup(const String& number) {
    sendAT(GF("H"), number);
    return waitResponse() == 1;
  }

  /*
   * Messaging functions
   */

  bool sendSMS(const String& number, const String& text) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    sendAT(GF("+CMGS=\""), number, GF("\""));
    if (waitResponse(GF(">")) != 1) {
      return false;
    }
    stream.print(text);
    stream.write((char)0x1A);
    return waitResponse(60000L) == 1;
  }

  /*
   * Location functions
   */

  /*
   * Battery functions
   */

private:
  String dnsIpQuery(const char* host) {
   sendAT(GF("+DNS=\""), host, GF("\""));
   if (waitResponse(10000L, GF(GSM_NL "+DNS:")) != 1) {
     return "";
   }
   String res = streamReadUntil('\n');
   waitResponse(GF("+DNS:OK" GSM_NL));
   res.trim();
   return res;
  }

  int modemConnect(const char* host, uint16_t port, uint8_t mux) {
    for (int i=0; i<3; i++) {
      String ip = dnsIpQuery(host);

      sendAT(GF("+TCPSETUP="), mux, GF(","), ip, GF(","), port);
      int rsp = waitResponse(75000L,
                            GF(",OK" GSM_NL),
                            GF(",FAIL" GSM_NL),
                            GF("+TCPSETUP:Error" GSM_NL));
      if (1 == rsp) {
        return true;
      } else if (3 == rsp) {
        sendAT(GF("+TCPCLOSE="), mux);
        waitResponse();
      }
      delay(1000);
    }
    return false;
  }

  int modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+TCPSEND="), mux, ',', len);
    if (waitResponse(GF(">")) != 1) {
      return 0;
    }
    stream.write((uint8_t*)buff, len);
    stream.write((char)0x0D);

    if (waitResponse(30000L, GF(GSM_NL "+TCPSEND:")) != 1) {
      return 0;
    }
    streamReadUntil('\n');
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
    DBG(GSM_NL, ">>> AT:", cmd...);
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
        } else if (data.endsWith(GF("+TCPRECV:"))) {
          mux = stream.readStringUntil(',').toInt();
          data += mux;
          data += (',');
          len = stream.readStringUntil(',').toInt();
          data += len;
          data += (',');
          gotData = true;
          index = 6;
          goto finish;
        } else if (data.endsWith(GF("+TCPCLOSE:"))) {
          mux = stream.readStringUntil(',').toInt();
          data += mux;
          data += (',');
          String concl = stream.readStringUntil('\n');
          data += concl;
          sockets[mux]->sock_connected = false;
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
  GsmClient*    sockets[2];
};

typedef TinyGsm::GsmClient TinyGsmClient;

#endif
