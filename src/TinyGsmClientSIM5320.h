/**
 * @file       TinyGsmClientSIM5320.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2019 Volodymyr Shymanskyy
 * @date       Aug 2019
 */

#ifndef TinyGsmClientSIM5320_h
#define TinyGsmClientSIM5320_h

//#define TINY_GSM_DEBUG Serial
//#define TINY_GSM_USE_HEX

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 64
#endif

#define TINY_GSM_MUX_COUNT 2

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
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};

enum TinyGSMDateTimeFormat {
  DATE_FULL = 0,
  DATE_TIME = 1,
  DATE_DATE = 2
};

class TinyGsmSim5320
{

public:

class GsmClient : public Client
{
  friend class TinyGsmSim5320;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmSim5320& modem, uint8_t mux = 1) {
    init(&modem, mux);
  }

  bool init(TinyGsmSim5320* modem, uint8_t mux = 1) {
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
    at->sendAT(GF("+CCHCLOSE="), mux);
    sock_connected = false;
    at->waitResponse();
    rx.clear();
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
    if (!rx.size() && sock_connected) {
      at->maintain();
    }
    return rx.size() + sock_available;
  }

  virtual int read(uint8_t *buf, size_t size) {
    TINY_GSM_YIELD();
    at->maintain();
    size_t cnt = 0;
    while (cnt < size && sock_connected) {
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
  TinyGsmSim5320* at;
  uint8_t        mux;
  uint16_t       sock_available;
  bool           sock_connected;
  bool           got_data;
  RxFifo         rx;
};

class GsmClientSecure : public GsmClient
{
public:
  GsmClientSecure() {}

  GsmClientSecure(TinyGsmSim5320& modem, uint8_t mux = 1)
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

public:

  TinyGsmSim5320(Stream& stream)
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
    sendAT(GF("&F"));  // Factory + Reset
    waitResponse();
    sendAT(GF("E0"));   // Echo Off
    if (waitResponse() != 1) {
      return false;
    }
    getSimStatus();
    return true;
  }

  void setBaud(unsigned long baud) {
    sendAT(GF("+IPREX="), baud);
  }

  bool testAT(unsigned long timeout = 10000L) {
    //streamWrite(GF("AAAAA" GSM_NL));  // TODO: extra A's to help detect the baud rate
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
    sendAT(GF("+IFC=0,0")); // No Flow Control
    waitResponse();
    sendAT(GF("+ICF=3,3")); // 8 data 0 parity 1 stop
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
    //Enable Local Time Stamp for getting network time
    // TODO: Find a better place for this
    sendAT(GF("+CTZU=1"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    sendAT(GF("&W"));
    waitResponse();
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
    sendAT(GF("+CPOF"));
    return waitResponse() == 1;
  }

  bool radioOff() {
    sendAT(GF("+CFUN=0"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);
    return true;
  }

  /*
   * SIM card functions
   */

  bool simUnlock(const char *pin) {
    sendAT(GF("+CPIN=\""), pin, GF("\""));
    return waitResponse() == 1;
  }

  String getSimCCID() {
    sendAT(GF("+CICCID"));
    if (waitResponse(GF(GSM_NL "+ICCID:")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  String getIMEI() {
    sendAT(GF("+CGSN"));
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

  int getSignalQuality() {
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
   * GPRS functions
   */
  bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {
    gprsDisconnect();
    
    // Set the APN
    sendAT(GF("+CGSOCKCONT=1,\"IP\",\""), apn, '"');  
    waitResponse();

    // Set the user name and password
    sendAT(GF("+CSOCKAUTH=1,1,\""), user, GF("\",\""), pwd, '"');
    waitResponse();

    // Define the PDP context
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Activate the PDP context
    sendAT(GF("+CGACT=1,1"));
    waitResponse(60000L);

    // Set to get data manually
    sendAT(GF("+CCHSET=0,1"));
    if (waitResponse() != 1) {
      return false;
    }

    // Acquire common channel service
    sendAT(GF("+CCHSTART"));
    if (waitResponse() != 1) {
      return false;
    }
    waitResponse(10000L, GF("+CCHSTART: 0"));

    return true;
  }

  bool gprsDisconnect() {
    // Close the channels
    for (int i=0; i<TINY_GSM_MUX_COUNT; i++) {
      sendAT(GF("+CCHCLOSE="), i);
      if (waitResponse() == 1) {
        waitResponse(60000L, GF("+CCHCLOSE: "));
      }
    }

    // Stop common channel service
    sendAT(GF("+CCHSTOP"));
    if (waitResponse() == 1) {
      waitResponse(60000L, GF("+CCHSTOP: "));
    }

    sendAT(GF("+CGACT=0"));  // Deactivate the PDP context
    if (waitResponse(60000L) != 1)
      return false;

    return true;
  }

  bool isGprsConnected() {
    sendAT(GF("+CGACT?"));
    if (waitResponse(GF(GSM_NL "+CGACT:")) != 1) {
      return false;
    }
    int res = stream.readStringUntil(',').toInt();
    waitResponse();
    if (res != 1)
      return false;

    sendAT(GF("+CCHADDR"));
    if (waitResponse() != 1)
      return false;

    return true;
  }

  String getLocalIP() {
    sendAT(GF("+CCHADDR")); // IPADDR
    String res;
    if (waitResponse(10000L, res) != 1) {
      return "";
    }
    res.replace(GSM_NL "OK" GSM_NL, "");
    res.replace(GSM_NL, "");
    res.trim();
    return res;
  }

  IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
  }

  /*
   * Phone Call functions
   */

  bool setGsmBusy() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool callAnswer() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  // Returns true on pick-up, false on error/busy
  bool callNumber() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool callHangup() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  // 0-9,*,#,A,B,C,D
  bool dtmfSend() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Messaging functions
   */

  String sendUSSD() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sendSMS() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sendSMS_UTF16() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * GPS location functions
   */

  bool enableGPS() {
    sendAT(GF("+CGPS=1,1"));
    if (waitResponse() != 1) {
      return false;
    }

    return true;
  }

  bool disableGPS() {
    sendAT(GF("+CGPS=0"));
    if (waitResponse() != 1) {
      return false;
    }

    return true;
  }

  // get the RAW GPS output
  String getGPSraw() {
    sendAT(GF("+CGPSINFO"));
    if (waitResponse(GF(GSM_NL "+CGPSINFO:")) != 1) {
      return "";
    }

    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  bool getGPS(float *lat, float *lon, float *speed=nullptr, int *alt=nullptr) {
    char latitude[12], longitude[13];
    bool result = getGPS(latitude, longitude, speed, alt);
    *lat = atof(latitude);
    *lon = atof(longitude);
    return result;
  }

  bool getGPS(char *lat, char *lon, float *speed=nullptr, int *alt=nullptr) {
    sendAT(GF("+CGPSINFO"));
    if (waitResponse(GF(GSM_NL "+CGPSINFO:")) != 1) {
      return false;
    }

    stream.readBytesUntil(',', lat, 12); // lat
    stream.readStringUntil(','); // N/S indicator
    stream.readBytesUntil(',', lon, 13); // lon
    stream.readStringUntil(','); // E/W indicator
    stream.readStringUntil(','); // date
    stream.readStringUntil(','); // utctime
    if (alt != nullptr) *alt =  stream.readStringUntil(',').toInt(); // alt
    if (speed != nullptr) *speed = stream.readStringUntil(',').toFloat(); // speed

    stream.readStringUntil('\n');
    waitResponse();

    return (0 != atof(lat));
  }

  // get GPS time
  bool getGPSTime(int *year, int *month, int *day, int *hour, int *minute, int *second) {
    sendAT(GF("+CGPSINFO"));
    if (waitResponse(GF(GSM_NL "+CGPSINFO:")) != 1) {
      return false;
    }

    stream.readStringUntil(','); // lat
    stream.readStringUntil(','); // N/S indicator
    stream.readStringUntil(','); // lon
    stream.readStringUntil(','); // E/W indicator

    String buffer = stream.readStringUntil(','); // date
    *day = buffer.substring(0, 2).toInt();
    *month = buffer.substring(2, 4).toInt();
    *year = buffer.substring(4, 6).toInt();

    buffer = stream.readStringUntil(','); // utctime
    *hour = buffer.substring(0, 2).toInt();
    *minute = buffer.substring(2, 4).toInt();
    *second = buffer.substring(4, 6).toInt();

    stream.readStringUntil('\n');
    waitResponse();

    return (0 == day);
  }

  /*
   * Time functions
   */
  String getGSMDateTime(TinyGSMDateTimeFormat format) {
    sendAT(GF("+CCLK?"));
    if (waitResponse(2000L, GF(GSM_NL "+CCLK: \"")) != 1) {
      return "";
    }

    String res;

    switch(format) {
      case DATE_FULL:
        res = stream.readStringUntil('"');
      break;
      case DATE_TIME:
        streamSkipUntil(',');
        res = stream.readStringUntil('"');
      break;
      case DATE_DATE:
        res = stream.readStringUntil(',');
      break;
    }
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

    uint16_t res = stream.readStringUntil('V').toInt();
    waitResponse();
    return res;
  }

  int getBattPercent() {
    sendAT(GF("+CBC"));
    if (waitResponse(GF(GSM_NL "+CBC:")) != 1) {
      return false;
    }
    stream.readStringUntil(',');
    int res = stream.readStringUntil(',').toInt();
    waitResponse();
    return res;
  }

protected:

  bool modemConnect(const char* host, uint16_t port, uint8_t mux, bool ssl = false) {
    uint8_t channel_type;
    if (ssl) {
      channel_type = 2;
    } else {
      channel_type = 1;
    }
    sendAT(GF("+CCHOPEN="), mux, GF(",\""), host, GF("\","), port, ',', channel_type);

    waitResponse();
    if (waitResponse(30000L, GF("+CCHOPEN: ")) != 1) {
      return false;
    }

    streamSkipUntil(','); // Skip mux
    uint8_t err = stream.readStringUntil('\n').toInt();
    return (0 == err);
  }

  size_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+CCHSEND="), mux, ',', len);
    if (waitResponse(GF(">")) != 1) {
      return 0;
    }
    stream.write((uint8_t*)buff, len);
    stream.flush();
    if (waitResponse() != 1) {
      return 0;
    }
    
    return len;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    sendAT(GF("+CCHRECV="), mux, ',', size);
    waitResponse();
    if (waitResponse(GF("+CCHRECV: DATA,")) != 1) {
      return 0;
    }
    streamSkipUntil(','); // Skip mux
    size_t len = stream.readStringUntil('\n').toInt();

    for (size_t i=0; i<len; i++) {
      while (!stream.available()) { TINY_GSM_YIELD(); }
      char c = stream.read();
      sockets[mux]->rx.put(c);
    }
    waitResponse(GF("+CCHRECV: "));
    streamSkipUntil('\n');
    return len;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (mux >= TINY_GSM_MUX_COUNT) return 0;
    sendAT(GF("+CCHRECV?"));
    size_t result = 0;
    if (waitResponse(GF("+CCHRECV: LEN,")) == 1) {
      result = stream.readStringUntil(',').toInt();
      if (mux == 1) {
        result = stream.readStringUntil('\n').toInt();
      }
    }
    waitResponse();

    if (!result) {
      sockets[mux]->sock_connected = modemGetConnected(mux);
    }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    if (mux >= TINY_GSM_MUX_COUNT) return false;
    sendAT(GF("+CCHSTATE"));
    bool isConnected = false;
    if (waitResponse(GF("+CCHSTATE: ")) == 1) {
      streamSkipUntil(','); // Skip network_state
      int result = 0;
      result = stream.readStringUntil(',').toInt();
      isConnected = (result == 4);
      if (mux == 1) {
        result = stream.readStringUntil('\n').toInt();
        isConnected = (result == 4);
      }
    }

    waitResponse();
    return isConnected;
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

  bool streamSkipUntil(const char c, const unsigned long timeout = 3000L) {
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
        } else if (data.endsWith(GF(GSM_NL "+CCHEVENT: "))) {
          int mux = stream.readStringUntil(',').toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data = true;
          }
          data = "";
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
