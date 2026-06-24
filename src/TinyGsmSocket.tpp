/**
 * @file       TinyGsmUDP.tpp
 * @author     Ivano Culmine
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2024 Ivano Culmine
 * @date       Jul 2024
 */

#ifndef SRC_TINYGSMSOCKET_H_
#define SRC_TINYGSMSOCKET_H_

#include "TinyGsmCommon.h"
#include "TinyGsmFifo.h"

#if !defined(TINY_GSM_RX_BUFFER)
#define TINY_GSM_RX_BUFFER 64
#endif

template <class modemType, uint8_t muxCount>
class TinyGsmSocket {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  class GsmSocket : public Stream {

    typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;
    friend class TinyGsmSocket<modemType, muxCount>;
    friend modemType;

    public:

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
        while (this->sock_available > 0 && (millis() - startMillis < maxWaitMs)) {
          this->rx.clear();
          at->modemRead(TinyGsmMin((uint16_t)this->rx.free(), this->sock_available), mux);
        }
        this->rx.clear();
        at->streamClear();

  #elif defined TINY_GSM_NO_MODEM_BUFFER
        this->rx.clear();
        at->streamClear();

  #else
  #error Modem client has been incorrectly created
  #endif
      }
      
      modemType* at;
      uint8_t    mux;
      bool       got_data;
      uint16_t   sock_available;
      bool       sock_connected;
      RxFifo     rx;
  };

  /*
   * Basic functions
   */
  void maintain() {
    thisModem().maintainImpl();
  }

  /*
   * CRTP Helper
   */
 protected:
  virtual ~TinyGsmSocket() {};

  modemType* modemInstance;

  inline const modemType& thisModem() const {
    //return static_cast<const modemType&>(*this);
    return *modemInstance;
  }
  inline modemType& thisModem() {
    //return static_cast<modemType&>(*this);
    return *modemInstance;
  }

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Basic functions
   */
 protected:
  void maintainImpl() {
#if defined TINY_GSM_BUFFER_READ_AND_CHECK_SIZE
    // Keep listening for modem URC's and proactively iterate through
    // sockets asking if any data is avaiable
    for (int mux = 0; mux < muxCount; mux++) {
      GsmSocket* sock = thisModem().sockets[mux];
      if (sock && sock->got_data) {
        sock->got_data       = false;
        sock->sock_available = thisModem().modemGetAvailable(mux);
      }
    }
    while (thisModem().stream.available()) {
      thisModem().waitResponse(15, nullptr, nullptr);
    }

#elif defined TINY_GSM_NO_MODEM_BUFFER || defined TINY_GSM_BUFFER_READ_NO_CHECK
    // Just listen for any URC's
    thisModem().waitResponse(100, nullptr, nullptr);

#else
#error Modem client has been incorrectly created
#endif
  }

  // Yields up to a time-out period and then reads a character from the stream
  // into the mux FIFO
  // TODO(SRGDamia1):  Do we really need to wait _two_ timeout periods for no
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

#endif  // SRC_TINYGSMSOCKET_H_