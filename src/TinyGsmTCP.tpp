/**
 * @file       TinyGsmTCP.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMTCP_H_
#define SRC_TINYGSMTCP_H_

#include "TinyGsmSocket.tpp"

#define TINY_GSM_MODEM_HAS_TCP

#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 64
#endif

// Because of the ordering of resolution of overrides in templates, these need
// to be written out every time.  This macro is to shorten that.
#define TINY_GSM_CLIENT_CONNECT_OVERRIDES                             \
  int connect(IPAddress ip, uint16_t port, int timeout_s) {           \
    return connect(TinyGsmStringFromIP(ip).c_str(), port, timeout_s); \
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
class TinyGsmTCP : virtual public TinyGsmSocket<modemType, muxCount> {

  /*
   * CRTP Helper
   */
 protected:
  TinyGsmTCP() { this->modemInstance = static_cast<modemType*>(this); }
  ~TinyGsmTCP() {}

  /*
   * Inner Client
   */
 public:
  class GsmClient : public TinyGsmSocket<modemType, muxCount>::GsmSocket, public Client {
    // Make all classes created from the modem template friends
    friend class TinyGsmTCP<modemType, muxCount>;
    typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

   public:
    // bool init(modemType* modem, uint8_t);
    // int connect(const char* host, uint16_t port, int timeout_s);

    // Connect to a IP address given as an IPAddress object by
    // converting said IP address to text
    // virtual int connect(IPAddress ip,uint16_t port, int timeout_s) {
    //   return connect(TinyGsmStringFromIP(ip).c_str(), port,
    //   timeout_s);
    // }
    // int connect(const char* host, uint16_t port) override {
    //   return connect(host, port, 75);
    // }
    // int connect(IPAddress ip,uint16_t port) override {
    //   return connect(ip, port, 75);
    // }

    /*static inline String TinyGsmStringFromIP(IPAddress ip) {
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
    }*/

    // void stop(uint32_t maxWaitMs);
    // void stop() override {
    //   stop(15000L);
    // }

    // Writes data out on the client using the modem send functionality
    size_t write(const uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      this->at->TinyGsmTCP::maintain();
      return this->at->modemSend(buf, size, this->mux);
    }

    size_t write(uint8_t c) override {
      return write(&c, 1);
    }

    size_t write(const char* str) {
      if (str == nullptr) return 0;
      return write((const uint8_t*)str, strlen(str));
    }

    int available() override {
      TINY_GSM_YIELD();
#if defined TINY_GSM_NO_MODEM_BUFFER
      // Returns the number of characters available in the TinyGSM fifo
      if (!this->rx.size() && this->sock_connected) { this->at->TinyGsmTCP::maintain(); }
      return this->rx.size();

#elif defined TINY_GSM_BUFFER_READ_NO_CHECK
      // Returns the combined number of characters available in the TinyGSM
      // fifo and the modem chips internal fifo.
      if (!this->rx.size()) { this->at->TinyGsmTCP::maintain(); }
      return static_cast<uint16_t>(this->rx.size()) + this->sock_available;

#elif defined TINY_GSM_BUFFER_READ_AND_CHECK_SIZE
      // Returns the combined number of characters available in the TinyGSM
      // fifo and the modem chips internal fifo, doing an extra check-in
      // with the modem to see if anything has arrived without a UURC.
      if (!this->rx.size()) {
        if (millis() - prev_check > 500) {
          // setting got_data to true will tell maintain to run
          // modemGetAvailable(this->mux)
          this->got_data   = true;
          prev_check = millis();
        }
        this->at->TinyGsmTCP::maintain();
      }
      return static_cast<uint16_t>(this->rx.size()) + this->sock_available;

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
        size_t chunk = TinyGsmMin(size - cnt, this->rx.size());
        if (chunk > 0) {
          this->rx.get(buf, chunk);
          buf += chunk;
          cnt += chunk;
          continue;
        } /* TODO: Read directly into user buffer? */
        if (!this->rx.size() && this->sock_connected) { this->at->TinyGsmTCP::maintain(); }
      }
      return cnt;

#elif defined TINY_GSM_BUFFER_READ_NO_CHECK
      // Reads characters out of the TinyGSM fifo, and from the modem chip's
      // internal fifo if avaiable.
      this->at->TinyGsmTCP::maintain();
      while (cnt < size) {
        size_t chunk = TinyGsmMin(size - cnt, this->rx.size());
        if (chunk > 0) {
          this->rx.get(buf, chunk);
          buf += chunk;
          cnt += chunk;
          continue;
        } /* TODO: Read directly into user buffer? */
        this->at->TinyGsmTCP::maintain();
        if (this->sock_available > 0) {
          int n = this->at->modemRead(TinyGsmMin((uint16_t)this->rx.free(), this->sock_available),
                                this->mux);
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
      this->at->TinyGsmTCP::maintain();
      while (cnt < size) {
        size_t chunk = TinyGsmMin(size - cnt, this->rx.size());
        if (chunk > 0) {
          this->rx.get(buf, chunk);
          buf += chunk;
          cnt += chunk;
          continue;
        }
        // Workaround: Some modules "forget" to notify about data arrival
        if (millis() - prev_check > 500) {
          // setting got_data to true will tell maintain to run
          // modemGetAvailable()
          this->got_data   = true;
          prev_check = millis();
        }
        // TODO(vshymanskyy): Read directly into user buffer?
        this->at->TinyGsmTCP::maintain();
        if (this->sock_available > 0) {
          int n = this->at->modemRead(TinyGsmMin((uint16_t)this->rx.free(), this->sock_available),
                                this->mux);
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
      return (uint8_t)this->rx.peek();
    }

    void flush() override {
      this->at->stream.flush();
    }

    uint8_t connected() override {
      if (available()) { return true; }
#if defined TINY_GSM_BUFFER_READ_AND_CHECK_SIZE
      // If the modem is one where we can read and check the size of the buffer,
      // then the 'available()' function will call a check of the current size
      // of the buffer and state of the connection. [available calls maintain,
      // maintain calls modemGetAvailable, modemGetAvailable calls
      // modemGetConnected]  This cascade means that the this->sock_connected value
      // should be correct and all we need
      return this->sock_connected;
#elif defined TINY_GSM_NO_MODEM_BUFFER || defined TINY_GSM_BUFFER_READ_NO_CHECK
      // If the modem doesn't have an internal buffer, or if we can't check how
      // many characters are in the buffer then the cascade won't happen.
      // We need to call modemGetConnected to check the sock state.
      return this->at->modemGetConnected(this->mux);
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
    /*inline void dumpModemBuffer(uint32_t maxWaitMs) {
#if defined TINY_GSM_BUFFER_READ_AND_CHECK_SIZE || \
    defined TINY_GSM_BUFFER_READ_NO_CHECK
      TINY_GSM_YIELD();
      uint32_t startMillis = millis();
      while (this->sock_available > 0 && (millis() - startMillis < maxWaitMs)) {
        this->rx.clear();
        this->at->modemRead(TinyGsmMin((uint16_t)this->rx.free(), this->sock_available), this->mux);
      }
      this->rx.clear();
      this->at->streamClear();

#elif defined TINY_GSM_NO_MODEM_BUFFER
      this->rx.clear();
      this->at->streamClear();

#else
#error Modem client has been incorrectly created
#endif
    }*/

    uint32_t   prev_check;
  };
};

#endif  // SRC_TINYGSMTCP_H_
