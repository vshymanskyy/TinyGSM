/**
 * @file       TinyGsmClientSIM5360.h
 * @brief      SIM5360 modem client and modem-trait definitions.
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */
/**
 * @defgroup simcom_sim5360 SIMCom SIM53xx and SIM71xx
 * @ingroup group_simcom
 * @brief Manufacturer: SIMCom. Models: SIM5320, SIM5360, SIM5300, SIM7100.
 *
 * # Connection Information
 *
 * - TCP sockets:
 *   - 10
 *   - SSL is not supported
 * - Socket Buffering:
 *   - The modem has an internal buffer for incoming data.
 * - Socket Numbering:
 *   - The modem respects user-specified multiplexing channel
 * numbers/identifiers for socket connections.
 */

#ifndef SRC_TINYGSMCLIENTSIM5360_H_
#define SRC_TINYGSMCLIENTSIM5360_H_
#pragma message("TinyGSM:  TinyGsmClientSIM5360")

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmCellularData.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmGPS.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmBattery.tpp"
#include "TinyGsmTemperature.tpp"

/// Registration status
/// @ingroup simcom_sim5360
enum class Sim5360RegStatus {
  REG_NO_RESULT    = -1,  ///< No registration result
  REG_UNREGISTERED = 0,   ///< Not registered on the network
  REG_SEARCHING    = 2,   ///< Searching for network
  REG_DENIED       = 3,   ///< Registration denied
  REG_OK_HOME      = 1,   ///< Registered on the home network
  REG_OK_ROAMING   = 5,   ///< Registered on a roaming network
  REG_UNKNOWN      = 4,   ///< Unknown registration status
};

/// Basic modem configurations for the SIMCom SIM5360 modem family
/// @ingroup simcom_sim5360
struct TinyGsmSim5360ModemConfig
    : public TinyGsmModemConfigPreset<Sim5360RegStatus> {
  /// The modem manufacturer
  static constexpr char MODEM_MANUFACTURER[] TINY_GSM_PROGMEM = "SIMCom";
#if defined(TINY_GSM_MODEM_SIM5320)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM5320";
#elif defined(TINY_GSM_MODEM_SIM5300)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM5300";
#elif defined(TINY_GSM_MODEM_SIM7100)
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM7100";
#else
  /// The modem model
  static constexpr char MODEM_MODEL[] TINY_GSM_PROGMEM = "SIM5360";
#endif
};

constexpr char TinyGsmSim5360ModemConfig::MODEM_MANUFACTURER[]
    __attribute__((weak));
constexpr char TinyGsmSim5360ModemConfig::MODEM_MODEL[] __attribute__((weak));

/**
 * @brief TCP behavior and limits for the SIMCom SIM5360 modem family.
 *
 * The SIM5360 can send up to 1500 bytes at a time with AT+CIPSEND
 * @ingroup simcom_sim5360
 */
struct TinyGsmSim5360TcpConfig
    : public TinyGsmTcpConfigPreset<
          /*bufferMode*/ TinyGsmTcpBufferMode::BufferReadAndCheckSize,
          /*muxMode*/ TinyGsmTcpMuxMode::Static,
          /*muxCount*/ 10,
          /*sendMaxSize*/ 1500,  // default
          /*connectTimeoutS*/ 15> {};

/// Class for the SIMCOM SIM5360, SIM5300, SIM5320, and SIM7100
/// @ingroup simcom_sim5360
class TinyGsmSim5360
    : public TinyGsmModem<TinyGsmSim5360, TinyGsmSim5360ModemConfig>,
      public TinyGsmCellularData<TinyGsmSim5360>,
      public TinyGsmTCP<TinyGsmSim5360, TinyGsmSim5360TcpConfig>,
      public TinyGsmSMS<TinyGsmSim5360>,
      public TinyGsmGSMLocation<TinyGsmSim5360>,
      public TinyGsmGPS<TinyGsmSim5360>,
      public TinyGsmTime<TinyGsmSim5360>,
      public TinyGsmNTP<TinyGsmSim5360>,
      public TinyGsmBattery<TinyGsmSim5360>,
      public TinyGsmTemperature<TinyGsmSim5360> {
  friend class TinyGsmModem<TinyGsmSim5360, TinyGsmSim5360ModemConfig>;
  friend class TinyGsmCellularData<TinyGsmSim5360>;
  friend class TinyGsmTCP<TinyGsmSim5360, TinyGsmSim5360TcpConfig>;
  friend class GsmClient<TinyGsmSim5360, TinyGsmSim5360TcpConfig>;
  friend class TinyGsmSMS<TinyGsmSim5360>;
  friend class TinyGsmGSMLocation<TinyGsmSim5360>;
  friend class TinyGsmGPS<TinyGsmSim5360>;
  friend class TinyGsmTime<TinyGsmSim5360>;
  friend class TinyGsmNTP<TinyGsmSim5360>;
  friend class TinyGsmBattery<TinyGsmSim5360>;
  friend class TinyGsmTemperature<TinyGsmSim5360>;

 public:
  using ModemConfig = TinyGsmSim5360ModemConfig;
  using TcpConfig   = TinyGsmSim5360TcpConfig;

  /*
   * Inner Client
   */
 public:
  /// Inner client
  /// @ingroup simcom_sim5360
  class GsmClientSim5360
      : public GsmClient<TinyGsmSim5360, TinyGsmSim5360TcpConfig> {
    friend class TinyGsmSim5360;

   public:
    using GsmClient<TinyGsmSim5360, TinyGsmSim5360TcpConfig>::connect;
    using GsmClient<TinyGsmSim5360, TinyGsmSim5360TcpConfig>::stop;
    using TcpConfig = TinyGsmSim5360TcpConfig;

    /*
     * Client constructors and initialization
     */
    /**
     * @brief Create a new TCP client.
     * @warning You must call the init() method before attempting to use a
     * client created with this constructor.
     */
    GsmClientSim5360() {
      is_secure = false;
    }
    /**
     * @brief Create a new TCP client and bind it to a modem and optionally a
     * multiplexing channel.
     * @param modem Modem instance used by this client.
     * @param mux The **zero-indexed** position of this client in the
     * corresponding modem's socket array.  This is identical to the identifier
     * the modem uses internally to identify the socket for the SIM5360.
     *
     * @note The SIM5360 and similar variants allow you choose the multiplexing
     * channel number, but if the input mux channel number is already in use and
     * other mux channels are available, this library will select the next
     * available one.  Use the getMux() function to get the assigned
     * multiplexing channel number after a successful connection.
     */
    explicit GsmClientSim5360(TinyGsmSim5360& modem, uint8_t mux = 0)
        : GsmClient<TinyGsmSim5360, TinyGsmSim5360TcpConfig>(modem, mux) {
      is_secure = false;
      init(&modem, mux);
    }

    /*
     * Arduino Client interface
     */
   public:
    TINY_GSM_STATIC_TCP_CONNECT

    /*
     * Extended Client API
     */
    // No extra extended API functions
  };

  /*
   * Inner Secure Client
   */
  // NOT SUPPORTED

  /*
   * GSM Modem Constructor
   */
 public:
  /**
   * @brief Construct a modem wrapper around a stream transport.
   * @param stream Stream used to communicate with the modem.
   */
  explicit TinyGsmSim5360(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientSIM5360"));

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

    // Disable time and time zone URC's
    sendAT(GF("+CTZR=0"));
    if (waitResponse(10000L) != 1) { return false; }

    // Enable automatic time zome update
    sendAT(GF("+CTZU=1"));
    if (waitResponse(10000L) != 1) { return false; }

    SimStatus ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the
    // sim
    if (ret != SIM_READY && pin != nullptr && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    } else {
      // if the sim is ready, or it's locked but no pin has been provided,
      // return true
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  bool factoryDefaultImpl() {  // these commands aren't supported
    return false;
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin) {
    if (!testAT()) { return false; }
    sendAT(GF("+REBOOT"));
    // Should return an 'OK' after reboot command is sent
    if (waitResponse(10000L) != 1) { return false; }
    // After booting, modem sends out messages as each of its
    // internal modules loads.  The final message is "PB DONE".
    if (waitResponse(40000L, GF("PB DONE")) != 1) { return false; }
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOF"));
    return waitResponse() == 1;
  }

  bool radioOffImpl() {
    if (!setPhoneFunctionality(4)) { return false; }
    delay(3000);
    return true;
  }

  bool sleepEnableImpl(bool enable) {
    sendAT(GF("+CSCLK="), enable);
    return waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 protected:
  Sim5360RegStatus getRegistrationStatusImpl() {
    return static_cast<Sim5360RegStatus>(getRegistrationStatusXREG("CGREG"));
  }

  bool isNetworkConnectedImpl() {
    Sim5360RegStatus s = this->getRegistrationStatus();
    return (s == Sim5360RegStatus::REG_OK_HOME ||
            s == Sim5360RegStatus::REG_OK_ROAMING);
  }

  String getLocalIPImpl() {
    sendAT(GF("+IPADDR"));  // Inquire Socket PDP address
    // sendAT(GF("+CGPADDR=1"));  // Show PDP address
    String res;
    if (waitResponse(10000L, res) != 1) { return ""; }
    cleanResponseString(res);
    return res;
  }

  /*
   * Network mode / type / technology functions
   */
 public:
  /**
   * @brief Get the available network modes of the modem.
   * @return A string representing the available network modes.
   */
  String getNetworkModes() {
    sendAT(GF("+CNMP=?"));
    if (waitResponse(GF("+CNMP:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    return res;
  }

  /**
   * @brief Get the current network mode of the modem.
   * @return The current network mode as an integer.
   */
  int16_t getNetworkMode() {
    sendAT(GF("+CNMP?"));
    if (waitResponse(GF("+CNMP:")) != 1) { return false; }
    int16_t mode = streamGetIntBefore('\n');
    waitResponse();
    return mode;
  }

  /**
   * @brief Set the network mode of the modem.
   * @param mode The network mode to set.
   * @return True if the operation was successful, false otherwise.
   */
  bool setNetworkMode(uint8_t mode) {
    sendAT(GF("+CNMP="), mode);
    return waitResponse() == 1;
  }

  /*
   * Secure socket layer (SSL) certificate management functions
   */
  // No functions of this type supported

  /*
   * Wifi functions
   */
  // No functions of this type supported

  /*
   * Cellular data functions
   */
 protected:
  bool dataConnectImpl(const char* apn, const char* user, const char* pwd) {
    dataDisconnect();  // Make sure we're not connected first

    // Define the PDP context

    // The CGDCONT commands set up the "external" PDP context

    // Set the external authentication
    if (user && strlen(user) > 0) {
      sendAT(GF("+CGAUTH=1,0,\""), user, GF("\",\""), pwd, '"');
      waitResponse();
    }

    // Define external PDP context 1
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"', ",\"0.0.0.0\",0,0");
    waitResponse();

    // The CGSOCKCONT commands define the "embedded" PDP context for TCP/IP

    // Define the socket PDP context
    sendAT(GF("+CGSOCKCONT=1,\"IP\",\""), apn, '"');
    waitResponse();

    // Set the embedded authentication
    if (user && strlen(user) > 0) {
      sendAT(GF("+CSOCKAUTH=1,1,\""), user, GF("\",\""), pwd, '"');
      waitResponse();
    }

    // Set active PDP context's profile number
    // This ties the embedded TCP/IP application to the external PDP context
    sendAT(GF("+CSOCKSETPN=1"));
    waitResponse();

    // Configure TCP parameters

    // Select TCP/IP application mode (command mode)
    sendAT(GF("+CIPMODE=0"));
    waitResponse();

    // Set Sending Mode - send without waiting for peer TCP ACK
    sendAT(GF("+CIPSENDMODE=0"));
    waitResponse();

    // Configure socket parameters
    // AT+CIPCCFG= <NmRetry>, <DelayTm>, <Ack>, <errMode>, <HeaderType>,
    //            <AsyncMode>, <TimeoutVal>
    // NmRetry = number of retransmission to be made for an IP packet
    //         = 10 (default)
    // DelayTm = number of milliseconds to delay before outputting received
    // data
    //          = 0 (default)
    // Ack = sets whether reporting a string "Send ok" = 0 (don't report)
    // errMode = mode of reporting error result code = 0 (numberic values)
    // HeaderType = which data header of receiving data in multi-client mode
    //            = 1 (+RECEIVE,<link num>,<data length>)
    // AsyncMode = sets mode of executing commands
    //           = 0 (synchronous command executing)
    // TimeoutVal = minimum retransmission timeout in milliseconds
    sendAT(GF("+CIPCCFG=10,0,0,0,1,0,"),
           TcpConfig::kRetransmissionTimeoutS * 1000L);
    if (waitResponse() != 1) { return false; }

    // Configure timeouts for opening and closing sockets
    // AT+CIPTIMEOUT=<netopen_timeout>, <cipopen_timeout>, <cipsend_timeout>
    sendAT(GF("+CIPTIMEOUT="), 75000, ',', TcpConfig::kConnectTimeoutS * 1000L,
           ',', TcpConfig::kSendTimeoutS * 1000L);
    waitResponse();

    // Start the socket service

    // This activates and attaches to the external PDP context that is tied
    // to the embedded context for TCP/IP (ie AT+CGACT=1,1 and AT+CGATT=1)
    // Response may be an immediate "OK" followed later by "+NETOPEN: 0".
    // We to ignore any immediate response and wait for the
    // URC to show it's really connected.
    sendAT(GF("+NETOPEN"));
    if (waitResponse(75000L, GF("+NETOPEN: 0")) != 1) { return false; }

    return true;
  }

  bool dataDisconnectImpl() {
    // Close any open sockets
    for (uint8_t mux = 0; mux < TcpConfig::kMuxCount; mux++) {
      GsmClientSim5360* sock = sockets[mux];
      if (sock) { sock->stop(); }
    }

    // Stop the socket service
    // Note: all sockets should be closed first - on 3G/4G models the sockets
    // must be closed manually
    sendAT(GF("+NETCLOSE"));
    waitResponse(60000L, GF("+NETCLOSE: 0"));  // allowed to fail

    sendAT(GF("+CGATT=0"));  // Detach from Packet Domain service
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  bool isDataConnectedImpl() {
    sendAT(GF("+NETOPEN?"));
    // May return +NETOPEN: 1, 0.  We just confirm that the first number is 1
    if (waitResponse(GF("+NETOPEN: 1")) != 1) { return false; }
    waitResponse();

    sendAT(GF("+IPADDR"));  // Inquire Socket PDP address
    // sendAT(GF("+CGPADDR=1")); // Show PDP address
    if (waitResponse() != 1) { return false; }

    return true;
  }

  String getProviderImpl() {
    sendAT(GF("+CSPN?"));
    if (waitResponse(GF("+CSPN:")) != 1) { return ""; }
    streamSkipUntil('"'); /* Skip mode and format */
    String res = stream.readStringUntil('"');
    waitResponse();
    return res;
  }

  /*
   * SIM card functions
   */
 protected:
  // Gets the CCID of a sim card via AT+CCID
  String getSimCCIDImpl() {
    sendAT(GF("+CICCID"));
    if (waitResponse(GF("+ICCID:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  /*
   * Phone call functions
   */
  // No functions of this type supported

  /*
   * Audio functions
   */
  // No functions of this type supported

  /*
   * Text messaging (SMS) functions
   */
  // Follows all text messaging (SMS) functions as inherited from
  // TinyGsmSMS.tpp

  /*
   * GSM location functions
   */
  // SIM5360 and SIM7100 can return a GSM-based location from CLBS as per the
  // template; SIM5320 doesn't not appear to be able to

  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // enable GPS
  bool enableGPSImpl() {
    sendAT(GF("+CGPS=1"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  bool disableGPSImpl() {
    sendAT(GF("+CGPS=0"));
    if (waitResponse() != 1) { return false; }
    return true;
  }

  // get the RAW GPS output
  String getGPSrawImpl() {
    sendAT(GF("+CGPSINFO"));
    if (waitResponse(GF("+CGPSINFO:")) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.trim();
    return res;
  }

  // get GPS informations
  bool getGPSImpl(float* lat, float* lon, float* speed, float* alt, int* vsat,
                  int* usat, float* accuracy, int* year, int* month, int* day,
                  int* hour, int* minute, int* second) {
    sendAT(GF("+CGPSINFO"));
    if (waitResponse(GF("+CGPSINFO:")) != 1) { return false; }
    delay(30);

    float   ilat = 0;
    char    north;
    float   ilon = 0;
    char    east;
    float   ispeed       = 0;
    float   ialt         = 0;
    int16_t ivsat        = 0;
    int16_t iusat        = 0;
    int16_t iyear        = 0;
    int16_t imonth       = 0;
    int16_t iday         = 0;
    int16_t ihour        = 0;
    int16_t imin         = 0;
    float   secondWithSS = 0;

    ilat  = streamGetFloatBefore(',');  // Latitude in ddmm.mmmmmm
    north = stream.read();              // N/S Indicator, N=north or S=south
    streamSkipUntil(',');               // BEIDOU satellite valid numbers
    ilon = streamGetFloatBefore(',');   // Longitude in dddmm.mmmmmm
    east = stream.read();               // E/W Indicator, E=east or W=west
    streamSkipUntil(',');               // BEIDOU satellite valid numbers

    // Date. Output format is ddmmyy
    // Read the whole date portion into a single uint32_t, then parse it into
    // day, month, year.  By doing it this way, we avoid issues of attempting to
    // read a set number of characters for each portion of the date if the date
    // is missing or malformed.
    uint32_t idate = streamGetULBefore(',');
    iday           = static_cast<int16_t>(idate / 10000);  // Two digit day
    imonth = static_cast<int16_t>((idate / 100) % 100);    // Two digit month
    iyear  = static_cast<int16_t>(idate % 100);            // Two digit year

    // UTC Time. Output format is hhmmss.s
    // Again, read the whole time portion into a single float, then parse that
    // into the hour, minute, and second portions.
    float itime = streamGetFloatBefore(',');
    if (itime >= 0.0F) {
      uint32_t itime_int      = static_cast<uint32_t>(itime);
      float    itime_fraction = itime - static_cast<float>(itime_int);
      ihour = static_cast<int16_t>(itime_int / 10000);       // Two digit hour
      imin = static_cast<int16_t>((itime_int / 100) % 100);  // Two digit minute
      secondWithSS = static_cast<float>(itime_int % 100) + itime_fraction;
    }

    ialt   = streamGetFloatBefore(',');  // MSL Altitude. Unit is meters
    ispeed = streamGetFloatBefore(',');  // Speed Over Ground. Unit is knots.

    // Set pointers **even if the fix is not valid!!**
    // The module returns avlues even without a valid GNSS fix, so we set the
    // pointers regardless. Allow the user to decide whether to use the data
    // based on the fix status.
    if (lat != nullptr)
      *lat = (floor(ilat / 100) + fmod(ilat, 100.) / 60) *
          (north == 'N' ? 1 : -1);
    if (lon != nullptr)
      *lon = (floor(ilon / 100) + fmod(ilon, 100.) / 60) *
          (east == 'E' ? 1 : -1);
    if (speed != nullptr) *speed = ispeed;
    if (alt != nullptr) *alt = ialt;
    if (vsat != nullptr) *vsat = ivsat;
    if (usat != nullptr) *usat = iusat;
    if (accuracy != nullptr) *accuracy = -9999;
    if (iyear < 2000) iyear += 2000;
    if (year != nullptr) *year = iyear;
    if (month != nullptr) *month = imonth;
    if (day != nullptr) *day = iday;
    if (hour != nullptr) *hour = ihour;
    if (minute != nullptr) *minute = imin;
    if (second != nullptr) *second = static_cast<int>(secondWithSS);

    waitResponse();

#if 0
    DBG(GF("Latitude:"), String(ilat, 8), GF("\tLongitude:"), String(ilon, 8),
        GF("\tAltitude:"), String(ialt, 4));
    DBG(GF("VSAT:"), ivsat, GF("\tUSAT:"), iusat, GF("\tAccuracy:"), -9999);
    DBG(GF("Year:"), iyear, GF("\tMonth:"), imonth, GF("\tDay:"), iday);
    DBG(GF("Hour:"), ihour, GF("\tMinute:"), imin, GF("\tSecond:"),
        String(secondWithSS, 3));
#endif

    return ilat != -9999.0F;
  }

  /*
   * Time functions
   */
  // Follows all clock functions as inherited from TinyGsmTime.tpp

  /*
   * NTP server functions
   */
  // Follows all NTP server functions as inherited from TinyGsmNTP.tpp

  /*
   * BLE functions
   */
  // No functions of this type supported

  /*
   * Battery functions
   */
 protected:
  // SRGD Note:  Returns voltage in VOLTS instead of millivolts
  int16_t getBattVoltageImpl() {
    sendAT(GF("+CBC"));
    if (waitResponse(GF("+CBC:")) != 1) { return 0; }
    streamSkipUntil(',');  // Skip battery charge status
    streamSkipUntil(',');  // Skip battery charge level
    // get voltage in VOLTS
    float voltage = streamGetFloatBefore('\n');
    // Wait for final OK
    waitResponse();
    // Return millivolts
    uint16_t res = voltage * 1000;
    return res;
  }

  // SRGD Note:  Returns voltage in VOLTS instead of millivolts
  bool getBattStatsImpl(int8_t& chargeState, int8_t& percent,
                        int16_t& milliVolts) {
    sendAT(GF("+CBC"));
    if (waitResponse(GF("+CBC:")) != 1) { return false; }
    chargeState = streamGetIntBefore(',');
    percent     = streamGetIntBefore(',');
    // get voltage in VOLTS
    float voltage = streamGetFloatBefore('\n');
    milliVolts    = voltage * 1000;
    // Wait for final OK
    waitResponse();
    return true;
  }

  /*
   * Temperature functions
   */
 protected:
  // get temperature in degree celsius
  float getTemperatureImpl() {
    // Enable Temparature Reading
    sendAT(GF("+CMTE=1"));
    if (waitResponse() != 1) { return 0; }
    // Get Temparature Value
    sendAT(GF("+CMTE?"));
    if (waitResponse(GF("+CMTE:")) != 1) { return false; }
    float res = streamGetFloatBefore('\n');
    // Wait for final OK
    waitResponse();
    return res;
  }

  /*
   * Client-related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t /*static*/ mux,
                    int timeout_s) {
    if (!isValidMux(mux)) { return false; }
    // Make sure we'll be getting data manually on this connection
    sendAT(GF("+CIPRXGET=1"));
    if (waitResponse() != 1) { return false; }

    // Establish a connection in multi-socket mode
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    sendAT(GF("+CIPOPEN="), mux, ',', GF("\"TCP"), GF("\",\""), host, GF("\","),
           port);
    // The reply is +CIPOPEN: ## of socket created
    if (waitResponse(timeout_ms, GF("+CIPOPEN:")) != 1) { return false; }
    return true;
  }

  bool modemStop(uint8_t mux, uint32_t /*maxWaitMs*/) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPCLOSE="), mux);
    return waitResponse() == 1;  // should return within 1s
  }

  bool modemBeginSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    sendAT(GF("+CIPSEND="), mux, ',', (uint16_t)len);
    return waitResponse(GF(">")) == 1;
  }
  // Between the modemBeginSend and modemEndSend, modemSend calls:
  // stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  // stream.flush();
  size_t modemEndSend(size_t len, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    if (waitResponse(GF("+CIPSEND:")) != 1) { return 0; }
    int16_t ret_mux = streamGetIntBefore(',');  // check mux
    streamSkipUntil(',');                       // Skip requested bytes to send
    uint16_t sent = streamGetIntBefore('\n');   // check send length
    if (sent != len) { DBG("### Sent:", sent, "of", len, "on", mux); }
    if (isExpectedMux(ret_mux, mux)) { return sent; }
    return 0;
  }

  size_t modemRead(size_t size, uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    size_t len_read = 0;

#ifdef TINY_GSM_USE_HEX
    sendAT(GF("+CIPRXGET=3,"), mux, ',', (uint16_t)size);
#else
    sendAT(GF("+CIPRXGET=2,"), mux, ',', (uint16_t)size);
#endif
    if (waitResponse(GF("+CIPRXGET:")) != 1) { return 0; }

    streamSkipUntil(',');  // Skip Rx mode 2/normal or 3/HEX
    int16_t ret_mux      = streamGetIntBefore(',');  // mux/cid (connecion id)
    int16_t len_reported = streamGetIntBefore(',');
    //  ^^ Requested number of data bytes (1-1460 bytes) to be read
    int16_t len_remaining = streamGetIntBefore('\n');
    // ^^ The data length which not read in the buffer
    if (isValidMux(ret_mux)) {
      // Move the data to the socket buffer of the returned mux as long as the
      // returned mux is valid, even if it doesn't match the expected mux.
#ifdef TINY_GSM_USE_HEX
      len_read = moveCharsFromStreamToFifo<true>(ret_mux, len_reported);
#else
      len_read = moveCharsFromStreamToFifo(ret_mux, len_reported);
#endif
      // update the amount remaining for the returned mux, even if it doesn't
      // match the expected mux.
      sockets[ret_mux]->sock_available = len_remaining >= 0 ? len_remaining : 0;
    }
    waitResponse();  // ending OK; the waitResponse function will toss all the
                     // characters before the OK if the mux was invalid

    if (!isExpectedMux(ret_mux, mux)) {
      // if we didn't get a read from the expected mux, set the read length to 0
      // and update the available data for the mux that was requested
      len_read                     = 0;
      sockets[mux]->sock_available = modemGetAvailable(mux);
    }
    return len_read;
  }

  size_t modemGetAvailable(uint8_t mux) {
    if (!isValidMux(mux)) { return 0; }
    sendAT(GF("+CIPRXGET=4,"), mux);
    size_t result = 0;
    if (waitResponse(GF("+CIPRXGET:")) == 1) {
      streamSkipUntil(',');                       // Skip mode 4
      int16_t ret_mux = streamGetIntBefore(',');  // mux
      result          = streamGetIntBefore('\n');
      if (!isExpectedMux(ret_mux, mux)) { result = 0; }
      waitResponse();
    }
    // DBG("### Available:", result, "on", mux);
    if (!result) { sockets[mux]->sock_connected = modemGetConnected(mux); }
    return result;
  }

  bool modemGetConnected(uint8_t mux) {
    if (!isValidMux(mux)) { return false; }
    // Read the status of all sockets at once
    sendAT(GF("+CIPCLOSE?"));
    if (waitResponse(GF("+CIPCLOSE:")) != 1) { return false; }
    for (uint8_t muxNo = 0; muxNo < TcpConfig::kMuxCount; muxNo++) {
      // +CIPCLOSE:<link0_state>,<link1_state>,...,<link9_state>
      bool muxState = stream.parseInt();
      if (sockets[muxNo]) { sockets[muxNo]->sock_connected = muxState; }
    }
    waitResponse();  // Should be an OK at the end
    return sockets[mux]->sock_connected;
  }

  /*
   * Utilities
   */
 protected:
  bool handleURCs(String& data) {
    if (data.endsWith(GF("+CIPRXGET:"))) {
      int8_t mode = streamGetIntBefore(',');
      if (mode == 1) {
        int16_t mux = streamGetIntBefore('\n');
        if (isValidMux(mux)) { sockets[mux]->got_data = true; }
        data = "";
        // DBG("### Got Data:", mux);
        return true;
      } else {
        data += mode;
        return false;
      }
    } else if (data.endsWith(GF("+RECEIVE:"))) {
      int16_t mux = streamGetIntBefore(',');
      int16_t len = streamGetIntBefore('\n');
      if (isValidMux(mux)) {
        sockets[mux]->got_data = true;
        if (len >= 0 && len <= 1024) { sockets[mux]->sock_available = len; }
      }
      data = "";
      // DBG("### Got Data:", len, "on", mux);
      return true;
    } else if (data.endsWith(GF("+IPCLOSE:"))) {
      int16_t mux = streamGetIntBefore(',');
      streamSkipUntil('\n');  // Skip the reason code
      if (isValidMux(mux)) { sockets[mux]->sock_connected = false; }
      data = "";
      DBG("### Closed: ", mux);
      return true;
    } else if (data.endsWith(GF("+CIPEVENT:"))) {
      // Need to close all open sockets and release the network library.
      // User will then need to reconnect.
      DBG("### Network error!");
      if (!isDataConnected()) { dataDisconnect(); }
      data = "";
      return true;
    }
    return false;
  }

 public:
  /// Stream used to communicate with the modem.
  Stream& stream;

 protected:
  GsmClientSim5360* sockets[TcpConfig::kMuxCount];
};

#endif  // SRC_TINYGSMCLIENTSIM5360_H_
