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

#define TINY_GSM_MODEM_HAS_TCP

#include "TinyGsmFifo.h"

#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 64
#endif

// Because of the ordering of resolution of overrides in templates, these need
// to be written out every time.  This macro is to shorten that.
#define TINY_GSM_CLIENT_CONNECT_OVERRIDES                             \
  int connect(IPAddress ip, uint16_t port, int timeout_s) {           \
    return connect(TinyGsmStringFromIp(ip).c_str(), port, timeout_s); \
  }                                                                   \
  int connect(const char* host, uint16_t port) override {             \
    return connect(host, port, 75);                                   \
  }                                                                   \
  int connect(IPAddress ip, uint16_t port) override {                 \
    return connect(ip, port, 75);                                     \
  }

// // For modules that do not store incoming data in any sort of buffer
// #define TINY_GSM_NO_MODEM_BUFFER
// // Data is stored in a buffer, but we can only read from the buffer,
// // not check how much data is stored in it
// #define TINY_GSM_BUFFER_READ_NO_CHECK
// // Data is stored in a buffer and we can both read and check the size
// // of the buffer
// #define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE

template <class modemType, uint8_t muxCount>
class TinyGsmTCP {
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
    friend class TinyGsmTCP<modemType, muxCount>;
    typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

   public:
    // bool init(modemType* modem, uint8_t);
    // int connect(const char* host, uint16_t port, int timeout_s);

    // Connect to a IP address given as an IPAddress object by
    // converting said IP address to text
    // virtual int connect(IPAddress ip,uint16_t port, int timeout_s) {
    //   return connect(TinyGsmStringFromIp(ip).c_str(), port,
    //   timeout_s);
    // }
    // int connect(const char* host, uint16_t port) override {
    //   return connect(host, port, 75);
    // }
    // int connect(IPAddress ip,uint16_t port) override {
    //   return connect(ip, port, 75);
    // }

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

    // void stop(uint32_t maxWaitMs);
    // void stop() override {
    //   stop(15000L);
    // }

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
#if defined TINY_GSM_NO_MODEM_BUFFER
      // Returns the number of characters available in the TinyGSM fifo
      if (!rx.size() && sock_connected) { at->maintain(); }
      return rx.size();

#elif defined TINY_GSM_BUFFER_READ_NO_CHECK
      // Returns the combined number of characters available in the TinyGSM
      // fifo and the modem chips internal fifo.
      if (!rx.size()) { at->maintain(); }
      return static_cast<uint16_t>(rx.size()) + sock_available;

#elif defined TINY_GSM_BUFFER_READ_AND_CHECK_SIZE
      // Returns the combined number of characters available in the TinyGSM
      // fifo and the modem chips internal fifo, doing an extra check-in
      // with the modem to see if anything has arrived without a UURC.
      if (!rx.size()) {
        if (millis() - prev_check > 500) {
          // setting got_data to true will tell maintain to run
          // modemGetAvailable(mux)
          got_data   = true;
          prev_check = millis();
        }
        at->maintain();
      }
      return static_cast<uint16_t>(rx.size()) + sock_available;

#else
#error Modem client has been incorrectly created
#endif
    }

    int read(uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      size_t cnt = 0;

#if defined TINY_GSM_NO_MODEM_BUFFER
      // Reads characters out of the TinyGSM fifo, waiting for any URC's
      // from the modem for new data if there's nothing in the fifo.
      uint32_t _startMillis = millis();
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

#elif defined TINY_GSM_BUFFER_READ_NO_CHECK
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
          int n = at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available),
                                mux);
          if (n == 0) break;
        } else {
          break;
        }
      }
      return cnt;

#elif defined TINY_GSM_BUFFER_READ_AND_CHECK_SIZE
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
          // setting got_data to true will tell maintain to run
          // modemGetAvailable()
          got_data   = true;
          prev_check = millis();
        }
        // TODO(vshymanskyy): Read directly into user buffer?
        at->maintain();
        if (sock_available > 0) {
          int n = at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available),
                                mux);
          if (n == 0) break;
        } else {
          break;
        }
      }
      return cnt;

#else
#error Modem client has been incorrectly created
#endif
    }

    int read() override {
      uint8_t c;
      if (read(&c, 1) == 1) { return c; }
      return -1;
    }

	int peek() override {
		return (uint8_t)rx.peek();
	}

    void flush() override {
      at->stream.flush();
    }

    uint8_t connected() override {
      if (available()) { return true; }
#if defined TINY_GSM_BUFFER_READ_AND_CHECK_SIZE
      // If the modem is one where we can read and check the size of the buffer,
      // then the 'available()' function will call a check of the current size
      // of the buffer and state of the connection. [available calls maintain,
      // maintain calls modemGetAvailable, modemGetAvailable calls
      // modemGetConnected]  This cascade means that the sock_connected value
      // should be correct and all we need
      return sock_connected;
#elif defined TINY_GSM_NO_MODEM_BUFFER || defined TINY_GSM_BUFFER_READ_NO_CHECK
      // If the modem doesn't have an internal buffer, or if we can't check how
      // many characters are in the buffer then the cascade won't happen.
      // We need to call modemGetConnected to check the sock state.
      return at->modemGetConnected(mux);
#else
#error Modem client has been incorrectly created
#endif
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
    inline void dumpModemBuffer(uint32_t maxWaitMs) {
#if defined TINY_GSM_BUFFER_READ_AND_CHECK_SIZE || \
    defined TINY_GSM_BUFFER_READ_NO_CHECK
      TINY_GSM_YIELD();
      uint32_t startMillis = millis();
      while (sock_available > 0 && (millis() - startMillis < maxWaitMs)) {
        rx.clear();
        at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available), mux);
      }
      rx.clear();
      at->streamClear();

#elif defined TINY_GSM_NO_MODEM_BUFFER
      rx.clear();
      at->streamClear();

#else
#error Modem client has been incorrectly created
#endif
    }

    modemType* at;
    uint8_t    mux;
    uint16_t   sock_available;
    uint32_t   prev_check;
    bool       sock_connected;
    bool       got_data;
    RxFifo     rx;
  };

  /*
   * Basic functions
   */
 protected:
  void maintainImpl() {
#if defined TINY_GSM_BUFFER_READ_AND_CHECK_SIZE
    // Keep listening for modem URC's and proactively iterate through
    // sockets asking if any data is avaiable
    for (int mux = 0; mux < muxCount; mux++) {
      GsmClient* sock = thisModem().sockets[mux];
      if (sock && sock->got_data) {
        sock->got_data       = false;
        sock->sock_available = thisModem().modemGetAvailable(mux);
      }
    }
    while (thisModem().stream.available()) {
      thisModem().waitResponse(15, NULL, NULL);
    }

#elif defined TINY_GSM_NO_MODEM_BUFFER || defined TINY_GSM_BUFFER_READ_NO_CHECK
    // Just listen for any URC's
    thisModem().waitResponse(100, NULL, NULL);

#else
#error Modem client has been incorrectly created
#endif
  }

  // Yields up to a time-out period and then reads a character from the stream
  // into the mux FIFO
  // TODO(SRGDamia1):  Do we need to wait two _timeout periods for no
  // character return?  Will wait once in the first "while
  // !stream.available()" and then will wait again in the stream.read()
  // function.
  inline void moveCharFromStreamToFifo(uint8_t mux) {
    if (!thisModem().sockets[mux]) return;
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
