/**
 * @file       TinyGsmClientXBee.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClientXBee_h
#define TinyGsmClientXBee_h

// #define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 64
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
  CELL  = 0,
  WIFI  = 1,
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
    bool sock_connected = false;
    if (at->commandMode())  {  // Don't try if we didn't successfully get into command mode
      sock_connected = at->modemConnect(host, port, mux, false);
      at->writeChanges();
      at->exitCommand();
    }
    at->streamClear();  // Empty anything remaining in the buffer;
    return sock_connected;
  }

  virtual int connect(IPAddress ip, uint16_t port) {
    at->streamClear();  // Empty anything remaining in the buffer;
    bool sock_connected = false;
    if (at->commandMode())  {  // Don't try if we didn't successfully get into command mode
      sock_connected = at->modemConnect(ip, port, mux, false);
      at->writeChanges();
      at->exitCommand();
    }
    at->streamClear();  // Empty anything remaining in the buffer;
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
    at->sendAT(GF("TM64"));  // Set socket timeout back to 10 seconds;
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
    bool sock_connected = false;
    if (at->commandMode())  {  // Don't try if we didn't successfully get into command mode
      sock_connected = at->modemConnect(host, port, mux, true);
      at->writeChanges();
      at->exitCommand();
    }
    at->streamClear();  // Empty anything remaining in the buffer;
    return sock_connected;
  }

  virtual int connect(IPAddress ip, uint16_t port) {
    at->streamClear();  // Empty anything remaining in the buffer;
    bool sock_connected = false;
    if (at->commandMode())  {  // Don't try if we didn't successfully get into command mode
      sock_connected = at->modemConnect(ip, port, mux, true);
      at->writeChanges();
      at->exitCommand();
    }
    at->streamClear();  // Empty anything remaining in the buffer;
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
    guardTime = 1100;  // Start with a default guard time of 1 second

    if (!commandMode()) return false;

    sendAT(GF("AP0"));  // Put in transparent mode
    bool ret_val = waitResponse() == 1;
    ret_val &= writeChanges();

    sendAT(GF("GT64")); // shorten the guard time to 100ms
    ret_val &= waitResponse();
    ret_val &= writeChanges();
    if (ret_val) guardTime = 125;

    sendAT(GF("HS"));  // Get the "Hardware Series"; 0x601 for S6B (Wifi)
    int res = waitResponse(GF("601"));
    if (res == 1) beeType = WIFI;
    else beeType = CELL;

    exitCommand();
    return ret_val;
  }

  void setBaud(unsigned long baud) {
    if (!commandMode()) return;
    switch(baud)
    {
      case 2400: sendAT(GF("BD1")); break;
      case 4800: sendAT(GF("BD2")); break;
      case 9600: sendAT(GF("BD3")); break;
      case 19200: sendAT(GF("BD4")); break;
      case 38400: sendAT(GF("BD5")); break;
      case 57600: sendAT(GF("BD6")); break;
      case 115200: sendAT(GF("BD7")); break;
      case 230400: sendAT(GF("BD8")); break;
      case 460800: sendAT(GF("BD9")); break;
      case 921600: sendAT(GF("BDA")); break;
      default: {
          DBG(GF("Specified baud rate is unsupported! Setting to 9600 baud."));
          sendAT(GF("BD3")); // Set to default of 9600
          break;
      }
    }
    waitResponse();
    writeChanges();
    exitCommand();
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
    if (!commandMode()) return false;  // Return immediately
    sendAT(GF("RE"));
    bool ret_val = waitResponse() == 1;
    ret_val &= writeChanges();
    exitCommand();
    return ret_val;
  }

  bool hasSSL() {
    if (beeType == WIFI) return false;
    else return true;
  }

  /*
   * Power functions
   */

  bool restart() {
    if (!commandMode()) return false;  // Return immediately
    sendAT(GF("AM1"));  // Digi suggests putting into airplane mode before restarting
                       // This allows the sockets and connections to close cleanly
    writeChanges();
    if (waitResponse() != 1) goto fail;
    sendAT(GF("FR"));
    if (waitResponse() != 1) goto fail;

    delay (2000);  // Actually resets about 2 seconds later

    // Wait until reboot complete and responds to command mode call again
    for (unsigned long start = millis(); millis() - start < 60000L; ) {
      if (commandMode(1)) {
        sendAT(GF("AM0"));  // Turn off airplane mode
        writeChanges();
        exitCommand();
        delay(250);  // wait a litle before trying again
      }
    }
    return true;


    fail:
      exitCommand();
      return false;
  }

  void setupPinSleep(bool maintainAssociation = false) {
    if (!commandMode()) return;  // Return immediately
    sendAT(GF("SM"),1);  // Pin sleep
    waitResponse();
    if (beeType == WIFI && !maintainAssociation) {
        sendAT(GF("SO"),200);  // For lowest power, dissassociated deep sleep
        waitResponse();
    }
    else if (!maintainAssociation){
        sendAT(GF("SO"),1);  // For lowest power, dissassociated deep sleep
                             // Not supported by all modules, will return "ERROR"
        waitResponse();
    }
    writeChanges();
    exitCommand();
  }

  /*
   * SIM card functions
   */

  bool simUnlock(const char *pin) TINY_GSM_ATTR_NOT_AVAILABLE; // Not supported

  String getSimCCID() {
    if (!commandMode()) return "";  // Return immediately
    sendAT(GF("S#"));
    String res = readResponse();
    exitCommand();
    return res;
  }

  String getIMEI() {
    if (!commandMode()) return "";  // Return immediately
    sendAT(GF("IM"));
    String res = readResponse();
    exitCommand();
    return res;
  }

  SimStatus getSimStatus(unsigned long timeout) TINY_GSM_ATTR_NOT_AVAILABLE; // Not supported

  RegStatus getRegistrationStatus() {
    if (!commandMode()) return REG_UNREGISTERED;  // Return immediately

    sendAT(GF("AI"));
    String res = readResponse();
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
    if (!commandMode()) return "";  // Return immediately
    sendAT(GF("MN"));
    String res = readResponse();
    exitCommand();
    return res;
  }

 /*
  * Generic network functions
  */

  int getSignalQuality() {
    if (!commandMode()) return 0;  // Return immediately
    if (beeType == WIFI) sendAT(GF("LM"));  // ask for the "link margin" - the dB above sensitivity
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
    if (beeType == WIFI) return -93 + intr;  // the maximum sensitivity is -93dBm
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

    if (!commandMode()) return false;  // return immediately

    sendAT(GF("EE"), 2);  // Set security to WPA2
    if (waitResponse() != 1) goto fail;

    sendAT(GF("ID"), ssid);
    if (waitResponse() != 1) goto fail;

    sendAT(GF("PK"), pwd);
    if (waitResponse() != 1) goto fail;

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
    if (!commandMode()) return "";  // Return immediately
    sendAT(GF("MY"));
    String IPaddr; IPaddr.reserve(16);
    // wait for the response - this response can be very slow
    IPaddr = readResponse(30000);
    exitCommand();
    return IPaddr;
  }

  IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
  }

  /*
   * GPRS functions
   */
  bool gprsConnect(const char* apn, const char* user = "", const char* pw = "") {
    if (!commandMode()) return false;  // Return immediately
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
    if (!commandMode()) return false;  // Return immediately

    sendAT(GF("IP"), 2);  // Put in text messaging mode
    if (waitResponse() !=1) goto fail;
    sendAT(GF("PH"), number);  // Set the phone number
    if (waitResponse() !=1) goto fail;
    sendAT(GF("TDD"));  // Set the text delimiter to the standard 0x0D (carriage return)
    if (waitResponse() !=1) goto fail;
    if (!writeChanges()) goto fail;

    exitCommand();
    stream.print(text);
    stream.write((char)0x0D);  // close off with the carriage return
    return true;

    fail:
      exitCommand();
      return false;
  }


private:

  bool modemConnect(const char* host, uint16_t port, uint8_t mux = 0, bool ssl = false) {
    String strIP; strIP.reserve(16);
    unsigned long startMillis = millis();
    bool gotIP = false;
    // XBee's require a numeric IP address for connection, but do provide the
    // functionality to look up the IP address from a fully qualified domain name
    while (!gotIP && millis() - startMillis < 30000)  // the lookup can take a while
    {
      sendAT(GF("LA"), host);
      while (stream.available() < 4) {};// wait for the response
      strIP = streamReadUntil('\r');  // read result
      if (!strIP.endsWith(GF("ERROR"))) gotIP = true;
      delay(100);  // short wait before trying again
    }
    if (gotIP) {  // No reason to continue if we don't know the IP address
      IPAddress ip = TinyGsmIpFromString(strIP);
      return modemConnect(ip, port, mux, ssl);
    }
    else return false;
  }

  bool modemConnect(IPAddress ip, uint16_t port, uint8_t mux = 0, bool ssl = false) {
    bool success = true;
    String host; host.reserve(16);
    host += ip[0];
    host += ".";
    host += ip[1];
    host += ".";
    host += ip[2];
    host += ".";
    host += ip[3];
    if (ssl) {
      sendAT(GF("IP"), 4);  // Put in SSL over TCP communication mode
      success &= (1 == waitResponse());
    } else {
      sendAT(GF("IP"), 1);  // Put in TCP mode
      success &= (1 == waitResponse());
    }
    sendAT(GF("DL"), host);  // Set the "Destination Address Low"
    success &= (1 == waitResponse());
    sendAT(GF("DE"), String(port, HEX));  // Set the destination port
    success &= (1 == waitResponse());
    return success;
  }

  int modemSend(const void* buff, size_t len, uint8_t mux = 0) {
    stream.write((uint8_t*)buff, len);
    stream.flush();
    return len;
  }

  bool modemGetConnected(uint8_t mux = 0) {
    if (!commandMode()) return false;
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

  bool commandMode(int retries = 2) {
    int triesMade = 0;
    bool success = false;
    while (!success and triesMade < retries) {
      // Cannot send anything for 1 "guard time" before entering command mode
      // Default guard time is 1s, but the init fxn decreases it to 250 ms
      delay(guardTime);
      streamWrite(GF("+++"));  // enter command mode
      // DBG("\r\n+++");
      success = (1 == waitResponse(guardTime*2));
      triesMade ++;
    }
    return success;
  }

  bool writeChanges(void) {
    sendAT(GF("WR"));  // Write changes to flash
    if (1 != waitResponse()) return false;
    sendAT(GF("AC"));  // Apply changes
    if (1 != waitResponse()) return false;
    return true;
  }

  void exitCommand(void) {
    sendAT(GF("CN"));  // Exit command mode
    waitResponse();
  }

  String readResponse(uint32_t timeout = 1000) {
    unsigned long startMillis = millis();
    while (!stream.available() && millis() - startMillis < timeout) {};
    String res = streamReadUntil('\r');  // lines end with carriage returns
    return res;
  }

  template<typename... Args>
  void sendAT(Args... cmd) {
    streamWrite("AT", cmd..., GSM_NL);
    stream.flush();
    TINY_GSM_YIELD();
    // DBG("### AT:", cmd...);
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
    data.reserve(16);  // Should never be getting much here for the XBee
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
      data.replace(GSM_NL, "\r\n    ");
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

private:
  int           guardTime;
  XBeeType      beeType;
  Stream&       stream;
  GsmClient*    sockets[TINY_GSM_MUX_COUNT];
};

#endif
