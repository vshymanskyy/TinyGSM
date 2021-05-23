/**
 * @file       TinyGsmPhoneBook.tpp
 * @author     Bouteville Pierre-Noel
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGPHONEBOOK_H_
#define SRC_TINYGPHONEBOOK_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_PHONEBOOK

#ifndef TINY_GSM_PHONEBOOK_RESULTS
  #define TINY_GSM_PHONEBOOK_RESULTS 5
#endif

template <class modemType>
class TinyGsmPhoneBook {
 public:
  /*
   * Phonebook functions
   */
  PhonebookStorage getPhonebookStorage() {
    return thisModem().getPhonebookStorageImpl(code);
  }

  bool setPhonebookStorage(const PhonebookStorageType type) {
    return thisModem().setPhonebookStorageImpl(type);
  }

  bool addPhonebookEntry(const String &number, const String &text) {
    return this.thisModem().addPhonebookEntryImpl(const String &number, const String &text);
  }

  bool deletePhonebookEntry(const uint8_t index) {
    return this.thisModem().deletePhonebookEntryImpl(index);
  }

  PhonebookEntry readPhonebookEntry(const uint8_t index) {
    return this.thisModem().readPhonebookEntryImpl(index);
  }

  PhonebookMatches findPhonebookEntries(const String &needle) {
    return this.thisModem().findPhonebookEntriesImpl(needle);
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
   * Phone Book functions
   * All phone book function is come from SIM800 and imported from 
   * https://github.com/szotsaki/TinyGSM
   * I suppose that work on all other GSM.
   */
 protected:
  // Supported by: SIM800 other is not tested
  PhonebookStorage getPhonebookStorageImpl() {
    sendAT(GF("+CPBS?")); // Phonebook Memory Storage
    if (waitResponse(GF(GSM_NL "+CPBS: \"")) != 1) {
      stream.readString();
      return {};
    }

    // AT reply:
    // +CPBS: <storage>,<used>,<total>

    PhonebookStorage phonebookStorage;

    const String mem = stream.readStringUntil('"');
    if (mem == GF("SM")) {
      phonebookStorage.type = PhonebookStorageType::SIM;
    } else if (mem == GF("ME")) {
      phonebookStorage.type = PhonebookStorageType::Phone;
    } else {
      stream.readString();
      return {};
    }

    // used, total
    streamSkipUntil(',');
    phonebookStorage.used = static_cast<uint8_t>(stream.readStringUntil(',').toInt());
    phonebookStorage.total = static_cast<uint8_t>(stream.readString().toInt());

    return phonebookStorage;
  }

  // Supported by: SIM800 other is not tested
  bool setPhonebookStorageImpl(const PhonebookStorageType type) {
    if (type == PhonebookStorageType::Invalid) {
      return false;
    }

    const auto storage = type == PhonebookStorageType::SIM ? GF("\"SM\"") : GF("\"ME\"");
    sendAT(GF("+CPBS="), storage); // Phonebook Memory Storage

    return waitResponse() == 1;
  }

  // Supported by: SIM800 other is not tested
  bool addPhonebookEntryImpl(const String &number, const String &text) {
    // Always use international phone number style (+12345678910).
    // Never use double quotes or backslashes in `text`, not even in escaped form.
    // Use characters found in the GSM alphabet.

    // Typical maximum length of `number`: 38
    // Typical maximum length of `text`:   14

    changeCharacterSet(GF("GSM"));

    // AT format:
    // AT+CPBW=<index>[,<number>,[<type>,[<text>]]]
    sendAT(GF("+CPBW=,\""), number, GF("\",145,\""), text, '"');  // Write Phonebook Entry

    return waitResponse(3000L) == 1;
  }

  // Supported by: SIM800 other is not tested
  bool deletePhonebookEntryImpl(const uint8_t index) {
    // AT+CPBW=<index>
    sendAT(GF("+CPBW="), index); // Write Phonebook Entry

    // Returns OK even if an empty index is deleted in the valid range
    return waitResponse(3000L) == 1;
  }

  // Supported by: SIM800 other is not tested
  PhonebookEntry readPhonebookEntryImpl(const uint8_t index) {
    changeCharacterSet(GF("GSM"));
    sendAT(GF("+CPBR="), index); // Read Current Phonebook Entries

    // AT response:
    // +CPBR:<index1>,<number>,<type>,<text>
    if (waitResponse(3000L, GF(GSM_NL "+CPBR: ")) != 1) {
      stream.readString();
      return {};
    }

    PhonebookEntry phonebookEntry;
    streamSkipUntil('"');
    phonebookEntry.number = stream.readStringUntil('"');
    streamSkipUntil('"');
    phonebookEntry.text = stream.readStringUntil('"');

    waitResponse();

    return phonebookEntry;
  }

  // Supported by: SIM800 other is not tested
  PhonebookMatches findPhonebookEntriesImpl(const String &needle) {
    // Search among the `text` entries only.
    // Only the first TINY_GSM_PHONEBOOK_RESULTS indices are returned.
    // Make your query more specific if you have more results than that.
    // Use characters found in the GSM alphabet.

    changeCharacterSet(GF("GSM"));
    sendAT(GF("+CPBF=\""), needle, '"'); // Find Phonebook Entries

    // AT response:
    // [+CPBF:<index1>,<number>,<type>,<text>]
    // [[...]<CR><LF>+CBPF:<index2>,<number>,<type>,<text>]
    if (waitResponse(30000L, GF(GSM_NL "+CPBF: ")) != 1) {
      stream.readString();
      return {};
    }

    PhonebookMatches matches;
    for (uint8_t i = 0; i < TINY_GSM_PHONEBOOK_RESULTS; ++i) {
      matches.index[i] = static_cast<uint8_t>(stream.readStringUntil(',').toInt());
      if (waitResponse(GF(GSM_NL "+CPBF: ")) != 1) {
        break;
      }
    }

    waitResponse();

    return matches;
  }
};

#endif  // SRC_TINYGSMPHONEBOOK_H_
