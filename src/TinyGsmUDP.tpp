/**
 * @file       TinyGsmUDP.tpp
 * @author     Ivano Culmine
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2024 Ivano Culmine
 * @date       Jul 2024
 */

#ifndef SRC_TINYGSMUDP_H_
#define SRC_TINYGSMUDP_H_

#include "TinyGsmSocket.tpp"

#define TINY_GSM_MODEM_HAS_UDP

#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 64
#endif

struct bufferElement {
  uint16_t length;
  uint8_t* buffer;
  struct bufferElement* next;
};

// // For modules that do not store incoming data in any sort of buffer
// #define TINY_GSM_NO_MODEM_BUFFER
// // Data is stored in a buffer, but we can only read from the buffer,
// // not check how much data is stored in it
// #define TINY_GSM_BUFFER_READ_NO_CHECK
// // Data is stored in a buffer and we can both read and check the size
// // of the buffer
// #define TINY_GSM_BUFFER_READ_AND_CHECK_SIZE

template <class modemType, uint8_t muxCount>
class TinyGsmUDP : virtual public TinyGsmSocket<modemType, muxCount> {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:

  /*
   * CRTP Helper
   */
 protected:
  TinyGsmUDP() { this->modemInstance = static_cast<modemType*>(this); }
  ~TinyGsmUDP() {}

  virtual uint8_t modemBeginUdp(uint16_t port, int timeout_s, uint8_t mux) = 0;

  /*
   * Inner UDP Client
   */
 public:
  class GsmUdp : public TinyGsmSocket<modemType, muxCount>::GsmSocket, public UDP {
    // Make all classes created from the modem template friends
    friend class TinyGsmUDP<modemType, muxCount>;
    typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

   protected:
   
   public:
    virtual ~GsmUdp() {};

    static inline String TinyGsmStringFromIP(IPAddress ip) {
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

    virtual void stop() {}

    int read(char* buf, size_t size) override {
      return read(reinterpret_cast<uint8_t*>(buf), size);
    }

    uint8_t begin(uint16_t port) override {
      return this->at->modemBeginUdp(port, 15, this->mux);
    }

    int beginPacket(IPAddress ip, uint16_t port) override {
      return beginPacket(TinyGsmStringFromIP(ip).c_str(), port);
    }

    int beginPacket(const char *host, uint16_t port) override {
      this->buffering = true;
      if (this->host != nullptr) {
        delete[] this->host;
      }
      this->host = new char[strlen(host)+1];
      strcpy(this->host, host);
      this->port = port;
      return 0;
    }

    int endPacket() override {
      if (!this->buffering) {
        return 0;
      }

      this->buffering = false;

      int size = 0;
      struct bufferElement** nextPointer = &this->packetBuffer;
      while (*nextPointer != nullptr) {
        size += (*nextPointer)->length;
        nextPointer = &(*nextPointer)->next;
      }
      nextPointer = &this->packetBuffer;

      uint8_t* buff = new uint8_t[size];
      int i = 0;
      while (*nextPointer != nullptr) {
        for (int j = 0; j < (*nextPointer)->length; j++) {
          buff[i++] = (*nextPointer)->buffer[j];
        }
        delete[] (*nextPointer)->buffer;
        struct bufferElement** newNextPointer = &(*nextPointer)->next;
        delete *nextPointer;
        nextPointer = newNextPointer;
      }
      int sizeRes = 0;
      sizeRes = this->at->modemSend(buff, size, this->mux, host, port);
      if (sizeRes == 0) {
        this->begin(this->port);
        sizeRes = this->at->modemSend(buff, size, this->mux, host, port);
      }
      delete[] buff;
      this->packetBuffer = nullptr;

      return sizeRes;
    }

    size_t write(uint8_t byte) override {
      return write(&byte, 1);
    }

    size_t write(const uint8_t *buffer, size_t size) override {
      struct bufferElement** nextPointer = &this->packetBuffer;
      while (*nextPointer != nullptr) {
        nextPointer = &(*nextPointer)->next;
      }
      *nextPointer = new bufferElement {
        size,
        new uint8_t[size],
        nullptr
      };
      for (int i = 0; i < size; i++) {
        (*nextPointer)->buffer[i] = buffer[i];
      }
      return size;
    }

    size_t write(const char* str) {
      if (str == nullptr) return 0;
      return write((const uint8_t*)str, strlen(str));
    }

    int parsePacket() override {
      return available();
    }
    
    int read() override {
      uint8_t c;
      if (read(&c, 1) == 1) { return c; }
      return -1;
    }

    int available() override {
      TINY_GSM_YIELD();
#if defined TINY_GSM_NO_MODEM_BUFFER
      // Returns the number of characters available in the TinyGSM fifo
      if (!this->rx.size() && this->sock_connected) { this->at->TinyGsmUDP::maintain(); }
      return this->rx.size();

#elif defined TINY_GSM_BUFFER_READ_NO_CHECK
      // Returns the combined number of characters available in the TinyGSM
      // fifo and the modem chips internal fifo.
      if (!this->rx.size()) { this->at->TinyGsmUDP::maintain(); }
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
        this->at->TinyGsmUDP::maintain();
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
        if (!this->rx.size() && this->sock_connected) { this->at->TinyGsmUDP::maintain(); }
      }
      return cnt;

#elif defined TINY_GSM_BUFFER_READ_NO_CHECK
      // Reads characters out of the TinyGSM fifo, and from the modem chip's
      // internal fifo if avaiable.
      this->at->TinyGsmUDP::maintain();
      while (cnt < size) {
        size_t chunk = TinyGsmMin(size - cnt, this->rx.size());
        if (chunk > 0) {
          this->rx.get(buf, chunk);
          buf += chunk;
          cnt += chunk;
          continue;
        } /* TODO: Read directly into user buffer? */
        this->at->TinyGsmUDP::maintain();
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
      this->at->TinyGsmUDP::maintain();
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
        this->at->TinyGsmUDP::maintain();
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
    
    int peek() override {
      return (uint8_t)this->rx.peek();
    }
    
    void flush() override {
      this->at->stream.flush();
    }
    
    operator bool() {
      return true;
    }

    IPAddress remoteIP() override {
      uint8_t bytes[4];
      return IPAddress(bytes);
    }

    uint16_t remotePort() override {
      return this->port;
    }

    /*
     * Extended API
     */

    protected:

    uint32_t   prev_check;
    char* host = nullptr;
    uint16_t port;
    bool buffering = false;
    struct bufferElement* packetBuffer = nullptr;
  };
};

#endif  // SRC_TINYGSMUDP_H_
