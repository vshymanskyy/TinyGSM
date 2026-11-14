/**
 * @file       TinyGsmCalling.tpp
 * @brief      Voice calling and DTMF helper mixin for modem implementations.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCALLING_TPP_
#define SRC_TINYGSMCALLING_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_CALLING
/// flag to indicate that the modem has calling functions
#define TINY_GSM_MODEM_HAS_CALLING
#endif

/**
 * @class TinyGsmCalling
 * @brief The CRTP parent class for voice and DTMF calling functions.
 * @tparam modemType The derived modem class
 */
template <class modemType>
class TinyGsmCalling {
 public:
  /// Compile-time capability flag indicating voice call support
  static constexpr bool hasCalling = true;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor calling_functions
   * @name Phone call functions
   */
  /**@{*/

  /**
   * @brief Answer an incoming phone call
   * @return True if the call was answered successfully, false otherwise
   */
  bool callAnswer() {
    return thisModem().callAnswerImpl();
  }

  /**
   * @brief Make a phone call
   *
   * @param number The number to call
   * @return True if the call was made successfully, false otherwise
   */
  bool callNumber(const String& number) {
    return thisModem().callNumberImpl(number);
  }

  /**
   * @brief Hang up an in-progress phone call
   * @return True if the call was hung up successfully, false otherwise
   */
  bool callHangup() {
    return thisModem().callHangupImpl();
  }

  /**
   * @brief Sent a DTMF (dual tone multi frequency) message over a phone call.
   *
   * @param cmd The command to send
   * @param duration_ms The tone duration
   * @return True if the DTMF message was sent successfully, false otherwise
   */
  bool dtmfSend(char cmd, int duration_ms = 100) {
    return thisModem().dtmfSendImpl(cmd, duration_ms);
  }
  /**@}*/


 protected:
  // destructor (protected!)
  ~TinyGsmCalling() {}

  /*
   * CRTP Helper
   */
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */
 protected:
  /*
   * Phone call functions
   */

  bool callAnswerImpl() {
    thisModem().sendAT('A');
    return thisModem().waitResponse() == 1;
  }

  // Returns true on pick-up, false on error/busy
  bool callNumberImpl(const String& number) {
    if (number == GF("last")) {
      thisModem().sendAT(GF("DL"));
    } else {
      thisModem().sendAT('D', number, ';');
    }
    int8_t status = thisModem().waitResponse(60000L, GF("OK"), GF("BUSY"),
                                             GF("NO ANSWER"), GF("NO CARRIER"));
    switch (status) {
      case 1: return true;
      case 2:
      case 3: return false;
      default: return false;
    }
  }

  bool callHangupImpl() {
    thisModem().sendAT('H');
    return thisModem().waitResponse() == 1;
  }

  // 0-9,*,#,A,B,C,D
  bool dtmfSendImpl(char cmd, int duration_ms) {
    duration_ms = constrain(duration_ms, 100, 1000);

    thisModem().sendAT(GF("+VTD="),
                       duration_ms / 100);  // VTD accepts in 1/10 of a second
    thisModem().waitResponse();

    thisModem().sendAT(GF("+VTS="), cmd);
    return thisModem().waitResponse(10000L) == 1;
  }
};

#endif  // SRC_TINYGSMCALLING_TPP_
