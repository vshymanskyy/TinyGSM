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

#include "TinyGsmCommon.h"

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


class TinyGsmSaraG450
{

public:

class GsmClient : public Client
{
  friend class TinyGsmSaraG450;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmSaraG450& modem, uint8_t mux = 0) {
    init(&modem, mux);
  }

  bool init(TinyGsmSaraG450* modem, uint8_t mux = 0) {
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

  virtual int connect(IPAddress ip, uint16_t port, int timeout_s) {
    String host; host.reserve(16);
    host += ip[0];
    host += ".";
    host += ip[1];
    host += ".";
    host += ip[2];
    host += ".";
    host += ip[3];
    return connect(host.c_str(), port, timeout_s);
  }
  virtual int connect(const char *host, uint16_t port) {
    return connect(host, port, 75);
  }
  virtual int connect(IPAddress ip, uint16_t port) {
    return connect(ip, port, 75);
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
    while (sock_connected && sock_available > 0) {
      at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
      rx.clear();
      at->maintain();
    }
    at->modemDisconnect(mux);
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
      /* Workaround: sometimes module forgets to notify about data arrival.
      TODO: Currently we ping the module periodically,
      but maybe there's a better indicator that we need to poll */
      if (millis() - prev_check > 500) {
        got_data = true;
        prev_check = millis();
      }
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
      /* Workaround: sometimes module forgets to notify about data arrival.
      TODO: Currently we ping the module periodically,
      but maybe there's a better indicator that we need to poll */
      if (millis() - prev_check > 500) {
        got_data = true;
        prev_check = millis();
      }
      /* TODO: Read directly into user buffer? */
      at->maintain();
      if (sock_available > 0) {
        int n = at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
        if (n == 0) break;
      } else {
        break;
      }
    }
    if(cnt == 0) return -1;
    return cnt;
  }
  virtual int read() {
    uint8_t c;
    if (read(&c, 1) == 1) {
      return c;
    }
    return -1;
  }

  virtual int peek() { return -1; } /* TODO */
  
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

  String remoteIP()  __attribute__((error("Not implemented")));

private:
  TinyGsmSaraG450*   at;
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

  GsmClientSecure(TinyGsmSaraG450& modem, uint8_t mux = 1)
    : GsmClient(modem, mux)
  {}

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

  TinyGsmSaraG450(Stream& stream)
    : stream(stream)
  {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */

  bool begin(const char* pin = NULL) {
    return init(pin);
  }

  bool init(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);

    powerOnModule();

    if (!testAT()) {
      return false;
    }

    delay(1000);
    clearRxStreamBuffer();

    sendAT(GF("E0"));   // Echo Off
    if (waitResponse() != 1) {
      return false;
    }
    sendAT(GF("&W"));   // Echo Off sometimes only takes affect after we force a save
    if (waitResponse(30000) != 1) { // Takes long to save
      return false;
    }
    // Echo off takes a while to be affective
    delay(200);
    clearRxStreamBuffer();

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    if(waitResponse() != 1) {
      return false;
    }

    if(!getModemName()) {
      return false;
    }

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

  void powerOffModuleHard() {
    #if defined(TINY_GSM_PIN_POWER_OFF)
      pinMode(TINY_GSM_PIN_POWER_OFF, OUTPUT);
      digitalWrite(TINY_GSM_PIN_POWER_OFF, LOW);
      delay(1000);
      digitalWrite(TINY_GSM_PIN_POWER_OFF, HIGH);
      pinMode(TINY_GSM_PIN_POWER_OFF, INPUT);
    #else
      #error "Please define TINY_GSM_PIN_POWER_OFF"
    #endif
  }

  void powerOnModule() {
    #if defined(TINY_GSM_PIN_POWER_ON)
      pinMode(TINY_GSM_PIN_POWER_ON, OUTPUT);
      digitalWrite(TINY_GSM_PIN_POWER_ON, LOW);
      delay(1000);
      digitalWrite(TINY_GSM_PIN_POWER_ON, HIGH);
      pinMode(TINY_GSM_PIN_POWER_ON, INPUT);
    #else
      #error "Please define TINY_GSM_PIN_POWER_ON"
    #endif
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

  void setBaud(unsigned long baud) {
    sendAT(GF("+IPR="), baud);
  }

  bool testAT(unsigned long timeout_ms = 10000L) {
    delay(1000);
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
      sendAT(GF(""));
      if (waitResponse(500) == 1) {
        return true;
      }
      delay(200);
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
    clearRxStreamBuffer();
  }

  void clearRxStreamBuffer() {
    while (stream.available()) {
      waitResponse(15, NULL, NULL);
    }
  }

  bool factoryDefault() {
    sendAT(GF("+UFACTORY=0,1"));  // No factory restore, erase NVM
    waitResponse();
    sendAT(GF("+CFUN=16"));   // Reset
    return waitResponse() == 1;
  }

  String getModemInfo() {
    clearRxStreamBuffer();
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

  bool hasWifi() {
    return false;
  }

  bool hasGPRS() {
    return true;
  }

  /*
   * Power functions
   */

  bool softRestart() {
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

  bool hardRestart() {
    powerOffModuleHard();
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

  bool sleepEnable(bool enable = true)  __attribute__((error("Not implemented")));

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
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  String getIMEI() {
    sendAT(GF("+CGSN"));
    // if (waitResponse(GF(GSM_NL)) != 1) {
    //   return "";
    // }
    String res = "";
    for(int i = 0; res == "" && i<3; i++) {
      res = stream.readStringUntil('\n');
      res.trim();
    }
    waitResponse();
    // res.trim();
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

  RegStatus getGsmRegistrationStatus() {
    sendAT(GF("+CREG?"));
    if (waitResponse(GF(GSM_NL "+CREG:")) != 1) {
      return REG_UNKNOWN;
    }
    streamSkipUntil(','); /* Skip format (0) */
    int status = stream.readStringUntil('\n').toInt();
    waitResponse();
    return (RegStatus)status;
  }

  RegStatus getGprsRegistrationStatus() {
    sendAT(GF("+CGREG?"));
    if (waitResponse(GF(GSM_NL "+CGREG:")) != 1) {
      return REG_UNKNOWN;
    }
    streamSkipUntil(','); /* Skip format (0) */
    int status = stream.readStringUntil('\n').toInt();
    waitResponse();
    return (RegStatus)status;
  }

  String getOperator() {
    sendAT(GF("+COPS?"));
    if (waitResponse(GF(GSM_NL "+COPS:")) != 1) {
      return "";
    }
    streamSkipUntil('"'); /* Skip mode and format */
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

  bool isNetworkGsmConnected() {
    RegStatus s = getGsmRegistrationStatus();
    if (s == REG_OK_HOME || s == REG_OK_ROAMING)
      return true;
    else return false;
  }

  bool isNetworkGprsConnected() {
    RegStatus s = getGprsRegistrationStatus();
    if (s == REG_OK_HOME || s == REG_OK_ROAMING)
      return true;
    else return false;
  }

  bool waitForNetworkGsm(unsigned long timeout_ms = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
      if (isNetworkGsmConnected()) {
        return true;
      }
      delay(1000);
    }
    return false;
  }

  bool waitForNetworkGprs(unsigned long timeout_ms = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
      if (isNetworkGprsConnected()) {
        return true;
      }
      delay(1000);
    }
    return false;
  }

  /*
   * GPRS functions
   */

  bool gprsAttach() {
    sendAT(GF("+CGATT=1"));  // attach to GPRS
    if (waitResponse(360000, GF(GSM_NL "+CGATT:")) != 1) {
      return false;
    }
    int status = stream.readStringUntil('\n').toInt();
    waitResponse();
    if(status==1) return true;
    else return false;
  }


  RegStatus getGprsAttachedStatus() {
    sendAT(GF("+CGATT?"));
    if (waitResponse(GF(GSM_NL "+CGATT:")) != 1) {
      return REG_UNKNOWN;
    }
    //streamSkipUntil(','); /* Skip format (0) */
    int status = stream.readStringUntil('\n').toInt();
    waitResponse();
    return (RegStatus)status;
  }

  bool isGprsAttached() {
    RegStatus s = getGprsRegistrationStatus();
    if (s == 1) {
      return true;
    }
    else return false;
  }

  bool waitForGprsAttached(unsigned long timeout_ms = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
      if (isGprsAttached()) {
        return true;
      }
      delay(1000);
    }
    return false;
  }

  bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {
    gprsDisconnect();

    DBG("Connecting GPRS");

    /*
    Profile might already be active
    */
    {
      sendAT(GF("+UPSND=0,8")); // Check if PSD profile 0 is now active
      int res = waitResponse(GF(",8,1"), GF(",8,0"));
      waitResponse();  // Should return another OK
      if (res == 1) {
        return true;  // It's active
      }
    }

    // sendAT(GF("+UPSD=0"));
    // waitResponse();

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

    //AT+UPSDA=0,3 returns an error. Try a delay
    delay(1000);

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
        // TODO: Sometimes we get +UUPSDA: 36 which will take 3 minutes to timeout
        return false;
      }
      streamSkipUntil('\n');  // Ignore the IP address, if returned
    } else {
      return false;
    }

    return true;
  }

  bool gprsDisconnect() {
    DBG("Disconnecting GPRS");
    sendAT(GF("+UPSDA=0,4"));  // Deactivate the PDP context associated with profile 0
    if (waitResponse() != 1) { // Wait for OK
      return true; // if we get an error we are disconnected
    }
    
    if(waitResponse(30000, "+UUPSDD: 0") == 1) {
      return true;
    }

    // sendAT(GF("+CGATT=0"));  // detach from GPRS
    // if (waitResponse(360000L) != 1) {
    //   return false;
    // }

    return false;
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
 
    return localIP() != IPAddress(0,0,0,0);
  }

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

  bool setGsmBusy(bool busy = true) __attribute__((error("Not implemented")));

  bool callAnswer()  __attribute__((error("Not implemented")));

  bool callNumber(const String& number)  __attribute__((error("Not implemented")));

  bool callHangup()  __attribute__((error("Not implemented")));

  /*
   * Messaging functions
   */

  String sendUSSD(const String& code)  __attribute__((error("Not implemented")));

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

  bool sendSMS_UTF16(const String& number, const void* text, size_t len)  __attribute__((error("Not implemented")));


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

  uint16_t getBattVoltage() __attribute__((error("Not implemented")));

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

  uint8_t getBattChargeState() __attribute__((error("Not implemented")));

  bool getBattStats(uint8_t &chargeState, int8_t &percent, uint16_t &milliVolts) {
    percent = getBattPercent();
    return true;
  }

  // This would only available for a small number of modules in this group (TOBY-L)
  float getTemperature()  __attribute__((error("Not implemented")));

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
    // TODO:  Use faster "asynchronous" connection?
    // We would have to wait for the +UUSOCO URC to verify connection
    sendAT(GF("+USOCO="), *mux, ",\"", host, "\",", port);
    int rsp = waitResponse(timeout_ms);
    waitResponse();
    // After a connect we can't immediately write data or else we get:
    // AT+USOWR=0,3
    // CME ERROR: Operation not allowed
    delay(200); // Give the connection 200ms to settle

    return (1 == rsp);
  }

  bool modemDisconnect(uint8_t mux) {
    TINY_GSM_YIELD();
    if (!modemGetConnected(mux)) {
      sockets[mux]->sock_connected = false;
      return true;
    }
    bool success;
    sendAT(GF("+USOCL="), mux);
    success = 1 == waitResponse();  // should return within 1s
    if (success) {
      sockets[mux]->sock_connected = false;
    }
    return success;
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
    sendAT(GF("+USOWR="), mux, ',', len);
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
    uint32_t startMillis = millis();

    sendAT(GF("+USORD="), mux, ',', size);
    if (waitResponse(GF(GSM_NL "+USORD:")) != 1) {
      // Might end in an error because the socket is closed
      sockets[mux]->sock_connected = modemGetConnected(mux);
      return 0;
    }
    streamSkipUntil(','); // Skip mux
    size_t len = stream.readStringUntil(',').toInt();
    streamSkipUntil('\"');

    for (size_t i=0; i<len; i++) {
      while (!stream.available() && (millis() - startMillis < sockets[mux]->_timeout)) { 
        delay(20);
      }
      char c = stream.read();
      sockets[mux]->rx.put(c);
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
    } else {
      // We received an error checking number of bytes to read
      DBG("modemGetAvailable error");
      sockets[mux]->sock_connected = false; // disconnect to prevent infinite loops
      return 0;
    }
    if (!result && sockets[mux]->sock_connected) {
      sockets[mux]->sock_connected = modemGetConnected(mux);
    }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    // NOTE:  Querying a closed socket gives an error "operation not allowed"
    sendAT(GF("+USOCTL="), mux, ",10");
    uint8_t res = waitResponse(GF(GSM_NL "+USOCTL:"), GF("+CME ERROR:"));
    if(res == 1) {
      // Valid response, need to read state
    }
    else if (res == 2) {
      // +CME ERROR: Operation not allowed - means socket is not connected
      stream.readStringUntil('\n'); // read the rest of the error string to purge the buffer
      return false;
    }
    else {
      // Unknown response
      DBG("Socket connection state error");
      return false;
    }

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

  template<typename T>
  void streamWrite(T last) {
    stream.print(last);
    delay(sizeof(last)); // delay 1ms per char sent out to be safe
  }
 
  template<typename T, typename... Args>
  void streamWrite(T head, Args... tail) {
    stream.print(head);
    delay(sizeof(head)); // delay 1ms per char sent out to be safe
    streamWrite(tail...);
  }
 
  template<typename... Args>
  void sendAT(Args... cmd) {
    streamWrite("AT", cmd..., GSM_NL);
    stream.flush(); // flush does not seem to work on SAMD boards
    TINY_GSM_YIELD();
    /* DBG("### AT:", cmd...); */
  }
 
  bool streamSkipUntil(const char c, const unsigned long timeout_ms = 1000L) {
    unsigned long startMillis = millis();
    while (millis() - startMillis < timeout_ms) {
      while (millis() - startMillis < timeout_ms && !stream.available()) {
        TINY_GSM_YIELD();
      }
      if (stream.read() == c) {
        return true;
      }
    }
    return false;
  }

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
