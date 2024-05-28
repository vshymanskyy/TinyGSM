/**
 * @file       TinyGsmClientXBee.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy, XBee module by Sara
 * Damiano
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCLIENTXBEE_H_
#define SRC_TINYGSMCLIENTXBEE_H_
// #pragma message("TinyGSM:  TinyGsmClientXBee")

// #define TINY_GSM_DEBUG Serial

// XBee's do not support multi-plexing in transparent/command mode
// The much more complicated API mode is needed for multi-plexing
#define TINY_GSM_MUX_COUNT 1
#define TINY_GSM_NO_MODEM_BUFFER
// XBee's have a default guard time of 1 second (1000ms, 10 extra for safety
// here)
#define TINY_GSM_XBEE_GUARD_TIME 1010

#ifdef AT_NL
#undef AT_NL
#endif
#define AT_NL "\r"

#ifdef MODEM_MANUFACTURER
#undef MODEM_MANUFACTURER
#endif
#define MODEM_MANUFACTURER "Digi"

#ifdef MODEM_MODEL
#undef MODEM_MODEL
#endif
#define MODEM_MODEL "XBee"

#include "TinyGsmModem.tpp"
#include "TinyGsmWifi.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmTemperature.tpp"
#include "TinyGsmBattery.tpp"

// Use this to avoid too many entrances and exits from command mode.
// The cellular Bee's often freeze up and won't respond when attempting
// to enter command mode too many times.
#define XBEE_COMMAND_START_DECORATOR(nAttempts, failureReturn)                \
  bool wasInCommandMode = inCommandMode;                                      \
  if (!wasInCommandMode) { /* don't re-enter command mode if already in it */ \
    if (!commandMode(nAttempts))                                              \
      return failureReturn; /* Return immediately if fails */                 \
  }
#define XBEE_COMMAND_END_DECORATOR                                       \
  if (!wasInCommandMode) { /* only exit if we weren't in command mode */ \
    exitCommand();                                                       \
  }

enum XBeeRegStatus {
  REG_OK           = 0,
  REG_UNREGISTERED = 1,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_UNKNOWN      = 4,
};

// These are responses to the HS command to get "hardware series"
enum XBeeType {
  XBEE_UNKNOWN   = 0,
  XBEE_S6B_WIFI  = 0x601,  // Digi XBee Wi-Fi
  XBEE_LTE1_VZN  = 0xB01,  // Digi XBee Cellular LTE Cat 1
  XBEE_3G        = 0xB02,  // Digi XBee Cellular 3G
  XBEE3_LTE1_ATT = 0xB06,  // Digi XBee3 Cellular LTE CAT 1
  XBEE3_LTEM_ATT = 0xB08,  // Digi XBee3 Cellular LTE-M
};

class TinyGsmXBee : public TinyGsmModem<TinyGsmXBee>,
                    public TinyGsmGPRS<TinyGsmXBee>,
                    public TinyGsmWifi<TinyGsmXBee>,
                    public TinyGsmTCP<TinyGsmXBee, TINY_GSM_MUX_COUNT>,
                    public TinyGsmSSL<TinyGsmXBee, TINY_GSM_MUX_COUNT>,
                    public TinyGsmSMS<TinyGsmXBee>,
                    public TinyGsmBattery<TinyGsmXBee>,
                    public TinyGsmTemperature<TinyGsmXBee> {
  friend class TinyGsmModem<TinyGsmXBee>;
  friend class TinyGsmGPRS<TinyGsmXBee>;
  friend class TinyGsmWifi<TinyGsmXBee>;
  friend class TinyGsmTCP<TinyGsmXBee, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmXBee, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSMS<TinyGsmXBee>;
  friend class TinyGsmBattery<TinyGsmXBee>;
  friend class TinyGsmTemperature<TinyGsmXBee>;

  /*
   * Inner Client
   */
 public:
  class GsmClientXBee : public GsmClient {
    friend class TinyGsmXBee;

   public:
    GsmClientXBee() {}

    explicit GsmClientXBee(TinyGsmXBee& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmXBee* modem, uint8_t = 0) {
      this->at       = modem;
      this->mux      = 0;
      sock_connected = false;

      at->sockets[0] = this;

      return true;
    }

   public:
    // NOTE:  The XBee saves all connection information (ssid/pwd etc) except
    // IP address and port number, in flash (NVM).
    // The NVM is be updated only when it is initialized.
    // The TCP connection itself is not opened until you attempt to send data.
    // Because all settings are saved to flash, it is possible (or likely) that
    // you could send data even if you haven't "made" any connection.
    virtual int connect(const char* host, uint16_t port, int timeout_s) {
      // NOTE:  Not caling stop() or yeild() here
      at->streamClear();  // Empty anything in the buffer before starting
      sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
      return sock_connected;
    }
    int connect(const char* host, uint16_t port) override {
      return connect(host, port, 75);
    }

    virtual int connect(IPAddress ip, uint16_t port, int timeout_s) {
      if (timeout_s != 0) {
        DBG("Timeout [", timeout_s, "] doesn't apply here.");
      }
      // NOTE:  Not caling stop() or yeild() here
      at->streamClear();  // Empty anything in the buffer before starting
      sock_connected = at->modemConnect(ip, port, mux, false);
      return sock_connected;
    }
    int connect(IPAddress ip, uint16_t port) override {
      return connect(ip, port, 0);
    }

    void stop(uint32_t maxWaitMs) {
      at->streamClear();  // Empty anything in the buffer
      // empty the saved currently-in-use destination address
      at->modemStop(maxWaitMs);
      at->streamClear();  // Empty anything in the buffer
      sock_connected = false;

      // Note:  because settings are saved in flash, the XBEE will attempt to
      // reconnect to the previous socket if it receives any outgoing data.
      // Setting sock_connected to false after the stop ensures that connected()
      // will return false after a stop has been ordered.  This makes it play
      // much more nicely with libraries like PubSubClient.
    }
    void stop() override {
      stop(5000L);
    }

    size_t write(const uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      return at->modemSend(buf, size, mux);
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
      return at->stream.available();
      /*
      if (!rx.size() || at->stream.available()) {
        at->maintain();
      }
      return at->stream.available() + rx.size();
      */
    }

    int read(uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      return at->stream.readBytes(reinterpret_cast<char*>(buf), size);
      /*
      size_t cnt = 0;
      uint32_t _startMillis = millis();
      while (cnt < size && millis() - _startMillis < _timeout) {
        size_t chunk = TinyGsmMin(size-cnt, rx.size());
        if (chunk > 0) {
          rx.get(buf, chunk);
          buf += chunk;
          cnt += chunk;
          continue;
        }
        // TODO(vshymanskyy): Read directly into user buffer?
        if (!rx.size() || at->stream.available()) {
          at->maintain();
        }
      }
      return cnt;
      */
    }

    int read() override {
      TINY_GSM_YIELD();
      return at->stream.read();
      /*
      uint8_t c;
      if (read(&c, 1) == 1) {
        return c;
      }
      return -1;
      */
    }

    int peek() override {
      return at->stream.peek();
    }
    void flush() override {
      at->stream.flush();
    }

    uint8_t connected() override {
      if (available()) {
        return true;
        // if we never got an IP, it can't be connected
      } else if (at->savedIP == IPAddress(0, 0, 0, 0)) {
        return false;
      }
      return sock_connected;
      // NOTE:  We don't check or return
      // modemGetConnected() because we don't
      // want to go into command mode.
      // return at->modemGetConnected();
    }
    operator bool() override {
      return connected();
    }

    /*
     * Extended API
     */

    String remoteIP() {
      IPAddress savedIP = at->savedIP;
      return TinyGsmStringFromIp(savedIP);
    }
  };

  /*
   * Inner Secure Client
   */
 public:
  class GsmClientSecureXBee : public GsmClientXBee {
   public:
    GsmClientSecureXBee() {}

    explicit GsmClientSecureXBee(TinyGsmXBee& modem, uint8_t mux = 0)
        : GsmClientXBee(modem, mux) {}

   public:
    int connect(const char* host, uint16_t port, int timeout_s) override {
      // NOTE:  Not caling stop() or yeild() here
      at->streamClear();  // Empty anything in the buffer before starting
      sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
      return sock_connected;
    }
    int connect(const char* host, uint16_t port) override {
      return connect(host, port, 75);
    }

    int connect(IPAddress ip, uint16_t port, int timeout_s) override {
      if (timeout_s != 0) {
        DBG("Timeout [", timeout_s, "] doesn't apply here.");
      }
      // NOTE:  Not caling stop() or yeild() here
      at->streamClear();  // Empty anything in the buffer before starting
      sock_connected = at->modemConnect(ip, port, mux, true);
      return sock_connected;
    }
    int connect(IPAddress ip, uint16_t port) override {
      return connect(ip, port, 0);
    }
  };

  /*
   * Constructor
   */
 public:
  explicit TinyGsmXBee(Stream& stream)
      : stream(stream),
        guardTime(TINY_GSM_XBEE_GUARD_TIME),
        beeType(XBEE_UNKNOWN),
        resetPin(-1),
        savedIP(IPAddress(0, 0, 0, 0)),
        savedHost(""),
        savedHostIP(IPAddress(0, 0, 0, 0)),
        savedOperatingIP(IPAddress(0, 0, 0, 0)),
        inCommandMode(false),
        lastCommandModeMillis(0) {
    // Start not knowing what kind of bee it is
    // Start with the default guard time of 1 second
    memset(sockets, 0, sizeof(sockets));
  }

  TinyGsmXBee(Stream& stream, int8_t resetPin)
      : stream(stream),
        guardTime(TINY_GSM_XBEE_GUARD_TIME),
        beeType(XBEE_UNKNOWN),
        resetPin(resetPin),
        savedIP(IPAddress(0, 0, 0, 0)),
        savedHost(""),
        savedHostIP(IPAddress(0, 0, 0, 0)),
        savedOperatingIP(IPAddress(0, 0, 0, 0)),
        inCommandMode(false),
        lastCommandModeMillis(0) {
    // Start not knowing what kind of bee it is
    // Start with the default guard time of 1 second
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = nullptr) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientXBee"));

    if (resetPin >= 0) {
      pinMode(resetPin, OUTPUT);
      digitalWrite(resetPin, HIGH);
    }

    XBEE_COMMAND_START_DECORATOR(10, false)

    bool changesMade = false;
    bool ret_val     = true;

    // if there's a pin, we need to re-write to flash each time
    if (pin && strlen(pin) > 0) {
      sendAT(GF("PN"), pin);
      if (waitResponse() != 1) {
        ret_val = false;
      } else {
        changesMade = true;
      }
    }

    // Put in transparent mode, if it isn't already
    changesMade |= changeSettingIfNeeded(GF("AP"), 0x0);

    // shorten the guard time to 100ms, if it was anything else
    sendAT(GF("GT"));
    if (readResponseInt() != 0x64) {
      sendAT(GF("GT"), 64);
      ret_val &= waitResponse() == 1;
      if (ret_val) {
        guardTime   = 110;
        changesMade = true;
      }
    } else {
      guardTime = 110;
    }

    // Make sure the command mode drop-out time is long enough that we won't
    // fall out of command mode without intentionally leaving it.  This is the
    // default drop out time of 0x64 x 100ms (10 seconds)
    changesMade |= changeSettingIfNeeded(GF("CT"), 0x64);

    if (changesMade) { ret_val &= writeChanges(); }

    getSeries();  // Get the "Hardware Series";

    XBEE_COMMAND_END_DECORATOR

    return ret_val;
  }

  String getModemNameImpl() {
    return getBeeName();
  }

  String getModemModelImpl() {
    switch (beeType) {
      case XBEE_S6B_WIFI: return "XBee Wi-Fi";
      case XBEE_LTE1_VZN: return "XBee Cellular LTE Cat 1";
      case XBEE_3G: return "XBee Cellular 3G";
      case XBEE3_LTE1_ATT: return "XBee3 Cellular LTE CAT 1";
      case XBEE3_LTEM_ATT: return "XBee3 Cellular LTE-M";
      default: return "XBee Unknown";
    }
  }
  // Gets the modem serial number
  String getModemSerialNumberImpl() {
    String xbeeSnLow =
        sendATGetString(GF("SL"));  // Request Module MAC/Serial Number Low
    String xbeeSnHigh =
        sendATGetString(GF("SH"));  // Request Module MAC/Serial Number High
    return xbeeSnLow + String(" ") + xbeeSnHigh;
  }

  // Gets the modem hardware version
  String getModemHardwareVersion() {
    return sendATGetString(GF("HV"));
  }

  // Gets the modem firmware version
  String getModemFirmwareVersion() {
    return sendATGetString(GF("VR"));
  }

  // Gets the modem combined version
  String getModemRevisionImpl() {
    String hw_ver = getModemHardwareVersion();
    String fw_ver = getModemFirmwareVersion();
    return hw_ver + String(" ") + fw_ver;
  }

  bool setBaudImpl(uint32_t baud) {
    XBEE_COMMAND_START_DECORATOR(5, false)
    bool changesMade = false;
    switch (baud) {
      case 2400: changesMade |= changeSettingIfNeeded(GF("BD"), 0x1); break;
      case 4800: changesMade |= changeSettingIfNeeded(GF("BD"), 0x2); break;
      case 9600: changesMade |= changeSettingIfNeeded(GF("BD"), 0x3); break;
      case 19200: changesMade |= changeSettingIfNeeded(GF("BD"), 0x4); break;
      case 38400: changesMade |= changeSettingIfNeeded(GF("BD"), 0x5); break;
      case 57600: changesMade |= changeSettingIfNeeded(GF("BD"), 0x6); break;
      case 115200: changesMade |= changeSettingIfNeeded(GF("BD"), 0x7); break;
      case 230400: changesMade |= changeSettingIfNeeded(GF("BD"), 0x8); break;
      case 460800: changesMade |= changeSettingIfNeeded(GF("BD"), 0x9); break;
      case 921600: changesMade |= changeSettingIfNeeded(GF("BD"), 0xA); break;
      default: {
        DBG(GF("Specified baud rate is unsupported! Setting to 9600 baud."));
        changesMade |= changeSettingIfNeeded(GF("BD"),
                                             0x3);  // Set to default of 9600
        break;
      }
    }
    if (changesMade) { writeChanges(); }
    XBEE_COMMAND_END_DECORATOR
    return true;
  }

  bool testATImpl(uint32_t timeout_ms = 10000L) {
    uint32_t start   = millis();
    bool     success = false;
    while (!success && millis() - start < timeout_ms) {
      if (!inCommandMode) {
        success = commandMode();
        if (success) exitCommand();
      } else {
        sendAT();
        if (waitResponse(200) == 1) {
          success = true;
        } else {
          // if we didn't respond to the AT, assume we're not in command mode
          inCommandMode = false;
        }
      }
      delay(250);
    }
    return success;
  }

  void maintainImpl() {
    // this only happens OUTSIDE command mode, so if we're getting characters
    // they should be data received from the TCP connection
    // TINY_GSM_YIELD();
    // if (!inCommandMode) {
    //   while (stream.available()) {
    //     char c = stream.read();
    //     if (c > 0) sockets[0]->rx.put(c);
    //   }
    // }
  }

  bool factoryDefaultImpl() {
    XBEE_COMMAND_START_DECORATOR(5, false)
    sendAT(GF("RE"));
    bool ret_val = waitResponse() == 1;
    ret_val &= writeChanges();
    XBEE_COMMAND_END_DECORATOR
    // Make sure the guard time for the modem object is set back to default
    // otherwise communication would fail after the reset
    guardTime = 1010;
    return ret_val;
  }

  String getModemInfoImpl() {
    return sendATGetString(GF("HS"));
  }

  /*
  bool thisHasSSL() {
    if (beeType == XBEE_S6B_WIFI)
      return false;
    else
      return true;
  }

  bool thisHasWifi() {
    if (beeType == XBEE_S6B_WIFI)
      return true;
    else
      return false;
  }

  bool thisHasGPRS() {
    if (beeType == XBEE_S6B_WIFI)
      return false;
    else
      return true;
  }
  */

 public:
  XBeeType getBeeType() {
    return beeType;
  }

  String getBeeName() {
    switch (beeType) {
      case XBEE_S6B_WIFI: return "Digi XBee Wi-Fi";
      case XBEE_LTE1_VZN: return "Digi XBee Cellular LTE Cat 1";
      case XBEE_3G: return "Digi XBee Cellular 3G";
      case XBEE3_LTE1_ATT: return "Digi XBee3 Cellular LTE CAT 1";
      case XBEE3_LTEM_ATT: return "Digi XBee3 Cellular LTE-M";
      default: return "Digi XBee Unknown";
    }
  }

  /*
   * Power functions
   */
 protected:
  // The XBee's have a bad habit of getting into an unresponsive funk
  // This uses the board's hardware reset pin to force it to reset
  void pinReset() {
    if (resetPin >= 0) {
      DBG("### Forcing a modem reset!\r\n");
      digitalWrite(resetPin, LOW);
      delay(1);
      digitalWrite(resetPin, HIGH);
    } else {
      DBG("### Attempting a modem software restart");
      restartImpl();
    }
  }

  bool restartImpl(const char* pin = nullptr) {
    if (!commandMode()) {
      DBG("### XBee not in command mode for restart; Exit");
      return false;
    }  // Return immediately

    if (beeType == XBEE_UNKNOWN) getSeries();  // how we restart depends on this

    if (beeType != XBEE_S6B_WIFI) {
      sendAT(GF("AM1"));  // Digi suggests putting cellular modules into
                          // airplane mode before restarting This allows the
                          // sockets and connections to close cleanly
      if (waitResponse() != 1) return exitAndFail();
      if (!writeChanges()) return exitAndFail();
    }

    sendAT(GF("FR"));
    if (waitResponse() != 1)
      return exitAndFail();
    else
      inCommandMode = false;  // Reset effectively exits command mode

    if (beeType == XBEE_S6B_WIFI)
      delay(2000);  // Wifi module actually resets about 2 seconds later
    else
      delay(100);  // cellular modules wait 100ms before reset happens

    // Wait until reboot completes and XBee responds to command mode call again
    for (uint32_t start = millis(); millis() - start < 60000L;) {
      if (commandMode(1)) break;
      delay(250);  // wait a litle before trying again
    }

    if (beeType != XBEE_S6B_WIFI) {
      sendAT(GF("AM0"));  // Turn off airplane mode
      if (waitResponse() != 1) return exitAndFail();
      if (!writeChanges()) return exitAndFail();
    }

    exitCommand();

    return init(pin);
  }

  void setupPinSleep(bool maintainAssociation = false) {
    XBEE_COMMAND_START_DECORATOR(5, )

    if (beeType == XBEE_UNKNOWN) getSeries();  // Command depends on series

    bool changesMade = false;

    // Pin sleep
    changesMade |= changeSettingIfNeeded(GF("SM"), 0x1);

    if (beeType == XBEE_S6B_WIFI && !maintainAssociation) {
      // For lowest power, dissassociated deep sleep
      changesMade |= changeSettingIfNeeded(GF("SO"), 0x200);
    } else if (!maintainAssociation) {
      // For supported cellular modules, maintain association
      // Not supported by all modules, will return "ERROR"
      changesMade |= changeSettingIfNeeded(GF("SO"), 0x1);
    }

    if (changesMade) { writeChanges(); }
    XBEE_COMMAND_END_DECORATOR
  }

  bool
  powerOffImpl() {  // NOTE:  Not supported for WiFi or older cellular firmware
    XBEE_COMMAND_START_DECORATOR(5, false)
    sendAT(GF("SD"));
    bool ret_val = waitResponse(120000L) == 1;
    // make sure we're really shut down
    if (ret_val) { ret_val &= (sendATGetString(GF("AI")) == "2D"); }
    XBEE_COMMAND_END_DECORATOR
    return ret_val;
  }

  // Enable airplane mode
  bool radioOffImpl() {
    bool success     = true;
    bool changesMade = false;
    XBEE_COMMAND_START_DECORATOR(5, false)
    changesMade = changeSettingIfNeeded(GF("AM"), 0x1, 5000L);
    if (changesMade) { success = writeChanges(); }
    XBEE_COMMAND_END_DECORATOR
    return success;
  }

  bool sleepEnableImpl(bool enable = true) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false)
      TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 public:
  XBeeRegStatus getRegistrationStatus() {
    XBEE_COMMAND_START_DECORATOR(5, REG_UNKNOWN)

    if (!inCommandMode) return REG_UNKNOWN;  // Return immediately

    if (beeType == XBEE_UNKNOWN)
      getSeries();  // Need to know the bee type to interpret response

    sendAT(GF("AI"));
    int16_t       intRes = readResponseInt(10000L);
    XBeeRegStatus stat   = REG_UNKNOWN;

    switch (beeType) {
      case XBEE_S6B_WIFI: {
        switch (intRes) {
          case 0x00:  // 0x00 Successfully joined an access point, established
                      // IP addresses and IP listening sockets
            stat = REG_OK;
            break;
          case 0x01:  // 0x01 Wi-Fi transceiver initialization in progress.
          case 0x02:  // 0x02 Wi-Fi transceiver initialized, but not yet
                      // scanning for access point.
          case 0x40:  // 0x40 Waiting for WPA or WPA2 Authentication.
          case 0x41:  // 0x41 Device joined a network and is waiting for IP
                      // configuration to complete
          case 0x42:  // 0x42 Device is joined, IP is configured, and listening
                      // sockets are being set up.
          case 0xFF:  // 0xFF Device is currently scanning for the configured
                      // SSID.
            stat = REG_SEARCHING;
            break;
          case 0x13:    // 0x13 Disconnecting from access point.
            restart();  // Restart the device; the S6B tends to get stuck
                        // "disconnecting"
            stat = REG_UNREGISTERED;
            break;
          case 0x23:  // 0x23 SSID not configured.
            stat = REG_UNREGISTERED;
            break;
          case 0x24:  // 0x24 Encryption key invalid (either NULL or invalid
                      // length for WEP).
          case 0x27:  // 0x27 SSID was found, but join failed.
            stat = REG_DENIED;
            break;
          default: stat = REG_UNKNOWN; break;
        }
        break;
      }
      default: {  // Cellular XBee's
        switch (intRes) {
          case 0x00:  // 0x00 Connected to the Internet.
            stat = REG_OK;
            break;
          case 0x22:  // 0x22 Registering to cellular network.
          case 0x23:  // 0x23 Connecting to the Internet.
          case 0xFF:  // 0xFF Initializing.
            stat = REG_SEARCHING;
            break;
          case 0x24:  // 0x24 The cellular component is missing, corrupt, or
                      // otherwise in error.
          case 0x2B:  // 0x2B USB Direct active.
          case 0x2C:  // 0x2C Cellular component is in PSM (power save mode).
            stat = REG_UNKNOWN;
            break;
          case 0x25:  // 0x25 Cellular network registration denied.
            stat = REG_DENIED;
            break;
          case 0x2A:            // 0x2A Airplane mode.
            sendAT(GF("AM0"));  // Turn off airplane mode
            waitResponse();
            writeChanges();
            stat = REG_UNKNOWN;
            break;
          case 0x2F:            // 0x2F Bypass mode active.
            sendAT(GF("AP0"));  // Set back to transparent mode
            waitResponse();
            writeChanges();
            stat = REG_UNKNOWN;
            break;
          default: stat = REG_UNKNOWN; break;
        }
        break;
      }
    }

    XBEE_COMMAND_END_DECORATOR
    return stat;
  }

 protected:
  int8_t getSignalQualityImpl() {
    XBEE_COMMAND_START_DECORATOR(5, 0);

    if (beeType == XBEE_UNKNOWN)
      getSeries();  // Need to know what type of bee so we know how to ask

    if (beeType == XBEE_S6B_WIFI)
      sendAT(GF("LM"));  // ask for the "link margin" - the dB above sensitivity
    else
      sendAT(GF("DB"));  // ask for the cell strength in dBm
    int16_t intRes = readResponseInt();

    XBEE_COMMAND_END_DECORATOR

    if (beeType == XBEE3_LTEM_ATT && intRes == 105)
      intRes = 0;  // tends to reply with "69" when signal is unknown

    if (beeType == XBEE_S6B_WIFI) {
      if (intRes == 0xFF) {
        return 0;  // 0xFF returned for unknown
      } else {
        return -93 + intRes;  // the maximum sensitivity is -93dBm
      }
    } else {
      return -1 * intRes;  // need to convert to negative number
    }
  }

  bool isNetworkConnectedImpl() {
    // first check for association indicator
    XBeeRegStatus s = getRegistrationStatus();
    if (s == REG_OK) {
      if (beeType == XBEE_S6B_WIFI) {
        // For wifi bees, if the association indicator is ok, check that a both
        // a local IP and DNS have been allocated
        IPAddress ip  = localIP();
        IPAddress dns = getDNSAddress();
        if (ip != IPAddress(0, 0, 0, 0) && dns != IPAddress(0, 0, 0, 0)) {
          return true;
        } else {
          return false;
        }
      } else {
        return true;
      }
    } else {
      return false;
    }
  }

  bool waitForNetworkImpl(uint32_t timeout_ms   = 60000L,
                          bool     check_signal = false) {
    bool retVal = false;
    XBEE_COMMAND_START_DECORATOR(5, false)
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      if (check_signal) { getSignalQuality(); }
      if (isNetworkConnected()) {
        retVal = true;
        break;
      }
      delay(250);  // per Neil H. - more stable with delay
    }
    XBEE_COMMAND_END_DECORATOR
    return retVal;
  }

  String getLocalIPImpl() {
    XBEE_COMMAND_START_DECORATOR(5, "")
    sendAT(GF("MY"));
    String IPaddr;
    IPaddr.reserve(16);
    // wait for the response - this response can be very slow
    IPaddr = readResponseString(30000);
    XBEE_COMMAND_END_DECORATOR
    IPaddr.trim();
    return IPaddr;
  }

  String getDNS() {
    XBEE_COMMAND_START_DECORATOR(5, "")
    switch (beeType) {
      case XBEE_S6B_WIFI: {
        sendAT(GF("NS"));
        break;
      }
      default: {
        sendAT(GF("N1"));
        break;
      }
    }
    String DNSaddr;
    DNSaddr.reserve(16);
    // wait for the response - this response can be very slow
    DNSaddr = readResponseString(30000);
    XBEE_COMMAND_END_DECORATOR
    DNSaddr.trim();
    return DNSaddr;
  }

  IPAddress getDNSAddress() {
    return TinyGsmIpFromString(getDNS());
  }

  /*
   * Secure socket layer (SSL) functions
   */
  // Follows functions as inherited from TinyGsmSSL.tpp

  /*
   * WiFi functions
   */
 protected:
  bool networkConnectImpl(const char* ssid, const char* pwd) {
    bool changesMade = false;
    bool retVal      = true;

    XBEE_COMMAND_START_DECORATOR(5, false)

    if (ssid == nullptr) retVal = false;

    changesMade |= changeSettingIfNeeded(GF("ID"), ssid);

    if (pwd && strlen(pwd) > 0) {
      // Set security to WPA2
      changesMade |= changeSettingIfNeeded(GF("EE"), 0x2);
      // set the password
      // the wifi bee will NOT return the previously set password,
      // so we have no way of knowing if the passwords has changed
      // and must re-write to flash each time
      sendAT(GF("PK"), pwd);
      if (waitResponse() != 1) {
        retVal = false;
      } else {
        changesMade = true;
      }
    } else {
      changesMade |= changeSettingIfNeeded(GF("EE"), 0x0);  // Set No security
    }

    if (changesMade) { retVal &= writeChanges(); }

    XBEE_COMMAND_END_DECORATOR

    return retVal;
  }

  bool networkDisconnectImpl() {
    XBEE_COMMAND_START_DECORATOR(5, false)
    sendAT(GF("NR0"));  // Do a network reset in order to disconnect
    // WARNING:  On wifi modules, using a network reset will not
    // allow the same ssid to re-join without rebooting the module.
    int8_t res = (1 == waitResponse(5000));
    writeChanges();
    XBEE_COMMAND_END_DECORATOR
    return res;
  }

  /*
   * GPRS functions
   */
 protected:
  bool gprsConnectImpl(const char* apn, const char* user = nullptr,
                       const char* pwd = nullptr) {
    bool success     = true;
    bool changesMade = false;
    XBEE_COMMAND_START_DECORATOR(5, false)

    // the cellular bees will NOT return the previously set username or
    // password, so we have no way of knowing if they have changed
    // and must re-write to flash each time
    if (user && strlen(user) > 0) {
      sendAT(GF("CU"), user);  // Set the user for the APN
      if (waitResponse() != 1) {
        success = false;
      } else {
        changesMade = true;
      }
    }
    if (pwd && strlen(pwd) > 0) {
      sendAT(GF("CW"), pwd);  // Set the password for the APN
      if (waitResponse() != 1) {
        success = false;
      } else {
        changesMade = true;
      }
    }
    changesMade |= changeSettingIfNeeded(GF("AN"), String(apn));  // Set the APN

    changesMade |= changeSettingIfNeeded(GF("AM"), 0x0,
                                         5000L);  // Airplane mode off

    if (changesMade) { success = writeChanges(); }
    XBEE_COMMAND_END_DECORATOR
    return success;
  }

  bool gprsDisconnectImpl() {
    bool success = true;
    XBEE_COMMAND_START_DECORATOR(5, false)
    // Cheating and disconnecting by turning on airplane mode
    bool changesMade = changeSettingIfNeeded(GF("AM"), 0x1, 5000L);

    if (changesMade) { success = writeChanges(); }
    XBEE_COMMAND_END_DECORATOR
    return success;
  }

  bool isGprsConnectedImpl() {
    return isNetworkConnected();
  }

  String getOperatorImpl() {
    return sendATGetString(GF("MN"));
  }

  /*
   * SIM card functions
   */
 protected:
  bool simUnlockImpl(const char* pin) {
    if (pin && strlen(pin) > 0) {
      sendAT(GF("PN"), pin);
      return waitResponse() == 1;
    }
    return false;
  }

  String getSimCCIDImpl() {
    return sendATGetString(GF("S#"));
  }

  String getIMEIImpl() {
    return sendATGetString(GF("IM"));
  }

  String getIMSIImpl() {
    return sendATGetString(GF("II"));
  }

  SimStatus getSimStatusImpl(uint32_t) {
    return SIM_READY;  // unsupported
  }

  /*
   * Phone Call functions
   */
  // No functions of this type supported

  /*
   * Audio functions
   */
  // No functions of this type supported

  /*
   * Text messaging (SMS) functions
   */
 protected:
  String sendUSSDImpl(const String& code) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool sendSMSImpl(const String& number, const String& text) {
    bool changesMade = false;
    if (!commandMode()) { return false; }  // Return immediately

    sendAT(GF("IP"));  // check mode
    if (readResponseInt() != 2) {
      sendAT(GF("IP"), 2);  // Put in text messaging mode
      if (waitResponse() != 1) {
        return exitAndFail();
      } else {
        changesMade = true;
      }
    }

    sendAT(GF("PH"));  // check last number
    if (readResponseString() != String(number)) {
      sendAT(GF("PH"), number);  // Set the phone number
      if (waitResponse() != 1) {
        return exitAndFail();
      } else {
        changesMade = true;
      }
    }

    sendAT(GF("TD"));  // check the text delimiter
    if (readResponseString() != String("D")) {
      sendAT(GF("TDD"));  // Set the text delimiter to the standard 0x0D
                          //(carriage return)
      if (waitResponse() != 1) {
        return exitAndFail();
      } else {
        changesMade = true;
      }
    }

    if (changesMade) {
      if (!writeChanges()) return exitAndFail();
    }
    // Get out of command mode to actually send the text
    exitCommand();

    streamWrite(text);
    stream.write(
        static_cast<char>(0x0D));  // close off with the carriage return

    return true;
  }

  /*
   * GSM Location functions
   */
  // No functions of this type supported

  /*
   * GPS/GNSS/GLONASS location functions
   */
  // No functions of this type supported

  /*
   * Time functions
   */
  // No functions of this type supported

  /*
   * NTP server functions
   */
  // No functions of this type supported

  /*
   * BLE functions
   */
  // No functions of this type supported

  /*
   * Battery functions
   */
 protected:
  // Use: float vBatt = modem.getBattVoltage() / 1000.0;
  int16_t getBattVoltageImpl() {
    int16_t intRes = 0;
    XBEE_COMMAND_START_DECORATOR(5, false)
    if (beeType == XBEE_UNKNOWN) getSeries();
    if (beeType == XBEE_S6B_WIFI) {
      sendAT(GF("%V"));
      intRes = readResponseInt();
    }
    XBEE_COMMAND_END_DECORATOR
    return intRes;
  }

  int8_t getBattPercentImpl() TINY_GSM_ATTR_NOT_AVAILABLE;
  int8_t getBattChargeStateImpl() TINY_GSM_ATTR_NOT_AVAILABLE;

  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    chargeState = 0;
    percent     = 0;
    milliVolts  = getBattVoltage();
    return true;
  }

  /*
   * Temperature functions
   */
 protected:
  float getTemperatureImpl() {
    XBEE_COMMAND_START_DECORATOR(5, static_cast<float>(-9999))
    String res = sendATGetString(GF("TP"));
    if (res == "") { return static_cast<float>(-9999); }
    char buf[5] = {
        0,
    };
    res.toCharArray(buf, 5);
    int8_t intRes = (int8_t)strtol(
        buf, 0,
        16);  // degrees Celsius displayed in 8-bit two's complement format.
    XBEE_COMMAND_END_DECORATOR
    return static_cast<float>(intRes);
  }

  /*
   * Client related functions
   */
 protected:
  int16_t getConnectionIndicator() {
    XBEE_COMMAND_START_DECORATOR(5, false)
    sendAT(GF("CI"));
    int16_t intRes = readResponseInt();
    XBEE_COMMAND_END_DECORATOR
    return intRes;
  }

  IPAddress getOperatingIP() {
    String strIP;
    strIP.reserve(16);

    XBEE_COMMAND_START_DECORATOR(5, IPAddress(0, 0, 0, 0))
    sendAT(GF("OD"));
    strIP = stream.readStringUntil('\r');  // read result
    strIP.trim();
    XBEE_COMMAND_END_DECORATOR

    if (strIP != "" && strIP != GF("ERROR")) {
      return TinyGsmIpFromString(strIP);
    } else {
      return IPAddress(0, 0, 0, 0);
    }
  }

  IPAddress lookupHostIP(const char* host, int timeout_s = 45) {
    String strIP;
    strIP.reserve(16);
    uint32_t startMillis = millis();
    uint32_t timeout_ms  = ((uint32_t)timeout_s) * 1000;
    bool     gotIP       = false;
    XBEE_COMMAND_START_DECORATOR(5, IPAddress(0, 0, 0, 0))
    // XBee's require a numeric IP address for connection, but do provide the
    // functionality to look up the IP address from a fully qualified domain
    // name
    // NOTE: the lookup can take a while
    while ((millis() - startMillis) < timeout_ms) {
      sendAT(GF("LA"), host);
      while (stream.available() < 4 && (millis() - startMillis < timeout_ms)) {
        TINY_GSM_YIELD()
      }
      strIP = stream.readStringUntil('\r');  // read result
      strIP.trim();
      if (strIP != "" && strIP != GF("ERROR")) {
        gotIP = true;
        break;
      }
      delay(2500);  // wait a bit before trying again
    }

    XBEE_COMMAND_END_DECORATOR

    if (gotIP) {
      return TinyGsmIpFromString(strIP);
    } else {
      return IPAddress(0, 0, 0, 0);
    }
  }

  bool modemConnect(const char* host, uint16_t port, uint8_t mux = 0,
                    bool ssl = false, int timeout_s = 75) {
    bool retVal = false;
    XBEE_COMMAND_START_DECORATOR(5, false)

    // If this is a new host name, replace the saved host and wipe out the saved
    // host IP
    if (this->savedHost != String(host)) {
      this->savedHost = String(host);
      savedHostIP     = IPAddress(0, 0, 0, 0);
    }

    // If we don't have a good IP for the host, we need to do a DNS search
    if (savedHostIP == IPAddress(0, 0, 0, 0)) {
      // This will return 0.0.0.0 if lookup fails
      savedHostIP = lookupHostIP(host, timeout_s);
    }

    // If we now have a valid IP address, use it to connect
    if (savedHostIP != IPAddress(0, 0, 0, 0)) {
      // Only re-set connection information if we have an IP address
      retVal = modemConnect(savedHostIP, port, mux, ssl);
    }

    XBEE_COMMAND_END_DECORATOR

    return retVal;
  }

  bool modemConnect(IPAddress ip, uint16_t port, uint8_t mux = 0,
                    bool ssl = false) {
    bool success     = true;
    bool changesMade = false;

    if (mux != 0) {
      DBG("XBee only supports 1 IP channel in transparent mode!");
    }

    // empty the saved currenty-in-use destination address
    savedOperatingIP = IPAddress(0, 0, 0, 0);

    XBEE_COMMAND_START_DECORATOR(5, false)

    if (ip != savedIP) {  // Can skip almost everything if there's no
                          // change in the IP address
      savedIP = ip;       // Set the newly requested IP address
      String host;
      host.reserve(16);
      host += ip[0];
      host += ".";
      host += ip[1];
      host += ".";
      host += ip[2];
      host += ".";
      host += ip[3];

      if (ssl) {
        // Put in SSL over TCP communication mode
        changesMade |= changeSettingIfNeeded(GF("IP"), 0x4);
      } else {
        // Put in unsecured TCP mode
        changesMade |= changeSettingIfNeeded(GF("IP"), 0x1);
      }
      bool changesMadeSSL = changesMade;

      changesMade |= changeSettingIfNeeded(
          GF("DL"), String(host));  // Set the "Destination Address Low"
      changesMade |= changeSettingIfNeeded(
          GF("DE"), String(port, HEX));  // Set the destination port

      // WiFi Bee is different
      if (beeType == XBEE_S6B_WIFI) { changesMade = changesMadeSSL; }

      if (changesMade) { success &= writeChanges(); }
    }

    // confirm the XBee type if needed so we know if we can know if connected
    if (beeType == XBEE_UNKNOWN) { getSeries(); }
    // we'll accept either unknown or connected
    if (beeType != XBEE_S6B_WIFI) {
      uint16_t ci = getConnectionIndicator();
      success &= (ci == 0x00 || ci == 0xFF || ci == 0x28);
    }

    if (success) { sockets[mux]->sock_connected = true; }

    XBEE_COMMAND_END_DECORATOR

    return success;
  }

  bool modemStop(uint32_t maxWaitMs) {
    streamClear();  // Empty anything in the buffer
    // empty the saved currently-in-use destination address
    savedOperatingIP = IPAddress(0, 0, 0, 0);

    XBEE_COMMAND_START_DECORATOR(5, false)

    // For WiFi models, there's no direct way to close the socket;
    // use DigiXBeeWifi::disconnectInternet(void)

    if (beeType != XBEE_S6B_WIFI) {
      // Get the current socket timeout
      sendAT(GF("TM"));
      String timeoutUsed = readResponseString(5000L);

      // For cellular models, per documentation: If you write the TM (socket
      // timeout) value while in Transparent Mode, the current connection is
      // immediately closed - this works even if the TM values is unchanged
      sendAT(GF("TM"), timeoutUsed);  // Re-set socket timeout
      waitResponse(maxWaitMs);        // This response can be slow
    }

    XBEE_COMMAND_END_DECORATOR
    return true;
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux = 0) {
    if (mux != 0) {
      DBG("XBee only supports 1 IP channel in transparent mode!");
    }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);
    stream.flush();

    if (beeType != XBEE_S6B_WIFI) {
      // After a send, verify the outgoing ip if it isn't set
      if (savedOperatingIP == IPAddress(0, 0, 0, 0)) {
        modemGetConnected(0);
      } else if (len > 5) {
        // After sending several characters, also re-check
        // NOTE:  I'm intentionally not checking after every single character!
        modemGetConnected(0);
      }
    }

    return len;
  }

  // NOTE:  The CI command returns the status of the TCP connection as open only
  // after data has been sent on the socket.  If it returns 0xFF the socket may
  // really be open, but no data has yet been sent.  We return this unknown
  // value as true so there's a possibility it's wrong.
  bool modemGetConnected(uint8_t) {
    // If the IP address is 0, it's not valid so we can't be connected
    if (savedIP == IPAddress(0, 0, 0, 0)) { return false; }

    XBEE_COMMAND_START_DECORATOR(5, false)

    if (beeType == XBEE_UNKNOWN)
      getSeries();  // Need to know the bee type to interpret response

    switch (beeType) {
      // The wifi be can only say if it's connected to the netowrk
      case XBEE_S6B_WIFI: {
        XBeeRegStatus s = getRegistrationStatus();
        XBEE_COMMAND_END_DECORATOR
        if (s != REG_OK) {
          sockets[0]->sock_connected = false;  // no multiplex
        }
        return (s == REG_OK);  // if it's connected, we hope the sockets are too
      }

      // Cellular XBee's
      default: {
        int16_t ci = getConnectionIndicator();
        // Get the operating destination address
        IPAddress od = getOperatingIP();
        XBEE_COMMAND_END_DECORATOR

        switch (ci) {
          // 0x00 = The socket is definitely open
          case 0x00: {
            savedOperatingIP = od;
            // but it's possible the socket is set to the wrong place
            if (od != IPAddress(0, 0, 0, 0) && od != savedIP) {
              sockets[0]->stop();
              return false;
            }
            return true;
          }

          // 0x28 = "Unknown."
          // 0xFF = No known status - always returned prior to sending data
          case 0x28:
          case 0xFF: {
            // If we previously had an operating destination and we no longer
            // do, the socket must have closed
            if (od == IPAddress(0, 0, 0, 0) &&
                savedOperatingIP != IPAddress(0, 0, 0, 0)) {
              savedOperatingIP           = od;
              sockets[0]->sock_connected = false;
              return false;
            } else if (od != IPAddress(0, 0, 0, 0) && od != savedIP) {
              // else if the operating destination exists, but is wrong
              // we need to close and re-open
              sockets[0]->stop();
              return false;
            } else if (od != IPAddress(0, 0, 0, 0) && od == savedIP) {
              // else if the operating destination exists and matches, we're
              // good to go
              savedOperatingIP = od;
              return true;
            } else {
              // If we never had an operating destination, then sock may be open
              // but data never sent - this is the dreaded "we don't know"
              savedOperatingIP = od;
              return true;
            }

            // // Ask for information about any open sockets
            // sendAT(GF("SI"));
            // String open_socks = stream.readStringUntil('\r');
            // open_socks.replace(AT_NL, "");
            // open_socks.trim();
            // if (open_socks != "") {
            //   // In transparent mode, only socket 0 should be possible
            //   sendAT(GF("SI0"));
            //   // read socket it
            //   String sock_id = stream.readStringUntil('\r');
            //   // read socket state
            //   String sock_state = stream.readStringUntil('\r');
            //   // read socket protocol (TCP/UDP)
            //   String sock_protocol = stream.readStringUntil('\r');
            //   // read local port number
            //   String local_port = stream.readStringUntil('\r');
            //   // read remote port number
            //   String remote_port = stream.readStringUntil('\r');
            //   // read remote ip address
            //   String remoted_address =
            //       stream.readStringUntil('\r');  // read result
            //   streamSkipUntil('\r');      // final carriage return
            // }
          }

          // 0x21 = User closed
          // 0x27 = Connection lost
          // If the connection is lost or timed out on our side,
          // we force close so it can reopen
          case 0x21:
          case 0x27: {
            sendAT(GF("TM"));  // Get socket timeout
            String timeoutUsed = readResponseString(5000L);
            sendAT(GF("TM"), timeoutUsed);  // Re-set socket timeout
            waitResponse(5000L);            // This response can be slow
          }

          // 0x02 = Invalid parameters (bad IP/host)
          // 0x12 = DNS query lookup failure
          // 0x25 = Unknown server - DNS lookup failed (0x22 for UDP socket!)
          // fall through
          case 0x02:
          case 0x12:
          case 0x25: {
            savedIP = IPAddress(0, 0, 0, 0);  // force a lookup next time!
          }

          // If it's anything else (inc 0x02, 0x12, and 0x25)...
          // it's definitely NOT connected
          // fall through
          default: {
            sockets[0]->sock_connected = false;
            savedOperatingIP           = od;
            return false;
          }
        }
      }
    }
  }

  /*
   * Utilities
   */
 public:
  void streamClear(void) {
    while (stream.available()) {
      stream.read();
      TINY_GSM_YIELD();
    }
  }
  // The XBee has no unsoliliced responses (URC's) when in command mode.
  bool handleURCs(String&) {
    return false;
  }

  bool commandMode(uint8_t retries = 5) {
    // If we're already in command mode, move on
    if (inCommandMode && (millis() - lastCommandModeMillis) < 10000L)
      return true;

    uint8_t triesMade = 0;
    int8_t  res;
    bool    success         = false;
    uint8_t triesUntilReset = 4;  // reset after number of tries
    if (beeType == XBEE_S6B_WIFI) { triesUntilReset = 9; }
    streamClear();  // Empty everything in the buffer before starting

    while (!success && triesMade < retries) {
      // Cannot send anything for 1 "guard time" before entering command mode
      // Default guard time is 1s, but the init fxn decreases it to 100 ms
      delay(guardTime + 10);
      streamWrite(GF("+++"));  // enter command mode

      if (beeType != XBEE_S6B_WIFI) {
        res = waitResponse(guardTime * 2);
      } else {
        // S6B wait a full second for OK
        res = waitResponse();
      }

      success = (1 == res);
      if (0 == res) {
        triesUntilReset--;
        if (triesUntilReset == 0) {
          triesUntilReset = 4;
          pinReset();  // if it's unresponsive, reset
          delay(250);  // a short delay to allow it to come back up
          // TODO(SRGDamia1) optimize this
        }
        if (beeType == XBEE_S6B_WIFI) {
          delay(5000);  // WiFi module frozen, wait longer
        }
      }
      triesMade++;
    }

    if (success) {
      inCommandMode         = true;
      lastCommandModeMillis = millis();
    }
    return success;
  }

  bool writeChanges(void) {
    sendAT(GF("WR"));  // Write changes to flash
    if (1 != waitResponse()) { return false; }
    sendAT(GF("AC"));  // Apply changes
    if (1 != waitResponse()) { return false; }
    return true;
  }

  void exitCommand(void) {
    // NOTE:  Here we explicitely try to exit command mode
    // even if the internal flag inCommandMode was already false
    sendAT(GF("CN"));  // Exit command mode
    waitResponse();
    inCommandMode = false;
  }

  bool exitAndFail(void) {
    exitCommand();  // Exit command mode
    return false;
  }

  void getSeries(void) {
    sendAT(GF("HS"));  // Get the "Hardware Series";
    int16_t intRes = readResponseInt();
    // if no response from module, then try again
    if (0xff == intRes) {
      sendAT(GF("HS"));  // Get the "Hardware Series";
      intRes = readResponseInt();
      if (0xff == intRes) {
        // Still no response, leave a known value - should reset
        intRes = XBEE_UNKNOWN;
      }
    }
    beeType = (XBeeType)intRes;
    DBG(GF("### Modem: "), getModemName(), beeType);
  }

  String readResponseString(uint32_t timeout_ms = 1000) {
    TINY_GSM_YIELD();
    uint32_t startMillis = millis();
    while (!stream.available() && millis() - startMillis < timeout_ms) {}
    String res =
        stream.readStringUntil('\r');  // lines end with carriage returns
    res.trim();
    return res;
  }

  int16_t readResponseInt(uint32_t timeout_ms = 1000) {
    String res = readResponseString(
        timeout_ms);  // it just works better reading a string first
    if (res == "") res = "FF";
    char buf[5] = {
        0,
    };
    res.toCharArray(buf, 5);
    int16_t intRes = strtol(buf, 0, 16);
    return intRes;
  }

  String sendATGetString(GsmConstStr cmd) {
    XBEE_COMMAND_START_DECORATOR(5, "")
    sendAT(cmd);
    String res = readResponseString();
    XBEE_COMMAND_END_DECORATOR
    return res;
  }

  bool changeSettingIfNeeded(GsmConstStr cmd, int16_t newValue,
                             uint32_t timeout_ms = 1000L) {
    sendAT(cmd);
    if (readResponseInt() != newValue) {
      sendAT(cmd, newValue);
      // return false if we attempted to change but failed
      if (waitResponse(timeout_ms) != 1) { return false; }
      // check if we succeeded in staging a change and retry once
      sendAT(cmd);
      if (readResponseInt() != newValue) {
        sendAT(cmd, newValue);
        if (waitResponse(timeout_ms) != 1) { return false; }
      }
      // return true if we succeeded in staging a change
      return true;
    }
    // return false if no change is needed
    return false;
  }

  bool changeSettingIfNeeded(GsmConstStr cmd, String newValue,
                             uint32_t timeout_ms = 1000L) {
    sendAT(cmd);
    if (readResponseString() != newValue) {
      sendAT(cmd, newValue);
      // return false if we attempted to change but failed
      if (waitResponse(timeout_ms) != 1) { return false; }
      // check if we succeeded in staging a change and retry once
      sendAT(cmd);
      if (readResponseString() != newValue) {
        sendAT(cmd, newValue);
        if (waitResponse(timeout_ms) != 1) { return false; }
      }
      return true;
    }
    // return false if no change is needed
    return false;
  }

  bool gotIPforSavedHost() {
    if (savedHost != "" && savedHostIP != IPAddress(0, 0, 0, 0))
      return true;
    else
      return false;
  }

 public:
  Stream& stream;

 protected:
  GsmClientXBee* sockets[TINY_GSM_MUX_COUNT];
  int16_t        guardTime;
  XBeeType       beeType;
  int8_t         resetPin;
  IPAddress      savedIP;
  String         savedHost;
  IPAddress      savedHostIP;
  IPAddress      savedOperatingIP;
  bool           inCommandMode;
  uint32_t       lastCommandModeMillis;
};

#endif  // SRC_TINYGSMCLIENTXBEE_H_
