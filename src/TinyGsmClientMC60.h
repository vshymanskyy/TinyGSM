/**
 * @file       TinyGsmClientMC60.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 *
 * @MC60 support added by Tamas Dajka 2017.10.15 - with fixes by Sara Damiano
 *
 */

#ifndef TinyGsmClientMC60_h
#define TinyGsmClientMC60_h
//#pragma message("TinyGSM:  TinyGsmClientMC60")

//#define TINY_GSM_DEBUG Serial
//#define TINY_GSM_USE_HEX

#define TINY_GSM_MUX_COUNT 6

#include <TinyGsmCommon.h>

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;

enum SimStatus {
  SIM_ERROR = 0,
  SIM_READY = 1,
  SIM_LOCKED = 2,
  SIM_ANTITHEFT_LOCKED = 3,
};

enum RegStatus {
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};


class TinyGsmMC60 : public TinyGsmModem
{

public:

class GsmClient : public Client
{
  friend class TinyGsmMC60;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmMC60& modem, uint8_t mux = 1) {
    init(&modem, mux);
  }

  bool init(TinyGsmMC60* modem, uint8_t mux = 1) {
    this->at = modem;
    this->mux = mux;
    sock_available = 0;
    sock_connected = false;
    got_data = false;

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
    // Read and dump anything remaining in the modem's internal buffer.
    // The socket will appear open in response to connected() even after it
    // closes until all data is read from the buffer.
    // Doing it this way allows the external mcu to find and get all of the data
    // that it wants from the socket even if it was closed externally.
    rx.clear();
    at->maintain();
    while (sock_available > 0) {
      sock_available -= at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
      rx.clear();
      at->maintain();
    }
    at->sendAT(GF("+QICLOSE="), mux);
    sock_connected = false;
    at->waitResponse(60000L, GF("CLOSED"), GF("CLOSE OK"), GF("ERROR"));
  }

  virtual size_t write(const uint8_t *buf, size_t size) {
    TINY_GSM_YIELD();
    at->maintain();
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
    if (!rx.size()) {
      at->maintain();
    }
    return rx.size() + sock_available;
  }

  virtual int read(uint8_t *buf, size_t size) {
    TINY_GSM_YIELD();
    at->maintain();
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
      at->maintain();
      if (sock_available > 0) {
        sock_available -= at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
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
  TinyGsmMC60*  at;
  uint8_t       mux;
  uint16_t      sock_available;
  bool          sock_connected;
  bool          got_data;
  RxFifo        rx;
};


// class GsmClientSecure : public GsmClient
// {
// public:
//   GsmClientSecure() {}
//
//   GsmClientSecure(TinyGsmMC60& modem, uint8_t mux = 1)
//     : GsmClient(modem, mux)
//   {}
//
// public:
//   virtual int connect(const char *host, uint16_t port) {
//     stop();
//     TINY_GSM_YIELD();
//     rx.clear();
//     sock_connected = at->modemConnect(host, port, mux, true);
//     return sock_connected;
//   }
// };


public:

  TinyGsmMC60(Stream& stream)
    : TinyGsmModem(stream), stream(stream)
  {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */

  bool init(const char* pin = NULL) {
    if (!testAT()) {
      return false;
    }
    sendAT(GF("&FZ"));  // Factory + Reset
    waitResponse();
    sendAT(GF("E0"));   // Echo Off
    if (waitResponse() != 1) {
      return false;
    }
    DBG(GF("### Modem:"), getModemName());
    getSimStatus();
    return true;
  }

  String getModemName() {
    #if defined(TINY_GSM_MODEM_MC60)
      return "Quectel MC60";
    #elif defined(TINY_GSM_MODEM_MC60E)
      return "Quectel MC60E";
    #endif
      return "Quectel MC60";
  }

  void setBaud(unsigned long baud) {
    sendAT(GF("+IPR="), baud);
  }

  bool testAT(unsigned long timeout = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      sendAT(GF(""));
      if (waitResponse(200) == 1) return true;
      delay(100);
    }
    return false;
  }

  void maintain() {
    for (int mux = 0; mux < TINY_GSM_MUX_COUNT; mux++) {
      GsmClient* sock = sockets[mux];
      if (sock && sock->got_data) {
        sock->got_data = false;
        sock->sock_available = modemGetAvailable(mux);
      }
    }
    while (stream.available()) {
      waitResponse(10, NULL, NULL);
    }
  }

  bool factoryDefault() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("+IPR=0"));   // Auto-baud
    waitResponse();
    sendAT(GF("&W"));       // Write configuration
    return waitResponse() == 1;
  }

  String getModemInfo() {
    sendAT(GF("I"));
    String res;
    if (waitResponse(1000L, res) != 1) {
      return "";
    }
    res.replace(GSM_NL "OK" GSM_NL, "");
    res.replace(GSM_NL, " ");
    res.trim();
    return res;
  }

  /*
  * under development
  */
  // bool hasSSL() {
  //   sendAT(GF("+QIPSSL=?"));
  //   if (waitResponse(GF(GSM_NL "+CIPSSL:")) != 1) {
  //     return false;
  //   }
  //   return waitResponse() == 1;
  // }

  bool hasSSL() {
    return false;  // TODO: For now
  }

  bool hasWifi() {
    return false;
  }

  bool hasGPRS() {
    return true;
  }

  /*
   * Power functions
   */

  bool restart() {
    if (!testAT()) {
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
    return init();
  }

  bool poweroff() {
    sendAT(GF("+QPOWD=1"));
    return waitResponse(GF("NORMAL POWER DOWN")) == 1;
  }

  bool radioOff() {
    if (!testAT()) {
      return false;
    }
    sendAT(GF("+CFUN=0"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);
    return true;
  }

  bool sleepEnable(bool enable = true) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * SIM card functions
   */

  bool simUnlock(const char *pin) {
    sendAT(GF("+CPIN=\""), pin, GF("\""));
    return waitResponse() == 1;
  }

  String getSimCCID() {
    sendAT(GF("+ICCID"));
    if (waitResponse(GF(GSM_NL "+ICCID:")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  String getIMEI() {
    sendAT(GF("+GSN"));
    if (waitResponse(GF(GSM_NL)) != 1) {
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
      if (waitResponse(GF(GSM_NL "+CPIN:")) != 1) {
        delay(1000);
        continue;
      }
      int status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"), GF("NOT INSERTED"), GF("PH_SIM PIN"), GF("PH_SIM PUK"));
      waitResponse();
      switch (status) {
        case 2:
        case 3:  return SIM_LOCKED;
        case 5:
        case 6:  return SIM_ANTITHEFT_LOCKED;
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
    int status = stream.readStringUntil('\n').toInt();
    waitResponse();
    return (RegStatus)status;
  }

  String getOperator() {
    sendAT(GF("+COPS?"));
    if (waitResponse(GF(GSM_NL "+COPS:")) != 1) {
      return "";
    }
    streamSkipUntil('"'); // Skip mode and format
    String res = stream.readStringUntil('"');
    waitResponse();
    return res;
  }

  /*
   * Generic network functions
   */

  int16_t getSignalQuality() {
    sendAT(GF("+CSQ"));
    if (waitResponse(GF(GSM_NL "+CSQ:")) != 1) {
      return 99;
    }
    int res = stream.readStringUntil(',').toInt();
    waitResponse();
    return res;
  }

  bool isNetworkConnected() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  /*
   * GPRS functions
   */
  bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {
    gprsDisconnect();

    // select foreground context 0 = VIRTUAL_UART_1
    sendAT(GF("+QIFGCNT=0"));
    if (waitResponse() != 1) {
      return false;
    }

    //Select GPRS (=1) as the Bearer
    sendAT(GF("+QICSGP=1,\""), apn, GF("\",\""), user, GF("\",\""), pwd, GF("\""));
    if (waitResponse() != 1) {
      return false;
    }

    //Define PDP context - is this necessary?
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Activate PDP context - is this necessary?
    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    //Start TCPIP Task and Set APN, User Name and Password
    sendAT("+QIREGAPP=\"", apn, "\",\"", user, "\",\"", pwd,  "\"" );
    if (waitResponse() != 1) {
      return false;
    }

    //Activate GPRS/CSD Context
    sendAT(GF("+QIACT"));
    if (waitResponse(60000L) != 1) {
      return false;
    }

    //Enable multiple TCP/IP connections
    sendAT(GF("+QIMUX=1"));
    if (waitResponse() != 1) {
      return false;
    }

    //Request an IP header for received data ("IPD(data length):")
    sendAT(GF("+QIHEAD=1"));
    if (waitResponse() != 1) {
      return false;
    }

    //Set Method to Handle Received TCP/IP Data - Retrieve Data by Command
    sendAT(GF("+QINDI=1"));
    if (waitResponse() != 1) {
      return false;
    }

    // Check that we have a local IP address
    if (localIP() != 0) {
      return true;
    }

    return false;
  }

  bool gprsDisconnect() {
    sendAT(GF("+QIDEACT"));
    return waitResponse(60000L, GF("DEACT OK"), GF("ERROR")) == 1;
  }

  bool isGprsConnected() {
    sendAT(GF("+CGATT?"));
    if (waitResponse(GF(GSM_NL "+CGATT:")) != 1) {
      return false;
    }
    int res = stream.readStringUntil('\n').toInt();
    waitResponse();
    if (res != 1)
      return false;

    return localIP() != 0;
  }

  /*
   * IP Address functions
   */

  String getLocalIP() {
    sendAT(GF("+QILOCIP"));
    stream.readStringUntil('\n');
    String res = stream.readStringUntil('\n');
    res.trim();
    return res;
  }

  /*
   * Messaging functions
   */

  String sendUSSD(const String& code) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    sendAT(GF("+CSCS=\"HEX\""));
    waitResponse();
    sendAT(GF("+CUSD=1,\""), code, GF("\""));
    if (waitResponse(10000L, GF(GSM_NL "+CUSD:")) != 1) {
      return "";
    }
    stream.readStringUntil('"');
    String hex = stream.readStringUntil('"');
    stream.readStringUntil(',');
    int dcs = stream.readStringUntil('\n').toInt();

    if (waitResponse() != 1) {
      return "";
    }

    if (dcs == 15) {
      return TinyGsmDecodeHex8bit(hex);
    } else if (dcs == 72) {
      return TinyGsmDecodeHex16bit(hex);
    } else {
      return hex;
    }
  }

  bool sendSMS(const String& number, const String& text) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    //Set GSM 7 bit default alphabet (3GPP TS 23.038)
    sendAT(GF("+CSCS=\"GSM\""));
    waitResponse();
    sendAT(GF("+CMGS=\""), number, GF("\""));
    if (waitResponse(GF(">")) != 1) {
      return false;
    }
    stream.print(text);
    stream.write((char)0x1A);
    stream.flush();
    return waitResponse(60000L) == 1;
  }

  bool sendSMS_UTF16(const String& number, const void* text, size_t len) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    sendAT(GF("+CSCS=\"HEX\""));
    waitResponse();
    sendAT(GF("+CSMP=17,167,0,8"));
    waitResponse();

    sendAT(GF("+CMGS=\""), number, GF("\""));
    if (waitResponse(GF(">")) != 1) {
      return false;
    }

    uint16_t* t = (uint16_t*)text;
    for (size_t i=0; i<len; i++) {
      uint8_t c = t[i] >> 8;
      if (c < 0x10) { stream.print('0'); }
      stream.print(c, HEX);
      c = t[i] & 0xFF;
      if (c < 0x10) { stream.print('0'); }
      stream.print(c, HEX);
    }
    stream.write((char)0x1A);
    stream.flush();
    return waitResponse(60000L) == 1;
  }

  /** Delete all SMS */
  bool deleteAllSMS() {
    sendAT(GF("+QMGDA=6"));
    if (waitResponse(waitResponse(60000L, GF("OK"), GF("ERROR")) == 1) ) {
      return true;
    }
    return false;
  }

  /*
   * Location functions
   */

  String getGsmLocation() {
    sendAT(GF("+CIPGSMLOC=1,1"));
    if (waitResponse(10000L, GF(GSM_NL "+CIPGSMLOC:")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Battery functions
   */

  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  uint16_t getBattVoltage() {
    sendAT(GF("+CBC"));
    if (waitResponse(GF(GSM_NL "+CBC:")) != 1) {
      return 0;
    }
    streamSkipUntil(','); // Skip
    streamSkipUntil(','); // Skip

    uint16_t res = stream.readStringUntil(',').toInt();
    waitResponse();
    return res;
  }

  int8_t getBattPercent() {
    sendAT(GF("+CBC"));
    if (waitResponse(GF(GSM_NL "+CBC:")) != 1) {
      return false;
    }
    stream.readStringUntil(',');
    int res = stream.readStringUntil(',').toInt();
    waitResponse();
    return res;
  }

  /*
   * Client related functions
   */

protected:

  bool modemConnect(const char* host, uint16_t port, uint8_t mux, bool ssl = false) {
    sendAT(GF("+QIOPEN="), mux, GF("\"TCP"), GF("\",\""), host, GF("\","), port);
    int rsp = waitResponse(75000L,
                           GF("CONNECT OK" GSM_NL),
                           GF("CONNECT FAIL" GSM_NL),
                           GF("ALREADY CONNECT" GSM_NL));
    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+QISEND="), mux, ',', len);
    if (waitResponse(GF(">")) != 1) {
      return 0;
    }
    stream.write((uint8_t*)buff, len);
    stream.flush();
    if (waitResponse(GF(GSM_NL "SEND OK")) != 1) {
      return 0;
    }

    bool allAcknowledged = false;
    // bool failed = false;
    while ( !allAcknowledged ) {
      sendAT( GF("+QISACK"));
      if (waitResponse(5000L, GF(GSM_NL "+QISACK:")) != 1) {
        return -1;
      } else {
        streamSkipUntil(','); /** Skip total */
        streamSkipUntil(','); /** Skip acknowledged data size */
        if ( stream.readStringUntil('\n').toInt() == 0 ) {
          allAcknowledged = true;
        }
      }
    }
    waitResponse(5000L);

    // streamSkipUntil(','); // Skip mux
    // return stream.readStringUntil('\n').toInt();
    return 1;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    sendAT(GF("+QIRD="), mux, ',', size);
    if (waitResponse(GF("+QIRD:")) != 1) {
      return 0;
    }
    size_t len = stream.readStringUntil('\n').toInt();

    for (size_t i=0; i<len; i++) {
      while (!stream.available()) { TINY_GSM_YIELD(); }
      char c = stream.read();
      sockets[mux]->rx.put(c);
    }
    waitResponse();
    DBG("### READ:", mux, ",", len);
    return len;
  }

  size_t modemGetAvailable(uint8_t mux) {
    sendAT(GF("+QIRD="), mux, GF(",0"));
    size_t result = 0;
    if (waitResponse(GF("+QIRD:")) == 1) {
      streamSkipUntil(','); // Skip total received
      streamSkipUntil(','); // Skip have read
      result = stream.readStringUntil('\n').toInt();
      DBG("### STILL:", mux, "has", result);
      waitResponse();
    }
    if (!result) {
      sockets[mux]->sock_connected = modemGetConnected(mux);
    }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+QISTATE=1,"), mux);
    //+QISTATE: 0,"TCP","151.139.237.11",80,5087,4,1,0,0,"uart1"

    if (waitResponse(GF("+QISTATE:")))
      return false;

    streamSkipUntil(','); // Skip mux
    streamSkipUntil(','); // Skip socket type
    streamSkipUntil(','); // Skip remote ip
    streamSkipUntil(','); // Skip remote port
    streamSkipUntil(','); // Skip local port
    int res = stream.readStringUntil(',').toInt(); // socket state

    waitResponse();

    // 0 Initial, 1 Opening, 2 Connected, 3 Listening, 4 Closing
    return 2 == res;
  }

public:

  /*
   Utilities
   */

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
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL, GsmConstStr r6=NULL)
  {
    /*String r1s(r1); r1s.trim();
    String r2s(r2); r2s.trim();
    String r3s(r3); r3s.trim();
    String r4s(r4); r4s.trim();
    String r5s(r5); r5s.trim();
    String r6s(r6); r6s.trim();
    DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s, ",", r6s);*/
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
        } else if (r6 && data.endsWith(r6)) {
          index = 6;
          goto finish;
        } else if (data.endsWith(GF(GSM_NL "+QIRD:"))) {
          streamSkipUntil(',');  // Skip the context
          streamSkipUntil(',');  // Skip the role
          int mux = stream.readStringUntil('\n').toInt();
          DBG("### Got Data:", mux);
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data = true;
          }
        } else if (data.endsWith(GF("CLOSED" GSM_NL))) {
          int nl = data.lastIndexOf(GSM_NL, data.length()-8);
          int coma = data.indexOf(',', nl+2);
          int mux = data.substring(nl+2, coma).toInt();
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
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL, GsmConstStr r6=NULL)
  {
    String data;
    return waitResponse(timeout, data, r1, r2, r3, r4, r5, r6);
  }

  uint8_t waitResponse(GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL, GsmConstStr r6=NULL)
  {
    return waitResponse(1000, r1, r2, r3, r4, r5, r6);
  }

public:
  Stream&       stream;

protected:
  GsmClient*    sockets[TINY_GSM_MUX_COUNT];
};

#endif
