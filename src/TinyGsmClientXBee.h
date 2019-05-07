/**
 * @file       TinyGsmClientXBee.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy, XBee module by Sara Damiano
 * @date       Nov 2016
 */

#ifndef TinyGsmClientXBee_h
#define TinyGsmClientXBee_h
//#pragma message("TinyGSM:  TinyGsmClientXBee")

//#define TINY_GSM_DEBUG Serial

// XBee's do not support multi-plexing in transparent/command mode
// The much more complicated API mode is needed for multi-plexing
#define TINY_GSM_MUX_COUNT 1
// XBee's have a default guard time of 1 second (1000ms, 10 extra for safety here)
#define TINY_GSM_XBEE_GUARD_TIME 1010

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
  REG_OK           = 0,
  REG_UNREGISTERED = 1,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_UNKNOWN      = 4,
};

// These are responses to the HS command to get "hardware series"
enum XBeeType {
  XBEE_UNKNOWN  = 0,
  XBEE_S6B_WIFI  = 0x601,  // Digi XBee® Wi-Fi
  XBEE_LTE1_VZN  = 0xB01,  // Digi XBee® Cellular LTE Cat 1
  XBEE_3G        = 0xB02,  // Digi XBee® Cellular 3G
  XBEE3_LTE1_ATT = 0xB06,  // Digi XBee3™ Cellular LTE CAT 1
  XBEE3_LTEM_ATT = 0xB08,  // Digi XBee3™ Cellular LTE-M
};


class TinyGsmXBee : public TinyGsmModem
{

public:

class GsmClient : public Client
{
  friend class TinyGsmXBee;
  // typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmXBee& modem, uint8_t mux = 0) {
    init(&modem, mux);
  }

  bool init(TinyGsmXBee* modem, uint8_t mux = 0) {
    this->at = modem;
    this->mux = mux;
    sock_connected = false;

    at->sockets[mux] = this;

    return true;
  }

public:
  // NOTE:  The XBee saves all paramter information in flash.  When you turn it
  // on it immediately prepares to re-connect to whatever was last connected to.
  // All the modemConnect() function does is tell it the paramters to put into
  // flash.  The connection itself is not opened until you attempt to send data.
  // Because all settings are saved to flash, it is possible (or likely) that
  // you could send out data even if you haven't "made" any connection.
  virtual int connect(const char *host, uint16_t port) {
    at->streamClear();  // Empty anything in the buffer before starting
    if (at->commandMode())  {  // Don't try if we didn't successfully get into command mode
      sock_connected = at->modemConnect(host, port, mux, false);
      at->writeChanges();
      at->exitCommand();
    }
    return sock_connected;
  }

  virtual int connect(IPAddress ip, uint16_t port) {
    at->streamClear();  // Empty anything in the buffer before starting
    if (at->commandMode())  {  // Don't try if we didn't successfully get into command mode
      sock_connected = at->modemConnect(ip, port, mux, false);
      at->writeChanges();
      at->exitCommand();
    }
    return sock_connected;
  }

  virtual void stop() {
    at->streamClear();  // Empty anything in the buffer
    at->commandMode();
    // For WiFi models, there's no direct way to close the socket.  This is a
    // hack to shut the socket by setting the timeout to zero.
    if (at->beeType == XBEE_S6B_WIFI) {
      at->sendAT(GF("TM0"));  // Set socket timeout (using Digi default of 10 seconds)
      at->waitResponse(5000);  // This response can be slow
      at->writeChanges();
    }
    // For cellular models, per documentation: If you change the TM (socket
    // timeout) value while in Transparent Mode, the current connection is
    // immediately closed.
    at->sendAT(GF("TM64"));  // Set socket timeout (using Digi default of 10 seconds)
    at->waitResponse(5000);  // This response can be slow
    at->writeChanges();
    at->exitCommand();
    at->streamClear();  // Empty anything remaining in the buffer
    sock_connected = false;
    // Note:  because settings are saved in flash, the XBEE will attempt to
    // reconnect to the previous socket if it receives any outgoing data.
    // Setting sock_connected to false after the stop ensures that connected()
    // will return false after a stop has been ordered.  This makes it play
    // much more nicely with libraries like PubSubClient.
  }

  virtual size_t write(const uint8_t *buf, size_t size) {
    TINY_GSM_YIELD();
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
    return at->stream.available();
    /*
    if (!rx.size() || at->stream.available()) {
      at->maintain();
    }
    return at->stream.available() + rx.size();
    */
  }

  virtual int read(uint8_t *buf, size_t size) {
    TINY_GSM_YIELD();
    return at->stream.readBytes((char *)buf, size);
    /*
    size_t cnt = 0;
    uint32_t _startMillis = millis();
    while (cnt < size && millis() - _startMillis < _timeout) {
      size_t chunk = TinyGsmMin(size-cnt, rx.size());
      if (chunk > 0) {
        rx.get(buf, chunk);
        buf += chunk;
        cnt += chunk;
        continue;
      }
      // TODO: Read directly into user buffer?
      if (!rx.size() || at->stream.available()) {
        at->maintain();
      }
    }
    return cnt;
    */
  }

  virtual int read() {
    TINY_GSM_YIELD();
    return at->stream.read();
    /*
    uint8_t c;
    if (read(&c, 1) == 1) {
      return c;
    }
    return -1;
    */
  }

  virtual int peek() { return at->stream.peek(); }
  virtual void flush() { at->stream.flush(); }

  virtual uint8_t connected() {
    if (available()) {
      return true;
    }
    // Double check that we don't know it's closed
    // NOTE:  modemGetConnected() is likely to return a "false" true because
    // it will return unknown until after data is sent over the connection.
    // If the socket is definitely closed, modemGetConnected() will set
    // sock_connected to false;
    at->modemGetConnected();
    return sock_connected;
  }
  virtual operator bool() { return connected(); }

  /*
   * Extended API
   */

  String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

private:
  TinyGsmXBee*    at;
  uint8_t         mux;
  bool            sock_connected;
  // RxFifo          rx;
};


class GsmClientSecure : public GsmClient
{
public:
  GsmClientSecure() {}

  GsmClientSecure(TinyGsmXBee& modem, uint8_t mux = 0)
    : GsmClient(modem, mux)
  {}

public:
  virtual int connect(const char *host, uint16_t port) {
    at->streamClear();  // Empty anything in the buffer before starting
    if (at->commandMode())  {  // Don't try if we didn't successfully get into command mode
      sock_connected = at->modemConnect(host, port, mux, true);
      at->writeChanges();
      at->exitCommand();
    }
    return sock_connected;
  }

  virtual int connect(IPAddress ip, uint16_t port) {
    at->streamClear();  // Empty anything in the buffer before starting
    if (at->commandMode())  {  // Don't try if we didn't successfully get into command mode
      sock_connected = at->modemConnect(ip, port, mux, false);
      at->writeChanges();
      at->exitCommand();
    }
    return sock_connected;
  }
};


public:

  TinyGsmXBee(Stream& stream)
    : TinyGsmModem(stream), stream(stream)
  {
      beeType = XBEE_UNKNOWN;  // Start not knowing what kind of bee it is
      guardTime = TINY_GSM_XBEE_GUARD_TIME;  // Start with the default guard time of 1 second
      resetPin = -1;
      savedIP = IPAddress(0,0,0,0);
      savedHost = "";
      memset(sockets, 0, sizeof(sockets));
  }

  TinyGsmXBee(Stream& stream, int8_t resetPin)
    : TinyGsmModem(stream), stream(stream)
  {
      beeType = XBEE_UNKNOWN;  // Start not knowing what kind of bee it is
      guardTime = TINY_GSM_XBEE_GUARD_TIME;  // Start with the default guard time of 1 second
      this->resetPin = resetPin;
      savedIP = IPAddress(0,0,0,0);
      savedHost = "";
      memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */

  bool init(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);

    if (resetPin >= 0) {
      pinMode(resetPin, OUTPUT);
      digitalWrite(resetPin, HIGH);
    }

    if (!commandMode(10)) return false;  // Try up to 10 times for the init

    sendAT(GF("AP0"));  // Put in transparent mode
    bool ret_val = waitResponse() == 1;
    ret_val &= writeChanges();

    sendAT(GF("GT64")); // shorten the guard time to 100ms
    ret_val &= waitResponse();
    ret_val &= writeChanges();
    if (ret_val) guardTime = 110;

    getSeries();  // Get the "Hardware Series";

    exitCommand();
    return ret_val;
  }

  String getModemName() {
    return getBeeName();
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
              exitCommand();
              return true;
          }
      }
      delay(100);
    }
    return false;
  }

  void maintain() {
    // this only happens OUTSIDE command mode, so if we're getting characters
    // they should be data received from the TCP connection
    // TINY_GSM_YIELD();
    // while (stream.available()) {
    //   char c = stream.read();
    //   if (c > 0) sockets[0]->rx.put(c);
    // }
  }

  bool factoryDefault() {
    if (!commandMode()) return false;  // Return immediately
    sendAT(GF("RE"));
    bool ret_val = waitResponse() == 1;
    ret_val &= writeChanges();
    exitCommand();
    // Make sure the guard time for the modem object is set back to default
    // otherwise communication would fail after the reset
    guardTime = 1010;
    return ret_val;
  }

  String getModemInfo() {
    String modemInf = "";
    if (!commandMode()) return modemInf;  // Try up to 10 times for the init

    sendAT(GF("HS"));  // Get the "Hardware Series"
    modemInf += readResponseString();

    exitCommand();
    return modemInf;
  }

  bool hasSSL() {
    if (beeType == XBEE_S6B_WIFI) return false;
    else return true;
  }

  bool hasWifi() {
    if (beeType == XBEE_S6B_WIFI) return true;
    else return false;
  }

  bool hasGPRS() {
    if (beeType == XBEE_S6B_WIFI) return false;
    else return true;
  }

  XBeeType getBeeType() {
    return beeType;
  }

  String getBeeName() {
    switch (beeType){
      case XBEE_S6B_WIFI: return "Digi XBee® Wi-Fi";
      case XBEE_LTE1_VZN: return "Digi XBee® Cellular LTE Cat 1";
      case XBEE_3G: return "Digi XBee® Cellular 3G";
      case XBEE3_LTE1_ATT: return "Digi XBee3™ Cellular LTE CAT 1";
      case XBEE3_LTEM_ATT: return "Digi XBee3™ Cellular LTE-M";
      default:  return "Digi XBee®";
    }
  }

  /*
   * Power functions
   */

  // The XBee's have a bad habit of getting into an unresponsive funk
  // This uses the board's hardware reset pin to force it to reset
  void pinReset() {
    if (resetPin >= 0) {
      DBG("### Forcing a modem reset!\r\n");
      digitalWrite(resetPin, LOW);
      delay(1);
      digitalWrite(resetPin, HIGH);
    }
  }

  bool restart() {
    if (!commandMode()) return false;  // Return immediately
    if (beeType == XBEE_UNKNOWN) getSeries();  // how we restart depends on this

    if (beeType != XBEE_S6B_WIFI) {
      sendAT(GF("AM1"));  // Digi suggests putting cellular modules into airplane mode before restarting
                          // This allows the sockets and connections to close cleanly
      if (waitResponse() != 1) return exitAndFail();
      if (!writeChanges()) return exitAndFail();
    }

    sendAT(GF("FR"));
    if (waitResponse() != 1) return exitAndFail();

    if (beeType == XBEE_S6B_WIFI) delay(2000);  // Wifi module actually resets about 2 seconds later
    else delay(100);  // cellular modules wait 100ms before reset happes

    // Wait until reboot complete and responds to command mode call again
    for (unsigned long start = millis(); millis() - start < 60000L; ) {
      if (commandMode(1)) break;
      delay(250);  // wait a litle before trying again
    }

    if (beeType != XBEE_S6B_WIFI) {
      sendAT(GF("AM0"));  // Turn off airplane mode
      if (waitResponse() != 1) return exitAndFail();
      if (!writeChanges()) return exitAndFail();
    }

    exitCommand();

    return init();
  }

  void setupPinSleep(bool maintainAssociation = false) {
    if (!commandMode()) return;  // Return immediately

    if (beeType == XBEE_UNKNOWN) getSeries();  // Command depends on series

    sendAT(GF("SM"),1);  // Pin sleep
    waitResponse();

    if (beeType == XBEE_S6B_WIFI && !maintainAssociation) {
        sendAT(GF("SO"),200);  // For lowest power, dissassociated deep sleep
        waitResponse();
    }

    else if (!maintainAssociation){
        sendAT(GF("SO"),1);  // For supported cellular modules, maintain association
                             // Not supported by all modules, will return "ERROR"
        waitResponse();
    }

    writeChanges();
    exitCommand();
  }

  bool poweroff() {  // Not supported
    return false;
  }

  bool radioOff() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sleepEnable(bool enable = true) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * SIM card functions
   */

  bool simUnlock(const char *pin) {  // Not supported
    return false;
  }

  String getSimCCID() {
    if (!commandMode()) return "";  // Return immediately
    sendAT(GF("S#"));
    String res = readResponseString();
    exitCommand();
    return res;
  }

  String getIMEI() {
    if (!commandMode()) return "";  // Return immediately
    sendAT(GF("IM"));
    String res = readResponseString();
    exitCommand();
    return res;
  }

  SimStatus getSimStatus(unsigned long timeout = 10000L) {
    return SIM_READY;  // unsupported
  }

  RegStatus getRegistrationStatus() {
    if (!commandMode()) return REG_UNKNOWN;  // Return immediately

    if (beeType == XBEE_UNKNOWN) getSeries();  // Need to know the bee type to interpret response

    sendAT(GF("AI"));
    int16_t intRes = readResponseInt();
    RegStatus stat = REG_UNKNOWN;

    switch (beeType){
      case XBEE_S6B_WIFI: {
        switch (intRes) {
          case 0x00:  // 0x00 Successfully joined an access point, established IP addresses and IP listening sockets
            stat = REG_OK;
            break;
          case 0x01:  // 0x01 Wi-Fi transceiver initialization in progress.
          case 0x02:  // 0x02 Wi-Fi transceiver initialized, but not yet scanning for access point.
          case 0x40:  // 0x40 Waiting for WPA or WPA2 Authentication.
          case 0x41:  // 0x41 Device joined a network and is waiting for IP configuration to complete
          case 0x42:  // 0x42 Device is joined, IP is configured, and listening sockets are being set up.
          case 0xFF:  // 0xFF Device is currently scanning for the configured SSID.
            stat = REG_SEARCHING;
            break;
          case 0x13:  // 0x13 Disconnecting from access point.
            restart();  // Restart the device; the S6B tends to get stuck "disconnecting"
            stat = REG_UNREGISTERED;
            break;
          case 0x23:  // 0x23 SSID not configured.
            stat = REG_UNREGISTERED;
            break;
          case 0x24:  // 0x24 Encryption key invalid (either NULL or invalid length for WEP).
          case 0x27:  // 0x27 SSID was found, but join failed.
            stat = REG_DENIED;
            break;
          default:
            stat = REG_UNKNOWN;
            break;
        }
        break;
      }
      default: {  // Cellular XBee's
        switch (intRes) {
          case 0x00:  // 0x00 Connected to the Internet.
            stat = REG_OK;
            break;
          case 0x22:  // 0x22 Registering to cellular network.
          case 0x23:  // 0x23 Connecting to the Internet.
          case 0xFF:  // 0xFF Initializing.
            stat = REG_SEARCHING;
          break;
          case 0x24:  // 0x24 The cellular component is missing, corrupt, or otherwise in error.
          case 0x2B:  // 0x2B USB Direct active.
          case 0x2C:  // 0x2C Cellular component is in PSM (power save mode).
            stat = REG_UNKNOWN;
            break;
          case 0x25:  // 0x25 Cellular network registration denied.
            stat = REG_DENIED;
            break;
          case  0x2A:  // 0x2A Airplane mode.
            sendAT(GF("AM0"));  // Turn off airplane mode
            waitResponse();
            writeChanges();
            stat = REG_UNKNOWN;
            break;
          case 0x2F:  // 0x2F Bypass mode active.
            sendAT(GF("AP0"));  // Set back to transparent mode
            waitResponse();
            writeChanges();
            stat = REG_UNKNOWN;
            break;
          default:
            stat = REG_UNKNOWN;
            break;
        }
        break;
      }
    }

    exitCommand();
    return stat;
  }

  String getOperator() {
    if (!commandMode()) return "";  // Return immediately
    sendAT(GF("MN"));
    String res = readResponseString();
    exitCommand();
    return res;
  }

 /*
  * Generic network functions
  */

  int16_t getSignalQuality() {
    if (!commandMode()) return 0;  // Return immediately
    if (beeType == XBEE_UNKNOWN) getSeries();  // Need to know what type of bee so we know how to ask
    if (beeType == XBEE_S6B_WIFI) sendAT(GF("LM"));  // ask for the "link margin" - the dB above sensitivity
    else sendAT(GF("DB"));  // ask for the cell strength in dBm
    int16_t intRes = readResponseInt();
    exitCommand();
    if (beeType == XBEE3_LTEM_ATT && intRes == 105) intRes = 0;  // tends to reply with "69" when signal is unknown
    if (beeType == XBEE_S6B_WIFI) return -93 + intRes;  // the maximum sensitivity is -93dBm
    else return -1*intRes; // need to convert to negative number
  }

  bool isNetworkConnected() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK);
  }

  bool waitForNetwork(unsigned long timeout = 60000L) {
    for (unsigned long start = millis(); millis() - start < timeout; ) {
      if (isNetworkConnected()) {
        return true;
      }
      delay(250);  // per Neil H. - more stable with delay
    }
    return false;
  }

  /*
   * WiFi functions
   */
  bool networkConnect(const char* ssid, const char* pwd) {

    if (!commandMode()) return false;  // return immediately
    //nh For no pwd don't set setscurity or pwd
    if (NULL == ssid ) return exitAndFail();

    if (NULL != pwd)
    {
      sendAT(GF("EE"), 2);  // Set security to WPA2
      if (waitResponse() != 1) return exitAndFail();
      sendAT(GF("PK"), pwd);
    } else {
      sendAT(GF("EE"), 0);  // Set No security
    }
    if (waitResponse() != 1) return exitAndFail();

    sendAT(GF("ID"), ssid);
    if (waitResponse() != 1) return exitAndFail();

    if (!writeChanges()) return exitAndFail();
    exitCommand();

    return true;
  }

  bool networkDisconnect() {
    if (!commandMode()) return false;  // return immediately
    sendAT(GF("NR0"));  // Do a network reset in order to disconnect
    // NOTE:  On wifi modules, using a network reset will not
    // allow the same ssid to re-join without rebooting the module.
    int8_t res = (1 == waitResponse(5000));
    writeChanges();
    exitCommand();
    return res;
  }

  /*
   * IP Address functions
   */

  String getLocalIP() {
    if (!commandMode()) return "";  // Return immediately
    sendAT(GF("MY"));
    String IPaddr; IPaddr.reserve(16);
    // wait for the response - this response can be very slow
    IPaddr = readResponseString(30000);
    exitCommand();
    IPaddr.trim();
    return IPaddr;
  }

  /*
   * GPRS functions
   */
  bool gprsConnect(const char* apn, const char* user = NULL, const char* pwd = NULL) {
    if (!commandMode()) return false;  // Return immediately
    sendAT(GF("AN"), apn);  // Set the APN
    waitResponse();
    writeChanges();
    exitCommand();
    return true;
  }

  bool gprsDisconnect() {
    if (!commandMode()) return false;  // return immediately
    sendAT(GF("AM1"));  // Cheating and disconnecting by turning on airplane mode
    int8_t res = (1 == waitResponse(5000));
    writeChanges();
    sendAT(GF("AM0"));  // Airplane mode off
    waitResponse(5000);
    writeChanges();
    exitCommand();
    return res;
  }

  bool isGprsConnected() {
    return isNetworkConnected();
  }

  /*
   * Messaging functions
   */

  String sendUSSD(const String& code) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sendSMS(const String& number, const String& text) {
    if (!commandMode()) return false;  // Return immediately

    sendAT(GF("IP"), 2);  // Put in text messaging mode
    if (waitResponse() !=1) return exitAndFail();
    sendAT(GF("PH"), number);  // Set the phone number
    if (waitResponse() !=1) return exitAndFail();
    sendAT(GF("TDD"));  // Set the text delimiter to the standard 0x0D (carriage return)
    if (waitResponse() !=1) return exitAndFail();
    if (!writeChanges()) return exitAndFail();

    exitCommand();
    streamWrite(text);
    stream.write((char)0x0D);  // close off with the carriage return
    return true;
  }

  /*
   * Location functions
   */

  String getGsmLocation() TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Battery functions
   */

  uint16_t getBattVoltage() TINY_GSM_ATTR_NOT_AVAILABLE;

  int8_t getBattPercent() TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Client related functions
   */

protected:

  IPAddress getHostIP(const char* host) {
    String strIP; strIP.reserve(16);
    unsigned long startMillis = millis();
    bool gotIP = false;
    // XBee's require a numeric IP address for connection, but do provide the
    // functionality to look up the IP address from a fully qualified domain name
    while (millis() - startMillis < 45000L)  // the lookup can take a while
    {
      sendAT(GF("LA"), host);
      while (stream.available() < 4 && (millis() - startMillis < 45000L)) {};  // wait for any response
      strIP = stream.readStringUntil('\r');  // read result
      strIP.trim();
      if (!strIP.endsWith(GF("ERROR"))) {
        gotIP = true;
        break;
      }
      delay(2500);  // wait a bit before trying again
    }
    if (gotIP) {  // No reason to continue if we don't know the IP address
      return TinyGsmIpFromString(strIP);
    }
    else return IPAddress(0,0,0,0);
  }

  bool modemConnect(const char* host, uint16_t port, uint8_t mux = 0, bool ssl = false) {
    // If requested host is the same as the previous one and we already
    // have a valid IP address, we don't have to do anything.
    if (this->savedHost == String(host) && savedIP != IPAddress(0,0,0,0)) {
      return true;
    }

    // Otherwise, set the new host and mark the IP as invalid
    this->savedHost = String(host);
    savedIP = getHostIP(host);  // This will return 0.0.0.0 if lookup fails

    // If we now have a valid IP address, use it to connect
    if (savedIP != IPAddress(0,0,0,0)) {  // Only re-set connection information if we have an IP address
      return modemConnect(savedIP, port, mux, ssl);
    }
    else return false;
  }

  bool modemConnect(IPAddress ip, uint16_t port, uint8_t mux = 0, bool ssl = false) {
    savedIP = ip;  // Set the newly requested IP address
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

  int16_t modemSend(const void* buff, size_t len, uint8_t mux = 0) {
    stream.write((uint8_t*)buff, len);
    stream.flush();
    return len;
  }

  // NOTE:  The CI command returns the status of the TCP connection as open only
  // after data has been sent on the socket.  If it returns 0xFF the socket may
  // really be open, but no data has yet been sent.  We return this unknown value
  // as true so there's a possibility it's wrong.
  bool modemGetConnected() {

    if (!commandMode()) return false;  // Return immediately

    // If the IP address is 0, it's not valid so we can't be connected
    if (savedIP == IPAddress(0,0,0,0)) return false;

    // Verify that we're connected to the *right* IP address
    // We might be connected - but to the wrong thing
    // NOTE:  In transparent mode, there is only one connection possible - no multiplex
    String strIP; strIP.reserve(16);
    sendAT(GF("DL"));
    strIP = stream.readStringUntil('\r');  // read result
    if (TinyGsmIpFromString(strIP) != savedIP) return exitAndFail();

    if (beeType == XBEE_UNKNOWN) getSeries();  // Need to know the bee type to interpret response

    switch (beeType){  // The wifi be can only say if it's connected to the netowrk
      case XBEE_S6B_WIFI: {
        RegStatus s = getRegistrationStatus();
        if (s != REG_OK) {
          sockets[0]->sock_connected = false;
        }
        return (s == REG_OK);  // if it's connected, we hope the sockets are too
      }
      default: {  // Cellular XBee's
        sendAT(GF("CI"));
        int16_t intRes = readResponseInt();
        exitCommand();
        switch(intRes) {
          case 0x00:  // 0x00 = The socket is definitely open
          case 0xFF:  // 0xFF = No known status - this is always returned prior to sending data
            return true;
          case 0x02:  // 0x02 = Invalid parameters (bad IP/host)
          case 0x12:  // 0x12 = DNS query lookup failure
          case 0x25:  // 0x25 = Unknown server - DNS lookup failed (0x22 for UDP socket!)
            savedIP = IPAddress(0,0,0,0);  // force a lookup next time!
          default:  // If it's anything else (inc 0x02, 0x12, and 0x25)...
            sockets[0]->sock_connected = false;  // ...it's definitely NOT connected
            return false;
        }
      }
    }
  }

public:

  /*
   Utilities
   */

  void streamClear(void) {
    while (stream.available()) {
      stream.read();
      TINY_GSM_YIELD();
    }
  }

  template<typename... Args>
  void sendAT(Args... cmd) {
    streamWrite("AT", cmd..., GSM_NL);
    stream.flush();
    TINY_GSM_YIELD();
    //DBG("### AT:", cmd...);
  }

  // TODO: Optimize this!
  // NOTE:  This function is used while INSIDE command mode, so we're only
  // waiting for requested responses.  The XBee has no unsoliliced responses
  // (URC's) when in command mode.
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
    int8_t index = 0;
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
        DBG("### NO RESPONSE FROM MODEM!\r\n");
      }
    } else {
      data.trim();
      data.replace(GSM_NL GSM_NL, GSM_NL);
      data.replace(GSM_NL, "\r\n    ");
      if (data.length()) {
      }
    }
    //DBG('<', index, '>');
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

  bool commandMode(uint8_t retries = 3) {
    uint8_t triesMade = 0;
    uint8_t triesUntilReset = 2;  // only reset after 2 failures
    bool success = false;
    streamClear();  // Empty everything in the buffer before starting
    while (!success and triesMade < retries) {
      // Cannot send anything for 1 "guard time" before entering command mode
      // Default guard time is 1s, but the init fxn decreases it to 100 ms
      delay(guardTime + 50);
      streamWrite(GF("+++"));  // enter command mode
      int res = waitResponse(guardTime*2);
      success = (1 == res);
      if (0 == res) {
        triesUntilReset--;
        if (triesUntilReset == 0) {
          triesUntilReset = 2;
          pinReset();  // if it's unresponsive, reset
          delay(250);  // a short delay to allow it to come back up TODO-optimize this
        }
      }
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

  bool exitAndFail(void) {
    exitCommand();  // Exit command mode
    return false;
  }

  void getSeries(void) {
    sendAT(GF("HS"));  // Get the "Hardware Series";
    int16_t intRes = readResponseInt();
    beeType = (XBeeType)intRes;
    DBG(GF("### Modem: "), getModemName());
  }

  String readResponseString(uint32_t timeout = 1000) {
    TINY_GSM_YIELD();
    unsigned long startMillis = millis();
    while (!stream.available() && millis() - startMillis < timeout) {};
    String res = stream.readStringUntil('\r');  // lines end with carriage returns
    res.trim();
    return res;
  }

  int16_t readResponseInt(uint32_t timeout = 1000) {
    String res = readResponseString(timeout);  // it just works better reading a string first
    char buf[5] = {0,};
    res.toCharArray(buf, 5);
    int16_t intRes = strtol(buf, 0, 16);
    return intRes;
  }

public:
  Stream&       stream;

protected:
  int16_t       guardTime;
  int8_t        resetPin;
  XBeeType      beeType;
  IPAddress     savedIP;
  String        savedHost;
  GsmClient*    sockets[TINY_GSM_MUX_COUNT];
};

#endif
