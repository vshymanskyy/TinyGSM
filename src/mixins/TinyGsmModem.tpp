/**
 * @file       TinyGsmModem.tpp
 * @brief      Core modem trait presets and shared modem interface helpers.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMMODEM_TPP_
#define SRC_TINYGSMMODEM_TPP_

#include "TinyGsmCommon.h"

/**
 * @brief Template class for modem config traits.
 *
 * Most modems share common defaults; this base lets modem-specific config
 * structs specify only the values that differ.
 *
 * @tparam regStatusType The type used for registration status values.
 */
template <typename regStatusType = int8_t>
struct TinyGsmModemConfigPreset {
  /// The type used for registration status values.
  using RegStatus = regStatusType;

  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "unknown";
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "unknown";

  /// The newline character(s) used in AT commands.
  static constexpr char GSM_NL[] TINY_GSM_PROGMEM = "\r\n";
  /// The AT string
  static constexpr char GSM_AT[] TINY_GSM_PROGMEM = "AT";
  /// The OK string.
  static constexpr char GSM_OK[] TINY_GSM_PROGMEM = "OK\r\n";
  /// The ERROR string
  static constexpr char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR\r\n";

#if defined TINY_GSM_DEBUG
  /// The verbose error string
  static constexpr char GSM_VERBOSE[] TINY_GSM_PROGMEM = "+CME ERROR:";
  /// A second verbose error string
  static constexpr char GSM_VERBOSE_2[] TINY_GSM_PROGMEM = "+CMS ERROR:";
#endif
};

// Out-of-class definitions for C++11 compliance

/// @brief Out-of-class definition of the default modem manufacturer string.
template <typename regStatusType>
constexpr char TinyGsmModemConfigPreset<regStatusType>::MODEM_MANUFACTURER[]
    __attribute__((weak));

/// @brief Out-of-class definition of the default modem model string.
template <typename regStatusType>
constexpr char TinyGsmModemConfigPreset<regStatusType>::MODEM_MODEL[]
    __attribute__((weak));

/// @brief Out-of-class definition of the default AT newline sequence.
template <typename regStatusType>
constexpr char TinyGsmModemConfigPreset<regStatusType>::GSM_NL[]
    __attribute__((weak));

/// @brief Out-of-class definition of the default AT command prefix.
template <typename regStatusType>
constexpr char TinyGsmModemConfigPreset<regStatusType>::GSM_AT[]
    __attribute__((weak));

/// @brief Out-of-class definition of the default modem OK response string.
template <typename regStatusType>
constexpr char TinyGsmModemConfigPreset<regStatusType>::GSM_OK[]
    __attribute__((weak));

/// @brief Out-of-class definition of the default modem ERROR response string.
template <typename regStatusType>
constexpr char TinyGsmModemConfigPreset<regStatusType>::GSM_ERROR[]
    __attribute__((weak));

#if defined TINY_GSM_DEBUG
/// @brief Out-of-class definition of the first verbose error response prefix.
template <typename regStatusType>
constexpr char TinyGsmModemConfigPreset<regStatusType>::GSM_VERBOSE[]
    __attribute__((weak));

/// @brief Out-of-class definition of the second verbose error response prefix.
template <typename regStatusType>
constexpr char TinyGsmModemConfigPreset<regStatusType>::GSM_VERBOSE_2[]
    __attribute__((weak));
#endif

/**
 * @class TinyGsmModem
 * @brief The CRTP parent class for basic modem functions.
 * @tparam modemType The derived modem class
 * @tparam modemConfig The modem-specific configuration class, which must be
 * derived from TinyGsmModemConfigPreset
 */
template <class modemType, class modemConfig = TinyGsmModemConfigPreset<int8_t>>
class TinyGsmModem {
 public:
  using ModemConfig   = modemConfig;
  using regStatusType = typename ModemConfig::RegStatus;

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @anchor basic_functions
   * @name Basic functions
   */
  /**@{*/

  /**
   * @brief Sets up the GSM module
   * @param pin A pin code to unlock the SIM, if necessary
   * @return True if the module was set up as expected, false otherwise.
   */
  bool begin(const char* pin = nullptr) {
    return thisModem().initImpl(pin);
  }
  /**
   * @copydoc TinyGsmModem::begin()
   */
  bool init(const char* pin = nullptr) {
    return thisModem().initImpl(pin);
  }

  /**
   * @brief Recursive variadic template to send AT commands
   *
   * @tparam Args
   * @param cmd The commands to send
   */
  template <typename... Args>
  void sendAT(Args... cmd) {
    thisModem().streamPrint(GFP(ModemConfig::GSM_AT), cmd...,
                            GFP(ModemConfig::GSM_NL));
    thisModem().stream.flush();
    /* DBG("### AT:", cmd...); */
  }

  /**
   * @brief Set the module baud rate
   * @param baud The baud rate to use
   * @return True if the baud rate was set successfully, false otherwise.
   *
   * @note After setting and applying the new baud rate, you will have to %end()
   * and %begin() the serial object.
   */
  bool setBaud(uint32_t baud) {
    return thisModem().setBaudImpl(baud);
  }

  /**
   * @brief Set the default baud rate for the modem, ie, the baud rate that the
   * modem will use after a reset or power cycle.
   *
   * @param baud The baud rate to set the modem to use after a reset or power
   * cycle.
   * @return True if the command was successful, false otherwise.
   */
  bool setDefaultBaud(uint32_t baud) {
    return thisModem().setDefaultBaudImpl(baud);
  }

  /**
   * @brief Attempt to set the modem baud rate by trying set the command to
   * change the baud rate to various common baud rates and seeing if the modem
   * responds to AT commands at that baud rate.
   *
   * Unlike TinyGsmAutoBaud, this function will try to set the baud rate on the
   * modem and will attempt to communicate with the modem at the new baud rate
   * whether or not it gets a response at the old baud rate.  This is useful in
   * cases where the modem is set to a baud rate that is just a little too fast
   * for the processor, where you might get a response but it will be garbled
   * and not recognized as a response, because in this case you still want to
   * try to set the baud rate.
   *
   * @tparam StreamObject The type of the serial object used for communicating
   * with the modem
   * @param at_serial The serial object to use for communicating with the modem
   * @param targetBaud The final baud rate to try to set the modem to
   * @return True if the modem responded after the baud rate was set, false
   * otherwise.
   *
   * @note After setting and applying the new baud rate, you will have to end()
   * and begin() the serial object.
   */
  template <class StreamObject>
  bool forceModemBaud(StreamObject& at_serial, uint32_t targetBaud) {
    static const uint32_t* rates = TINY_GSM_AUTOBAUD_RATES;

    uint32_t maximum = 921600;
#if defined(F_CPU)
    if (F_CPU <= 8000000L) {
      maximum = 57600;
    } else if (F_CPU <= 16000000L) {
      maximum = 115200;
    }
#endif
    if (targetBaud > maximum) {
      DBG("Target baud rate", targetBaud,
          "is too high for this processor.  Maximum is", maximum);
      targetBaud = maximum;
    }

    // start the modem serial at the current baud rate
    at_serial.end();
    at_serial.begin(targetBaud);
    delay(25);  // settle
    // test for at response from the modem
    bool at_success = testAT(1500L);
    // if we got a response and it's the baud rate we want, we're done
    if (at_success) {
      DBG("Modem responded at rate", targetBaud);
      return true;
    }

    // If we didn't get the right response, or if we got a response but it's
    // not the baud we want, try to set the baud rate.
    // NOTE: We try to set the baud rate even if we *didn't* get a response
    // because if the modem is set in a baud that's just a *little* too fast
    // for the processor, we might get a response but it will be garbled and
    // not recognized as a response.  In this case, we still want to try to
    // set the baud rate.

    for (uint8_t i = 0; i < 14;
         i++) {  // sizeof(TINY_GSM_AUTOBAUD_RATES)/sizeof(uint32_t)
      uint32_t rate = TINY_GSM_F_READ(&rates[i]);
      for (uint8_t j = 0; j < 3; j++) {
        DBG("Trying to set the baud rate from a rate of", rate, "...");
        at_serial.end();
        at_serial.begin(rate);
        delay(25);  // settle

        setBaud(targetBaud);

        at_serial.end();
        at_serial.begin(targetBaud);
        delay(25);  // settle

        // test for at response from the modem
        DBG("Checking for a response at", targetBaud, "...");
        at_success = testAT(1500L);
        // if we got a response and it's the baud rate we want, we're done
        if (at_success) {
          DBG(GF("Successfully changed the baud rate from"), rate, GF("to"),
              targetBaud);
          return true;
        }
      }
    }
    DBG(GF("Failed to successfully find the baud at any common rate or to "
           "change "
           "the baud rate to"),
        targetBaud, GF("..."));
    at_serial.begin(targetBaud);
    return false;
  }

  /**
   * @brief Test response to AT commands
   *
   * @param timeout_ms the amount of time to test for; optional with a
   * default value of 10s.
   * @return True if the module responded to AT commands, false otherwise.
   */
  bool testAT(uint32_t timeout_ms = 10000L) {
    return thisModem().testATImpl(timeout_ms);
  }

  /**
   * @brief Listen for responses to commands and handle URCs
   *
   * @param timeout_ms The time to wait for a response. If a response is in
   * progress when this expires, a bounded inter-character grace period allows
   * the current response to finish.
   * @param data A string of data to fill in with response results
   * @param r1 The first output to test against, optional with a default value
   * of "OK"
   * @param r2 The second output to test against, optional with a default value
   * of "ERROR"
   * @param r3 The third output to test against, optional with a default value
   * of nullptr
   * @param r4 The fourth output to test against, optional with a default value
   * of nullptr
   * @param r5 The fifth output to test against, optional with a default value
   * of nullptr
   * @param r6 The sixth output to test against, optional with a default value
   * of nullptr
   * @param r7 The seventh output to test against, optional with a default value
   * of nullptr
   * @param r8 The eighth output to test against, optional with a default value
   * of nullptr
   * @return *int8_t* the index of the response input
   */
  int8_t waitResponse(uint32_t timeout_ms, String& data,
                      GsmConstStr r1 = GFP(ModemConfig::GSM_OK),
                      GsmConstStr r2 = GFP(ModemConfig::GSM_ERROR),
                      GsmConstStr r3 = nullptr, GsmConstStr r4 = nullptr,
                      GsmConstStr r5 = nullptr, GsmConstStr r6 = nullptr,
                      GsmConstStr r7 = nullptr, GsmConstStr r8 = nullptr) {
    return thisModem().waitResponseImpl(timeout_ms, data, r1, r2, r3, r4, r5,
                                        r6, r7, r8);
  }

  /**
   * @brief Listen for responses to commands and handle URCs
   *
   * @param timeout_ms The time to wait for a response. If a response is in
   * progress when this expires, a bounded inter-character grace period allows
   * the current response to finish.
   * @param r1 The first output to test against, optional with a default value
   * of "OK"
   * @param r2 The second output to test against, optional with a default value
   * of "ERROR"
   * @param r3 The third output to test against, optional with a default value
   * of nullptr
   * @param r4 The fourth output to test against, optional with a default value
   * of nullptr
   * @param r5 The fifth output to test against, optional with a default value
   * of nullptr
   * @param r6 The sixth output to test against, optional with a default value
   * of nullptr
   * @param r7 The seventh output to test against, optional with a default value
   * of nullptr
   * @param r8 The eighth output to test against, optional with a default value
   * of nullptr
   * @return *int8_t* the index of the response input
   */
  int8_t waitResponse(uint32_t    timeout_ms,
                      GsmConstStr r1 = GFP(ModemConfig::GSM_OK),
                      GsmConstStr r2 = GFP(ModemConfig::GSM_ERROR),
                      GsmConstStr r3 = nullptr, GsmConstStr r4 = nullptr,
                      GsmConstStr r5 = nullptr, GsmConstStr r6 = nullptr,
                      GsmConstStr r7 = nullptr, GsmConstStr r8 = nullptr) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5, r6, r7, r8);
  }

  /**
   * @brief Listen for responses to commands and handle URCs; listening for 1
   * second.
   *
   * @param r1 The first output to test against, optional with a default value
   * of "OK"
   * @param r2 The second output to test against, optional with a default value
   * of "ERROR"
   * @param r3 The third output to test against, optional with a default value
   * of nullptr
   * @param r4 The fourth output to test against, optional with a default value
   * of nullptr
   * @param r5 The fifth output to test against, optional with a default value
   * of nullptr
   * @param r6 The sixth output to test against, optional with a default value
   * of nullptr
   * @param r7 The seventh output to test against, optional with a default value
   * of nullptr
   * @param r8 The eighth output to test against, optional with a default value
   * of nullptr
   * @return *int8_t* the index of the response input
   */
  int8_t waitResponse(GsmConstStr r1 = GFP(ModemConfig::GSM_OK),
                      GsmConstStr r2 = GFP(ModemConfig::GSM_ERROR),
                      GsmConstStr r3 = nullptr, GsmConstStr r4 = nullptr,
                      GsmConstStr r5 = nullptr, GsmConstStr r6 = nullptr,
                      GsmConstStr r7 = nullptr, GsmConstStr r8 = nullptr) {
    return waitResponse(1000L, r1, r2, r3, r4, r5, r6, r7, r8);
  }

  /**
   * @brief Gets the configured modem manufacture and model based on the
   * compile-time traits.
   *
   * @remark This does **not** query the modem for its actual manufacture and
   * model, but rather returns the values that were set at compile time.
   * @return *String* The configured modem manufacture and model based on the
   * compile-time traits.
   */
  String getConfiguredModem() {
    return thisModem().getConfiguredModemImpl();
  }

  /**
   * @brief Asks for modem information via the 3GPP TS 27.007 standard ATI
   * command
   *
   * @note  The actual value and style of the response is quite varied
   * @return *String* Some info about the GSM module.
   */
  String getModemInfo() {
    return thisModem().getModemInfoImpl();
  }

  /**
   * @brief Get the modem name - a combination of the manufacturer and model, as
   * the modem calls itself
   *
   * @return *String*  The modem name
   */
  String getModemName() {
    return thisModem().getModemNameImpl();
  }

  /**
   * @brief Get the modem manufacturer
   * @return *String* The modem manufacturer
   */
  String getModemManufacturer() {
    return thisModem().getModemManufacturerImpl();
  }

  /**
   * @brief Get the modem model
   * @return *String* The modem model, as it calls itself
   */
  String getModemModel() {
    return thisModem().getModemModelImpl();
  }

  /**
   * @brief Get the modem revision information.
   *
   * What is returned as the revision may be either a hardware or a firmware
   * version or some combination of both.
   *
   * @return *String* The modem revision information
   */
  String getModemRevision() {
    return thisModem().getModemRevisionImpl();
  }

  /**
   * @brief Get the modem serial number
   *
   * This is usually equivalent to the IMEI for cellular modems and the MAC
   * address for WiFi modems.
   *
   * @return *String* The modem serial number
   */
  String getModemSerialNumber() {
    return thisModem().getModemSerialNumberImpl();
  }

  /**
   * @brief Reset the module to factory defaults.
   *
   * This generally restarts the module as well.
   *
   * @return True if the module successfully reset to default, false otherwise.
   */
  bool factoryDefault() {
    return thisModem().factoryDefaultImpl();
  }
  /**@}*/

  /**
   * @anchor power_functions
   * @name Power functions
   */
  /**@{*/

  /**
   * @brief Restart the module
   * @param pin A pin code to unlock the SIM, if necessary
   * @return True if the module was successfully restarted, false otherwise.
   */
  bool restart(const char* pin = nullptr) {
    return thisModem().restartImpl(pin);
  }
  /**
   * @brief Power off the module
   * @return True if the module was successfully powered down, false otherwise.
   * @deprecated Use powerOff() instead. This function name will be removed in a
   * future version.
   */
  bool poweroff() __attribute__((__deprecated__("Use powerOff() instead"))) {
    return thisModem().powerOffImpl();
  }
  /**
   * @brief Power off the module
   * @return True if the module was successfully powered down, false otherwise.
   */
  bool powerOff() {
    return thisModem().powerOffImpl();
  }
  /**
   * @brief Turn off the module radio
   * @return True if the module radio was successfully turned off, false
   * otherwise.
   */
  bool radioOff() {
    return thisModem().radioOffImpl();
  }

  /**
   * @brief Enable sleep on the module.
   *
   * For some modules this immediately puts
   * the module to sleep, for others this sets them to be able to sleep based on
   * pin levels.
   *
   * @param enable True to enable sleep, false to disable
   * @return True if sleep was successfully enabled or disabled, false
   * otherwise.
   */
  bool sleepEnable(bool enable = true) {
    return thisModem().sleepEnableImpl(enable);
  }

  /**
   * @brief Set the phone functionality
   *
   * @param fun The phone functionality setting. The value and meaning of this
   * varies by module; check your documentation.
   * @param reset True to reset the module before changing the functionality.
   * @return True if the phone functionality was successfully changed, false
   * otherwise.
   */
  bool setPhoneFunctionality(uint8_t fun, bool reset = false) {
    return thisModem().setPhoneFunctionalityImpl(fun, reset);
  }
  /**@}*/

  /**
   * @anchor network_functions
   * @name Generic network functions
   */
  /**@{*/

  /**
   * @brief Get the modem registration status on the network.
   * @return The modem-specific registration status value.
   */
  regStatusType getRegistrationStatus() {
    return thisModem().getRegistrationStatusImpl();
  }

  /**
   * @brief Confirm whether the module is currently connected to the cellular
   * data (GSM/GPRS/eGPRS/EPS/LTE, etc) network.
   *
   * @return True if the module is connected to the network, false otherwise.
   */
  bool isNetworkConnected() {
    return thisModem().isNetworkConnectedImpl();
  }

  /**
   * @brief Wait until the module has connected to the network
   *
   * @param timeout_ms The time to wait for attachment in milliseconds. Optional
   * with a default value of 1 minute.
   * @param check_signal True to alternate between checking for connection and
   * checking the signal strength.
   * @return True if the module is now connected to the network, false
   * otherwise.
   */
  bool waitForNetwork(uint32_t timeout_ms = 60000L, bool check_signal = false) {
    return thisModem().waitForNetworkImpl(timeout_ms, check_signal);
  }

  /**
   * @brief Get the signal quality report
   *
   * This is often a "CSQ" value ranging from 0 to 32, but may be an RSSI or a
   * percent.
   *
   * @return *int16_t* The signal quality, or -9999 if the modem does not
   * respond or the response is invalid.
   */
  int16_t getSignalQuality() {
    return thisModem().getSignalQualityImpl();
  }

  /**
   * @brief Get the Local IP address assigned to the module by the network as a
   * String
   *
   * @return *String* The local IP address
   */
  String getLocalIP() {
    return thisModem().getLocalIPImpl();
  }

  /**
   * @brief Get the Local IP address assigned to the module by the network as an
   * IPAddress object.
   *
   * @return *IPAddress* The local IP address
   */
  IPAddress localIP() {
    return thisModem().TinyGsmIpFromString(thisModem().getLocalIP());
  }
  /**@}*/

 protected:
  // destructor (protected!)
  ~TinyGsmModem() {}

  /**
   * @anchor crtp_helper
   * @name CRTP Helper
   */
  /**@{*/
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }
  /**@}*/


  /**
   * @anchor modem_utilities
   * @name Utilities
   */
  /**@{*/
 public:
  // Utility templates for writing/skipping characters on a stream
  /**
   * @brief Write a value to the modem stream.
   *
   * @tparam T The type of the value to write.
   * @param last The value to write.
   */
  template <typename T>
  void streamPrint(T last) {
    thisModem().stream.print(last);
  }

  /**
   * @brief Recursively write multiple values to the modem stream.
   *
   * @tparam T The type of the first value to write.
   * @tparam Args The types of the remaining values to write.
   * @param head The first value to write.
   * @param tail The remaining values to write.
   */
  template <typename T, typename... Args>
  void streamPrint(T head, Args... tail) {
    thisModem().stream.print(head);
    thisModem().streamPrint(tail...);
  }

  /**
   * @brief Write length-delimited data from RAM to the modem stream.
   *
   * @param data Pointer to the data in RAM.
   * @param len The number of bytes to write.
   */
  void streamWrite(const char* data, uint16_t len) {
    thisModem().stream.write(reinterpret_cast<const uint8_t*>(data), len);
  }

#if defined(TINY_GSM_HAS_PROGMEM)
  /**
   * @brief Write length-delimited data from program memory to the modem stream.
   *
   * @param data Pointer to the data in program memory.
   * @param len The number of bytes to write.
   */
  void streamWrite(GsmConstStr data, uint16_t len) {
    const uint8_t* source = reinterpret_cast<const uint8_t*>(data);
    uint8_t        buffer[32];
    uint16_t       offset = 0;

    while (offset < len) {
      const uint16_t chunk = TinyGsmMin(static_cast<uint16_t>(sizeof(buffer)),
                                        static_cast<uint16_t>(len - offset));
      memcpy_P(buffer, source + offset, chunk);
      thisModem().stream.write(buffer, chunk);
      offset += chunk;
    }
  }
#endif

  /**
   * @brief Clear the modem stream attempting to parse any URC's in it.
   *
   * @warning Never call this inside of a waitResponse() call or a URC parser!
   * @todo Should this be protected?
   */
  inline void streamClear() {
    while (thisModem().stream.available()) {
      thisModem().waitResponse(50, nullptr, nullptr);
    }
  }

  /**
   * @brief Dump the modem stream, completely discarding any data in it without
   * attempting to parse any URC's in it.
   *
   * This is good for discarding an expected number of characters from the
   * stream without waiting a full timeout for each character.
   *
   * @param expected_len The number of characters to discard from the stream.
   * @todo Should this be protected?
   */
  inline void streamDump(int16_t expected_len) {
    if (expected_len <= 0) { return; }
    size_t  len           = static_cast<size_t>(expected_len);
    size_t  len_read      = 0;
    uint8_t char_failures = 0;

    // allow up to 3 timeouts on individual characters before we quit the whole
    // read operation
    while (len && char_failures < 3) {
      if (thisModem().streamTimedRead(1000L) < 0) {
        DBG("### ERROR: Timed out waiting for character from stream!");
        char_failures++;
      } else {
        len--;
        len_read++;
      }
    }

    if (len_read) { DBG("### Discarded", len_read, "characters"); }
  }


 protected:
  /**
   * @brief Wait for characters to become available without consuming them.
   *
   * @param required The minimum number of characters to wait for.
   * @param timeout_ms Maximum time to wait for the characters.
   * @return The number of characters available when the requested count is
   *         reached or the timeout expires.
   */
  inline int streamTimedAvailable(const size_t   required,
                                  const uint32_t timeout_ms) {
    int available = thisModem().stream.available();
    if ((available >= 0 && static_cast<size_t>(available) >= required) ||
        timeout_ms == 0) {
      return available;
    }

    uint32_t startMillis = millis();
    while (true) {
      yield();
      available = thisModem().stream.available();
      if (available >= 0 && static_cast<size_t>(available) >= required) {
        return available;
      }
      if (millis() - startMillis >= timeout_ms) { return available; }
    }
  }

  /**
   * @brief Peek at the next character in the modem stream with a timeout.
   *
   * Unlike Stream::timedPeek(), this helper is accessible to modem mixins and
   * accepts an explicit timeout without changing the Stream object's shared
   * timeout.
   *
   * @param timeout_ms Maximum time to wait for a character.
   * @return The next character value without consuming it, or -1 if no
   *         character arrived in time.
   */
  inline int streamTimedPeek(const uint32_t timeout_ms) {
    int c = thisModem().stream.peek();
    if (c >= 0 || timeout_ms == 0) { return c; }

    uint32_t startMillis = millis();
    while (true) {
      yield();
      if (millis() - startMillis >= timeout_ms) { return -1; }
      c = thisModem().stream.peek();
      if (c >= 0) { return c; }
    }
  }

  /**
   * @brief Read one character from the modem stream with a timeout.
   *
   * Unlike Stream::timedRead(), this helper accepts an explicit timeout and
   * yields while waiting without changing the Stream object's shared timeout.
   *
   * @param timeout_ms Maximum time to wait for a character.
   * @return The character value, or -1 if no character arrived in time.
   */
  inline int streamTimedRead(const uint32_t timeout_ms) {
    int c = thisModem().stream.read();
    if (c >= 0 || timeout_ms == 0) { return c; }

    uint32_t startMillis = millis();
    while (true) {
      yield();
      if (millis() - startMillis >= timeout_ms) { return -1; }
      c = thisModem().stream.read();
      if (c >= 0) { return c; }
    }
  }

  /**
   * @brief Read one character using an existing operation timeout.
   *
   * The timeout is measured from @p startMillis, allowing multiple reads to
   * share one overall timeout instead of restarting the timeout for every
   * character.
   *
   * @param startMillis The value of millis() when the operation began.
   * @param timeout_ms Maximum total time for the operation.
   * @return The character value, or -1 if no character arrived in time.
   */
  inline int streamTimedRead(const uint32_t startMillis,
                             const uint32_t timeout_ms) {
    int c = thisModem().stream.read();
    if (c >= 0 || timeout_ms == 0) { return c; }

    while (true) {
      yield();
      if (millis() - startMillis >= timeout_ms) { return -1; }
      c = thisModem().stream.read();
      if (c >= 0) { return c; }
    }
  }

  /**
   * @brief Reads a fixed number of characters from the modem stream into a
   * buffer.
   *
   * This function attempts to read exactly @p numChars characters from the
   * modem stream into the provided buffer @p buf. If the required number of
   * characters cannot be read within the specified timeout, the function
   * returns false.
   *
   * Unlike Stream::readBytes(), this function enforces reading the exact number
   * of characters requested and treats the timeout as a the time for the entire
   * count of characters instead of each individual character.
   *
   * @param buf        Pointer to the buffer where the characters will be
   * stored.
   * @param numChars   Number of characters to read.
   * @param timeout_ms Maximum time, in milliseconds, to wait for the
   * characters.
   * @return true if the requested number of characters were successfully read,
   *         false otherwise.
   */
  inline bool streamGetLength(char* buf, int8_t numChars,
                              const uint32_t timeout_ms = 1000L) {
    if (!buf || numChars <= 0) { return false; }

    // Clear the buffer before reading into it
    memset(buf, 0, numChars);

    uint32_t startMillis = millis();
    for (int8_t i = 0; i < numChars; ++i) {
      int readChar = thisModem().streamTimedRead(startMillis, timeout_ms);
      if (readChar < 0) { return false; }
      buf[i] = static_cast<char>(readChar);
    }
    return true;
  }

  /**
   * @brief Reads a fixed-length decimal integer from the modem stream.
   *
   * Reads exactly @p numChars characters from the modem stream and converts
   * them to a signed 16-bit decimal integer. Leading and trailing non-digits
   * are ignored. A leading '-' or '+' are supported.
   *
   * This is a lightweight replacement for strtol()/atoi() intended for
   * parsing the small decimal integer fields returned by modems. It does
   * not perform general-purpose numeric parsing or require a null-terminated
   * string.
   *
   * @param numChars   Number of characters to read and convert. Valid values
   *                   are 1 through 6.
   * @param timeout_ms Maximum time, in milliseconds, to wait for the requested
   *                   characters.
   *
   * @return The parsed signed 16-bit integer, or -9999 if the requested
   *         character count is invalid or the characters could not be read
   *         from the stream.
   */
  inline int16_t streamGetIntLength(int8_t         numChars,
                                    const uint32_t timeout_ms = 1000L) {
    // max 6 digits for int16_t (-32767)
    if (numChars <= 0 || numChars > 6) { return -9999; }

    char buf[6] = {};

    if (!streamGetLength(buf, numChars, timeout_ms)) { return -9999; }

    int16_t res      = 0;
    bool    negative = false;
    uint8_t i        = 0;

    // Skip until a digit or sign; '.' and '/' cannot start a number.
    while (i < numChars && buf[i] != '+' && buf[i] != '-' &&
           (buf[i] < '0' || buf[i] > '9')) {
      ++i;
    }

    if (i < numChars && (buf[i] == '-' || buf[i] == '+')) {
      negative = buf[i] == '-';
      ++i;
    }

    // Parse until the first non-digit (accept only 0-9).
    for (; i < numChars && (buf[i] >= 0x30 && buf[i] <= 0x39); ++i) {
      uint8_t digit = buf[i] - '0';
      // Check for overflow before multiply-and-add
      if (res > (32767 - digit) / 10) { return -9999; }
      res = res * 10 + digit;
    }

    // Check sign overflow: -32768 is valid, but 32768 is not
    if (negative && res > 32768) { return -9999; }

    return negative ? -res : res;
  }


  /**
   * @brief Reads a decimal integer from the modem stream up to a delimiter.
   *
   * Reads characters from the modem stream until @p lastChar is encountered
   * and converts the characters preceding it to a signed 16-bit decimal
   * integer. Leading and trailing non-digits are ignored. A leading '-' or '+'
   * is supported.  Decimals points are treated as non-digit characters - they
   * terminate the integer parsing.
   *
   * This is a lightweight replacement for strtol()/atoi() intended for
   * parsing the small decimal integer fields returned by modems. It does
   * not require a null-terminated string.
   *
   * The input buffer is limited to seven characters, allowing for a sign and
   * up to six numeric characters.
   *
   * @param lastChar Character that terminates the numeric response.
   *
   * @return The parsed signed 16-bit integer, or -9999 if no characters were
   *         received before the delimiter.
   */
  inline int16_t streamGetIntBefore(char lastChar) {
    char buf[7] = {};

    size_t bytesRead = thisModem().stream.readBytesUntil(lastChar, buf,
                                                         sizeof(buf));

    if (!bytesRead) { return -9999; }

    int16_t res      = 0;
    bool    negative = false;
    uint8_t i        = 0;

    // Skip until a digit or sign; '.' and '/' cannot start a number.
    while (i < bytesRead && buf[i] != '+' && buf[i] != '-' &&
           (buf[i] < '0' || buf[i] > '9')) {
      ++i;
    }

    if (i < bytesRead && (buf[i] == '-' || buf[i] == '+')) {
      negative = buf[i] == '-';
      ++i;
    }

    // Parse until the first non-digit (accept only 0-9).
    for (; i < bytesRead && (buf[i] >= 0x30 && buf[i] <= 0x39); ++i) {
      uint8_t digit = buf[i] - '0';
      // Check for overflow before multiply-and-add
      if (res > (32767 - digit) / 10) { return -9999; }
      res = res * 10 + digit;
    }

    // Check sign overflow: -32768 is valid, but 32768 is not
    if (negative && res > 32768) { return -9999; }

    return negative ? -res : res;
  }

  /**
   * @brief Reads a fixed-length unsigned long integer from the modem stream.
   *
   * Reads exactly @p numChars characters from the modem stream and converts
   * them to an unsigned long integer. Leading and trailing non-digits
   * are ignored.
   *
   * @param numChars   Number of characters to read and convert. Valid values
   *                   are 1 through 12.
   * @param timeout_ms Maximum time, in milliseconds, to wait for the requested
   *                   characters.
   *
   * @return The parsed unsigned long or -1 on failure
   */
  inline uint32_t streamGetULLength(int8_t         numChars,
                                    const uint32_t timeout_ms = 1000L) {
    // max 12 digits for unsigned long
    if (numChars <= 0 || numChars > 12) { return static_cast<uint32_t>(-1); }

    char buf[12] = {};

    if (!streamGetLength(buf, numChars, timeout_ms)) {
      return static_cast<uint32_t>(-1);
    }

    uint32_t res = 0;
    uint8_t  i   = 0;

    // Skip leading non-digits (accept only 0-9).
    while (i < numChars && (buf[i] < 0x30 || buf[i] > 0x39)) { ++i; }

    // Parse until the first non-digit (accept only 0-9).
    for (; i < numChars && (buf[i] >= 0x30 && buf[i] <= 0x39); ++i) {
      uint8_t digit = buf[i] - '0';
      // Check for overflow before multiply-and-add
      if (res > (4294967295U - digit) / 10) {
        return static_cast<uint32_t>(-1);
      }
      res = res * 10 + digit;
    }

    return res;
  }


  /**
   * @brief Reads a unsigned long integer from the modem stream up to a
   * delimiter.
   *
   * Reads characters from the modem stream until @p lastChar is encountered
   * and converts the characters preceding it to an unsigned long decimal
   * integer. Leading and trailing non-digits are ignored.
   *
   * The input buffer is limited to 13 characters, allowing for a sign and
   * up to 12 numeric characters.
   *
   * @param lastChar Character that terminates the numeric response.
   *
   * @return The parsed unsigned long integer, or -1 if no characters were
   *         received before the delimiter.
   */
  inline uint32_t streamGetULBefore(char lastChar) {
    char buf[13] = {};

    size_t bytesRead = thisModem().stream.readBytesUntil(lastChar, buf,
                                                         sizeof(buf));

    if (!bytesRead) { return static_cast<uint32_t>(-1); }

    uint32_t res = 0;
    uint8_t  i   = 0;

    // Skip leading non-digits (accept only 0-9).
    while (i < bytesRead && (buf[i] < 0x30 || buf[i] > 0x39)) { ++i; }

    // Parse until the first non-digit (accept only 0-9).
    for (; i < bytesRead && (buf[i] >= 0x30 && buf[i] <= 0x39); ++i) {
      uint8_t digit = buf[i] - '0';
      // Check for overflow before multiply-and-add
      if (res > (4294967295U - digit) / 10) {
        return static_cast<uint32_t>(-1);
      }
      res = res * 10 + digit;
    }

    return res;
  }


  /**
   * @brief Reads a fixed-length decimal floating-point value.
   *
   * Reads exactly @p numChars characters from the modem stream and converts
   * them to a floating-point value. Leading and trailing non-digits are
   * ignored. A leading '-' or '+' and a decimal point are supported.
   *
   * This is a lightweight replacement for strtof()/atof() intended for
   * parsing the ordinary decimal values returned by modems, including
   * NMEA-like latitude, longitude, altitude, speed, accuracy, and time fields.
   * Scientific notation is intentionally not supported.
   *
   * @param numChars   Number of characters to read and convert.
   * @param timeout_ms Maximum time, in milliseconds, to wait for the requested
   *                   characters.
   *
   * @return The parsed floating-point value, or -9999.0F if the requested
   *         characters could not be read from the stream.
   */
  inline float streamGetFloatLength(int8_t         numChars,
                                    const uint32_t timeout_ms = 1000L) {
    // max 15 digits, sign, and decimal point
    if (numChars <= 0 || numChars > 16) { return -9999.0F; }

    char buf[16] = {};

    if (!streamGetLength(buf, numChars, timeout_ms)) { return -9999.0F; }

    float   result   = 0.0F;
    uint8_t decimals = 0;
    bool    negative = false;
    bool    decimal  = false;
    uint8_t i        = 0;

    // Skip until a digit or sign; '.' and '/' cannot start a number.
    while (i < numChars && buf[i] != '+' && buf[i] != '-' &&
           (buf[i] < '0' || buf[i] > '9')) {
      ++i;
    }

    if (i < numChars && (buf[i] == '-' || buf[i] == '+')) {
      negative = buf[i] == '-';
      ++i;
    }

    // Parse until the first non-digit (also accept a single '.'; '/' is not a
    // digit).
    for (; i < numChars && (buf[i] == '.' || (buf[i] >= '0' && buf[i] <= '9'));
         ++i) {
      char c = buf[i];

      if (c == '.') {
        decimal = true;
      } else {
        result = result * 10.0F + c - '0';
        if (decimal) { ++decimals; }
      }
    }

    while (decimals--) { result *= 0.1F; }

    return negative ? -result : result;
  }


  /**
   * @brief Reads a decimal floating-point value up to a delimiter.
   *
   * Reads characters from the modem stream until @p lastChar is encountered
   * and converts the characters preceding it to a floating-point value.
   * Leading and trailing non-digits are ignored. A leading '-' or '+' and a
   * decimal point are supported.
   *
   * This is a lightweight replacement for strtof()/atof() intended for
   * parsing the ordinary decimal values returned by modems, including
   * NMEA-like latitude, longitude, altitude, speed, accuracy, and time fields.
   * Scientific notation is intentionally not supported.
   *
   * The input buffer is limited to 16 characters.
   *
   * @param lastChar Character that terminates the numeric response.
   *
   * @return The parsed floating-point value, or -9999.0F if no characters were
   *         received before the delimiter.
   */
  inline float streamGetFloatBefore(char lastChar) {
    char buf[16] = {};

    size_t bytesRead = thisModem().stream.readBytesUntil(lastChar, buf,
                                                         sizeof(buf));

    if (!bytesRead) { return -9999.0F; }

    float   result   = 0.0F;
    uint8_t decimals = 0;
    bool    negative = false;
    bool    decimal  = false;
    uint8_t i        = 0;

    // Skip until a digit or sign; '.' and '/' cannot start a number.
    while (i < bytesRead && buf[i] != '+' && buf[i] != '-' &&
           (buf[i] < '0' || buf[i] > '9')) {
      ++i;
    }

    if (i < bytesRead && (buf[i] == '-' || buf[i] == '+')) {
      negative = buf[i] == '-';
      ++i;
    }

    // Parse until the first non-digit (also accept a single '.'; '/' is not a
    // digit).
    for (; i < bytesRead && (buf[i] == '.' || (buf[i] >= '0' && buf[i] <= '9'));
         ++i) {
      char c = buf[i];

      if (c == '.') {
        decimal = true;
      } else {
        result = result * 10.0F + c - '0';
        if (decimal) { ++decimals; }
      }
    }

    while (decimals--) { result *= 0.1F; }

    return negative ? -result : result;
  }

  inline bool streamSkipUntil(const char c, const uint32_t timeout_ms = 1000L) {
    if (timeout_ms == 0) { return false; }
    uint32_t startMillis = millis();
    while (true) {
      int readChar = thisModem().streamTimedRead(startMillis, timeout_ms);
      if (readChar < 0) { return false; }
      if (readChar == c) { return true; }
      if (millis() - startMillis >= timeout_ms) { return false; }
    }
  }

  inline void cleanResponseString(String& res) {
    // Remove the OK from the string, as well as any newlines
    const String nlResponse = String(GFP(ModemConfig::GSM_NL));
    String       okResponse = nlResponse + String(GFP(ModemConfig::GSM_OK));
    res.replace(okResponse, "");
    res.replace(nlResponse, " ");
    res.trim();
  }

  static inline IPAddress TinyGsmIpFromString(const String& strIP) {
    int  Parts[4]          = {0};
    int  Part              = 0;
    bool addressStarted    = false;
    bool hasDigitInPart[4] = {
        false};  // Track if each octet has at least one digit

    for (uint8_t i = 0; i < strIP.length(); i++) {
      char c = strIP[i];
      if (!addressStarted) {
        if (c < '0' || c > '9') { continue; }
        addressStarted = true;
      }

      if (c == '.') {
        // Reject dot when octet is empty
        if (!hasDigitInPart[Part]) { return IPAddress(0, 0, 0, 0); }
        Part++;
        if (Part > 3) { return IPAddress(0, 0, 0, 0); }
        continue;
      } else if (c >= '0' && c <= '9') {
        hasDigitInPart[Part] = true;
        int nextValue        = Parts[Part] * 10 + (c - '0');
        if (nextValue > 255) { return IPAddress(0, 0, 0, 0); }
        Parts[Part] = nextValue;
      } else {
        // Framing may follow a complete dotted-quad, but not interrupt it.
        if (Part == 3 && hasDigitInPart[Part]) { break; }
        return IPAddress(0, 0, 0, 0);
      }
    }

    // Reject final empty octet
    if (!hasDigitInPart[Part]) { return IPAddress(0, 0, 0, 0); }

    // Validate: must have exactly 3 dots (Part must equal 3)
    if (Part != 3) { return IPAddress(0, 0, 0, 0); }

    // Validate: each octet must be 0-255
    for (int i = 0; i < 4; i++) {
      if (Parts[i] < 0 || Parts[i] > 255) { return IPAddress(0, 0, 0, 0); }
    }

    return IPAddress(Parts[0], Parts[1], Parts[2], Parts[3]);
  }

  String TinyGsmIptoString(IPAddress ip) {
    String host;
    host.reserve(16);
    host += ip[0];
    host += '.';
    host += ip[1];
    host += '.';
    host += ip[2];
    host += '.';
    host += ip[3];
    return host;
  }

  void TinyGsmIptoBuffer(IPAddress ip, char* buffer) {
    if (!buffer) { return; }
    memset(buffer, '\0', 16);  // Clear the buffer before writing the IP address
    char buf[4] = {'\0'};
    for (uint8_t i = 0; i < 4; i++) {
      itoa(ip[i], buf, 10);
      strncat(buffer, buf, sizeof(buf));
      if (i < 3) { strncat(buffer, ".", 2); }
    }
  }

  /**@}*/

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */
 protected:
  /*
   * Basic functions
   */

  bool initImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool setBaudImpl(uint32_t baud) {
    thisModem().sendAT(GF("+IPR="), baud);
    return thisModem().waitResponse() == 1;
  }

  bool setDefaultBaudImpl(uint32_t baud) TINY_GSM_ATTR_NOT_AVAILABLE;

  bool testATImpl(uint32_t timeout_ms) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      thisModem().sendAT(GF(""));
      if (thisModem().waitResponse(200) == 1) { return true; }
      delay(100);
    }
    return false;
  }

  int8_t waitResponseImpl(uint32_t timeout_ms, String& data, GsmConstStr r1,
                          GsmConstStr r2, GsmConstStr r3, GsmConstStr r4,
                          GsmConstStr r5, GsmConstStr r6, GsmConstStr r7,
                          GsmConstStr r8) {
    data.reserve(64);

    // put the possible responses into an array so we can loop through them
    const GsmConstStr responses[8] = {r1, r2, r3, r4, r5, r6, r7, r8};

#ifdef TINY_GSM_DEBUG_DEEP
    DBG(GF("r1 <"), r1 ? r1 : GF("NULL"), GF("> r2 <"), r2 ? r2 : GF("NULL"),
        GF("> r3 <"), r3 ? r3 : GF("NULL"), GF("> r4 <"), r4 ? r4 : GF("NULL"),
        GF("> r5 <"), r5 ? r5 : GF("NULL"), GF("> r6 <"), r6 ? r6 : GF("NULL"),
        GF("> r7 <"), r7 ? r7 : GF("NULL"), GF("> r8 <"), r8 ? r8 : GF("NULL"),
        '>');
#endif
    bool hasExpectedResponse = false;  //< Flag indicating if there is at least
                                       // one expected response defined
    for (uint8_t i = 0; i < 8; i++) {
      if (responses[i]) {
        hasExpectedResponse = true;
        break;
      }
    }

    uint8_t  index = 0;  //< The index of the current response being checked
    uint32_t startMillis = millis();  //< The overall timeout timer start
    uint32_t lastCharMillis =
        startMillis;  //< The timestamp of the last character read
    uint32_t graceStartMillis =
        0;  //< The timestamp when the grace period started
    bool responseStarted = false;  //< Flag indicating if a response has started
    bool inGracePeriod =
        false;  //< Flag indicating if we are in the grace period
    bool firstRead =
        true;  //< Flag indicating if this is the first character read
    uint32_t now = startMillis;
#if defined TINY_GSM_DEBUG
    const String verbosePrefix1 = String(GFP(ModemConfig::GSM_VERBOSE));
    const String verbosePrefix2 = String(GFP(ModemConfig::GSM_VERBOSE_2));
    // check how long the new line is
    // should be either 1 ('\r' or '\n') or 2 ("\r\n"))
    const String atnlString  = String(GFP(ModemConfig::GSM_NL));
    const int    len_atnl    = atnlString.length();
    const char   last_atnl_c = len_atnl > 0 ? atnlString[len_atnl - 1] : '\n';
#endif

    while (true) {
      uint32_t elapsed        = now - startMillis;
      uint32_t readTimeout_ms = 0;

      // The conditionals below are to decide what phase of our wait we are in
      // and how long to wait for the next character
      if (elapsed < timeout_ms) {
        // If we're still within the overall timeout, just update the time to
        // wait for the next character read based on what's left of the overall
        // timeout
        readTimeout_ms = timeout_ms - elapsed;
      } else if (timeout_ms == 0) {
        // If the overall timeout is set to 0, consume only data that is already
        // buffered without waiting for another character.
        if (!firstRead && thisModem().stream.available() <= 0) { break; }
      } else if (firstRead) {
        // If we just got the first character, but we're now outside of the
        // overall timeout, reserve the initial read attempt if setup consumed a
        // very short timeout before the loop began.
      } else {
        if (!hasExpectedResponse || !responseStarted || !data.length()) {
          // if the timeout has been exceeded and we got nothing, break out of
          // the loop
          break;
        }

        uint32_t interCharElapsed = now - lastCharMillis;
        if (interCharElapsed >= TINY_GSM_INTERCHAR_TIMEOUT_MS) {
          // if the inter-character timeout has *also* been exceeded, break out
          // of the loop
          break;
        }

        if (!inGracePeriod) {
          // if we hadn't already set the grace period flag, set it
          inGracePeriod    = true;
          graceStartMillis = now;
        }

        uint32_t graceElapsed = now - graceStartMillis;
        // if the grace period has been exceeded, break out of the loop
        if (graceElapsed >= TINY_GSM_RESPONSE_MAX_GRACE_MS) { break; }

        // update the amount of time to wait for the next character read based
        // on the remaining grace period and inter-character timeout
        uint32_t graceRemaining = TINY_GSM_RESPONSE_MAX_GRACE_MS - graceElapsed;
        readTimeout_ms = TINY_GSM_INTERCHAR_TIMEOUT_MS - interCharElapsed;
        if (readTimeout_ms > graceRemaining) {
          readTimeout_ms = graceRemaining;
        }
      }

      // we're about to attempt to read the first character, so unset the flag
      firstRead = false;
      int a     = thisModem().streamTimedRead(now, readTimeout_ms);
      if (a < 0) {
        now = millis();
        if (timeout_ms > 0 && !inGracePeriod &&
            now - startMillis >= timeout_ms) {
          // if we got no return from the read (ie, a  -1 response) and the
          // overall timeout has been exceeded, continue to the next iteration.
          // In the next iteration, the grace period will be checked and action
          // taken accordingly.
          continue;
        }
        // but if we're still within the overall timeout or in the grace period,
        // break out of the loop once we can't get a character from the read
        break;
      }

      // In this inner loop, process up to 16 buffered characters before
      // checking the clock again. This reduces timer overhead without allowing
      // continuous input to postpone timeout checks indefinitely.
      int bufferedChars = thisModem().stream.available();
      if (bufferedChars > 15) { bufferedChars = 15; }
      bool receivedResponseData = false;
      while (true) {
        if (a > 0) {  // Skip 0x00 bytes, just in case
          receivedResponseData = true;
          // append the character to the response data string being assembled
          data += static_cast<char>(a);
          // loop through the possible responses and see if we have a match
          for (uint8_t i = 0; i < 8; i++) {
            if (responses[i] && data.endsWith(responses[i])) {
              index = i + 1;
              goto finish;
            }
          }
#if defined TINY_GSM_DEBUG
          // check the verbose prefixes for a match
          if ((data.endsWith(verbosePrefix1)) ||
              (data.endsWith(verbosePrefix2))) {
            // Read out the verbose message, until the last character of the
            // new line
            data += thisModem().stream.readStringUntil(last_atnl_c);
#ifdef TINY_GSM_DEBUG_DEEP
            data.trim();
            DBG(GF("Verbose details <<<"), data, GF(">>>"));
#endif
            data = "";
            goto finish;
          }
#endif
          if (thisModem().handleURCs(data)) { data = ""; }
        }

        // If we run out of buffered characters, break the loop
        if (bufferedChars-- <= 0) { break; }
        // Try to get the next character, break if there isn't one
        a = thisModem().stream.read();
        if (a < 0) { break; }

        // Breaking from this inner loop will dump us in the outer loop which
        // will try to read with the timeout and grace period checks again.
      }

      // Update the clock after the inner loop finishes
      now = millis();
      if (receivedResponseData) {
        responseStarted = true;
        lastCharMillis  = now;
      }
    }
  finish:
#ifdef TINY_GSM_DEBUG_DEEP
    data.replace("\r", "←");
    data.replace("\n", "↓");
#endif
    if (!index) {
      data.trim();
      if (data.length()) { DBG("### Unhandled:", data); }
      data = "";
    } else {
#ifdef TINY_GSM_DEBUG_DEEP
      DBG('<', index, '>', data);
#endif
    }
    return index;
  }

  String getConfiguredModemImpl() {
    return String(GFP(ModemConfig::MODEM_MANUFACTURER)) + ' ' +
        String(GFP(ModemConfig::MODEM_MODEL));
  }

  String getModemInfoImpl() {
    thisModem().sendAT('I');  // 3GPP TS 27.007
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return ""; }
    thisModem().cleanResponseString(res);
    return res;
  }

  String getModemNameImpl() {
    String manufacturer = getModemManufacturer();
    String model        = getModemModel();
    String name         = manufacturer + ' ' + model;
    DBG("### Modem:", name);
    return name;
  }

  // Gets the modem manufacturer
  String getModemManufacturerImpl() {
    thisModem().sendAT(GF("+CGMI"));  // 3GPP TS 27.007 standard
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) {
      return String(GFP(ModemConfig::MODEM_MANUFACTURER));
    }
    thisModem().cleanResponseString(res);
    return res;
  }

  // Gets the modem hardware version
  String getModemModelImpl() {
    thisModem().sendAT(GF("+CGMM"));  // 3GPP TS 27.007 standard
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) {
      return String(GFP(ModemConfig::MODEM_MODEL));
    }
    thisModem().cleanResponseString(res);
    return res;
  }

  // Gets the modem firmware version
  String getModemRevisionImpl() {
    thisModem().sendAT(GF("+CGMR"));  // 3GPP TS 27.007 standard
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return "unknown"; }
    thisModem().cleanResponseString(res);
    return res;
  }

  // Gets the modem serial number
  String getModemSerialNumberImpl() {
    thisModem().sendAT(GF("+CGSN"));  // 3GPP TS 27.007 standard
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return "unknown"; }
    thisModem().cleanResponseString(res);
    return res;
  }

  bool factoryDefaultImpl() {
    thisModem().sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    thisModem().waitResponse();
    thisModem().sendAT(GF("+IPR=0"));  // Auto-baud
    thisModem().waitResponse();
    thisModem().sendAT(GF("&W"));  // Write configuration
    return thisModem().waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  bool radioOffImpl() {
    if (!thisModem().setPhoneFunctionality(0)) { return false; }
    delay(3000);
    return true;
  }

  bool sleepEnableImpl(bool enable) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool setPhoneFunctionalityImpl(uint8_t fun,
                                 bool    reset) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 protected:
  regStatusType getRegistrationStatusImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  // Gets the modem's registration status via CREG/CGREG/CEREG
  // CREG = Generic network registration
  // CGREG = GPRS service registration
  // CEREG = EPS registration for LTE modules
  int8_t getRegistrationStatusXREG(const char* regCommand) {
    thisModem().sendAT('+', regCommand, '?');
    // check for any of the three for simplicity
    int8_t resp = thisModem().waitResponse(GF("+CREG:"), GF("+CGREG:"),
                                           GF("+CEREG:"),
                                           GFP(ModemConfig::GSM_ERROR));
    if (resp != 1 && resp != 2 && resp != 3) { return -1; }
    thisModem().streamSkipUntil(','); /* Skip format (0) */
    const String atnlString = String(GFP(ModemConfig::GSM_NL));
    const char   nl_c       = atnlString.length() ? atnlString[0] : '\r';
    int          status     = thisModem().streamGetIntBefore(nl_c);
    thisModem().waitResponse();
    return status;
  }

  bool waitForNetworkImpl(uint32_t timeout_ms, bool check_signal) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      if (check_signal) { thisModem().getSignalQuality(); }
      if (thisModem().isNetworkConnected()) { return true; }
      delay(250);
    }
    return false;
  }

  // Gets signal quality report according to 3GPP TS command AT+CSQ
  int16_t getSignalQualityImpl() {
    thisModem().sendAT(GF("+CSQ"));
    if (thisModem().waitResponse(GF("+CSQ:")) != 1) { return -9999; }
    int16_t res = thisModem().streamGetIntBefore(',');
    if (thisModem().waitResponse() != 1 || res < 0 || res > 32) {
      return -9999;
    }
    return res;
  }

  String getLocalIPImpl() {
    // AT+CGPADDR=<cid> where cid is the context id, which is usually 1 for the
    // first context
    thisModem().sendAT(GF("+CGPADDR=1"));
    if (thisModem().waitResponse(GF("+CGPADDR:")) != 1) { return ""; }
    thisModem().streamSkipUntil(',');  // Skip context id
    String res = thisModem().stream.readStringUntil('\r');
    if (thisModem().waitResponse() != 1) { return ""; }
    return res;
  }
};

#endif  // SRC_TINYGSMMODEM_TPP_
