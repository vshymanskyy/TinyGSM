/**
 * @file       TinyGsmClientXBee.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClientXBee_h
#define TinyGsmClientXBee_h

//#define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 256
#endif

#define TINY_GSM_MUX_COUNT 1  // Multi-plexing isn't supported using command mode

#include <TinyGsmCommon.h>

#define GSM_NL "\r"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;

enum SimStatus {
  SIM_ERROR = 0,
  SIM_READY = 1,
  SIM_LOCKED = 2,
};

enum XBeeType {
  S6B    = 0,
  LTEC1  = 1,
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

class GsmClient : public Client
{
  friend class TinyGsm;

public:
  GsmClient() {}

  GsmClient(TinyGsm& modem, uint8_t mux = 0) {
    init(&modem, mux);
  }

  bool init(TinyGsm* modem, uint8_t mux = 0) {
    this->at = modem;
    this->mux = mux;
    sock_connected = false;

    at->sockets[mux] = this;

    return true;
  }

public:
  virtual int connect(const char *host, uint16_t port) {
    at->streamClear();  // Empty anything remaining in the buffer;
    at->commandMode();
    sock_connected = at->modemConnect(host, port, mux, false);
    at->writeChanges();
    at->exitCommand();
    return sock_connected;
  }

  virtual int connect(IPAddress ip, uint16_t port) {
    at->streamClear();  // Empty anything remaining in the buffer;
    at->commandMode();
    sock_connected = at->modemConnect(ip, port, mux, false);
    at->writeChanges();
    at->exitCommand();
    return sock_connected;
  }

  // This is a hack to shut the socket by setting the timeout to zero and
  //  then sending an empty line to the server.
  virtual void stop() {
    at->streamClear();  // Empty anything remaining in the buffer;
    at->commandMode();
    at->sendAT(GF("TM0"));  // Set socket timeout to 0;
    at->waitResponse();
    at->writeChanges();
    at->exitCommand();
    at->modemSend("", 1, mux);
    at->commandMode();
    at->sendAT(GF("TM64"));  // Set socket timeout back to 10seconds;
    at->waitResponse();
    at->writeChanges();
    at->exitCommand();
    at->streamClear();  // Empty anything remaining in the buffer;
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
    TINY_GSM_YIELD();
    return at->stream.readBytes(buf, size);
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

  /*
   * Extended API
   */

  String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

private:
  TinyGsm*      at;
  uint8_t       mux;
  bool          sock_connected;
};

class GsmClientSecure : public GsmClient
{
public:
  GsmClientSecure() {}

  GsmClientSecure(TinyGsm& modem, uint8_t mux = 1)
    : GsmClient(modem, mux)
  {}

public:
  virtual int connect(const char *host, uint16_t port) {
    at->streamClear();  // Empty anything remaining in the buffer;
    at->commandMode();
    sock_connected = at->modemConnect(host, port, mux, true);
    at->writeChanges();
    at->exitCommand();
    return sock_connected;
  }

  virtual int connect(IPAddress ip, uint16_t port) {
    at->streamClear();  // Empty anything remaining in the buffer;
    at->commandMode();
    sock_connected = at->modemConnect(ip, port, mux, true);
    at->writeChanges();
    at->exitCommand();
    return sock_connected;
  }
};

public:

  TinyGsm(Stream& stream)
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
    guardTime = 1100;
    commandMode();
    sendAT(GF("AP0"));  // Put in transparent mode
    waitResponse();
    sendAT(GF("GT64")); // shorten the guard time to 100ms
    waitResponse();
    writeChanges();
    sendAT(GF("HS"));  // Get the "Hardware Series"; 0x601 for S6B (Wifi)
    // wait for the response
    unsigned long startMillis = millis();
    while (!stream.available() && millis() - startMillis < 1000) {};
    String res = streamReadUntil('\r');  // Does not send an OK, just the result
    exitCommand();
    if (res == "601") beeType = S6B;
    else beeType = LTEC1;
    guardTime = 125;
    return true;
  }

  bool testAT(unsigned long timeout = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      if (commandMode())
      {
          sendAT();
          if (waitResponse(200) == 1) {
              return true;
          }
          exitCommand();
      }
      delay(100);
    }
    return false;
  }

  void maintain() {}

  bool factoryDefault() {
    commandMode();
    sendAT(GF("RE"));
    bool ret_val = waitResponse() == 1;
    writeChanges();
    exitCommand();
    return ret_val;
  }

  bool hasSSL() {
    if (beeType == S6B) return false;
    else return true;
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

  void setupPinSleep() {
    commandMode();
    sendAT(GF("SM"),1);
    waitResponse();
    if (beeType == S6B) {
        sendAT(GF("SO"),200);
        waitResponse();
    }
    writeChanges();
    exitCommand();
  }

  /*
   * SIM card functions
   */

  bool simUnlock(const char *pin) {  // Not supported
    return false;
  }

  String getSimCCID() {
    commandMode();
    sendAT(GF("S#"));
    // wait for the response
    unsigned long startMillis = millis();
    while (!stream.available() && millis() - startMillis < 1000) {};
    String res = streamReadUntil('\r');  // Does not send an OK, just the result
    exitCommand();
    return res;
  }

  String getIMEI() {
    commandMode();
    sendAT(GF("IM"));
    // wait for the response
    unsigned long startMillis = millis();
    while (!stream.available() && millis() - startMillis < 1000) {};
    String res = streamReadUntil('\r');  // Does not send an OK, just the result
    exitCommand();
    return res;
  }

  SimStatus getSimStatus(unsigned long timeout = 10000L) {
    return SIM_READY;  // unsupported
  }

  RegStatus getRegistrationStatus() {
    commandMode();
    sendAT(GF("AI"));
    // wait for the response
    unsigned long startMillis = millis();
    while (!stream.available() && millis() - startMillis < 1000) {};
    String res = streamReadUntil('\r');  // Does not send an OK, just the result
    exitCommand();

    if(res == GF("0"))
      return REG_OK_HOME;

    else if(res == GF("13") || res == GF("2A"))
      return REG_UNREGISTERED;

    else if(res == GF("FF") || res == GF("22") || res == GF("23") ||
            res == GF("40") || res == GF("41") || res == GF("42"))
      return REG_SEARCHING;

    else if(res == GF("24") || res == GF("25") || res == GF("27"))
      return REG_DENIED;

    else return REG_UNKNOWN;
  }

  String getOperator() {
    commandMode();
    sendAT(GF("MN"));
    // wait for the response
    unsigned long startMillis = millis();
    while (!stream.available() && millis() - startMillis < 1000) {};
    String res = streamReadUntil('\r');  // Does not send an OK, just the result
    exitCommand();
    return res;
  }

 /*
  * Generic network functions
  */

  int getSignalQuality() {
    commandMode();
    if (beeType == S6B) sendAT(GF("LM"));  // ask for the "link margin" - the dB above sensitivity
    else sendAT(GF("DB"));  // ask for the cell strength in dBm
    // wait for the response
    unsigned long startMillis = millis();
    while (!stream.available() && millis() - startMillis < 1000) {};
    char buf[2] = {0};  // Set up buffer for response
    buf[0] = streamRead();
    buf[1] = streamRead();
    // DBG(buf[0], buf[1], "\n");
    exitCommand();
    int intr = strtol(buf, 0, 16);
    if (beeType == S6B) return -93 + intr;  // the maximum sensitivity is -93dBm
    else return -1*intr; // need to convert to negative number
  }

  bool isNetworkConnected() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  bool waitForNetwork(unsigned long timeout = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      if (isNetworkConnected()) {
        return true;
      }
      delay(250);
    }
    return false;
  }

  /*
   * WiFi functions
   */
  bool networkConnect(const char* ssid, const char* pwd) {

    commandMode();

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

  String getLocalIP() {
    commandMode();
    sendAT(GF("MY"));
    String IPaddr; IPaddr.reserve(16);
    // wait for the response
    unsigned long startMillis = millis();
    while (stream.available() < 8 && millis() - startMillis < 30000) {};
    IPaddr = streamReadUntil('\r');  // read result
    return IPaddr;
  }

  IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
  }

  /*
   * GPRS functions
   */
  bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {
    commandMode();
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

  String sendUSSD(const String& code) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sendSMS(const String& number, const String& text) {
    commandMode();
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


private:

  int modemConnect(const char* host, uint16_t port, uint8_t mux = 0, bool ssl = false) {
    sendAT(GF("LA"), host);
    String strIP; strIP.reserve(16);
    // wait for the response
    unsigned long startMillis = millis();
    while (stream.available() < 8 && millis() - startMillis < 30000) {};
    strIP = streamReadUntil('\r');  // read result
    IPAddress ip = TinyGsmIpFromString(strIP);
    return modemConnect(ip, port, mux, ssl);
  }

  int modemConnect(IPAddress ip, uint16_t port, uint8_t mux = 0, bool ssl = false) {
    String host; host.reserve(16);
    host += ip[0];
    host += ".";
    host += ip[1];
    host += ".";
    host += ip[2];
    host += ".";
    host += ip[3];
    if (ssl) {
      sendAT(GF("IP"), 4);  // Put in TCP mode
      waitResponse();
    } else {
      sendAT(GF("IP"), 1);  // Put in TCP mode
      waitResponse();
    }
    sendAT(GF("DL"), host);  // Set the "Destination Address Low"
    waitResponse();
    sendAT(GF("DE"), String(port, HEX));  // Set the destination port
    int rsp = waitResponse();
    return rsp;
  }

  int modemSend(const void* buff, size_t len, uint8_t mux = 0) {
    stream.write((uint8_t*)buff, len);
    stream.flush();
    return len;
  }

  bool modemGetConnected(uint8_t mux = 0) {
    commandMode();
    sendAT(GF("AI"));
    int res = waitResponse(GF("0"));
    exitCommand();
    return 1 == res;
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

  int streamRead() { return stream.read(); }

  String streamReadUntil(char c) {
    TINY_GSM_YIELD();
    String return_string = stream.readStringUntil(c);
    return_string.trim();
    // DBG(return_string, c);
    return return_string;
  }

  void streamClear(void) {
    while (stream.available()) { streamRead(); }
  }

  bool commandMode(void) {
    delay(guardTime);  // cannot send anything for 1 second before entering command mode
    streamWrite(GF("+++"));  // enter command mode
    // DBG("\r\n+++\r\n");
    return 1 == waitResponse(guardTime*2);
  }

  void writeChanges(void) {
    sendAT(GF("WR"));  // Write changes to flash
    waitResponse();
    sendAT(GF("AC"));  // Apply changes
    waitResponse();
  }

  void exitCommand(void) {
    sendAT(GF("CN"));  // Exit command mode
    waitResponse();
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
        DBG("### Unhandled:", data, "\r\n");
      } else {
        DBG("### NO RESPONSE!\r\n");
      }
    } else {
      data.trim();
      data.replace(GSM_NL GSM_NL, GSM_NL);
      data.replace(GSM_NL, "\r\n    ");
      if (data.length()) {
        // DBG("<<< ", data);
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

public:
  Stream&       stream;

protected:
  int           guardTime;
  XBeeType      beeType;
  GsmClient*    sockets[TINY_GSM_MUX_COUNT];
};

#endif
