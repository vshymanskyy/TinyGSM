/**
 * @file       TinyGsmClientM95.h
 * @author     Volodymyr Shymanskyy, Pacman Pereira, and Replicade Ltd.
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy, (c)2017 Replicade Ltd. <http://www.replicade.com>
 * @date       Nov 2016
 */

#ifndef TinyGsmClientM95_h
#define TinyGsmClientM95_h
//#pragma message("TinyGSM:  TinyGsmClientM95")

//#define TINY_GSM_DEBUG Serial
//#define TINY_GSM_USE_HEX

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 64
#endif

#define TINY_GSM_MUX_COUNT 6

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


class TinyGsmM95
{

public:

class GsmClient : public Client
{
  friend class TinyGsmM95;
  typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

public:
  GsmClient() {}

  GsmClient(TinyGsmM95& modem, uint8_t mux = 1) {
    init(&modem, mux);
  }

  bool init(TinyGsmM95* modem, uint8_t mux = 1) {
    this->at = modem;
    this->mux = mux;
    sock_available = 0;
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
    sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
    return sock_connected;
  }

TINY_GSM_CLIENT_CONNECT_OVERLOADS()

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
      at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
      rx.clear();
      at->maintain();
    }
    at->sendAT(GF("+QICLOSE="), mux);
    sock_connected = false;
    at->waitResponse(60000L, GF("CLOSED"), GF("CLOSE OK"), GF("ERROR"));
  }

TINY_GSM_CLIENT_WRITE()

TINY_GSM_CLIENT_AVAILABLE_NO_BUFFER_CHECK()

TINY_GSM_CLIENT_READ_NO_BUFFER_CHECK()

TINY_GSM_CLIENT_PEEK_FLUSH_CONNECTED()

  /*
   * Extended API
   */

  String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

private:
  TinyGsmM95*     at;
  uint8_t         mux;
  uint16_t        sock_available;
  bool            sock_connected;
  bool            got_data;
  RxFifo          rx;
};


// class GsmClientSecure : public GsmClient
// {
// public:
//   GsmClientSecure() {}
//
//   GsmClientSecure(TinyGsmm95& modem, uint8_t mux = 1)
//     : GsmClient(modem, mux)
//   {}
//
// public:
//   virtual int connect(const char *host, uint16_t port, int timeout_s) {
//     stop();
//     TINY_GSM_YIELD();
//     rx.clear();
//     sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
//     return sock_connected;
//   }
// };


public:

  TinyGsmM95(Stream& stream)
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
    if (!testAT()) {
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
    DBG(GF("### Modem:"), getModemName());
    getSimStatus();
    return true;
  }

  String getModemName() {
    return "Quectel M95";
  }

TINY_GSM_MODEM_SET_BAUD_IPR()

TINY_GSM_MODEM_TEST_AT()

TINY_GSM_MODEM_MAINTAIN_LISTEN()

  bool factoryDefault() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("+IPR=0"));   // Auto-baud
    waitResponse();
    sendAT(GF("&W"));       // Write configuration
    return waitResponse() == 1;
  }

TINY_GSM_MODEM_GET_INFO_ATI()

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
    if (waitResponse(10000L, GF("NORMAL POWER DOWN"), GF("OK"), GF("FAIL")) == 3) {
      return false;
    }
    sendAT(GF("+CFUN=1"));
    if (waitResponse(10000L, GF("Call Ready"), GF("OK"), GF("FAIL")) == 3) {
      return false;
    }
    return init();
  }

  bool poweroff() {
    sendAT(GF("+QPOWD=1"));
    return waitResponse(300, GF("NORMAL POWER DOWN")) == 1;
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

  String getSimCCID() {
    sendAT(GF("+QCCID"));
    if (waitResponse(GF(GSM_NL "+QCCID:")) != 1) {
      return "";
    }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

TINY_GSM_MODEM_GET_IMEI_GSN()

  SimStatus getSimStatus(unsigned long timeout_ms = 10000L) {
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) {
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

TINY_GSM_MODEM_GET_REGISTRATION_XREG(CREG)

TINY_GSM_MODEM_GET_OPERATOR_COPS()

  /*
   * Generic network functions
   */

TINY_GSM_MODEM_GET_CSQ()

  bool isNetworkConnected() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  void setHostFormat( bool useDottedQuad ) {
    if ( useDottedQuad ) {
      sendAT(GF("+QIDNSIP=0"));
    } else {
      sendAT(GF("+QIDNSIP=1"));
    }
    waitResponse();
  }

TINY_GSM_MODEM_WAIT_FOR_NETWORK()

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

    //Start TCPIP Task and Set APN, User Name and Password
    sendAT("+QIREGAPP=\"", apn, "\",\"", user, "\",\"", pwd,  "\"" );
    if (waitResponse() != 1) {
      return false;
    }

    //Activate GPRS/CSD Context
    sendAT(GF("+QIACT"));
    if (waitResponse(10000) != 1) {
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

    return true;
  }

  bool gprsDisconnect() {
    sendAT(GF("+QIDEACT"));
    return waitResponse(60000L, GF("DEACT OK"), GF("ERROR")) == 1;
  }

TINY_GSM_MODEM_GET_GPRS_IP_CONNECTED()

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

  IPAddress localIP() {
    return TinyGsmIpFromString(getLocalIP());
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
   * Phone Call functions
   */

  bool setGsmBusy(bool busy = true) TINY_GSM_ATTR_NOT_AVAILABLE;
  bool callAnswer() TINY_GSM_ATTR_NOT_AVAILABLE;
  bool callNumber(const String& number) TINY_GSM_ATTR_NOT_AVAILABLE;
  bool callHangup() TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Location functions
   */

  String getGsmLocation() TINY_GSM_ATTR_NOT_AVAILABLE;

  /*
   * Battery & temperature functions
   */

  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  uint16_t getBattVoltage() {
    sendAT(GF("+CBC"));
    if (waitResponse(GF(GSM_NL "+CBC:")) != 1) {
      return 0;
    }
    streamSkipUntil(','); // Skip battery charge status
    streamSkipUntil(','); // Skip battery charge level
    // return voltage in mV
    uint16_t res = stream.readStringUntil(',').toInt();
    // Wait for final OK
    waitResponse();
    return res;
  }

  int8_t getBattPercent() {
    sendAT(GF("+CBC"));
    if (waitResponse(GF(GSM_NL "+CBC:")) != 1) {
      return false;
    }
    streamSkipUntil(','); // Skip battery charge status
    // Read battery charge level
    int res = stream.readStringUntil(',').toInt();
    // Wait for final OK
    waitResponse();
    return res;
  }

  uint8_t getBattChargeState() {
    sendAT(GF("+CBC?"));
    if (waitResponse(GF(GSM_NL "+CBC:")) != 1) {
      return false;
    }
    // Read battery charge status
    int res = stream.readStringUntil(',').toInt();
    // Wait for final OK
    waitResponse();
    return res;
  }

  bool getBattStats(uint8_t &chargeState, int8_t &percent, uint16_t &milliVolts) {
    sendAT(GF("+CBC?"));
    if (waitResponse(GF(GSM_NL "+CBC:")) != 1) {
      return false;
    }
    chargeState = stream.readStringUntil(',').toInt();
    percent = stream.readStringUntil(',').toInt();
    milliVolts = stream.readStringUntil('\n').toInt();
    // Wait for final OK
    waitResponse();
    return true;
  }

  float getTemperature() {
    sendAT(GF("+QTEMP"));
    if (waitResponse(GF(GSM_NL "+QTEMP:")) != 1) {
      return (float)-9999;
    }
    streamSkipUntil(','); // Skip mode
    // Read charge of thermistor
    // milliVolts = stream.readStringUntil(',').toInt();
    streamSkipUntil(','); // Skip thermistor charge
    float temp = stream.readStringUntil('\n').toFloat();
    // Wait for final OK
    waitResponse();
    return temp;
  }

  /*
   * Client related functions
   */

protected:

  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75)
 {
    uint32_t timeout_ms = ((uint32_t)timeout_s)*1000;
    sendAT(GF("+QIOPEN="), mux, GF("\"TCP"), GF("\",\""), host, GF("\","), port);
    int rsp = waitResponse(timeout_ms,
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
    return len;  // TODO
  }

  size_t modemRead(size_t size, uint8_t mux) {
    // TODO:  Does this work????
    // AT+QIRD=<id>,<sc>,<sid>,<len>
    // id = GPRS context number - 0, set in GPRS connect
    // sc = roll in connection - 1, client of connection
    // sid = index of connection - mux
    // len = maximum length of data to send
    sendAT(GF("+QIRD=0,1,"), mux, ',', size);
    // sendAT(GF("+QIRD="), mux, ',', size);
    if (waitResponse(GF("+QIRD:")) != 1) {
      return 0;
    }
    streamSkipUntil(':');  // skip IP address
    streamSkipUntil(',');  // skip port
    streamSkipUntil(',');  // skip connection type (TCP/UDP)
    size_t len = stream.readStringUntil('\n').toInt();  // read length
    for (size_t i=0; i<len; i++) {
      TINY_GSM_MODEM_STREAM_TO_MUX_FIFO_WITH_DOUBLE_TIMEOUT
      sockets[mux]->sock_available--;
      // ^^ One less character available after moving from modem's FIFO to our FIFO
    }
    waitResponse();  // ends with an OK
    DBG("### READ:", len, "from", mux);
    return len;
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

TINY_GSM_MODEM_STREAM_UTILITIES()

  // TODO: Optimize this!
  uint8_t waitResponse(uint32_t timeout_ms, String& data,
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
        } else if (data.endsWith(GF(GSM_NL "+QIRD:"))) {  // TODO:  QIRD? or QIRDI?
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
    } while (millis() - startMillis < timeout_ms);
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

  uint8_t waitResponse(uint32_t timeout_ms,
                       GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                       GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL)
  {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
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
