/**
 * @file       TinyGsmTCP.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMTCP_H_
#define SRC_TINYGSMTCP_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_HAS_TCP

enum modemInternalBuffferType {
  NO_MODEM_BUFFER =
      0,  // For modules that do not store incoming data in any sort of buffer
  READ_NO_CHECK = 1,  // Data is stored in a buffer, but we can only read from
                      // the buffer, not check how much data is stored in it
  READ_AND_CHECK_SIZE = 2,  // Data is stored in a buffer and we can both read
                            // and check the size of the buffer
};

// The fully generic template is empty
template <class modemType, modemInternalBuffferType bufType, uint8_t muxCount>
class TinyGsmTCP {};

// Partially specialized templates

/* ===========================================================================
* Specialized template for modems no internal buffer
=========================================================================== */
template <class modemType, modemInternalBuffferType bufType, uint8_t muxCount>
class TinyGsmTCP<modemType, NO_MODEM_BUFFER, muxCount> {
 public:
  /*
   * Basic functions
   */
  void maintain() {
    return thisModem().maintainImpl();
  }

  /*
   * CRTP Helper
   */
 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }

  /*
   * Inner Client
   */
 public:
  class GsmClient : public Client {
    // Make all classes created from the modem template friends
    friend class TinyGsmModem<modemType, bufType, muxCount>;
    typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

   public:
    static inline String TinyGsmStringFromIp(IPAddress ip) {
      String host;
      host.reserve(16);
      host += ip[0];
      host += ".";
      host += ip[1];
      host += ".";
      host += ip[2];
      host += ".";
      host += ip[3];
      return host;
    }

    // Writes data out on the client using the modem send functionality
    size_t write(const uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      at->maintain();
      return at->modemSend(buf, size, mux);
    }

    size_t write(uint8_t c) override {
      return write(&c, 1);
    }

    size_t write(const char* str) {
      if (str == NULL) return 0;
      return write((const uint8_t*)str, strlen(str));
    }

    int available() override {
      TINY_GSM_YIELD();
        // Returns the number of characters available in the TinyGSM fifo
          if (!rx.size() && sock_connected) { at->maintain(); }
          return rx.size();
    }

    int read(uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      size_t   cnt          = 0;
      uint32_t _startMillis = millis();
        // Reads characters out of the TinyGSM fifo, waiting for any URC's
        // from the modem for new data if there's nothing in the fifo.
          while (cnt < size && millis() - _startMillis < _timeout) {
            size_t chunk = TinyGsmMin(size - cnt, rx.size());
            if (chunk > 0) {
              rx.get(buf, chunk);
              buf += chunk;
              cnt += chunk;
              continue;
            } /* TODO: Read directly into user buffer? */
            if (!rx.size() && sock_connected) { at->maintain(); }
          }
          return cnt;
    }

    int read() override {
      uint8_t c;
      if (read(&c, 1) == 1) { return c; }
      return -1;
    }

    // TODO(SRGDamia1): Implement peek
    int peek() override {
      return -1;
    }

    void flush() override {
      at->stream.flush();
    }

    uint8_t connected() override {
      if (available()) { return true; }
      return sock_connected;
    }
    operator bool() override {
      return connected();
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

   protected:
    modemType* at;
    uint8_t    mux;
    bool       sock_connected;
    RxFifo     rx;
  };

  /*
   * Basic functions
   */
 protected:
  void maintainImpl() {
        // Just listen for any URC's
        thisModem().waitResponse(100, NULL, NULL);
  }
};

/* ===========================================================================
* Specialized template for modems with an internal buffer that can be read
* but NOT queried for current size.
=========================================================================== */
template <class modemType, modemInternalBuffferType bufType, uint8_t muxCount>
class TinyGsmTCP<modemType, READ_NO_CHECK, muxCount> {
 public:
  /*
   * Basic functions
   */
  void maintain() {
    return thisModem().maintainImpl();
  }

  /*
   * CRTP Helper
   */
 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }

  /*
   * Inner Client
   */
 public:
  class GsmClient : public Client {
    // Make all classes created from the modem template friends
    friend class TinyGsmModem<modemType, bufType, muxCount>;
    typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

   public:
    static inline String TinyGsmStringFromIp(IPAddress ip) {
      String host;
      host.reserve(16);
      host += ip[0];
      host += ".";
      host += ip[1];
      host += ".";
      host += ip[2];
      host += ".";
      host += ip[3];
      return host;
    }

    // Writes data out on the client using the modem send functionality
    size_t write(const uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      at->maintain();
      return at->modemSend(buf, size, mux);
    }

    size_t write(uint8_t c) override {
      return write(&c, 1);
    }

    size_t write(const char* str) {
      if (str == NULL) return 0;
      return write((const uint8_t*)str, strlen(str));
    }

    int available() override {
      TINY_GSM_YIELD();

        // Returns the combined number of characters available in the TinyGSM
        // fifo and the modem chips internal fifo.
          if (!rx.size()) { at->maintain(); }
          return rx.size() + sock_available;
    }

    int read(uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      size_t   cnt          = 0;
      uint32_t _startMillis = millis();

        // Reads characters out of the TinyGSM fifo, and from the modem chip's
        // internal fifo if avaiable.
          at->maintain();
          while (cnt < size) {
            size_t chunk = TinyGsmMin(size - cnt, rx.size());
            if (chunk > 0) {
              rx.get(buf, chunk);
              buf += chunk;
              cnt += chunk;
              continue;
            } /* TODO: Read directly into user buffer? */
            at->maintain();
            if (sock_available > 0) {
              int n = at->modemRead(
                  TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
              if (n == 0) break;
            } else {
              break;
            }
          }
          return cnt;
    }

    int read() override {
      uint8_t c;
      if (read(&c, 1) == 1) { return c; }
      return -1;
    }

    // TODO(SRGDamia1): Implement peek
    int peek() override {
      return -1;
    }

    void flush() override {
      at->stream.flush();
    }

    uint8_t connected() override {
      if (available()) { return true; }
      return sock_connected;
    }
    operator bool() override {
      return connected();
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

   protected:
    // Read and dump anything remaining in the modem's internal buffer.
    // Using this in the client stop() function.
    // The socket will appear open in response to connected() even after it
    // closes until all data is read from the buffer.
    // Doing it this way allows the external mcu to find and get all of the
    // data that it wants from the socket even if it was closed externally.
    void inline dumpModemBuffer(uint32_t maxWaitMs) {
      TINY_GSM_YIELD();
      rx.clear();
      at->maintain();
      uint32_t startMillis = millis();
      while (sock_available > 0 && (millis() - startMillis < maxWaitMs)) {
        at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
        rx.clear();
        at->maintain();
      }
    }

    modemType* at;
    uint8_t    mux;
    uint16_t   sock_available;
    bool       sock_connected;
    bool       got_data;
    RxFifo     rx;
  };

  /*
   * Basic functions
   */
 protected:
  void maintainImpl() {
        // Just listen for any URC's
        thisModem().waitResponse(100, NULL, NULL);
        break;
  }

  // Yields up to a time-out period and then reads a character from the stream
  // into the mux FIFO
  // TODO(SRGDamia1):  Do we need to wait two _timeout periods for no
  // character return?  Will wait once in the first "while
  // !stream.available()" and then will wait again in the stream.read()
  // function.
  void inline moveCharFromStreamToFifo(uint8_t mux) {
    uint32_t startMillis = millis();
    while (!thisModem().stream.available() &&
           (millis() - startMillis < thisModem().sockets[mux]->_timeout)) {
      TINY_GSM_YIELD();
    }
    char c = thisModem().stream.read();
    thisModem().sockets[mux]->rx.put(c);
  }
};


/* ===========================================================================
* Specialized template for modems with an internal buffer that can be read
* and queried for current size.
=========================================================================== */
template <class modemType, modemInternalBuffferType bufType, uint8_t muxCount>
class TinyGsmTCP<modemType, READ_AND_CHECK_SIZE, muxCount> {
 public:
  /*
   * Basic functions
   */
  void maintain() { return thisModem().maintainImpl(); }

  /*
   * CRTP Helper
   */
 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() { return static_cast<modemType&>(*this); }

  /*
   * Inner Client
   */
 public:
  class GsmClient : public Client {
    // Make all classes created from the modem template friends
    friend class TinyGsmModem<modemType, bufType, muxCount>;
    typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

   public:
    static inline String TinyGsmStringFromIp(IPAddress ip) {
      String host;
      host.reserve(16);
      host += ip[0];
      host += ".";
      host += ip[1];
      host += ".";
      host += ip[2];
      host += ".";
      host += ip[3];
      return host;
    }

    // Writes data out on the client using the modem send functionality
    size_t write(const uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      at->maintain();
      return at->modemSend(buf, size, mux);
    }

    size_t write(uint8_t c) override { return write(&c, 1); }

    size_t write(const char* str) {
      if (str == NULL) return 0;
      return write((const uint8_t*)str, strlen(str));
    }

    int available() override {
      TINY_GSM_YIELD();
        // Returns the combined number of characters available in the TinyGSM
        // fifo and the modem chips internal fifo, doing an extra check-in
        // with the modem to see if anything has arrived without a UURC.
          if (!rx.size()) {
            if (millis() - prev_check > 500) {
              got_data = true;
              prev_check = millis();
            }
            at->maintain();
          }
          return rx.size() + sock_available;
    }

    int read(uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      size_t cnt = 0;
      uint32_t _startMillis = millis();
        // Reads characters out of the TinyGSM fifo, and from the modem chips
        // internal fifo if avaiable, also double checking with the modem if
        // data has arrived without issuing a UURC.
          at->maintain();
          while (cnt < size) {
            size_t chunk = TinyGsmMin(size - cnt, rx.size());
            if (chunk > 0) {
              rx.get(buf, chunk);
              buf += chunk;
              cnt += chunk;
              continue;
            }
            // Workaround: Some modules "forget" to notify about data arrival
            if (millis() - prev_check > 500) {
              got_data = true;
              prev_check = millis();
            }
            // TODO(vshymanskyy): Read directly into user buffer?
            at->maintain();
            if (sock_available > 0) {
              int n = at->modemRead(
                  TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
              if (n == 0) break;
            } else {
              break;
            }
          }
          return cnt;
    }

    int read() override {
      uint8_t c;
      if (read(&c, 1) == 1) {
        return c;
      }
      return -1;
    }

    // TODO(SRGDamia1): Implement peek
    int peek() override { return -1; }

    void flush() override { at->stream.flush(); }

    uint8_t connected() override {
      if (available()) {
        return true;
      }
      return sock_connected;
    }
    operator bool() override { return connected(); }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;

   protected:
    // Read and dump anything remaining in the modem's internal buffer.
    // Using this in the client stop() function.
    // The socket will appear open in response to connected() even after it
    // closes until all data is read from the buffer.
    // Doing it this way allows the external mcu to find and get all of the
    // data that it wants from the socket even if it was closed externally.
    void inline dumpModemBuffer(uint32_t maxWaitMs) {
      TINY_GSM_YIELD();
      rx.clear();
      at->maintain();
      uint32_t startMillis = millis();
      while (sock_available > 0 && (millis() - startMillis < maxWaitMs)) {
        at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
        rx.clear();
        at->maintain();
      }
    }

    modemType* at;
    uint8_t mux;
    uint16_t sock_available;
    uint32_t prev_check;
    bool sock_connected;
    bool got_data;
    RxFifo rx;
  };

  /*
   * Basic functions
   */
 protected:
  void maintainImpl() {
        // Keep listening for modem URC's and proactively iterate through
        // sockets asking if any data is avaiable
        for (int mux = 0; mux < muxCount; mux++) {
          GsmClient* sock = thisModem().sockets[mux];
          if (sock && sock->got_data) {
            sock->got_data = false;
            sock->sock_available = thisModem().modemGetAvailable(mux);
          }
        }
        while (thisModem().stream.available()) {
          thisModem().waitResponse(15, NULL, NULL);
        }
  }

  // Yields up to a time-out period and then reads a character from the stream
  // into the mux FIFO
  // TODO(SRGDamia1):  Do we need to wait two _timeout periods for no
  // character return?  Will wait once in the first "while
  // !stream.available()" and then will wait again in the stream.read()
  // function.
  void inline moveCharFromStreamToFifo(uint8_t mux) {
    uint32_t startMillis = millis();
    while (!thisModem().stream.available() &&
           (millis() - startMillis < thisModem().sockets[mux]->_timeout)) {
      TINY_GSM_YIELD();
    }
    char c = thisModem().stream.read();
    thisModem().sockets[mux]->rx.put(c);
  }
};

#endif  // SRC_TINYGSMTCP_H_
