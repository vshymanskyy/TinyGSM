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
  bool sendSMS(const String& number, const String& text, const String &charset = "GSM") {
    return thisModem().sendSMSImpl(number, text,charset);
  }
  bool sendSMS_UTF16(const char* const number, const void* text, size_t len) {
    return thisModem().sendSMS_UTF16Impl(number, text, len);
  }

  bool changeCharacterSet(const String &alphabet) {
    return thisModem().changeCharacterSetImpl(alphabet);
  }

  bool receiveNewSMSIndication(const bool enabled = true, const bool cbmIndication = false, const bool statusReport = false) {
    return receiveNewSMSIndicationImpl(enabled,cbmIndication,statusReport);
  }

  Sms readSMSText(const uint8_t index, const bool changeStatusToRead = true) {
    return thisModem().readSMSTextImpl(index,changeStatusToRead);
  }

  String readSMSPDU(const uint8_t index, const bool changeStatusToRead = true) {
    return thisModem().readSMSPDUImpl(index,changeStatusToRead);
  }

  bool deleteSMS(const uint8_t index) {
    return thisModem().deleteSMSImpl(index);
  }

  bool deleteAllSMS(const DeleteAllSmsMethod method) {
    return thisModem().deleteAllSMSImpl(method);
  }

  MessageStorage getPreferredSMSStorage() {
    return thisModem().getPreferredSMSStorageImpl();
  }

  bool setPreferredSMSStorage(const MessageStorageType type[3]) {
    return thisModem().setPreferredSMSStorageImpl(type);
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
  static inline String TinyGsmDecodeHex7bit(const String& instr) {
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

  static inline String TinyGsmDecodeHex8bit(const String& instr) {
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

  static inline String TinyGsmDecodeHex16bit(const String& instr) {
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
    changeCharacterSet(GF("HEX"));
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

  bool sendSMSImpl(const String& number, const String& text,const String &charset = "GSM") {
    // Set preferred message format to text mode
    thisModem().sendAT(GF("+CMGF=1"));
    thisModem().waitResponse();
    // Set GSM 7 bit default alphabet (3GPP TS 23.038)
    changeCharacterSet(charset);
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
    thisModem().changeCharacterSetImpl("HEX");
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

  bool changeCharacterSetImpl(const String &alphabet) {
    thisModem().sendAT(GF("+CSCS=\""), alphabet, '"');
    return thisModem().waitResponse() == 1;
  }

  // Supported by: SIM800 other is not tested
  String readSMSPDUImpl(const uint8_t index, const bool changeStatusToRead = true)
  {
    int res = 0;

    thisModem().sendAT(GF("+CMGF=0"));
    thisModem().waitResponse();

    thisModem().sendAT(GF("+CMGR="), index, GF(","), static_cast<const uint8_t>(!changeStatusToRead));

    for(;;)
    {
        String value;
        res = thisModem().streamGetAtLine(value);
        if ( res < 0 )
        {
            printf("readSMSPDUImpl timeout\n");
            return "";
        }

        if ( value.length() == 0)
        {
            continue;
        }

        printf("readSMSPDUImpl value %s\n", value.c_str());
        if ( value == String("OK") )
        {
            printf("readSMSPDUImpl no message\n");
            return "";
        }
        if ( value.startsWith("+CMGR:") )
        {
            break;
        }
        if ( value == String("ERROR") )
        {
            printf("readSMSPDUImpl ERROR\n");
            return "";
        }
    }

    String PDU;
    if ( thisModem().streamGetAtLine(PDU) != 1 )
    {
        return "";
    }

    return PDU;
  }
  // Supported by: SIM800 other is not tested
  Sms readSMSTextImpl(const uint8_t index, const bool changeStatusToRead = true) {

    thisModem().sendAT(GF("+CMGF=1"));
    thisModem().waitResponse();

    thisModem().sendAT(GF("+CMGR="), index, GF(","), static_cast<const uint8_t>(!changeStatusToRead)); // Read SMS Message
    if (thisModem().waitResponse(5000L, GF("+CMGR: ")) != 1) {
      String s = thisModem().stream.readString();
      return {};
    }


    // AT reply:
    // <stat>,<oa>[,<alpha>],<scts>[,<tooa>,<fo>,<pid>,<dcs>,<sca>,<tosca>,<length>]<CR><LF><data>

    int res = 0;
    String args[10];
    int argsNbr = 0;

    while( (argsNbr < 10 ) && ( res == 0) )
    {
        res = thisModem().streamGetAtValue(args[argsNbr]);
        if ( res < 0 ) { return {}; }
        argsNbr ++;
        Serial.printf("Args %d : '%s'\n",argsNbr,args[argsNbr-1].c_str());
        if ( res > 0 ) { break; }
    }

    char c;
    String data;
    while(thisModem().stream.readBytes(&c,1) == 1)
    {
        Serial.printf("\n c=%d : '%c'\n",c,c);
    }
    Serial.println("fin\n");

    Sms sms;

    //<stat>
    if (args[0] == GF("REC READ")) {
      sms.status = SmsStatus::REC_READ;
    } else if (args[0] == GF("REC UNREAD")) {
      sms.status = SmsStatus::REC_UNREAD;
    } else if (args[0] == GF("STO UNSENT")) {
      sms.status = SmsStatus::STO_UNSENT;
    } else if (args[0] == GF("STO SENT")) {
      sms.status = SmsStatus::STO_SENT;
    } else if (args[0] == GF("ALL")) {
      sms.status = SmsStatus::ALL;
    } else {
      thisModem().stream.readString();
      return {};
    }

    // <oa>
    thisModem().streamSkipUntil('"');
    sms.originatingAddress = thisModem().stream.readStringUntil('"');

    // <alpha>
    thisModem().streamSkipUntil('"');
    sms.phoneBookEntry = thisModem().stream.readStringUntil('"');

    // <scts>
    thisModem().streamSkipUntil('"');
    sms.serviceCentreTimeStamp = thisModem().stream.readStringUntil('"');
    thisModem().streamSkipUntil(',');

    thisModem().streamSkipUntil(','); // <tooa>
    thisModem().streamSkipUntil(','); // <fo>
    thisModem().streamSkipUntil(','); // <pid>

    // <dcs>
    const uint8_t alphabet = (thisModem().streamGetIntBefore(',') >> 2) & B11;
    switch (alphabet) {
    case B00:
      sms.alphabet = SmsAlphabet::GSM_7bit;
      break;
    case B01:
      sms.alphabet = SmsAlphabet::Data_8bit;
      break;
    case B10:
      sms.alphabet = SmsAlphabet::UCS2;
      break;
    case B11:
    default:
      sms.alphabet = SmsAlphabet::Reserved;
      break;
    }

    thisModem().streamSkipUntil(','); // <sca>
    thisModem().streamSkipUntil(','); // <tosca>

    // <length>, CR, LF
    const long length = thisModem().streamGetIntBefore('\n');

    data.remove(static_cast<const unsigned int>(length));
    switch (sms.alphabet) {
    case SmsAlphabet::GSM_7bit:
      sms.message = data;
      break;
    case SmsAlphabet::Data_8bit:
      sms.message = TinyGsmDecodeHex8bit(data);
      break;
    case SmsAlphabet::UCS2:
      sms.message = TinyGsmDecodeHex16bit(data);
      break;
    case SmsAlphabet::Reserved:
      return {};
    }

    return sms;
  }

  // Supported by: SIM800 other is not tested
  MessageStorage getPreferredSMSStorageImpl() {
    thisModem().sendAT(GF("+CPMS?")); // Preferred SMS Message Storage
    if (thisModem().waitResponse(GF("+CPMS:")) != 1) {
      thisModem().stream.readString();
      return {};
    }

    // AT reply:
    // +CPMS: <mem1>,<used1>,<total1>,<mem2>,<used2>,<total2>,<mem3>,<used3>,<total3>

    MessageStorage messageStorage;
    for (uint8_t i = 0; i < 3; ++i) {
      // type
      thisModem().streamSkipUntil('"');
      const String mem = thisModem().stream.readStringUntil('"');
      if (mem == GF("SM")) {
        messageStorage.type[i] = MessageStorageType::SIM;
      } else if (mem == GF("ME")) {
        messageStorage.type[i] = MessageStorageType::Phone;
      } else if (mem == GF("SM_P")) {
        messageStorage.type[i] = MessageStorageType::SIMPreferred;
      } else if (mem == GF("ME_P")) {
        messageStorage.type[i] = MessageStorageType::PhonePreferred;
      } else if (mem == GF("MT")) {
        messageStorage.type[i] = MessageStorageType::Either_SIMPreferred;
      } else {
        thisModem().stream.readString();
        return {};
      }

      // used
      thisModem().streamSkipUntil(',');
      messageStorage.used[i] = static_cast<uint8_t>(thisModem().streamGetIntBefore(','));

      // total
      if (i < 2) {
        messageStorage.total[i] = static_cast<uint8_t>(thisModem().streamGetIntBefore(','));
      } else {
        messageStorage.total[i] = static_cast<uint8_t>(thisModem().streamGetIntBefore());
      }
    }

    return messageStorage;
  }

  // Supported by: SIM800 other is not tested
  bool setPreferredSMSStorageImpl(const MessageStorageType type[3]) {
    const auto convertMstToString = [](const MessageStorageType &type) {
      switch (type) {
      case MessageStorageType::SIM:
        return GF("\"SM\"");
      case MessageStorageType::Phone:
        return GF("\"ME\"");
      case MessageStorageType::SIMPreferred:
        return GF("\"SM_P\"");
      case MessageStorageType::PhonePreferred:
        return GF("\"ME_P\"");
      case MessageStorageType::Either_SIMPreferred:
        return GF("\"MT\"");
      }

      return GF("");
    };

    thisModem().sendAT(GF("+CPMS="),
           convertMstToString(type[0]), GF(","),
           convertMstToString(type[1]), GF(","),
           convertMstToString(type[2]));

    return thisModem().waitResponse() == 1;
  }

  // Supported by: SIM800 other is not tested
  bool deleteSMSImpl(const uint8_t index) {
    thisModem().sendAT(GF("+CMGD="), index, GF(","), 0); // Delete SMS Message from <mem1> location
    return thisModem().waitResponse(5000L) == 1;
  }

  // Supported by: SIM800 other is not tested
  bool deleteAllSMSImpl(const DeleteAllSmsMethod method) {
    // Select SMS Message Format: PDU mode. Spares us space now
    thisModem().sendAT(GF("+CMGF=0"));
    if (thisModem().waitResponse() != 1) {
        return false;
    }

    thisModem().sendAT(GF("+CMGDA="), static_cast<const uint8_t>(method)); // Delete All SMS
    const bool ok = thisModem().waitResponse(25000L) == 1;

    thisModem().sendAT(GF("+CMGF=1"));
    thisModem().waitResponse();

    return ok;
  }

  // Supported by: SIM800 other is not tested
  bool receiveNewSMSIndicationImpl(const bool enabled = true, const bool cbmIndication = false, const bool statusReport = false) {
    thisModem().sendAT(GF("+CNMI=2,"),           // New SMS Message Indications
           enabled, GF(","),         // format: +CMTI: <mem>,<index>
           cbmIndication, GF(","),   // format: +CBM: <sn>,<mid>,<dcs>,<page>,<pages><CR><LF><data>
           statusReport, GF(",0"));  // format: +CDS: <fo>,<mr>[,<ra>][,<tora>],<scts>,<dt>,<st>

    return thisModem().waitResponse() == 1;
  }
};

#endif  // SRC_TINYGSMSMS_H_
