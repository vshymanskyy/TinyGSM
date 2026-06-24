/**
 * @file       TinyGsmClientEC200.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 *
 * @EC200 support added by EICUT 2025
 *
 */

 #ifndef SRC_TINYGSMCLIENTEC200_H_
 #define SRC_TINYGSMCLIENTEC200_H_
 // #pragma message("TinyGSM:  TinyGsmClientEC200")
 
 // #define TINY_GSM_DEBUG Serial
 
 #define TINY_GSM_MUX_COUNT 6
 #define TINY_GSM_BUFFER_READ_NO_CHECK
 #ifdef AT_NL
 #undef AT_NL
 #endif
 #define AT_NL "\r\n"
 
 #ifdef MODEM_MANUFACTURER
 #undef MODEM_MANUFACTURER
 #endif
 #define MODEM_MANUFACTURER "Quectel"
 
 #ifdef MODEM_MODEL
 #undef MODEM_MODEL
 #endif
 #if defined(TINY_GSM_MODEM_EC200)
 #define MODEM_MODEL "EC200"
 #endif

#include "TinyGsmModem.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmBattery.tpp"
#include "TinyGsmAudio.tpp"
#include "TinyGsmBluetooth.tpp"
#include "TinyGsmNTP.tpp"
#include "TinyGsmGPS.tpp"

enum EC200RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};

class TinyGsmEC200 : public TinyGsmModem<TinyGsmEC200>,
                    public TinyGsmGPRS<TinyGsmEC200>,
                    public TinyGsmTCP<TinyGsmEC200, TINY_GSM_MUX_COUNT>,
                    public TinyGsmCalling<TinyGsmEC200>,
                    public TinyGsmSMS<TinyGsmEC200>,
                    public TinyGsmTime<TinyGsmEC200>,
                    public TinyGsmBattery<TinyGsmEC200>,
                    public TinyGSMAudio<TinyGsmEC200>,
                    public TinyGsmGPS<TinyGsmEC200>,
                    public TinyGsmBluetooth<TinyGsmEC200>,
                    public TinyGsmNTP<TinyGsmEC200>  {
  friend class TinyGsmModem<TinyGsmEC200>;
  friend class TinyGsmGPRS<TinyGsmEC200>;
  friend class TinyGsmTCP<TinyGsmEC200, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmCalling<TinyGsmEC200>;
  friend class TinyGsmSMS<TinyGsmEC200>;
  friend class TinyGsmTime<TinyGsmEC200>;
  friend class TinyGsmBattery<TinyGsmEC200>;
  friend class TinyGSMAudio<TinyGsmEC200>;
  friend class TinyGsmBluetooth<TinyGsmEC200>;
  friend class TinyGsmNTP<TinyGsmEC200>;
   friend class TinyGsmGPS<TinyGsmEC200>;
  /*
   * Inner Client
   */
 public:
  class GsmClientEC200 : public GsmClient {
    friend class TinyGsmEC200;

   public:
    GsmClientEC200() {}

    explicit GsmClientEC200(TinyGsmEC200& modem, uint8_t mux = 0) { //mux=multiplexing
      init(&modem, mux);
    }

    bool init(TinyGsmEC200* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_available = 0;
      sock_connected = false;

      if (mux < TINY_GSM_MUX_COUNT) {
        this->mux = mux;
      } else {
        this->mux = (mux % TINY_GSM_MUX_COUNT);
      }
      at->sockets[this->mux] = this;

      return true;
    }

   public:

    virtual int connect(const char* host, uint16_t port, int timeout_s) {
      stop();                 // Ensure old socket is closed
      TINY_GSM_YIELD();       // Yield control to main loop (TinyGSM safe behavior)
      rx.clear();             // Clear RX buffer

      // Connect without sending data here — or pass initData if needed
      sock_connected = at->modemConnect(host, port, mux, false, timeout_s);

      return sock_connected;
  }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void stop(uint32_t maxWaitMs) {
      uint32_t startMillis = millis();
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("+QICLOSE="), mux);
      sock_connected = false;
      at->waitResponse((maxWaitMs - (millis() - startMillis)), GF("CLOSED"),
                       GF("CLOSE OK"), GF("ERROR"));
    }
    void stop() override {
      stop(75000L);
    }

    virtual size_t write(const uint8_t *buf, size_t size) override {
      int16_t sent = at->modemSend(buf, size, mux);
      if (sent < 0) return 0;
      return (size_t)sent;
    }

// Read a single byte, blocking up to timeout
virtual int read() override {
    TINY_GSM_YIELD();

    uint32_t startMillis = millis();
    while ((millis() - startMillis) < 1000) { // 1-second timeout
        if (rx.size() > 0) {
            uint8_t c;
            rx.get(&c, 1);  // Read one byte from FIFO
            return c;
        }
        // If socket is connected, maintain modem to get new data
        if (sock_connected) {
            at->maintain();
        } else {
            return -1; // socket disconnected
        }
    }
    return -1; // timeout
}

// Optional: read multiple bytes
virtual int read(uint8_t* buf, size_t size) override {
    TINY_GSM_YIELD();

    size_t cnt = 0;
    uint32_t startMillis = millis();

    while (cnt < size && (millis() - startMillis) < 1000) {
        size_t chunk = TinyGsmMin(size - cnt, rx.size());
        if (chunk > 0) {
            rx.get(buf + cnt, chunk);
            cnt += chunk;
            startMillis = millis(); // reset timeout after successful read
            continue;
        }
        if (sock_connected) {
            at->maintain();
        } else {
            break;
        }
    }
    return cnt > 0 ? cnt : -1;
}


    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  }; //end of GsmClientEC200

  /*
   * Inner Secure Client
   */
  // NOT SUPPORTED

  /*
   * Constructor of outer class
   */
 public:
  explicit TinyGsmEC200(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = nullptr) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientEC200"));

    if (!testAT()) { return false; }

    // sendAT(GF("&FZ"));  // Factory + Reset
    // waitResponse();

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    DBG(GF("### Modem:"), getModemName());

    // Enable network time synchronization
    sendAT(GF("+CTZU=1")); //!QNITZ replaced with CTUZ - set module to update time zone via NITZ automatically 
    if (waitResponse(10000L) != 1) { return false; }
    sendAT(GF("+QLTS=1")); 
    if (waitResponse(10000L) != 1) { return false; }

    SimStatus ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != nullptr && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    } else {
      // if the sim is ready, or it's locked but no pin has been provided,
      // return true
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = nullptr) {
    if (!testAT()) { return false; }
    if (!setPhoneFunctionality(0)) { return false; }
    if (!setPhoneFunctionality(1, true)) { return false; }
    delay(3000);
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+QPOWD=1"));
    return waitResponse(GF("NORMAL POWER DOWN")) == 1;
  }

  // When entering into sleep mode is enabled, DTR is pulled up, and WAKEUP_IN
  // is pulled up, the module can directly enter into sleep mode.If entering
  // into sleep mode is enabled, DTR is pulled down, and WAKEUP_IN is pulled
  // down, there is a need to pull the DTR pin and the WAKEUP_IN pin up first,
  // and then the module can enter into sleep mode.
  bool sleepEnableImpl(bool enable = true) {
    sendAT(GF("+QSCLK="), enable);
    return waitResponse() == 1;
  }

  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 public:
  EC200RegStatus getRegistrationStatus() {
    return (EC200RegStatus)getRegistrationStatusXREG("CGREG");
  }

  int8_t getRegistrationStatusXREG(const char* regCommand) {
    sendAT('+', regCommand, '?');
    // check for any of the three for simplicity
    int8_t resp = waitResponse(GF("+CREG:"), GF("+CGREG:"),
                                           GF("+CEREG:"));
    if (resp != 1 && resp != 2 && resp != 3) { return -1; }
    streamSkipUntil(','); /* Skip format (0) */
    int status = stream.parseInt();
    waitResponse();
    return status;
  }
  
/**
 * 
 */
 protected:
  bool isNetworkConnectedImpl() {
    EC200RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  String getLocalIPImpl() {
    sendAT(GF("+CGPADDR")); //! QILOCIP replaced with CGPADDR
    streamSkipUntil('\n');
    String res = stream.readStringUntil('\n');
    res.trim();
    return res;
  }

  /*
   * Secure socket layer (SSL) functions
   */
  // No functions of this type supported

  /*
   * WiFi functions
   */
  // No functions of this type supported

  /*
   * GPRS functions
   */

   /*
 * EC200-compatible GPRS connect function
 */
protected:
  bool gprsConnectImpl(const char* apn, const char* user = nullptr,
                       const char* pwd = nullptr) {
    gprsDisconnect();  // your existing disconnect logic, if any

    // 1. Set PDP context parameters
    sendAT(GF("+QICSGP=1,1,\""), apn, GF("\",\""),
           (user ? user : ""), GF("\",\""), (pwd ? pwd : ""), GF("\",1"));
    if (waitResponse() != 1) { return false; }

    // 3. Activate PDP context
    sendAT(GF("+QIACT=1"));
    if (waitResponse(60000L) != 1) { return false; }
    // 4. Verify that we have a local IP address
    sendAT(GF("+CGPADDR=1"));
    if (waitResponse() != 1) { return false; }
    // Parse +CGPADDR response here to confirm non-zero IP
    streamSkipUntil('"'); 
    String res = stream.readStringUntil('"');  // read the provider
    String ip;

    DBG("Got IP: " + ip);
    return true;
  }

  bool gprsDisconnectImpl() {
    sendAT(GF("+QIDEACT=1"));  // Deactivate the bearer context
    return waitResponse(60000L, GF("OK"), GF("ERROR")) == 1;
  }

  String getProviderImpl() {
    sendAT(GF("+QSPN?"));
    if (waitResponse(GF("+QSPN:")) != 1) { return ""; }
    streamSkipUntil('"');                      // Skip mode and format
    String res = stream.readStringUntil('"');  // read the provider
    waitResponse();                            // skip anything else
    return res;
  }

  /*
   * SIM card functions
   */
 protected:
  SimStatus getSimStatusImpl(uint32_t timeout_ms = 10000L) {
    for (uint32_t start = millis(); millis() - start < timeout_ms;) {
      sendAT(GF("+CPIN?"));
      if (waitResponse(GF(AT_NL "+CPIN:")) != 1) {
        delay(1000);
        continue;
      }
      int8_t status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"),
                                   GF("NOT INSERTED"), GF("PH_SIM PIN"),
                                   GF("PH_SIM PUK"));
      waitResponse();
      switch (status) {
        case 2:
        case 3: return SIM_LOCKED;
        case 5:
        case 6: return SIM_ANTITHEFT_LOCKED;
        case 1: return SIM_READY;
        default: return SIM_ERROR;
      }
    }
    return SIM_ERROR;
  }

  /*
   * Phone Call functions
   */
  // Follows all phone call functions as inherited from TinyGsmCalling.tpp

  /*
   * Audio functions
   */
  // Follows all audio functions as inherited from TinyGsmAudio.tpp

  /*
   * Text messaging (SMS) functions
   */
  // Follows all text messaging (SMS) functions as inherited from TinyGsmSMS.tpp

 public:
  /** Delete all SMS */
  bool deleteAllSMS() {
    sendAT(GF("+CMGD=1,1")); //! +QMGDA=6 is replaced with AT+CMGD=1,1
    if (waitResponse(waitResponse(60000L, GF("OK"), GF("ERROR")) == 1)) {
      return true;
    }
    return false;
  }

  /*
   * GSM Location functions
   */
  // No functions of this type supported
 public:
  /**
   * @brief Set the speaker volume (0 to 5).
   *
   * @param level Volume level
   * @return true if successful
   */
  bool setSpeakerVolume(uint8_t level=5) {
    if(level>5){
      DBG("Level should be between 0-5. I changed it to 5 for you!");
      level=5;
    }
    return setSpeakerVolumeImpl(level);
  }
/**
 * +QICMIC: (0-7),(0-15)
 * Set Uplink Gains of MIC
 */
    bool setMicGainImpl(uint8_t gain) {
    if (gain > 15) gain = 15;
    sendAT(GF("+QICMIC=0,"), gain);
    return waitResponse() == 1;
  }

/**
 * Set Monitor Speaker Loudness
 */
  bool setMonitorLoudnessImpl(uint8_t level) {
    DBG("Does not support by EC200!");
    return waitResponse() == 1;
  }


   /*
   * DTMF
   */
   public:
  bool dtmfSendImpl(char cmd, int duration_ms = 100) {
    DBG("It is not supported!\n");
  }


    /*
   * Text messaging (SMS) functions
   */
  public:

  String sendUSSDImpl(const String& code) {
  // Check URC port
  sendAT(GF("+QURCCFG?"));
  streamSkipUntil(',');
  streamSkipUntil('"');
  String res = stream.readStringUntil('"');  // read the provider
  if (res != "uart1") {
      DBG("Changing URC port to uart1...");
      sendAT(GF("+QURCCFG=\"urcport\",\"uart1\""));
      waitResponse();  // optional: wait for "OK"
  } else {
      DBG("URC port already set to uart1.");
  }

  sendAT(GF("+CMGF=1"));
  waitResponse();

  sendAT(GF("+CSCS=\"HEX\""));
  waitResponse();

  sendAT(GF("+CUSD=1,\""), code, GF("\""));
  if (waitResponse() != 1) return "";

  if (waitResponse(10000L, GF("+CUSD:")) != 1) return "";

  stream.readStringUntil('"');
  String hex = stream.readStringUntil('"');
  stream.readStringUntil(',');
  int8_t dcs = streamGetIntBefore('\n');

  if (dcs == 15) {
    return TinyGsmDecodeHex8bit(hex);
  } else if (dcs == 72) {
    return TinyGsmDecodeHex16bit(hex);
  } else {
    return hex;
  }
}


    /*
   * GPS/GNSS/GLONASS location functions
   */
  public:
  // Enable GNSS
  bool enableGPSImpl() {
    sendAT("+QGPS=1");  // Power on GNSS
    return waitResponse(GF("OK")) == 1;
  }

  // Disable GNSS
  bool disableGPSImpl() {
    sendAT("+QGPSEND");  // Power off GNSS
    return waitResponse(GF("OK")) == 1;
  }

  /*
  NMEA Sentence Reference Table:

  +--------+---------------------------------------------+---------------------------+
  | Type   | Description                                 | Useful for                |
  +--------+---------------------------------------------+---------------------------+
  | GGA    | Fix info: position, altitude, sats used     | 3D location & quality     |
  | RMC    | Date, time, position, speed, course         | Basic GPS nav info        |
  | GSV    | Satellite visibility (all visible sats)     | GNSS diagnostics          |
  | GSA    | Sats in use, fix type (2D/3D), DOP           | Precision quality         |
  | VTG    | Track made good and ground speed            | Movement vector           |
  +--------+---------------------------------------------+---------------------------+
*/

  // Get RAW GNSS NMEA sentence

String getGPSrawImpl() {
  const char* sentences[] = { "RMC", "GGA", "GSA", "GSV", "VTG" };
  String resGNSS;

  for (int i = 0; i < sizeof(sentences) / sizeof(sentences[0]); i++) {
    String cmd = "+QGPSGNMEA=\"" + String(sentences[i]) + "\"";
    DBG("Sending GNSS command: ", cmd);

    sendAT(cmd);
    String response = "", line = "";
    unsigned long start = millis();

    while (millis() - start < 3000UL) {
      while (stream.available()) {
        char c = stream.read();
        line += c;

        if (line.endsWith("\r\n")) {
          line.trim();

          if (line.startsWith("+QGPSGNMEA:")) {
            // Skip the prefix and just grab the actual NMEA sentence
            String sentence = line.substring(12);
            resGNSS += "[$" + String(sentences[i]) + "] " + sentence + "\r\n";
          }

          if (line == "OK") break;
          line = "";
        }
      }
    }
  }

  DBG("GNSS Data:\n", resGNSS);
  return resGNSS;
}



  // Get RMC (Recommended Minimum GNSS Data) information
  String getGPSRMCImpl() {
    sendAT("+QGPSGNMEA=\"RMC\"");
    String res;
    if (waitResponse(5000, res, GF("OK")) != 1) return "";
    res.trim();
    return res;
  }

  // Get GSA (GNSS Satellite Data) information
  String getGPSGSAImpl() {
    sendAT("+QGPSGNMEA=\"GSA\"");
    String res;
    if (waitResponse(5000, res, GF("OK")) != 1) return "";
    res.trim();
    return res;
  }
  /**
   * 
   */
bool getGPSImpl(float* lat, float* lon, float* speed = nullptr, float* alt = nullptr,
                int* vsat = nullptr, int* usat = nullptr, float* accuracy = nullptr,
                int* year = nullptr, int* month = nullptr, int* day = nullptr,
                int* hour = nullptr, int* minute = nullptr, int* second = nullptr)
{
    // Send AT command to get RMC sentence
    sendAT("+QGPSGNMEA=\"RMC\"");
    String response;
    if (waitResponse(5000, response, GF("OK")) != 1) return false;

    // Find the line that starts with "$GNRMC"
    int idx = response.indexOf("$GNRMC");
    if (idx == -1) return false;

    String rmcLine = response.substring(idx);
    int endIdx = rmcLine.indexOf('\n');
    if (endIdx != -1) {
        rmcLine = rmcLine.substring(0, endIdx);
    }

    // Example: $GNRMC,hhmmss.sss,A,llll.ll,a,yyyyy.yy,a,x.x,x.x,ddmmyy,,,A*hh
    // Fields: [0] Type, [1] UTC time, [2] Status, [3] Lat, [4] N/S, [5] Lon, [6] E/W, ...
    char type[16], status;
    float rawLat, rawLon, knots;
    char latDir, lonDir;
    int rawTime, rawDate;

    int n = sscanf(rmcLine.c_str(), "$%[^,],%d.%*d,%c,%f,%c,%f,%c,%f,,%d",
                   type, &rawTime, &status, &rawLat, &latDir,
                   &rawLon, &lonDir, &knots, &rawDate);

    if (n < 9 || status != 'A') {
        DBG("Invalid or no GPS fix");
        return false;
    }

    // Convert raw lat/lon in NMEA format to decimal degrees
    auto convertToDecimal = [](float val, char dir) -> float {
        int deg = (int)(val / 100);
        float min = val - (deg * 100);
        float dec = deg + min / 60.0;
        if (dir == 'S' || dir == 'W') dec = -dec;
        return dec;
    };

    if (lat) *lat = convertToDecimal(rawLat, latDir);
    if (lon) *lon = convertToDecimal(rawLon, lonDir);
    if (speed) *speed = knots * 1.852;  // Convert knots to km/h

    // Parse date/time
    if (rawTime > 0) {
        int hh = rawTime / 10000;
        int mm = (rawTime / 100) % 100;
        int ss = rawTime % 100;
        if (hour) *hour = hh;
        if (minute) *minute = mm;
        if (second) *second = ss;
    }

    if (rawDate > 0) {
        int dd = rawDate / 10000;
        int mm = (rawDate / 100) % 100;
        int yy = rawDate % 100;
        if (day) *day = dd;
        if (month) *month = mm;
        if (year) *year = 2000 + yy;
    }

    return true;
}



  /*
   * Time functions
   */
  // Follows all clock functions as inherited from TinyGsmTime.tpp

  /*
   * NTP server functions
   */

public:
  byte NTPServerSyncImpl(int contextID=1, String server = "pool.ntp.org", int port = 123) {
    // Set the NTP server and port
    sendAT(GF("+QNTP="),contextID,",\"", server, "\",", String(port));
    if (waitResponse(120000L) != 1) return -1;

    // Execute NTP sync request
    sendAT(GF("+QNTP?"));
    if (waitResponse(120000L, GF("+QNTP:"))) {
      String result = stream.readStringUntil('\n');
      result.trim();
      if (TinyGsmIsValidNumber(result)) return result.toInt();
    }
    return -1;
  }

  String getNTPServerImpl() {
    sendAT(GF("+QNTP?"));
    if (waitResponse(10000L, GF("+QNTP:")) != 1) return "";
    String res = stream.readStringUntil('\n');
    res.trim();
    return res;
  }

  /*
   * BLE functions
   */
// Enable Bluetooth
public:
// Define BT profiles as enum with bitmasks
 enum TinyGsmBtProfile : uint8_t {
  BT_MODE_OFF            = 0,
  BT_MODE_BLE_GATT_SERVER = 1,
  BT_MODE_BLE_GATT_CLIENT = 2,
  BT_MODE_SPP             = 4,
  BT_MODE_HFP             = 8,
  BT_MODE_A2DP_AVRCP      = 16,
  BT_MODE_A2DP_SOURCE_AVRCP_TARGET = 32, // Not supported yet
  BT_MODE_HFP_AG          = 64
};


enum TinyGsmBtDataType : uint8_t {
  BT_DATA_BLE_NOTIFY = 0,
  BT_DATA_BLE_INDICATE = 1,
  BT_DATA_SPP = 2
};

enum TinyGsmBtSendMode : uint8_t {
  BT_SEND_DIRECT = 0,
  BT_SEND_TRANSPARENT = 2
};


// Disable Bluetooth
bool disableBluetoothImpl() {
  sendAT(GF("+QBTPWER=0"));
  return waitResponse() == 1;
}

bool enableBluetoothImpl(TinyGsmBtProfile profile=BT_MODE_SPP) {
  sendAT(GF("+QBTPWER="),profile);
  return waitResponse() == 1;
}

// Set Bluetooth visibility
bool setBluetoothVisibilityImpl(bool visible) {
  DBG("setBluetoothVisibilityImpl is not supported by EC200 use setBluetoothAdvertisingImpl instead");
  return waitResponse() == 1;
}

// Set Bluetooth host name
bool setBluetoothHostNameImpl(const char* name) {
  sendAT(GF("+QBTNAME="), name);
  return waitResponse() == 1;
}


bool setBluetoothAdvertisingImpl(bool enable) {
  sendAT(GF("+QBTADV="), enable ? 1 : 0);
  return waitResponse() == 1;
}



bool sendBluetoothDataImpl(TinyGsmBtDataType type, TinyGsmBtSendMode mode, const String &data) {
  if (mode == BT_SEND_TRANSPARENT) {
    sendAT(GF("+QBTSEND="), type, ",", mode);
  } else {
    sendAT(GF("+QBTSEND="), type, ",", mode, ",", "\"", data, "\"");
  }
  return waitResponse() == 1;
}
  /*
   * Battery functions
   */
  // Follows all battery functions as inherited from TinyGsmBattery.tpp

  /*
   * Temperature functions
   */
  // No functions of this type supported

  /*
   * Client related functions
   */
protected: //! change it to protected after debugging
/**
 * @brief Connects to a TCP server using AT+QIOPEN and checks result using +QIOPEN:<mux>,<err> response.
 *        Sends optional init data after a successful connection.
 * 
 * @param host       Remote server domain or IP address.
 * @param port       Remote TCP port number.
 * @param mux        Connection ID (0–11).
 * @param ssl        Unused on EC200; keep false.
 * @param timeout_s  Timeout (in seconds) to wait for connection result.
 * @param initData   Optional pointer to initial data to send after connection (can be NULL).
 * @param initLen    Length of initData (if provided).
 * 
 * @return true on successful connection and init data send (if applicable); false otherwise.
 */
bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                  bool ssl = false, int timeout_s = 75,
                  const void* initData = nullptr, size_t initLen = 0) {
  if (ssl) {
    DBG("SSL not supported.");
    return false;
  }

  uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;

  // Send QIOPEN command
  sendAT(GF("+QIOPEN=1,"), mux, GF(",\"TCP\",\""), host, GF("\","), port, GF(",0,1"));

  // First expect an OK
  if (waitResponse(5000L, GFP(GSM_OK), GFP(GSM_ERROR)) != 1) {
    DBG("QIOPEN command not accepted");
    return false;
  }

  // Now wait asynchronously for +QIOPEN result
  uint32_t start = millis();
  while (millis() - start < timeout_ms) {
    if (waitResponse(1000L, GF("+QIOPEN:")) == 1) {
      streamSkipUntil(':');  // Skip to colon
      stream.read();         // Skip space
      int respMux = stream.readStringUntil(',').toInt();
      int errCode = stream.readStringUntil('\n').toInt();

      if (respMux != mux) {
        DBG("MUX mismatch");
        return false;
      }
      if (errCode != 0) {
        DBG("QIOPEN failed, error:", errCode);
        return false;
      }

      // Connected
      if (initData && initLen > 0) {
        int16_t sent = modemSend(initData, initLen, mux);
        if (sent != (int16_t)initLen) {
          DBG("Initial data send failed");
          return false;
        }
      }
      return true;
    }
    // give modem time
    delay(100);
  }

  DBG("QIOPEN timeout");
  return false;
}

/**
 * @brief Sends data over the modem using AT+QISEND and checks if all data 
 *        was acknowledged using AT+QISEND=0,0 response parsing.
 * 
 * @param buff Pointer to the data buffer to send.
 * @param len Length of the data in bytes.
 * @param mux MUX ID (0–11).
 * @return int16_t Number of bytes sent on success, -1 on failure.
 */
int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
  // Step 1: Send AT+QISEND command
  sendAT(GF("+QISEND="), mux);
  if (waitResponse(GF(">")) != 1) {
    DBG("Send prompt '>' not received.");
    return 0;
  }

  // Step 2: Send data and terminate with Ctrl+Z
  stream.write(reinterpret_cast<const uint8_t*>(buff), len);
  stream.write(0x1A); // Ctrl+Z to end transmission
  stream.flush();

  // Step 3: Wait for SEND OK confirmation
  if (waitResponse(10000L, GF(AT_NL "SEND OK"), GF("ERROR")) != 1) {
    DBG("SEND OK not received.");
    return 0;
  }

  // Step 4: Query send status using AT+QISEND=0,0
  sendAT(GF("+QISEND="), mux, ",0");
  if (waitResponse(2000L, GF("+QISEND:")) != 1) {
    DBG("QISEND status check failed.");
    return -1;
  }

  // Parse response: +QISEND: <mux>,0,<sent_len>
  streamSkipUntil(','); // Skip <mux>
  int resultCode = streamGetIntBefore(','); // 0 = success
  int sentLen = streamGetIntBefore('\r');   // actual length sent

  if (resultCode != 0 || sentLen != (int)len) {
    DBG("Data not fully acknowledged. Sent:", sentLen);
    return -1;
  }

  return (int16_t)len;
}


  size_t modemRead(size_t size, uint8_t mux) {
    if (!sockets[mux]) return 0;
    // TODO(?):  Does this even work????
    // AT+QIRD=<id>,<sc>,<sid>,<len>
    // id = GPRS context number = 0, set in GPRS connect
    // sc = role in connection = 1, client of connection
    // sid = index of connection = mux
    // len = maximum length of data to retrieve
    sendAT(GF("+QIRD=0,1,"), mux, ',', (uint16_t)size);
    // If it replies only OK for the write command, it means there is no
    // received data in the buffer of the connection.
    int8_t res = waitResponse(GF("+QIRD:"), GFP(GSM_OK), GFP(GSM_ERROR));
    if (res == 1) {
      streamSkipUntil(':');  // skip IP address
      streamSkipUntil(',');  // skip port
      streamSkipUntil(',');  // skip connection type (TCP/UDP)
      // read the real length of the retrieved data
      uint16_t len = streamGetIntBefore('\n');
      // It's possible that the real length available is less than expected
      // This is quite likely if the buffer is broken into packets - which may
      // be different sizes.
      // If so, make sure we make sure we re-set the amount of data available.
      if (len < size) { sockets[mux]->sock_available = len; }
      for (uint16_t i = 0; i < len; i++) {
        moveCharFromStreamToFifo(mux);
        sockets[mux]->sock_available--;
        // ^^ One less character available after moving from modem's FIFO to our
        // FIFO
      }
      waitResponse();  // ends with an OK
      // DBG("### READ:", len, "from", mux);
      return len;
    } else {
      sockets[mux]->sock_available = 0;
      return 0;
    }
  }

  // Not possible to check the number of characters remaining in buffer
  size_t modemGetAvailable(uint8_t) {
    return 0;
  }

  bool modemGetConnected(uint8_t mux) {
    sendAT(GF("+QISTATE=1,"), mux);
    // +QISTATE: 0,"TCP","151.139.237.11",80,5087,4,1,0,0,"uart1"

    if (waitResponse(GF("+QISTATE:")) != 1) { return false; }

    streamSkipUntil(',');                  // Skip mux
    streamSkipUntil(',');                  // Skip socket type
    streamSkipUntil(',');                  // Skip remote ip
    streamSkipUntil(',');                  // Skip remote port
    streamSkipUntil(',');                  // Skip local port
    int8_t res = streamGetIntBefore(',');  // socket state

    waitResponse();

    // 0 Initial, 1 Opening, 2 Connected, 3 Listening, 4 Closing
    return 2 == res;
  }

  /*
   * Utilities
   */
 public:
  bool handleURCs(String& data) {
    if (data.endsWith(GF(AT_NL "+QIRD:"))) { 
      // +QIRDI: <id>,<sc>,<sid>,<num>,<len>,< tlen>
      streamSkipUntil(',');  // Skip the context
      streamSkipUntil(',');  // Skip the role
      // read the connection id
      int8_t mux = streamGetIntBefore(',');
      // read the number of packets in the buffer
      int8_t num_packets = streamGetIntBefore(',');
      // read the length of the current packet
      streamSkipUntil(
          ',');  // Skip the length of the current package in the buffer
      int16_t len_total =
          streamGetIntBefore('\n');  // Total length of all packages
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux] &&
          num_packets >= 0 && len_total >= 0) {
        sockets[mux]->sock_available = len_total;
      }
      data = "";
      // DBG("### Got Data:", len_total, "on", mux);
      return true;
    } else if (data.endsWith(GF("CLOSED" AT_NL))) {
      int8_t nl   = data.lastIndexOf(AT_NL, data.length() - 8);
      int8_t coma = data.indexOf(',', nl + 2);
      int8_t mux  = data.substring(nl + 2, coma).toInt();
      if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
        sockets[mux]->sock_connected = false;
      }
      data = "";
      DBG("### Closed: ", mux);
      return true;
    } else if (data.endsWith(GF("+QLTS:"))) { 
      streamSkipUntil('\n');  // URC for time sync
      DBG("### Network time updated.");
      data = "";
    }
    return false;
  }

 public:
  Stream& stream;

 protected:
  GsmClientEC200* sockets[TINY_GSM_MUX_COUNT];
};
 
 #endif  // SRC_TINYGsmClientEC200_H_
 