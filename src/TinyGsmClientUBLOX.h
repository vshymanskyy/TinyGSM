/**
 * @file       TinyGsmClientUBLOX.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmClientUBLOX_h
#define TinyGsmClientUBLOX_h
//#pragma message("TinyGSM:  TinyGsmClientUBLOX")

//#define TINY_GSM_DEBUG Serial

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 64
#endif

#define TINY_GSM_MUX_COUNT 7

#include <TinyGsmCommon.h>

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;
static const char GSM_CME_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CME ERROR:";

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


class TinyGsmUBLOX
{

public:

class GsmClient : public Client
{
  friend class TinyGsmUBLOX;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmUBLOX& modem, uint8_t mux = 0) {
    init(&modem, mux);
  }

  virtual ~GsmClient(){}

  bool init(TinyGsmUBLOX* modem, uint8_t mux = 0) {
    this->at = modem;
    this->mux = mux;
    sock_available = 0;
    prev_check = 0;
    sock_connected = false;
    got_data = false;

    at->sockets[mux] = this;

    return true;
  }

public:
  virtual int connect(const char *host, uint16_t port, int timeout_s) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();

    uint8_t oldMux = mux;
    sock_connected = at->modemConnect(host, port, &mux, false, timeout_s);
    if (mux != oldMux) {
        DBG("WARNING:  Mux number changed from", oldMux, "to", mux);
        at->sockets[oldMux] = NULL;
    }
    at->sockets[mux] = this;
    at->maintain();

    return sock_connected;
  }

TINY_GSM_CLIENT_CONNECT_OVERLOADS()

  virtual void stop(uint32_t maxWaitMs) {
    TINY_GSM_CLIENT_DUMP_MODEM_BUFFER()
    at->sendAT(GF("+USOCL="), mux);
    at->waitResponse();  // should return within 1s
    sock_connected = false;
  }

  virtual void stop() { stop(15000L); }

TINY_GSM_CLIENT_WRITE()

TINY_GSM_CLIENT_AVAILABLE_WITH_BUFFER_CHECK()

TINY_GSM_CLIENT_READ_WITH_BUFFER_CHECK()

TINY_GSM_CLIENT_PEEK_FLUSH_CONNECTED()

  /*
   * Extended API
   */

  String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

private:
  TinyGsmUBLOX*   at;
  uint8_t         mux;
  uint16_t        sock_available;
  uint32_t        prev_check;
  bool            sock_connected;
  bool            got_data;
  RxFifo          rx;
};


class GsmClientSecure : public GsmClient
{
public:
  GsmClientSecure() {}

  GsmClientSecure(TinyGsmUBLOX& modem, uint8_t mux = 1)
    : GsmClient(modem, mux)
  {}

  virtual ~GsmClientSecure(){}

public:
  virtual int connect(const char *host, uint16_t port, int timeout_s) {
    stop();
    TINY_GSM_YIELD();
    rx.clear();
    uint8_t oldMux = mux;
    sock_connected = at->modemConnect(host, port, &mux, true, timeout_s);
    if (mux != oldMux) {
        DBG("WARNING:  Mux number changed from", oldMux, "to", mux);
        at->sockets[oldMux] = NULL;
    }
    at->sockets[mux] = this;
    at->maintain();
    return sock_connected;
  }
};


public:

  TinyGsmUBLOX(Stream& stream)
    : stream(stream)
  {
    memset(sockets, 0, sizeof(sockets));
  }

  virtual ~TinyGsmUBLOX() {}

  /*
   * Basic functions
   */

  bool begin(const char* pin = NULL) {
    return init(pin);
  }

  bool init(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);

    if (!testAT()) {
      return false;
    }

    sendAT(GF("E0"));   // Echo Off
    if (waitResponse() != 1) {
      return false;
    }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    getModemName();

    int ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != NULL && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    }
    // if the sim is ready, or it's locked but no pin has been provided, return true
    else {
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  String getModemName() {
    sendAT(GF("+CGMI"));
    String res1;
    if (waitResponse(1000L, res1) != 1) {
      return "u-blox Cellular Modem";
    }
    res1.replace(GSM_NL "OK" GSM_NL, "");
    res1.trim();

    sendAT(GF("+GMM"));
    String res2;
    if (waitResponse(1000L, res2) != 1) {
      return "u-blox Cellular Modem";
    }
    res2.replace(GSM_NL "OK" GSM_NL, "");
    res2.trim();

    String name = res1 + String(' ') + res2;
    DBG("### Modem:", name);
    if (name.startsWith("u-blox SARA-R4") || name.startsWith("u-blox SARA-N4")) {
      DBG("### WARNING:  You are using the wrong TinyGSM modem!");
    }
    else if (name.startsWith("u-blox SARA-N2")) {
      DBG("### SARA N2 NB-IoT modems not supported!");
    }

    return name;
  }

TINY_GSM_MODEM_SET_BAUD_IPR()

TINY_GSM_MODEM_TEST_AT()

TINY_GSM_MODEM_MAINTAIN_CHECK_SOCKS()

  bool factoryDefault() {
    sendAT(GF("+UFACTORY=0,1"));  // No factory restore, erase NVM
    waitResponse();
    sendAT(GF("+CFUN=16"));   // Reset
    return waitResponse() == 1;
  }

TINY_GSM_MODEM_GET_INFO_ATI()

  bool hasSSL() {
    return true;
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
    sendAT(GF("+CFUN=16"));
    if (waitResponse(10000L) != 1) {
      return false;
    }
    delay(3000);  // TODO:  Verify delay timing here
    return init();
  }

  bool poweroff() {
    sendAT(GF("+CPWROFF"));
    return waitResponse(40000L) == 1;
  }

  bool radioOff() {
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

TINY_GSM_MODEM_SIM_UNLOCK_CPIN()

TINY_GSM_MODEM_GET_SIMCCID_CCID()

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

  SimStatus getSimStatus(unsigned long timeout_ms = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
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

TINY_GSM_MODEM_GET_REGISTRATION_XREG(CGREG)

TINY_GSM_MODEM_GET_OPERATOR_COPS()

  /*
   * Generic network functions
   */

TINY_GSM_MODEM_GET_CSQ()

  bool isNetworkConnected() {
    RegStatus s = getRegistrationStatus();
    if (s == REG_OK_HOME || s == REG_OK_ROAMING)
      return true;
    else if (s == REG_UNKNOWN)  // for some reason, it can hang at unknown..
      return isGprsConnected();
    else return false;
  }

TINY_GSM_MODEM_WAIT_FOR_NETWORK()

  /*
   * GPRS functions
   */

  bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {
    gprsDisconnect();

    sendAT(GF("+CGATT=1"));  // attach to GPRS
    if (waitResponse(360000L) != 1) {
      return false;
    }

    // Setting up the PSD profile/PDP context with the UPSD commands sets up an
    // "internal" PDP context, i.e. a data connection using the internal IP
    // stack and related AT commands for sockets.

    // Packet switched data configuration
    // AT+UPSD=<profile_id>,<param_tag>,<param_val>
    // profile_id = 0 - PSD profile identifier, in range 0-6 (NOT PDP context)
    // param_tag = 1: APN
    // param_tag = 2: username
    // param_tag = 3: password
    // param_tag = 7: IP address Note: IP address set as "0.0.0.0" means
    //    dynamic IP address assigned during PDP context activation
    sendAT(GF("+UPSD=0,1,\""), apn, '"');  // Set APN for PSD profile 0
    waitResponse();

    if (user && strlen(user) > 0) {
      sendAT(GF("+UPSD=0,2,\""), user, '"');  // Set user for PSD profile 0
      waitResponse();
    }
    if (pwd && strlen(pwd) > 0) {
      sendAT(GF("+UPSD=0,3,\""), pwd, '"');  // Set password for PSD profile 0
      waitResponse();
    }

    sendAT(GF("+UPSD=0,7,\"0.0.0.0\"")); // Dynamic IP on PSD profile 0
    waitResponse();

    // Packet switched data action
    // AT+UPSDA=<profile_id>,<action>
    // profile_id = 0: PSD profile identifier, in range 0-6 (NOT PDP context)
    // action = 3: activate; it activates a PDP context with the specified profile,
    // using the current parameters
    sendAT(GF("+UPSDA=0,3")); // Activate the PDP context associated with profile 0
    if (waitResponse(360000L) != 1) {  // Should return ok
      return false;
    }

    // Packet switched network-assigned data - Returns the current (dynamic)
    // network-assigned or network-negotiated value of the specified parameter
    // for the active PDP context associated with the specified PSD profile.
    // AT+UPSND=<profile_id>,<param_tag>
    // profile_id = 0: PSD profile identifier, in range 0-6 (NOT PDP context)
    // param_tag = 8: PSD profile status: if the profile is active the return value is 1, 0 otherwise
    sendAT(GF("+UPSND=0,8")); // Check if PSD profile 0 is now active
    int res = waitResponse(GF(",8,1"), GF(",8,0"));
    waitResponse();  // Should return another OK
    if (res == 1) {
      return true;  // It's now active
    } else if (res == 2) {  // If it's not active yet, wait for the +UUPSDA URC
      if (waitResponse(180000L, GF("+UUPSDA: 0")) != 1) {  // 0=successful
        return false;
      }
      streamSkipUntil('\n');  // Ignore the IP address, if returned
    } else {
      return false;
    }

    return true;
  }

  bool gprsDisconnect() {
    sendAT(GF("+UPSDA=0,4"));  // Deactivate the PDP context associated with profile 0
    if (waitResponse(360000L) != 1) {
      return false;
    }

    sendAT(GF("+CGATT=0"));  // detach from GPRS
    if (waitResponse(360000L) != 1) {
      return false;
    }

    return true;
  }

TINY_GSM_MODEM_GET_GPRS_IP_CONNECTED()

  /*
   * IP Address functions
   */

  String getLocalIP() {
    sendAT(GF("+UPSND=0,0"));
    if (waitResponse(GF(GSM_NL "+UPSND:")) != 1) {
      return "";
    }
    streamSkipUntil(',');  // Skip PSD profile
    streamSkipUntil('\"'); // Skip request type
    String res = stream.readStringUntil('\"');
    if (waitResponse() != 1) {
      return "";
    }
    return res;
  }

  IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
  }

  /*
   * Phone Call functions
   */

  bool setGsmBusy(bool busy = true) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool callAnswer() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool callNumber(const String& number) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool callHangup() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Messaging functions
   */

  String sendUSSD(const String& code) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sendSMS(const String& number, const String& text) {
    sendAT(GF("+CSCS=\"GSM\""));  // Set GSM default alphabet
    waitResponse();
    sendAT(GF("+CMGF=1"));  // Set preferred message format to text mode
    waitResponse();
    sendAT(GF("+CMGS=\""), number, GF("\""));  // set the phone number
    if (waitResponse(GF(">")) != 1) {
      return false;
    }
    stream.print(text);  // Actually send the message
    stream.write((char)0x1A);
    stream.flush();
    return waitResponse(60000L) == 1;
  }

  bool sendSMS_UTF16(const String& number, const void* text, size_t len) TINY_GSM_ATTR_NOT_IMPLEMENTED;


  /*
   * Location functions
   */

  String getGsmLocation() {
    sendAT(GF("+ULOC=2,3,0,120,1"));
    if (waitResponse(30000L, GF(GSM_NL "+UULOC:")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Battery & temperature functions
   */

  uint16_t getBattVoltage() TINY_GSM_ATTR_NOT_AVAILABLE;

  int8_t getBattPercent() {
    sendAT(GF("+CIND?"));
    if (waitResponse(GF(GSM_NL "+CIND:")) != 1) {
      return 0;
    }

    int res = stream.readStringUntil(',').toInt();
    int8_t percent = res*20;  // return is 0-5
    // Wait for final OK
    waitResponse();
    return percent;
  }

  uint8_t getBattChargeState() TINY_GSM_ATTR_NOT_AVAILABLE;

  bool getBattStats(uint8_t &chargeState, int8_t &percent, uint16_t &milliVolts) {
    chargeState = 0;
    percent = getBattPercent();
    milliVolts = 0;
    return true;
  }

  // This would only available for a small number of modules in this group (TOBY-L)
  float getTemperature() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Client related functions
   */

protected:

  bool modemConnect(const char* host, uint16_t port, uint8_t* mux,
                    bool ssl = false, int timeout_s = 120)
  {
    uint32_t timeout_ms = ((uint32_t)timeout_s)*1000;
    sendAT(GF("+USOCR=6"));  // create a socket
    if (waitResponse(GF(GSM_NL "+USOCR:")) != 1) {  // reply is +USOCR: ## of socket created
      return false;
    }
    *mux = stream.readStringUntil('\n').toInt();
    waitResponse();

    if (ssl) {
      sendAT(GF("+USOSEC="), *mux, ",1");
      waitResponse();
    }

    // Enable NODELAY
    sendAT(GF("+USOSO="), *mux, GF(",6,1,1"));
    waitResponse();

    // Enable KEEPALIVE, 30 sec
    //sendAT(GF("+USOSO="), *mux, GF(",6,2,30000"));
    //waitResponse();

    // connect on the allocated socket
    sendAT(GF("+USOCO="), *mux, ",\"", host, "\",", port);
    int rsp = waitResponse(timeout_ms);
    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+USOWR="), mux, ',', (uint16_t)len);
    if (waitResponse(GF("@")) != 1) {
      return 0;
    }
    // 50ms delay, see AT manual section 25.10.4
    delay(50);
    stream.write((uint8_t*)buff, len);
    stream.flush();
    if (waitResponse(GF(GSM_NL "+USOWR:")) != 1) {
      return 0;
    }
    streamSkipUntil(','); // Skip mux
    int sent = stream.readStringUntil('\n').toInt();
    waitResponse();  // sends back OK after the confirmation of number sent
    return sent;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    sendAT(GF("+USORD="), mux, ',', (uint16_t)size);
    if (waitResponse(GF(GSM_NL "+USORD:")) != 1) {
      return 0;
    }
    streamSkipUntil(','); // Skip mux
    int len = stream.readStringUntil(',').toInt();
    streamSkipUntil('\"');

    for (int i=0; i<len; i++) {
      TINY_GSM_MODEM_STREAM_TO_MUX_FIFO_WITH_DOUBLE_TIMEOUT
    }
    streamSkipUntil('\"');
    waitResponse();
    DBG("### READ:", len, "from", mux);
    sockets[mux]->sock_available = modemGetAvailable(mux);
    return len;
  }

  size_t modemGetAvailable(uint8_t mux) {
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
    sendAT(GF("+USORD="), mux, ",0");
    size_t result = 0;
    uint8_t res = waitResponse(GF(GSM_NL "+USORD:"));
    // Will give error "operation not allowed" when attempting to read a socket
    // that you have already told to close
    if (res == 1) {
      streamSkipUntil(','); // Skip mux
      result = stream.readStringUntil('\n').toInt();
      // if (result) DBG("### DATA AVAILABLE:", result, "on", mux);
      waitResponse();
    }
    if (!result) {
      sockets[mux]->sock_connected = modemGetConnected(mux);
    }
    DBG("### AVAILABLE:", result, "on", mux);
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
    sendAT(GF("+USOCTL="), mux, ",10");
    uint8_t res = waitResponse(GF(GSM_NL "+USOCTL:"));
    if (res != 1)
      return false;

    streamSkipUntil(','); // Skip mux
    streamSkipUntil(','); // Skip type
    int result = stream.readStringUntil('\n').toInt();
    // 0: the socket is in INACTIVE status (it corresponds to CLOSED status
    // defined in RFC793 "TCP Protocol Specification" [112])
    // 1: the socket is in LISTEN status
    // 2: the socket is in SYN_SENT status
    // 3: the socket is in SYN_RCVD status
    // 4: the socket is in ESTABILISHED status
    // 5: the socket is in FIN_WAIT_1 status
    // 6: the socket is in FIN_WAIT_2 status
    // 7: the sokcet is in CLOSE_WAIT status
    // 8: the socket is in CLOSING status
    // 9: the socket is in LAST_ACK status
    // 10: the socket is in TIME_WAIT status
    waitResponse();
    return (result != 0);
  }

public:

  /*
   Utilities
   */

TINY_GSM_MODEM_STREAM_UTILITIES()

  // TODO: Optimize this!
  uint8_t waitResponse(uint32_t timeout_ms, String& data,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=GFP(GSM_CME_ERROR), GsmConstStr r4=NULL, GsmConstStr r5=NULL)
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
        TINY_GSM_YIELD();
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
          if (r3 == GFP(GSM_CME_ERROR)) {
            streamSkipUntil('\n');  // Read out the error
          }
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (data.endsWith(GF("+UUSORD:"))) {
          int mux = stream.readStringUntil(',').toInt();
          int len = stream.readStringUntil('\n').toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data = true;
            sockets[mux]->sock_available = len;
          }
          data = "";
          DBG("### URC Data Received:", len, "on", mux);
        } else if (data.endsWith(GF("+UUSOCL:"))) {
          int mux = stream.readStringUntil('\n').toInt();
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->sock_connected = false;
          }
          data = "";
          DBG("### URC Sock Closed: ", mux);
        }
      }
    } while (millis() - startMillis < timeout_ms);
finish:
    if (!index) {
      data.trim();
      if (data.length()) {
        DBG("### Unhandled:", data);
      }
      data = "";
    }
    //data.replace(GSM_NL, "/");
    //DBG('<', index, '>', data);
    return index;
  }

  uint8_t waitResponse(uint32_t timeout_ms,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=GFP(GSM_CME_ERROR), GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
  }

  uint8_t waitResponse(GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=GFP(GSM_CME_ERROR), GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    return waitResponse(1000, r1, r2, r3, r4, r5);
  }

public:
  Stream&       stream;

protected:
  GsmClient*    sockets[TINY_GSM_MUX_COUNT];
};

#endif
