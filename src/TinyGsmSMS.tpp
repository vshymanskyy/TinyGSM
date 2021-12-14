/**
 * @file       TinyGsmSMS.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMSMS_H_
#define SRC_TINYGSMSMS_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_SMS

template <class modemType>
class TinyGsmSMS {
 public:
  /*
   * Messaging functions
   */
  String sendUSSD(const String& code) {
    return thisModem().sendUSSDImpl(code);
  }
  bool sendSMS(const String& number, const String& text) {
    return thisModem().sendSMSImpl(number, text);
  }
  bool sendSMS_UTF16(const char* const number, const void* text, size_t len) {
    return thisModem().sendSMS_UTF16Impl(number, text, len);
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
   * Messaging functions
   */
 protected:
  static inline String TinyGsmDecodeHex7bit(String& instr) {
    String result;
    byte   reminder = 0;
    int8_t bitstate = 7;
    for (uint8_t i = 0; i < instr.length(); i += 2) {
      char buf[4] = {
          0,
      };
      buf[0] = instr[i];
      buf[1] = instr[i + 1];
      byte b = strtol(buf, NULL, 16);

      byte bb = b << (7 - bitstate);
      char c  = (bb + reminder) & 0x7F;
      result += c;
      reminder = b >> bitstate;
      bitstate--;
      if (bitstate == 0) {
        char cc = reminder;
        result += cc;
        reminder = 0;
        bitstate = 7;
      }
    }
    return result;
  }

  static inline String TinyGsmDecodeHex8bit(String& instr) {
    String result;
    for (uint16_t i = 0; i < instr.length(); i += 2) {
      char buf[4] = {
          0,
      };
      buf[0] = instr[i];
      buf[1] = instr[i + 1];
      char b = strtol(buf, NULL, 16);
      result += b;
    }
    return result;
  }

  static inline String TinyGsmDecodeHex16bit(String& instr) {
    String result;
    for (uint16_t i = 0; i < instr.length(); i += 4) {
      char buf[4] = {
          0,
      };
      buf[0] = instr[i];
      buf[1] = instr[i + 1];
      char b = strtol(buf, NULL, 16);
      if (b) {  // If high byte is non-zero, we can't handle it ;(
#if defined(TINY_GSM_UNICODE_TO_HEX)
        result += "\\x";
        result += instr.substring(i, i + 4);
#else
        result += "?";
#endif
      } else {
        buf[0] = instr[i + 2];
        buf[1] = instr[i + 3];
        b      = strtol(buf, NULL, 16);
        result += b;
      }
    }
    return result;
  }

  String sendUSSDImpl(const String& code) {
    // Set preferred message format to text mode
    thisModem().sendAT(GF("+CMGF=1"));
    thisModem().waitResponse();
    // Set 8-bit hexadecimal alphabet (3GPP TS 23.038)
    thisModem().sendAT(GF("+CSCS=\"HEX\""));
    thisModem().waitResponse();
    // Send the message
    thisModem().sendAT(GF("+CUSD=1,\""), code, GF("\""));
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
    thisModem().sendAT(GF("+CMGS=\""), number, GF("\""));
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

    thisModem().sendAT(GF("+CMGS=\""), number, GF("\""));
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

    uint16_t* t =
        const_cast<uint16_t*>(reinterpret_cast<const uint16_t*>(text));
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

#endif  // SRC_TINYGSMSMS_H_
