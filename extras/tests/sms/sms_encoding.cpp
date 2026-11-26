// Host regression for the production SMS mixin. No modem or network is used.
#include <assert.h>
#include <stdio.h>
#include <mixins/TinyGsmSMS.tpp>

extern "C" void          delay(unsigned long) {}
extern "C" unsigned long millis() {
  return 0;
}

class Capture : public Print {
 public:
  String bytes;
  size_t write(uint8_t c) override {
    bytes += static_cast<char>(c);
    return 1;
  }
  void flush() {}
};

class Modem : public TinyGsmSMS<Modem> {
 public:
  Capture stream;
  Capture commands;
  int     coding  = 0;
  bool    unicode = false;
  bool    prompt  = true;

  template <typename... Args>
  void sendAT(Args... args) {
    Capture command;
    append(command, args...);
    commands.print(command.bytes);
    commands.print('\n');
    if (command.bytes == "+CSMP=17,167,0,8") coding = 8;
    if (command.bytes == "+CSMP=17,167,0,0") coding = 0;
    if (command.bytes == "+CSCS=\"HEX\"") unicode = true;
    if (command.bytes == "+CSCS=\"GSM\"") unicode = false;
    if (command.bytes.startsWith("+CMGS=")) stream.bytes = "";
  }
  int waitResponse() {
    return 1;
  }
  int waitResponse(const char*) {
    return prompt ? 1 : 0;
  }
  int waitResponse(long) {
    // The modem's retained DCS=8 selects UCS-2, with its 70-character limit.
    return !unicode && coding == 8 && stream.bytes.length() > 71 ? 0 : 1;
  }

 private:
  void append(Capture&) {}
  template <typename Head, typename... Tail>
  void append(Capture& output, Head head, Tail... tail) {
    output.print(head);
    append(output, tail...);
  }
};

int main() {
  const uint16_t unicode[] = {0x03A9};
  const char*    longMessage =
      "01234567890123456789012345678901234567890123456789"
      "01234567890123456789012345678901234567890123456789";
  Modem modem;
  assert(modem.sendSMS("12345", longMessage));
  assert(modem.coding == 0);
  assert(modem.sendSMS_UTF16("12345", unicode, 1));
  assert(modem.coding == 8);
  assert(modem.stream.bytes == String("03A9") + static_cast<char>(0x1a));
  const bool sent = modem.sendSMS("12345", longMessage);
  if (!sent || modem.coding != 0) {
    fprintf(stderr, "GSM send after UTF-16: sent=%d, DCS=%d (expected 1,0)\n",
            sent, modem.coding);
    return 1;
  }
  assert(modem.stream.bytes == String(longMessage) + static_cast<char>(0x1a));
  assert(modem.sendSMS_UTF16("12345", unicode, 1));
  assert(modem.coding == 8);
  assert(modem.sendSMS("12345", "short"));
  assert(modem.coding == 0);
  modem.prompt = false;
  assert(!modem.sendSMS("12345", "not sent"));
  assert(modem.stream.bytes.length() == 0);
  puts("SMS encoding transition and rejected-prompt checks passed");
}
