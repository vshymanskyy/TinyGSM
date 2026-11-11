/**
 * @file       TinyGsmClientEspressif.h
 * @brief      Shared Espressif modem client base types and helpers.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 *
 * @defgroup espressif_at Espressif AT
 * @brief Manufacturer: Espressif. Models: ESP8266, ESP32.
 * @ingroup group_supported_modules
 */

#ifndef SRC_TINYGSMCLIENTESPRESSIF_H_
#define SRC_TINYGSMCLIENTESPRESSIF_H_
#pragma message("TinyGSM:  TinyGsmClientEspressif")

#include "mixins/TinyGsmModem.tpp"
#include "mixins/TinyGsmWifi.tpp"

/// The length of a SSL/TLS certificate filename, not including the namespace.
/// This is a dot and a single digit number (0 or 1).
#define TINY_GSM_ESP_SSL_FILENAME_SIZE 2

/**
 * @brief Parent class for the Espressif ESP8266 and ESP32 modules
 *
 * @tparam EspressifType The derived class type (ESP8266 or ESP32)
 * @tparam EspressifModemConfig The modem config type (ESP8266 or ESP32)
 */
template <class EspressifType, class EspressifModemConfig>
class TinyGsmEspressif
    : public TinyGsmModem<EspressifType, EspressifModemConfig>,
      public TinyGsmWifi<EspressifType> {
  friend class TinyGsmModem<EspressifType, EspressifModemConfig>;
  friend class TinyGsmWifi<EspressifType>;

 public:
  using ModemConfig = EspressifModemConfig;

  /*
   * CRTP Helper
   */
 protected:
  inline const EspressifType& thisModem() const {
    return static_cast<const EspressifType&>(*this);
  }
  inline EspressifType& thisModem() {
    return static_cast<EspressifType&>(*this);
  }
  ~TinyGsmEspressif() {}

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmEspressif(Stream& stream) : stream(stream) {}

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientEspressif"));

    bool success = true;

    if (!thisModem().testAT()) { return false; }
    if (pin && strlen(pin) > 0) {
      DBG("Espressif modules do not use an unlock pin!");
    }
    thisModem().sendAT(GF("E0"));  // Echo Off
    success &= thisModem().waitResponse() == 1;

#ifdef TINY_GSM_DEBUG
    thisModem().sendAT(GF("+SYSLOG=1"));  // turn on verbose error codes
#else
    thisModem().sendAT(GF("+SYSLOG=0"));  // turn off error codes
#endif
    thisModem().waitResponse();

    thisModem().sendAT(GF("+CIPMUX=1"));  // Enable Multiple Connections
    success &= thisModem().waitResponse() == 1;
    thisModem().sendAT(GF("+CWMODE=1"));  // Put into "station" mode
    if (thisModem().waitResponse() != 1) {
      thisModem().sendAT(
          GF("+CWMODE_CUR=1"));  // Attempt "current" station mode command
                                 // for some firmware variants if needed
      success &= thisModem().waitResponse() == 1;
    }
    thisModem().sendAT(
        GF("+CIPDINFO=0"));  // do not show the remote host and port in “+IPD”
                             // and “+CIPRECVDATA” messages.
    success &= thisModem().waitResponse() == 1;
    DBG(GF("### Modem:"), thisModem().getModemName());
    return success;
  }

 protected:
  bool setBaudImpl(uint32_t baud) {
    thisModem().sendAT(GF("+UART_CUR="), baud, ",8,1,0,0");
    bool res = thisModem().waitResponse() == 1;
    if (!res) {
      thisModem().sendAT(GF("+UART="), baud,
                         ",8,1,0,0");  // Really old firmwares might need this
      // if (thisModem().waitResponse() != 1) {
      //   thisModem().sendAT(GF("+IPR="), baud);  // First release firmwares
      //   might need this
      return thisModem().waitResponse() == 1;
      // }
    }
    return res;
  }

  bool setDefaultBaudImpl(uint32_t baud) {
    thisModem().sendAT(GF("+UART_DEF="), baud, ",8,1,0,0");
    return thisModem().waitResponse() == 1;
  }

  String getModemInfoImpl() {
    thisModem().sendAT(GF("+GMR"));
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return ""; }
    thisModem().cleanResponseString(res);
    return res;
  }

  // Gets the modem hardware version
  String getModemManufacturerImpl() {
    return String(GFP(ModemConfig::MODEM_MANUFACTURER));
  }

  // Gets the modem hardware version
  String getModemModelImpl() {
    String model = String(GFP(ModemConfig::MODEM_MODEL));
    thisModem().sendAT(GF("+GMR"));
    thisModem().streamSkipUntil('\n');  // skip the AT version
    thisModem().streamSkipUntil('\n');  // skip the SDK version
    thisModem().streamSkipUntil('\n');  // skip the compile time
    // read the hardware from the Bin version
    thisModem().streamSkipUntil('(');  // skip the text "Bin version"
    String wroom = stream.readStringUntil(
        ')');  // read the WRoom version in the parenthesis
    // skip the bin version itself while waiting for the ending OK
    if (thisModem().waitResponse(1000L) == 1) {  // wait for the ending OK
      wroom.trim();
      if (wroom.length() > 0) { return wroom; }
    }
    return model;
  }

  // Gets the modem firmware version
  String getModemRevisionImpl() {
    thisModem().sendAT(GF("+GMR"));  // GMR instead of CGMR
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return ""; }
    thisModem().cleanResponseString(res);
    return res;
  }

  // Gets the modem serial number
  // For Espressif modules, this is the MAC address of the WiFi interface
  String getModemSerialNumberImpl() {
    thisModem().sendAT(GF("+CIPSTAMAC?"));
    String res;
    if (thisModem().waitResponse(1000L, res) != 1) { return ""; }
    thisModem().cleanResponseString(res);
    return res;
  }

  bool factoryDefaultImpl() {
    thisModem().sendAT(GF("+RESTORE"));
    return thisModem().waitResponse() == 1;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin) {
    if (!thisModem().testAT()) { return false; }
    thisModem().sendAT(GF("+RST"));
    if (thisModem().waitResponse(10000L) != 1) { return false; }
    if (thisModem().waitResponse(10000L, GF("ready\r\n")) != 1) {
      return false;
    }
    delay(850);
    return thisModem().init(pin);
  }

  bool powerOffImpl() {
    // AT+GSLP=<time>
    // <time>: the duration when the device stays in Deep-sleep. Unit:
    // millisecond. When the time is up, the device automatically wakes up,
    // calls Deep-sleep wake stub, and then proceeds to load the application.
    //   For ESP32 devices:
    //     0 means restarting right now [ie, **not sleeping**]
    //     the maximum Deep-sleep time is about 28.8 days (2 31-1 milliseconds)
    //   For ESP8266 devices:
    //     0 means staying in Deep-sleep mode forever
    //     the maximum Deep-sleep time is about 3 hours (due to hardware
    //     limitation, more time will lead to setting failure or internal time
    //     overflow)

    thisModem().sendAT(
        GF("+GSLP=0"));  // Power down indefinitely - until manually reset!
    return thisModem().waitResponse() == 1;
  }

  bool radioOffImpl() TINY_GSM_ATTR_NOT_IMPLEMENTED;

  bool sleepEnableImpl(bool enable) {
    // AT+SLEEP=<sleep mode>
    // <sleep mode>:
    //   0: Disable the sleep mode.
    //   1: Modem-sleep DTIM mode. RF will be periodically closed according to
    //   AP DTIM.
    //   2: Light-sleep mode. CPU will automatically sleep and RF will be
    //   periodically closed according to listen interval set by AT+CWJAP.
    //   3: Modem-sleep listen interval mode. RF will be periodically closed
    //   according to listen interval set by AT+CWJAP.
    thisModem().sendAT(GF("+SLEEP="), enable ? 2 : 0);
    return thisModem().waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun,
                                 bool    reset) TINY_GSM_ATTR_NOT_IMPLEMENTED;

  /*
   * Generic network functions
   */
 protected:
  int16_t getSignalQualityImpl() {
    thisModem().sendAT(GF("+CWJAP?"));
    int8_t res1 = thisModem().waitResponse(GF("No AP"), GF("+CWJAP:"),
                                           GFP(ModemConfig::GSM_OK),
                                           GFP(ModemConfig::GSM_ERROR));
    if (res1 != 2) {
      thisModem().waitResponse();
      thisModem().sendAT(GF("+CWJAP_CUR?"));  // attempt "current" as used by
                                              // some Non-OS firmware versions
      res1 = thisModem().waitResponse(GF("No AP"), GF("+CWJAP_CUR:"),
                                      GFP(ModemConfig::GSM_OK),
                                      GFP(ModemConfig::GSM_ERROR));
      if (res1 != 2) {
        thisModem().waitResponse();
        return -9999;
      }
    }
    thisModem().streamSkipUntil(',');  // Skip SSID
    thisModem().streamSkipUntil(',');  // Skip BSSID/MAC address
    thisModem().streamSkipUntil(',');  // Skip Chanel number
    int16_t res2 = thisModem().streamGetIntBefore(',');  // Read RSSI
    thisModem().streamSkipUntil('\n');                   // Skip the rest
    if (thisModem().waitResponse() != 1 || res2 == -9999) { return -9999; }
    return res2;
  }

  String getLocalIPImpl() {
    // attempt with and without 'current' flag
    thisModem().sendAT(GF("+CIPSTA?"));
    thisModem().waitResponse(GF("ERROR"), GF("+CIPSTA:"));
    String res2 = stream.readStringUntil('\n');
    res2.replace("ip:", "");  // newer firmwares have this
    res2.replace("\"", "");
    res2.trim();
    thisModem().waitResponse();
    return res2;
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
  // No functions of this type supported in this parent template


  /*
   * Wifi functions
   */
 protected:
  bool networkConnectImpl(const char* ssid, const char* pwd) {
    // attempt first without than with the 'current' flag used in some firmware
    // versions
    // AT+CWJAP=[<ssid>],[<pwd>][,<bssid>][,<pci_en>][,<reconn_interval>][,<listen_interval>][,<scan_mode>][,<jap_timeout>][,<pmf>]
    // Response:
    // WIFI CONNECTED
    // WIFI GOT IP
    // OK
    // -- or --
    // +CWJAP:<error code>
    // ERROR
    // <error code>: (for reference only)
    // 1: connection timeout.
    // 2: wrong password.
    // 3: cannot find the target AP.
    // 4: connection failed.
    // others: unknown error occurred.
    thisModem().sendAT(GF("+CWJAP=\""), ssid, GF("\",\""), pwd, '"');
    if (thisModem().waitResponse(30000L, GFP(ModemConfig::GSM_OK),
                                 GF("FAIL\r\n"),
                                 GFP(ModemConfig::GSM_ERROR)) != 1) {
      thisModem().sendAT(GF("+CWJAP_CUR=\""), ssid, GF("\",\""), pwd, '"');
      if (thisModem().waitResponse(30000L, GFP(ModemConfig::GSM_OK),
                                   GF("FAIL\r\n"),
                                   GFP(ModemConfig::GSM_ERROR)) != 1) {
        return false;
      }
    }

    return true;
  }

  bool networkDisconnectImpl() {
    thisModem().sendAT(GF("+CWQAP"));
    bool retVal = thisModem().waitResponse(10000L) == 1;
    thisModem().waitResponse(GF("WIFI DISCONNECT"));
    return retVal;
  }


  /*
   * Cellular data functions
   */
  // No functions of this type supported

  /*
   * SIM card functions
   */
  // No functions of this type supported

  /*
   * Audio functions
   */
  // No functions of this type supported

  /*
   * Text messaging (SMS) functions
   */
  // No functions of this type supported

  /*
   * GSM location functions
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
  // No functions of this type supported

  /*
   * Temperature functions
   */
  // No functions of this type supported

  /*
   * Client-related functions
   */
 protected:
  bool modemStop(uint8_t mux, uint32_t maxWaitMs) {
    if (mux >= EspressifType::TcpConfig::kMuxCount ||
        !thisModem().sockets[mux]) {
      return false;
    }
    // same command for SSL and not SSL
    thisModem().sendAT(GF("+CIPCLOSE="), mux);
    return thisModem().waitResponse(maxWaitMs) == 1;
  }

  /*
   * Utilities
   */

 public:
  /// Stream used to communicate with the modem.
  Stream& stream;
};

#endif  // SRC_TINYGSMCLIENTESPRESSIF_H_
