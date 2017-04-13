/**
 * @file       TinyWiFiClientESP8266.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClientXBee_h
#define TinyGsmClientXBee_h

// #define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 256
#endif

#include <TinyGsmCommon.h>

#define GSM_NL "\r"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;

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
    at->commandMode();
    sock_connected = at->modemConnect(host, port, mux);
    at->writeChanges();
    at->exitCommand();
    return sock_connected;
  }

  virtual int connect(IPAddress ip, uint16_t port) {
    TINY_GSM_YIELD();
    rx.clear();
    at->commandMode();
    sock_connected = at->modemConnect(ip, port, mux);
    at->writeChanges();
    at->exitCommand();
    return sock_connected;
  }

  virtual void stop() {  // Not supported
    sock_connected = false;
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
    return at->stream.available();
  }

  virtual int read(uint8_t *buf, size_t size) {
    return available();
  }

  virtual int read() {
    TINY_GSM_YIELD();
    return at->stream.read();
  }

  virtual int peek() { return at->stream.peek(); }
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
    guardTime = 1100;
    commandMode();
    sendAT(GF("AP0"));  // Put in transparent mode
    waitResponse();
    sendAT(GF("GTFA")); // shorten the guard time to 250ms
    waitResponse();
    writeChanges();
    exitCommand();
    guardTime = 300;
    return true;
  }

  bool autoBaud(unsigned long timeout = 10000L) {  // not supported
    return false;
  }

  void maintain() {
    //while (stream.available()) {
      waitResponse(10, NULL, NULL);
    //}
  }

  bool factoryDefault() {
    commandMode();
    sendAT(GF("RE"));
    bool ret_val = waitResponse() == 1;
    writeChanges();
    exitCommand();
    return ret_val;
  }

  /*
   * Power functions
   */

  bool restart() {
    commandMode();
    sendAT(GF("FR"));
    if (waitResponse() != 1) {
      return false;
    }
    delay (2000);  // Actually resets about 2 seconds later
    for (unsigned long start = millis(); millis() - start < 60000L; ) {
      if (commandMode()) {
        exitCommand();
        return true;
      }
    }
    exitCommand();
    return false;;
  }

  /*
   * SIM card & Networ Operator functions
   */

  bool simUnlock(const char *pin) {  // Not supported
    return false;
  }

  String getSimCCID() {
    commandMode();
    sendAT(GF("S#"));
    String res = streamReadUntil('\r');  // Does not send an OK, just the result
    exitCommand();
    return res;
  }

  String getIMEI() {
    commandMode();
    sendAT(GF("IM"));
    String res = streamReadUntil('\r');  // Does not send an OK, just the result
    exitCommand();
    return res;
  }

  int getSignalQuality() {
    commandMode();
    sendAT(GF("DB"));
    char buf[4] = { 0, };  // Does not send an OK, just the result
    buf[0] = streamRead();
    buf[1] = streamRead();
    buf[2] = streamRead();
    buf[3] = streamRead();
    exitCommand();
    int intr = strtol(buf, 0, 16);
    return intr;
  }

  SimStatus getSimStatus(unsigned long timeout = 10000L) {
    return SIM_READY;  // unsupported
  }

  RegStatus getRegistrationStatus() {
    commandMode();
    sendAT(GF("AI"));
    String res = streamReadUntil('\r');  // Does not send an OK, just the result
    exitCommand();

    if(res == GF("0x00"))
      return REG_OK_HOME;

    else if(res == GF("0x13") || res == GF("0x2A"))
      return REG_UNREGISTERED;

    else if(res == GF("0xFF") || res == GF("0x22") || res == GF("0x23") ||
            res == GF("0x40") || res == GF("0x41") || res == GF("0x42"))
      return REG_SEARCHING;

    else if(res == GF("0x24"))
      return REG_DENIED;

    else return REG_UNKNOWN;
  }

  String getOperator() {
    commandMode();
    sendAT(GF("MN"));
    String res = streamReadUntil('\r');  // Does not send an OK, just the result
    exitCommand();
    return res;
  }


  bool waitForNetwork(unsigned long timeout = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      commandMode();
      sendAT(GF("AI"));
      String res = streamReadUntil('\r');  // Does not send an OK, just the result
      exitCommand();
      if (res == GF("0")) {
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

    commandMode();

    sendAT(GF("AP"), 0);  // Put in transparent mode
    waitResponse();
    sendAT(GF("IP"), 1);  // Put in TCP mode
    waitResponse();
    sendAT(GF("EE"), 2);  // Set security to WPA2
    waitResponse();

    sendAT(GF("ID"), ssid);
    if (waitResponse() != 1) {
      goto fail;
    }

    sendAT(GF("PK"), pwd);
    if (waitResponse() != 1) {
      goto fail;
    }

    writeChanges();
    exitCommand();

    return true;

    fail:
      exitCommand();
      return false;
  }

  bool networkDisconnect() {
    return false;  // Doesn't support disconnecting
  }

  /*
   * GPRS functions
   */
  bool gprsConnect(const char* apn, const char* user = "", const char* pw = "") {
    commandMode();
    sendAT(GF("AP"), 0);  // Put in transparent mode
    waitResponse();
    sendAT(GF("IP"), 1);  // Put in TCP mode
    waitResponse();
    sendAT(GF("AN"), apn);  // Set the APN
    waitResponse();
    writeChanges();
    exitCommand();
    return true;
  }

  bool gprsDisconnect() {  // TODO
    return false;
  }

  /*
   * Messaging functions
   */

  void sendUSSD() {
  }

  void sendSMS() {
  }

  bool sendSMS(const String& number, const String& text) {
    commandMode();
    sendAT(GF("AP"), 0);  // Put in transparent mode
    waitResponse();
    sendAT(GF("IP"), 2);  // Put in text messaging mode
    waitResponse();
    sendAT(GF("PH"), number);  // Set the phone number
    waitResponse();
    sendAT(GF("TDD"));  // Set the text delimiter to the standard 0x0D (carriabe return)
    waitResponse();
    writeChanges();
    exitCommand();
    stream.print(text);
    stream.write((char)0x0D);  // close off with the carriage return
    return true;
  }

  /* Public Utilities */
  template<typename... Args>
  void sendAT(Args... cmd) {
    streamWrite("AT", cmd..., GSM_NL);
    stream.flush();
    TINY_GSM_YIELD();
    DBG(">>> AT ", cmd..., "\r\n");
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
        }
      }
    } while (millis() - startMillis < timeout);
  finish:
    if (!index) {
      data.trim();
      data.replace(GSM_NL GSM_NL, GSM_NL);
      data.replace(GSM_NL, "\r\n" "    ");
      if (data.length()) {
        DBG("### Unhandled:", data);
      }
    }
    else {
      data.trim();
      data.replace(GSM_NL GSM_NL, GSM_NL);
      data.replace(GSM_NL, "\r\n    ");
      if (data.length()) {
        DBG("<<< ", data, "\r\n");
      }
    }
    // if (gotData) {
    //   sockets[mux]->sock_available = modemGetAvailable(mux);
    // }
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
  int modemConnect(const char* host, uint16_t port, uint8_t mux = 1) {
    sendAT(GF("LA"), host);
    String ipadd; ipadd.reserve(16);
    ipadd = streamReadUntil('\r');
    IPAddress ip;
    ip.fromString(ipadd);
    return modemConnect(ip, port);
  }

  int modemConnect(IPAddress ip, uint16_t port, uint8_t mux = 1) {
    String host; host.reserve(16);
    host += ip[0];
    host += ".";
    host += ip[1];
    host += ".";
    host += ip[2];
    host += ".";
    host += ip[3];
    sendAT(GF("DL"), host);
    waitResponse();
    sendAT(GF("DE"), String(port, HEX));
    int rsp = waitResponse();
    return rsp;
  }

  int modemSend(const void* buff, size_t len, uint8_t mux = 1) {
    stream.write((uint8_t*)buff, len);
    return len;
  }

  bool modemGetConnected(uint8_t mux = 1) {
    commandMode();
    sendAT(GF("AI"));
    int res = waitResponse(GF("0"));
    exitCommand();
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

  int streamRead() {
    int c = stream.read();
    DBG((char)c);
    return c;
  }

  String streamReadUntil(char c) {
    String return_string = stream.readStringUntil(c);
    return_string.trim();
    if (String(c) == GSM_NL){
      DBG(return_string, "\r\n");
    } else DBG(return_string, c);
    return return_string;
  }

  bool commandMode(void){
    delay(guardTime);  // cannot send anything for 1 second before entering command mode
    streamWrite(GF("+++"));  // enter command mode
    DBG("\r\n+++\r\n");
    waitResponse(guardTime);
    return 1 == waitResponse(1100);  // wait another second for an "OK\r"
  }

  void writeChanges(void){
    sendAT(GF("WR"));  // Write changes to flash
    waitResponse();
    sendAT(GF("AC"));  // Apply changes
    waitResponse();
  }

  void exitCommand(void){
    sendAT(GF("CN"));  // Exit command mode
    waitResponse();
  }

private:
  int           guardTime;
  Stream&       stream;
  GsmClient*    sockets[1];
};

typedef TinyGsm::GsmClient TinyGsmClient;

#endif
