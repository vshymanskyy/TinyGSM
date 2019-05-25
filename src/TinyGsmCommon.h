/**
 * @file       TinyGsmCommon.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef TinyGsmCommon_h
#define TinyGsmCommon_h

// The current library version number
#define TINYGSM_VERSION "0.7.7"

#if defined(SPARK) || defined(PARTICLE)
  #include "Particle.h"
#elif defined(ARDUINO)
  #if ARDUINO >= 100
    #include "Arduino.h"
  #else
    #include "WProgram.h"
  #endif
#endif

#if defined(ARDUINO_DASH)
  #include <ArduinoCompat/Client.h>
#else
  #include <Client.h>
#endif

#include <TinyGsmFifo.h>

#ifndef TINY_GSM_YIELD
  #define TINY_GSM_YIELD() { delay(0); }
#endif

#define TINY_GSM_ATTR_NOT_AVAILABLE __attribute__((error("Not available on this modem type")))
#define TINY_GSM_ATTR_NOT_IMPLEMENTED __attribute__((error("Not implemented")))

#if defined(__AVR__)
  #define TINY_GSM_PROGMEM PROGMEM
  typedef const __FlashStringHelper* GsmConstStr;
  #define GFP(x) (reinterpret_cast<GsmConstStr>(x))
  #define GF(x)  F(x)
#else
  #define TINY_GSM_PROGMEM
  typedef const char* GsmConstStr;
  #define GFP(x) x
  #define GF(x)  x
#endif

#ifdef TINY_GSM_DEBUG
namespace {
  template<typename T>
  static void DBG_PLAIN(T last) {
    TINY_GSM_DEBUG.println(last);
  }

  template<typename T, typename... Args>
  static void DBG_PLAIN(T head, Args... tail) {
    TINY_GSM_DEBUG.print(head);
    TINY_GSM_DEBUG.print(' ');
    DBG_PLAIN(tail...);
  }

  template<typename... Args>
  static void DBG(Args... args) {
    TINY_GSM_DEBUG.print(GF("["));
    TINY_GSM_DEBUG.print(millis());
    TINY_GSM_DEBUG.print(GF("] "));
    DBG_PLAIN(args...);
  }
}
#else
  #define DBG_PLAIN(...)
  #define DBG(...)
#endif

template<class T>
const T& TinyGsmMin(const T& a, const T& b)
{
    return (b < a) ? b : a;
}

template<class T>
const T& TinyGsmMax(const T& a, const T& b)
{
    return (b < a) ? a : b;
}

template<class T>
uint32_t TinyGsmAutoBaud(T& SerialAT, uint32_t minimum = 9600, uint32_t maximum = 115200)
{
  static uint32_t rates[] = { 115200, 57600, 38400, 19200, 9600, 74400, 74880, 230400, 460800, 2400, 4800, 14400, 28800 };

  for (unsigned i = 0; i < sizeof(rates)/sizeof(rates[0]); i++) {
    uint32_t rate = rates[i];
    if (rate < minimum || rate > maximum) continue;

    DBG("Trying baud rate", rate, "...");
    SerialAT.begin(rate);
    delay(10);
    for (int i=0; i<10; i++) {
      SerialAT.print("AT\r\n");
      String input = SerialAT.readString();
      if (input.indexOf("OK") >= 0) {
        DBG("Modem responded at rate", rate);
        return rate;
      }
    }
  }
  return 0;
}

static inline
IPAddress TinyGsmIpFromString(const String& strIP) {
  int Parts[4] = {0, };
  int Part = 0;
  for (uint8_t i=0; i<strIP.length(); i++) {
    char c = strIP[i];
    if (c == '.') {
      Part++;
      if (Part > 3) {
        return IPAddress(0,0,0,0);
      }
      continue;
    } else if (c >= '0' && c <= '9') {
      Parts[Part] *= 10;
      Parts[Part] += c - '0';
    } else {
      if (Part == 3) break;
    }
  }
  return IPAddress(Parts[0], Parts[1], Parts[2], Parts[3]);
}

static inline
String TinyGsmDecodeHex7bit(String &instr) {
  String result;
  byte reminder = 0;
  int bitstate = 7;
  for (unsigned i=0; i<instr.length(); i+=2) {
    char buf[4] = { 0, };
    buf[0] = instr[i];
    buf[1] = instr[i+1];
    byte b = strtol(buf, NULL, 16);

    byte bb = b << (7 - bitstate);
    char c = (bb + reminder) & 0x7F;
    result += c;
    reminder = b >> bitstate;
    bitstate--;
    if (bitstate == 0) {
      char c = reminder;
      result += c;
      reminder = 0;
      bitstate = 7;
    }
  }
  return result;
}

static inline
String TinyGsmDecodeHex8bit(String &instr) {
  String result;
  for (unsigned i=0; i<instr.length(); i+=2) {
    char buf[4] = { 0, };
    buf[0] = instr[i];
    buf[1] = instr[i+1];
    char b = strtol(buf, NULL, 16);
    result += b;
  }
  return result;
}

static inline
String TinyGsmDecodeHex16bit(String &instr) {
  String result;
  for (unsigned i=0; i<instr.length(); i+=4) {
    char buf[4] = { 0, };
    buf[0] = instr[i];
    buf[1] = instr[i+1];
    char b = strtol(buf, NULL, 16);
    if (b) { // If high byte is non-zero, we can't handle it ;(
#if defined(TINY_GSM_UNICODE_TO_HEX)
      result += "\\x";
      result += instr.substring(i, i+4);
#else
      result += "?";
#endif
    } else {
      buf[0] = instr[i+2];
      buf[1] = instr[i+3];
      b = strtol(buf, NULL, 16);
      result += b;
    }
  }
  return result;
}


// Connect to a IP address given as an IPAddress object by
// converting said IP address to text
#define TINY_GSM_CLIENT_CONNECT_OVERLOADS() \
  virtual int connect(IPAddress ip, uint16_t port, int timeout_s) { \
    String host; host.reserve(16); \
    host += ip[0]; \
    host += "."; \
    host += ip[1]; \
    host += "."; \
    host += ip[2]; \
    host += "."; \
    host += ip[3]; \
    return connect(host.c_str(), port, timeout_s); \
  } \
  virtual int connect(const char *host, uint16_t port) { \
    return connect(host, port, 75); \
  } \
  virtual int connect(IPAddress ip, uint16_t port) { \
    return connect(ip, port, 75); \
  }


// Writes data out on the client using the modem send functionality
#define TINY_GSM_CLIENT_WRITE() \
  virtual size_t write(const uint8_t *buf, size_t size) { \
    TINY_GSM_YIELD(); \
    at->maintain(); \
    return at->modemSend(buf, size, mux); \
  } \
  \
  virtual size_t write(uint8_t c) {\
    return write(&c, 1); \
  }\
  \
  virtual size_t write(const char *str) { \
    if (str == NULL) return 0; \
    return write((const uint8_t *)str, strlen(str)); \
  }


// Returns the combined number of characters available in the TinyGSM fifo
// and the modem chips internal fifo, doing an extra check-in with the
// modem to see if anything has arrived without a UURC.
#define TINY_GSM_CLIENT_AVAILABLE_WITH_BUFFER_CHECK() \
  virtual int available() { \
    TINY_GSM_YIELD(); \
    if (!rx.size()) { \
      /* Workaround: sometimes module forgets to notify about data arrival.
      TODO: Currently we ping the module periodically,
      but maybe there's a better indicator that we need to poll */ \
      if (millis() - prev_check > 500) { \
        got_data = true; \
        prev_check = millis(); \
      } \
      at->maintain(); \
    } \
    return rx.size() + sock_available; \
  }


// Returns the combined number of characters available in the TinyGSM fifo and
// the modem chips internal fifo.  Use this if you don't expect to miss any URC's.
#define TINY_GSM_CLIENT_AVAILABLE_NO_BUFFER_CHECK() \
  virtual int available() { \
    TINY_GSM_YIELD(); \
    if (!rx.size()) { \
      at->maintain(); \
    } \
    return rx.size() + sock_available; \
  }


// Returns the number of characters avaialable in the TinyGSM fifo
// Assumes the modem chip has no internal fifo
#define TINY_GSM_CLIENT_AVAILABLE_NO_MODEM_FIFO() \
  virtual int available() { \
    TINY_GSM_YIELD(); \
    if (!rx.size() && sock_connected) { \
      at->maintain(); \
    } \
    return rx.size(); \
  }


#define TINY_GSM_CLIENT_READ_OVERLOAD() \
  virtual int read() { \
    uint8_t c; \
    if (read(&c, 1) == 1) { \
      return c; \
    } \
    return -1; \
  }

// Reads characters out of the TinyGSM fifo, and from the modem chips internal
// fifo if avaiable, also double checking with the modem if data has arrived
// without issuing a UURC.
#define TINY_GSM_CLIENT_READ_WITH_BUFFER_CHECK() \
  virtual int read(uint8_t *buf, size_t size) { \
    TINY_GSM_YIELD(); \
    at->maintain(); \
    size_t cnt = 0; \
    while (cnt < size) { \
      size_t chunk = TinyGsmMin(size-cnt, rx.size()); \
      if (chunk > 0) { \
        rx.get(buf, chunk); \
        buf += chunk; \
        cnt += chunk; \
        continue; \
      } \
      /* Workaround: sometimes module forgets to notify about data arrival.
      TODO: Currently we ping the module periodically,
      but maybe there's a better indicator that we need to poll */ \
      if (millis() - prev_check > 500) { \
        got_data = true; \
        prev_check = millis(); \
      } \
      /* TODO: Read directly into user buffer? */ \
      at->maintain(); \
      if (sock_available > 0) { \
        int n = at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux); \
        if (n == 0) break; \
      } else { \
        break; \
      } \
    } \
    return cnt; \
  } \
  TINY_GSM_CLIENT_READ_OVERLOAD()


// Reads characters out of the TinyGSM fifo, and from the modem chips internal
// fifo if avaiable.  Use this if you don't expect to miss any URC's.
#define TINY_GSM_CLIENT_READ_NO_BUFFER_CHECK() \
  virtual int read(uint8_t *buf, size_t size) { \
    TINY_GSM_YIELD(); \
    at->maintain(); \
    size_t cnt = 0; \
    while (cnt < size) { \
      size_t chunk = TinyGsmMin(size-cnt, rx.size()); \
      if (chunk > 0) { \
        rx.get(buf, chunk); \
        buf += chunk; \
        cnt += chunk; \
        continue; \
      } \
      /* TODO: Read directly into user buffer? */ \
      at->maintain(); \
      if (sock_available > 0) { \
        int n = at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux); \
        if (n == 0) break; \
      } else { \
        break; \
      } \
    } \
    return cnt; \
  } \
  TINY_GSM_CLIENT_READ_OVERLOAD()


// Reads characters out of the TinyGSM fifo, waiting for any URC's from the
// modem for new data if there's nothing in the fifo.  This assumes the
//modem chip itself has no fifo.
#define TINY_GSM_CLIENT_READ_NO_MODEM_FIFO() \
  virtual int read(uint8_t *buf, size_t size) { \
    TINY_GSM_YIELD(); \
    size_t cnt = 0; \
    uint32_t _startMillis = millis(); \
    while (cnt < size && millis() - _startMillis < _timeout) { \
      size_t chunk = TinyGsmMin(size-cnt, rx.size()); \
      if (chunk > 0) { \
        rx.get(buf, chunk); \
        buf += chunk; \
        cnt += chunk; \
        continue; \
      } \
      /* TODO: Read directly into user buffer? */ \
      if (!rx.size() && sock_connected) { \
        at->maintain(); \
      } \
    } \
    return cnt; \
  } \
  \
  virtual int read() { \
    uint8_t c; \
    if (read(&c, 1) == 1) { \
      return c; \
    } \
    return -1; \
  }


// The peek, flush, and connected functions
#define TINY_GSM_CLIENT_PEEK_FLUSH_CONNECTED() \
  virtual int peek() { return -1; } /* TODO */ \
  \
  virtual void flush() { at->stream.flush(); } \
  \
  virtual uint8_t connected() { \
    if (available()) { \
      return true; \
    } \
    return sock_connected; \
  } \
  virtual operator bool() { return connected(); }


// Set baud rate via the V.25TER standard IPR command
#define TINY_GSM_MODEM_SET_BAUD_IPR() \
  void setBaud(unsigned long baud) { \
    sendAT(GF("+IPR="), baud); \
  }


// Test response to AT commands
#define TINY_GSM_MODEM_TEST_AT() \
  bool testAT(unsigned long timeout_ms = 10000L) { \
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) { \
      sendAT(GF("")); \
      if (waitResponse(200) == 1) return true; \
      delay(100); \
    } \
    return false; \
  }


// Keeps listening for modem URC's and iterates through sockets
// to see if any data is avaiable
#define TINY_GSM_MODEM_MAINTAIN_CHECK_SOCKS() \
  void maintain() { \
    for (int mux = 0; mux < TINY_GSM_MUX_COUNT; mux++) { \
      GsmClient* sock = sockets[mux]; \
      if (sock && sock->got_data) { \
        sock->got_data = false; \
        sock->sock_available = modemGetAvailable(mux); \
      } \
    } \
    while (stream.available()) { \
      waitResponse(15, NULL, NULL); \
    } \
  }


// Keeps listening for modem URC's - doesn't check socks because
// modem has no internal fifo
#define TINY_GSM_MODEM_MAINTAIN_LISTEN() \
  void maintain() { \
    waitResponse(10, NULL, NULL); \
  }


// Asks for modem information via the V.25TER standard ATI command
// NOTE:  The actual value and style of the response is quite varied
#define TINY_GSM_MODEM_GET_INFO_ATI() \
  String getModemInfo() { \
    sendAT(GF("I")); \
    String res; \
    if (waitResponse(1000L, res) != 1) { \
      return ""; \
    } \
    res.replace(GSM_NL "OK" GSM_NL, ""); \
    res.replace(GSM_NL, " "); \
    res.trim(); \
    return res; \
  }


// Unlocks a sim via the 3GPP TS command AT+CPIN
#define TINY_GSM_MODEM_SIM_UNLOCK_CPIN() \
  bool simUnlock(const char *pin) { \
    sendAT(GF("+CPIN=\""), pin, GF("\"")); \
    return waitResponse() == 1; \
  }


// Gets the CCID of a sim card via AT+CCID
#define TINY_GSM_MODEM_GET_SIMCCID_CCID() \
  String getSimCCID() { \
    sendAT(GF("+CCID")); \
    if (waitResponse(GF(GSM_NL "+CCID:")) != 1) { \
      return ""; \
    } \
    String res = stream.readStringUntil('\n'); \
    waitResponse(); \
    res.trim(); \
    return res; \
  }


// Asks for TA Serial Number Identification (IMEI) via the V.25TER standard AT+GSN command
#define TINY_GSM_MODEM_GET_IMEI_GSN() \
  String getIMEI() { \
    sendAT(GF("+GSN")); \
    if (waitResponse(GF(GSM_NL)) != 1) { \
      return ""; \
    } \
    String res = stream.readStringUntil('\n'); \
    waitResponse(); \
    res.trim(); \
    return res; \
  }


// Gets the modem's registration status via CREG/CGREG/CEREG
// CREG = Generic network registration
// CGREG = GPRS service registration
// CEREG = EPS registration for LTE modules
#define TINY_GSM_MODEM_GET_REGISTRATION_XREG(regCommand) \
  RegStatus getRegistrationStatus() { \
    sendAT(GF("+" #regCommand "?")); \
    if (waitResponse(GF(GSM_NL "+" #regCommand ":")) != 1) { \
      return REG_UNKNOWN; \
    } \
    streamSkipUntil(','); /* Skip format (0) */ \
    int status = stream.readStringUntil('\n').toInt(); \
    waitResponse(); \
    return (RegStatus)status; \
  }


// Gets the current network operator via the 3GPP TS command AT+COPS
#define TINY_GSM_MODEM_GET_OPERATOR_COPS() \
  String getOperator() { \
    sendAT(GF("+COPS?")); \
    if (waitResponse(GF(GSM_NL "+COPS:")) != 1) { \
      return ""; \
    } \
    streamSkipUntil('"'); /* Skip mode and format */ \
    String res = stream.readStringUntil('"'); \
    waitResponse(); \
    return res; \
  }


// Waits for network attachment
#define TINY_GSM_MODEM_WAIT_FOR_NETWORK() \
  bool waitForNetwork(unsigned long timeout_ms = 60000L) { \
    for (unsigned long start = millis(); millis() - start < timeout_ms; ) { \
      if (isNetworkConnected()) { \
        return true; \
      } \
      delay(250); \
    } \
    return false; \
  }


// Checks if current attached to GPRS/EPS service
#define TINY_GSM_MODEM_GET_GPRS_IP_CONNECTED() \
  bool isGprsConnected() { \
    sendAT(GF("+CGATT?")); \
    if (waitResponse(GF(GSM_NL "+CGATT:")) != 1) { \
      return false; \
    } \
    int res = stream.readStringUntil('\n').toInt(); \
    waitResponse(); \
    if (res != 1) \
      return false; \
  \
    return localIP() != IPAddress(0,0,0,0); \
  }


// Gets signal quality report according to 3GPP TS command AT+CSQ
#define TINY_GSM_MODEM_GET_CSQ() \
  int16_t getSignalQuality() { \
    sendAT(GF("+CSQ")); \
    if (waitResponse(GF(GSM_NL "+CSQ:")) != 1) { \
      return 99; \
    } \
    int res = stream.readStringUntil(',').toInt(); \
    waitResponse(); \
    return res; \
  }


// Yields up to a time-out period and then reads a character from the stream into the mux FIFO
// TODO:  Do we need to wait two _timeout periods for no character return?  Will wait once in the first
// "while !stream.available()" and then will wait again in the stream.read() function.
#define TINY_GSM_MODEM_STREAM_TO_MUX_FIFO_WITH_DOUBLE_TIMEOUT \
  uint32_t startMillis = millis(); \
  while (!stream.available() && (millis() - startMillis < sockets[mux]->_timeout)) { TINY_GSM_YIELD(); } \
  char c = stream.read(); \
  sockets[mux]->rx.put(c);


// Utility templates for writing/skipping characters on a stream
#define TINY_GSM_MODEM_STREAM_UTILITIES() \
  template<typename T> \
  void streamWrite(T last) { \
    stream.print(last); \
  } \
  \
  template<typename T, typename... Args> \
  void streamWrite(T head, Args... tail) { \
    stream.print(head); \
    streamWrite(tail...); \
  } \
  \
  template<typename... Args> \
  void sendAT(Args... cmd) { \
    streamWrite("AT", cmd..., GSM_NL); \
    stream.flush(); \
    TINY_GSM_YIELD(); \
    /* DBG("### AT:", cmd...); */ \
  } \
  \
  bool streamSkipUntil(const char c, const unsigned long timeout_ms = 1000L) { \
    unsigned long startMillis = millis(); \
    while (millis() - startMillis < timeout_ms) { \
      while (millis() - startMillis < timeout_ms && !stream.available()) { \
        TINY_GSM_YIELD(); \
      } \
      if (stream.read() == c) { \
        return true; \
      } \
    } \
    return false; \
  }


#endif
