/**
 * @file       TinyGsmSMS.tpp
 * @brief      SMS messaging helper mixin.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMSMS_TPP_
#define SRC_TINYGSMSMS_TPP_

#include "TinyGsmCommon.h"

#ifndef TINY_GSM_MODEM_HAS_SMS
/// flag to indicate that the modem has Short Message Service (SMS) functions
#define TINY_GSM_MODEM_HAS_SMS
#endif

/**
 * @brief The CRTP parent class for Short Message Service (SMS) functions.
 * @tparam modemType The derived modem class
 */
template <class modemType>
class TinyGsmSMS {
 public:
  /// Compile-time capability flag indicating SMS messaging support
  static constexpr bool hasSMS = true;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor sms_functions
   * @name Text messaging (SMS) functions
   */
  /**@{*/

  /**
   * @brief Send a USSD code to the network.
   *
   * @param code The USSD code to send.
   * @return The response from the network as a String.
   */
  String sendUSSD(const String& code) {
    return thisModem().sendUSSDImpl(code);
  }
  /**
   * @brief Send an SMS message.
   *
   * @param number The recipient's phone number.
   * @param text The message text.
   * @return True if the message was successfully sent, false otherwise.
   */
  bool sendSMS(const String& number, const String& text) {
    return thisModem().sendSMSImpl(number, text);
  }
  /**
   * @brief Send an SMS message in UTF-16 encoding.
   *
   * @param number The recipient's phone number.
   * @param text The message text in UTF-16 encoding.
   * @param len The length of the message text.
   * @return True if the message was successfully sent, false otherwise.
   */
  bool sendSMS_UTF16(const char* const number, const void* text, size_t len) {
    return thisModem().sendSMS_UTF16Impl(number, text, len);
  }
  /**@}*/


 protected:
  // destructor (protected!)
  ~TinyGsmSMS() {}

  /*
   * CRTP Helper
   */
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }


  /**
   * @anchor sms_hex_utilities
   * @name Hex Decoding Utilities
   */
  /**@{*/
 protected:
  /**
   * @brief Convert one hexadecimal character to its numeric nibble value.
   * @param c The hexadecimal character to convert.
   * @return The decoded nibble value.
   */
  static inline uint8_t TinyGsmHexCharToNibble(char c) {
    return (c <= '9') ? c - '0' : (c & 0x0F) + 9;
  }

  /**
   * @brief Decode a hexadecimal GSM 7-bit string.
   * @param instr The hexadecimal string to decode.
   * @return The decoded GSM 7-bit data.
   */
  static inline String TinyGsmDecodeHex7bit(const String& instr) {
    String  result;
    uint8_t remainder = 0;
    int8_t  bitstate  = 7;

    for (uint16_t i = 0; i < instr.length(); i += 2) {
      uint8_t b = TinyGsmHexCharToNibble(instr[i]);
      b         = (b << 4) | TinyGsmHexCharToNibble(instr[i + 1]);

      result += (char)(((b << (7 - bitstate)) + remainder) & 0x7F);
      remainder = b >> bitstate;

      if (--bitstate == 0) {
        result += (char)remainder;
        remainder = 0;
        bitstate  = 7;
      }
    }

    return result;
  }

  /**
   * @brief Decode a hexadecimal 8-bit string.
   * @param instr The hexadecimal string to decode.
   * @return The decoded 8-bit data.
   */
  static inline String TinyGsmDecodeHex8bit(const String& instr) {
    String result;

    for (uint16_t i = 0; i < instr.length(); i += 2) {
      uint8_t b = TinyGsmHexCharToNibble(instr[i]);
      b         = (b << 4) | TinyGsmHexCharToNibble(instr[i + 1]);

      result += (char)b;
    }

    return result;
  }

  /**
   * @brief Decode a hexadecimal UCS-2 string.
   * @param instr The hexadecimal UCS-2 string to decode.
   * @return The decoded data. When a character's high byte is nonzero,
   * TINY_GSM_UNICODE_TO_HEX controls its representation. Characters with a
   * zero high byte, such as 00E9, are appended as their low byte (0xE9)
   * regardless of that setting.
   */
  static inline String TinyGsmDecodeHex16bit(const String& instr) {
    String result;

    for (uint16_t i = 0; i < instr.length(); i += 4) {
      uint8_t b = TinyGsmHexCharToNibble(instr[i]);
      b         = (b << 4) | TinyGsmHexCharToNibble(instr[i + 1]);

      if (b) {
#if defined(TINY_GSM_UNICODE_TO_HEX)
        result += "\\x";
        result += instr[i];
        result += instr[i + 1];
        result += instr[i + 2];
        result += instr[i + 3];
#else
        result += '?';
#endif
      } else {
        b = TinyGsmHexCharToNibble(instr[i + 2]);
        b = (b << 4) | TinyGsmHexCharToNibble(instr[i + 3]);

        result += (char)b;
      }
    }

    return result;
  }
  /**@}*/

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */
 protected:
  /*
   * Text messaging (SMS) functions
   */

  String sendUSSDImpl(const String& code) {
    // Set preferred message format to text mode
    thisModem().sendAT(GF("+CMGF=1"));
    thisModem().waitResponse();
    // Set 8-bit hexadecimal alphabet (3GPP TS 23.038)
    thisModem().sendAT(GF("+CSCS=\"HEX\""));
    thisModem().waitResponse();
    // Send the message
    thisModem().sendAT(GF("+CUSD=1,\""), code, '"');
    if (thisModem().waitResponse() != 1) { return ""; }
    if (thisModem().waitResponse(10000L, GF("+CUSD:")) != 1) { return ""; }
    thisModem().stream.readStringUntil('"');
    String hex = thisModem().stream.readStringUntil('"');
    thisModem().stream.readStringUntil(',');
    int8_t dcs = thisModem().streamGetIntBefore('\n');

    if (dcs == 15) {
      return TinyGsmDecodeHex8bit(hex);
    } else if (dcs == 72) {
      return TinyGsmDecodeHex16bit(hex);
    } else {
      return hex;
    }
  }

  bool sendSMSImpl(const String& number, const String& text) {
    // Set preferred message format to text mode
    thisModem().sendAT(GF("+CMGF=1"));
    thisModem().waitResponse();
    // Set GSM 7 bit default alphabet (3GPP TS 23.038)
    thisModem().sendAT(GF("+CSCS=\"GSM\""));
    thisModem().waitResponse();
    thisModem().sendAT(GF("+CMGS=\""), number, '"');
    if (thisModem().waitResponse(GF(">")) != 1) { return false; }
    thisModem().stream.print(text);  // Actually send the message
    thisModem().stream.write(static_cast<char>(0x1A));  // Terminate the message
    thisModem().stream.flush();
    return thisModem().waitResponse(60000L) == 1;
  }

  // Common methods for UTF8/UTF16 SMS.
  // Supported by: BG96, M95, MC60, SIM5360, SIM7000, SIM7600, SIM800
  class UTF8Print : public Print {
   public:
    explicit UTF8Print(Print& p) : p(p) {}
    size_t write(const uint8_t c) override {
      if (prv < 0xC0) {
        if (c < 0xC0) printHex(c);
        prv = c;
      } else {
        uint16_t v = uint16_t(prv) << 8 | c;
        v -= (v >> 8 == 0xD0) ? 0xCC80 : 0xCD40;
        printHex(v);
        prv = 0;
      }
      return 1;
    }

   private:
    Print&  p;
    uint8_t prv = 0;
    void    printHex(const uint16_t v) {
      uint8_t c = v >> 8;
      if (c < 0x10) p.print('0');
      p.print(c, HEX);
      c = v & 0xFF;
      if (c < 0x10) p.print('0');
      p.print(c, HEX);
    }
  };

  bool sendSMS_UTF8_begin(const char* const number) {
    thisModem().sendAT(GF("+CMGF=1"));
    thisModem().waitResponse();
    thisModem().sendAT(GF("+CSCS=\"HEX\""));
    thisModem().waitResponse();
    thisModem().sendAT(GF("+CSMP=17,167,0,8"));
    thisModem().waitResponse();

    thisModem().sendAT(GF("+CMGS=\""), number, '"');
    return thisModem().waitResponse(GF(">")) == 1;
  }
  bool sendSMS_UTF8_end() {
    thisModem().stream.write(static_cast<char>(0x1A));
    thisModem().stream.flush();
    return thisModem().waitResponse(60000L) == 1;
  }
  UTF8Print sendSMS_UTF8_stream() {
    return UTF8Print(thisModem().stream);
  }

  bool sendSMS_UTF16Impl(const char* const number, const void* text,
                         size_t len) {
    if (!sendSMS_UTF8_begin(number)) { return false; }

    const uint16_t* t = reinterpret_cast<const uint16_t*>(text);
    for (size_t i = 0; i < len; i++) {
      uint8_t c = t[i] >> 8;
      if (c < 0x10) { thisModem().stream.print('0'); }
      thisModem().stream.print(c, HEX);
      c = t[i] & 0xFF;
      if (c < 0x10) { thisModem().stream.print('0'); }
      thisModem().stream.print(c, HEX);
    }

    return sendSMS_UTF8_end();
  }
};

#endif  // SRC_TINYGSMSMS_TPP_
