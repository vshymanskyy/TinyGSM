/**
 * @file       TinyGsmClientGL868.h
 * @author     Jimbo jangrypool@gmail.com
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2024 jangrypool@gmail.com
 * @date       Feb 2024
 */

#ifndef SRC_TINYGSMCLIENTGL868_H_
#define SRC_TINYGSMCLIENTGL868_H_

#define TINY_GSM_MUX_COUNT 6
#define TINY_GSM_BUFFER_READ_NO_CHECK
#define TINY_GSM_RX_BUFFER 1500

#include "TinyGsmBattery.tpp"
#include "TinyGsmCalling.tpp"
#include "TinyGsmGPRS.tpp"
#include "TinyGsmGSMLocation.tpp"
#include "TinyGsmModem.tpp"
#include "TinyGsmSMS.tpp"
#include "TinyGsmSSL.tpp"
#include "TinyGsmTCP.tpp"
#include "TinyGsmTime.tpp"
#include "TinyGsmNTP.tpp"

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM    = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;
#if defined       TINY_GSM_DEBUG
static const char GSM_CME_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CME ERROR:";
static const char GSM_CMS_ERROR[] TINY_GSM_PROGMEM = GSM_NL "+CMS ERROR:";
#endif

enum RegStatus {
  REG_NO_RESULT    = -1,
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};
class TinyGsmGL868 : public TinyGsmModem<TinyGsmGL868>,
                      public TinyGsmGPRS<TinyGsmGL868>,
                      public TinyGsmTCP<TinyGsmGL868, TINY_GSM_MUX_COUNT>,
                      public TinyGsmSSL<TinyGsmGL868>,
                      public TinyGsmCalling<TinyGsmGL868>,
                      public TinyGsmSMS<TinyGsmGL868>,
                      public TinyGsmGSMLocation<TinyGsmGL868>,
                      public TinyGsmTime<TinyGsmGL868>,
                      public TinyGsmNTP<TinyGsmGL868>,
                      public TinyGsmBattery<TinyGsmGL868> {
  friend class TinyGsmModem<TinyGsmGL868>;
  friend class TinyGsmGPRS<TinyGsmGL868>;
  friend class TinyGsmTCP<TinyGsmGL868, TINY_GSM_MUX_COUNT>;
  friend class TinyGsmSSL<TinyGsmGL868>;
  friend class TinyGsmCalling<TinyGsmGL868>;
  friend class TinyGsmSMS<TinyGsmGL868>;
  friend class TinyGsmGSMLocation<TinyGsmGL868>;
  friend class TinyGsmTime<TinyGsmGL868>;
  friend class TinyGsmNTP<TinyGsmGL868>;
  friend class TinyGsmBattery<TinyGsmGL868>;

  /*
   * Inner Client
   */
 public:
  class GsmClientGL868 : public GsmClient {
    friend class TinyGsmGL868;

   public:
    GsmClientGL868() {}

    explicit GsmClientGL868(TinyGsmGL868& modem, uint8_t mux = 0) {
      init(&modem, mux);
    }

    bool init(TinyGsmGL868* modem, uint8_t mux = 0) {
      this->at       = modem;
      sock_available = 0;
      prev_check     = 0;
      sock_connected = false;
      got_data       = false;

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
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, false, timeout_s);
      return sock_connected;
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES

    void stop(uint32_t maxWaitMs) {
      dumpModemBuffer(maxWaitMs);
      at->sendAT(GF("#SH=1"));  // Quick close
      sock_connected = false;
      at->waitResponse();
      at->sendAT(GF("#SLED=2"));
			at->waitResponse();	
    }
    void stop() override {
      stop(15000L);
    }

    /*
     * Extended API
     */

    String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;
  };

  /*
   * Inner Secure Client
   */
 public:
  class GsmClientSecureGL868 : public GsmClientGL868 {
   public:
    GsmClientSecureGL868() {}

    explicit GsmClientSecureGL868(TinyGsmGL868& modem, uint8_t mux = 0)
        : GsmClientGL868(modem, mux) {}

   public:
    int connect(const char* host, uint16_t port, int timeout_s) override {
      stop();
      TINY_GSM_YIELD();
      rx.clear();
      sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
      return sock_connected;
    }
    size_t write(const uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      at->maintain();
      return at->modemSend(buf, size, mux, true);
    }
    uint8_t connected() override {
      if (available()) { return true; }
      return at->modemGetConnected(mux, true);
	}
	int read(uint8_t* buf, size_t size) override {
      TINY_GSM_YIELD();
      size_t cnt = 0;
      at->maintain();
      while (cnt < size) {
        size_t chunk = TinyGsmMin(size - cnt, rx.size());
        if (chunk > 0) {
          rx.get(buf, chunk);
          buf += chunk;
          cnt += chunk;
          continue;
        } /* TODO: Read directly into user buffer? */
        at->maintain();
        if (sock_available > 0) {
          int n = at->modemRead(TinyGsmMin((uint16_t)rx.free(), sock_available),
                                mux, true);
          if (n == 0) break;
        } else {
          break;
        }
      }
      return cnt;
    }
    int read() override {
      uint8_t c;
      if (read(&c, 1) == 1) { return c; }
      return -1;
    }
    void stop() override {
      at->sendAT(GF("#SSLH=1"));  // Quick close
      sock_connected = false;
      at->waitResponse();
      at->sendAT(GF("#SLED=2"));
			at->waitResponse();	
      at->sendAT(GF("#CPUMODE=0"));
      at->waitResponse();
    }
    TINY_GSM_CLIENT_CONNECT_OVERRIDES
  };

  /*
   * Constructor
   */
 public:
  explicit TinyGsmGL868(Stream& stream) : stream(stream) {
    memset(sockets, 0, sizeof(sockets));
  }

  /*
   * Basic functions
   */
 protected:
  bool initImpl(const char* pin = NULL) {
    DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
    DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientGL868"));

    if (!testAT()) { return false; }

    sendAT(GF("E0"));  // Echo Off
    if (waitResponse() != 1) { return false; }

#ifdef TINY_GSM_DEBUG
    sendAT(GF("+CMEE=2"));  // turn on verbose error codes
#else
    sendAT(GF("+CMEE=0"));  // turn off error codes
#endif
    waitResponse();

    //DBG(GF("### Modem:"), getModemName());

    SimStatus ret = getSimStatus();
    // if the sim isn't ready and a pin has been provided, try to unlock the sim
    if (ret != SIM_READY && pin != NULL && strlen(pin) > 0) {
      simUnlock(pin);
      return (getSimStatus() == SIM_READY);
    } else {
      // if the sim is ready, or it's locked but no pin has been provided,
      // return true
      return (ret == SIM_READY || ret == SIM_LOCKED);
    }
  }

  bool factoryDefaultImpl() {
    sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
    waitResponse();
    sendAT(GF("+IPR=0"));  // Auto-baud
    waitResponse();
    sendAT(GF("+IFC=0,0"));  // No Flow Control
    waitResponse();
    sendAT(GF("+ICF=3,3"));  // 8 data 0 parity 1 stop
    waitResponse();
    sendAT(GF("+CSCLK=0"));  // Disable Slow Clock
    waitResponse();
    sendAT(GF("&W"));  // Write configuration
    return waitResponse() == 1;
  }

  void maintainImpl() {
    // Keep listening for modem URC's and proactively iterate through
    // sockets asking if any data is avaiable
    bool check_socks = false;
    for (int mux = 0; mux < TINY_GSM_MUX_COUNT; mux++) {
      GsmClientGL868* sock = sockets[mux];
      if (sock && sock->got_data) {
        sock->got_data = false;
        check_socks    = true;
      }
    }
    //DBG("maintainImpl ", check_socks);
    // modemGetAvailable checks all socks, so we only want to do it once
    // modemGetAvailable calls modemGetConnected(), which also checks allf
    if (check_socks) { modemGetAvailable(0); }
    while (stream.available()) { waitResponse(15, NULL, NULL); }
  }

  /*
   * Power functions
   */
 protected:
  bool restartImpl(const char* pin = NULL) {
    if (!testAT()) { return false; }
    sendAT(GF("&W"));
    waitResponse();
    if (!setPhoneFunctionality(0)) { return false; }
    if (!setPhoneFunctionality(1, true)) { return false; }
    delay(3000);
    return init(pin);
  }

  bool powerOffImpl() {
    sendAT(GF("+CPOWD=1"));
    return waitResponse(10000L, GF("NORMAL POWER DOWN")) == 1;
  }

  // During sleep, the GL868 module has its serial communication disabled. In
  // order to reestablish communication pull the DRT-pin of the GL868 module
  // LOW for at least 50ms. Then use this function to disable sleep mode. The
  // DTR-pin can then be released again.
  bool sleepEnableImpl(bool enable = true) {
    sendAT(GF("+CSCLK="), enable);
    return waitResponse() == 1;
  }

  // <fun> 0 Minimum functionality
  // <fun> 1 Full functionality (Default)
  // <fun> 4 Disable phone both transmit and receive RF circuits.
  // <rst> Reset the MT before setting it to <fun> power level.
  bool setPhoneFunctionalityImpl(uint8_t fun, bool reset = false) {
    sendAT(GF("+CFUN="), fun, reset ? ",1" : "");
    return waitResponse(10000L) == 1;
  }

  /*
   * Generic network functions
   */
 public:
  RegStatus getRegistrationStatus() {
    return (RegStatus)getRegistrationStatusXREG("CREG");
  }

 protected:
  bool isNetworkConnectedImpl() {
    RegStatus s = getRegistrationStatus();
    return (s == REG_OK_HOME || s == REG_OK_ROAMING);
  }

  String getLocalIPImpl() {
    sendAT(GF("+CGPADDR=1"));
    String res;
    if (waitResponse(10000L, res) != 1) { return ""; }
    res.replace(GSM_NL "OK" GSM_NL, "");
    res.replace(GSM_NL, "");
    res.replace("+CGPADDR: 1,", "");
    res.trim();
    return res;
  }

  /*
   * GPRS functions
   */
 protected:
   // Checks if current attached to GPRS/EPS service
  bool isGprsConnectedImpl() {
    sendAT(GF("+CGATT?"));
    if (waitResponse(GF("+CGATT:")) != 1) { return false; }
    int8_t res = streamGetIntBefore('\n');
    waitResponse();
    if (res != 1) { return false; }

    return localIP() != IPAddress(0, 0, 0, 0);
  }
 
 
  bool gprsConnectImpl(const char* apn, const char* user = NULL,
                       const char* pwd = NULL) {
    //gprsDisconnect();
    
    // Define the PDP context
    sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
    waitResponse();    
	  delay(1000);
    
    //sendAT(GF("&K0"));
    //waitResponse();
    
    sendAT(GF("#SCFGEXT="), GF("1,1,0,0,0,0"));
	  waitResponse();
	  sendAT(GF("#SCFGEXT2="), GF("1,0,0,0,0,0"));
	  waitResponse();
	  sendAT(GF("#SCFGEXT3="), GF("1,0,1,0,0,0"));
	  //sendAT(GF("#SCFGEXT3?"));
	  waitResponse(); 
    
    sendAT(GF("#GPRS=1"));
    if (waitResponse(10000L, GF("OK" GSM_NL)) != 1) { return false; }    

    return true;
  }

  bool gprsDisconnectImpl() {
    // Shut the TCP/IP connection
    sendAT(GF("+CGATT=0"));  // Detach from GPRS
    if (waitResponse(60000L) != 1) { return false; }

    return true;
  }

  /*
   * SIM card functions
   */
 protected:
  // May not return the "+CCID" before the number
  String getSimCCIDImpl() {
    sendAT(GF("#CCID"));
    if (waitResponse(GF(GSM_NL)) != 1) { return ""; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    // Trim out the CCID header in case it is there
    res.replace("CCID:", "");
    res.trim();
    return res;
  }

  /*
   * Phone Call functions
   */
 public:
  bool setGsmBusy(bool busy = true) {
    sendAT(GF("+GSMBUSY="), busy ? 1 : 0);
    return waitResponse() == 1;
  }

  /*
   * Messaging functions
   */
 protected:
  String sendUSSDImpl(const String& code) {
    sendAT(GF("+CMGF=1"));
    waitResponse();
    sendAT(GF("+CSCS=\"IRA\""));
    waitResponse();
    sendAT(GF("+CUSD=1,\""), code, GF("\""));
    if (waitResponse(10000L) != 1) { return ""; }
    //DBG("### SEND USSD: ", code);
    if (waitResponse(10000L, GF(GSM_NL "+CUSD:")) != 1) { return ""; }
    streamSkipUntil('"');
    String hex = stream.readStringUntil('"');
    //DBG("### READ USSD: ", hex);
    streamSkipUntil(',');
    int8_t dcs = streamGetIntBefore('\n');

    if (dcs == 15) {
      return hex;
    } else if (dcs == 72) {
      return TinyGsmDecodeHex16bit(hex);
    } else {
      return hex;
    }
  }
  /*
   * GSM Location functions
   */
 protected:
  // Depending on the exacty model and firmware revision, should return a
  // GSM-based location from CLBS as per the template
  // TODO(?):  Check number of digits in year (2 or 4)

  /*
   * GPS/GNSS/GLONASS location functions
   */
 protected:
  // No functions of this type supported

  /*
   * Audio functions
   */
 public:
  bool setVolume(uint8_t volume = 50) {
    // Set speaker volume
    sendAT(GF("+CLVL="), volume);
    return waitResponse() == 1;
  }

  uint8_t getVolume() {
    // Get speaker volume
    sendAT(GF("+CLVL?"));
    if (waitResponse(GF(GSM_NL)) != 1) { return 0; }
    String res = stream.readStringUntil('\n');
    waitResponse();
    res.replace("+CLVL:", "");
    res.trim();
    return res.toInt();
  }

  bool setMicVolume(uint8_t channel, uint8_t level) {
    if (channel > 4) { return 0; }
    sendAT(GF("+CMIC="), level);
    return waitResponse() == 1;
  }

  bool setAudioChannel(uint8_t channel) {
    sendAT(GF("+CHFA="), channel);
    return waitResponse() == 1;
  }

  bool playToolkitTone(uint8_t tone, uint32_t duration) {
    sendAT(GF("STTONE="), 1, tone);
    delay(duration);
    sendAT(GF("STTONE="), 0);
    return waitResponse();
  }

  /*
   * Time functions
   */
 protected:
  // Can follow the standard CCLK function in the template

  /*
   * NTP server functions
   */
  // Can sync with server using CNTP as per template

  /*
   * Battery functions
   */
 protected:
  // Follows all battery functions per template

  /*
   * NTP server functions
   */
  // Can sync with server using CNTP as per template

  /*
   * Client related functions
   */
 protected:
  bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                    bool ssl = false, int timeout_s = 75) {
    int8_t   rsp;
    uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
    if (ssl) { 
      sendAT(GF("#CPUMODE=3"));
      waitResponse();
      sendAT(GF("#SSLEN=1,1")); 
      waitResponse();
      sendAT(GF("#SSLD="),(mux+1), GF(","), port, ',', GF("\""), host,  GF("\",0,1,100"));
    }else{
      sendAT(GF("#SD="),(mux+1), GF(",0,"), port, ',', GF("\""), host,  GF("\",255,0,1"));
    }
    
    rsp = waitResponse(
        timeout_ms, GF("OK" GSM_NL), GF("ERROR" GSM_NL));
    if(rsp == 1){
      sendAT(GF("#SLED=1"));
      waitResponse();		
    }
    return (1 == rsp);
  }

  int16_t modemSend(const void* buff, size_t len, uint8_t mux, bool ssl = false) {
	if (ssl) { 
		sendAT(GF("#SSLSENDEXT="), (mux+1), ",",len,",500");
    }else{
		sendAT(GF("#SSENDEXT="), (mux+1), ",",len);
	}
    if (waitResponse(GF(">")) != 1) { return 0; }
    stream.write(reinterpret_cast<const uint8_t*>(buff), len);    
    stream.write(0x1A); //(char)26
    stream.flush();
    if (waitResponse(GF("OK" GSM_NL)) != 1) { return 0; }
    DBG("### SEND:", len);
    return len;
  }
  
  size_t modemRead(size_t size, uint8_t mux, bool ssl = false) {
    if (!sockets[mux]) return 0;
    // TODO(?):  Does this work????
    // AT+QIRD=<id>,<sc>,<sid>,<len>
    // id = GPRS context number = 0, set in GPRS connect
    // sc = role in connection = 1, client of connection
    // sid = index of connection = mux
    // len = maximum length of data to retrieve
    //delay(1000);
    if (ssl) { 
      sendAT(GF("#SSLRECV="), (mux+1), ',', (uint16_t)size);
      if (waitResponse(15000, GF("#SSLRECV:")) != 1) { 
        sockets[mux]->sock_available = 0;
        return 0; 
      }		
    }else{
      sendAT(GF("#SRECV="), (mux+1), ',', (uint16_t)size);
      if (waitResponse(GF("#SRECV:")) != 1) { 
        sockets[mux]->sock_available = 0;
        return 0; 
      }
      streamSkipUntil(',');  // Skip mux
    }
    
    int16_t len = streamGetIntBefore('\n');
		if (len == 0) { 
			sockets[mux]->sock_available = 0;
      //sockets[mux]->got_data = false;
      waitResponse();
			return 0; 
		}

    if (len < size) { sockets[mux]->sock_available = len; }
    for (uint16_t i = 0; i < len; i++) {
	  moveCharFromStreamToFifo(mux);
	  sockets[mux]->sock_available--;
	  // ^^ One less character available after moving from modem's FIFO to our
	  // FIFO
    }
    //waitResponse();  // ends with an OK
#if defined TINY_GSM_DEBUG
    waitResponse(15000, GF("OK" GSM_NL), GFP(GSM_CME_ERROR));
#else
    waitResponse(15000, GF("OK" GSM_NL));
#endif    
    sendAT(GF("#SLED=1"));
		waitResponse();	
    DBG("### READ:", len);
    return len;
  }

  size_t modemGetAvailable(uint8_t) {
    return 0;
  }

  bool modemGetConnected(uint8_t mux, bool ssl = false) {
    if (ssl) { 
      //return true;
      sendAT(GF("#SSLS="), mux+1);
      waitResponse(GF("#SSLS:"));			
    }else{
      sendAT(GF("#SS="), mux+1);
      waitResponse(GF("#SS:"));
    }
    streamSkipUntil(',');  // Skip mux
    int8_t res = streamGetIntLength(1);
    if (ssl && (2 == res)) { 
      streamSkipUntil(',');  // Skip res
      int8_t resd = streamGetIntLength(1);
      if(resd == 3){
       // DBG("### resd:", resd);
        sockets[mux]->got_data = true;
        sockets[mux]->sock_available = TINY_GSM_RX_BUFFER-1;
      }
    }
    
    waitResponse(15000, GF("OK" GSM_NL), GF("ERROR" GSM_NL));
    return (2 == res) || (3 == res);
  }

  /*
   * Utilities
   */
 public:
  // TODO(vshymanskyy): Optimize this!
  int8_t waitResponse(uint32_t timeout_ms, String& data,
                      GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    /*String r1s(r1); r1s.trim();
    String r2s(r2); r2s.trim();
    String r3s(r3); r3s.trim();
    String r4s(r4); r4s.trim();
    String r5s(r5); r5s.trim();
    DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s);*/
    data.reserve(TINY_GSM_RX_BUFFER);
    uint8_t  index       = 0;
    uint32_t startMillis = millis();
    do {
      TINY_GSM_YIELD();
      while (stream.available() > 0) {
        TINY_GSM_YIELD();
        int8_t a = stream.read();
        if (a <= 0) continue;  // Skip 0x00 bytes, just in case
        data += static_cast<char>(a);
        if (r1 && data.endsWith(r1)) {
          index = 1;
          goto finish;
        } else if (r2 && data.endsWith(r2)) {
          index = 2;
          goto finish;
        } else if (r3 && data.endsWith(r3)) {
#if defined TINY_GSM_DEBUG
          if (r3 == GFP(GSM_CME_ERROR)) {
            streamSkipUntil('\n');  // Read out the error
          }
#endif
          index = 3;
          goto finish;
        } else if (r4 && data.endsWith(r4)) {
          index = 4;
          goto finish;
        } else if (r5 && data.endsWith(r5)) {
          index = 5;
          goto finish;
        } else if (data.endsWith(GF(GSM_NL "SRING:"))) {
          int8_t  mux = streamGetIntBefore(',') - 1;
          int16_t len = streamGetIntBefore('\n');
          if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
            sockets[mux]->got_data = true;
            if (len >= 0 && len <= 8192) { sockets[mux]->sock_available = len; }
          }
          data = "";
          //DBG("### Got Data:", len, "on", mux);
        } else if (data.endsWith(GF("NO CARRIER" GSM_NL))) {
          int8_t mux  = 0;
          sockets[mux]->sock_connected = false;
          sendAT(GF("#SLED=2"));
          waitResponse();	
          //}
          data = "";
          DBG("### Closed: ", mux);
        } else if (data.endsWith(GF("*PSNWID:"))) {
          streamSkipUntil('\n');  // Refresh network name by network
          data = "";
          DBG("### Network name updated.");
        } else if (data.endsWith(GF("*PSUTTZ:"))) {
          streamSkipUntil('\n');  // Refresh time and time zone by network
          data = "";
          DBG("### Network time and time zone updated.");
        } else if (data.endsWith(GF("+CTZV:"))) {
          streamSkipUntil('\n');  // Refresh network time zone by network
          data = "";
          DBG("### Network time zone updated.");
        } else if (data.endsWith(GF("DST:"))) {
          streamSkipUntil(
              '\n');  // Refresh Network Daylight Saving Time by network
          data = "";
          DBG("### Daylight savings time state updated.");
        }
      }
    } while (millis() - startMillis < timeout_ms);
  finish:
    if (!index) {
      data.trim();
      if (data.length()) { DBG("### Unhandled:", data); }
      data = "";
    }
    // data.replace(GSM_NL, "/");
    // DBG('<', index, '>', data);
    return index;
  }

  int8_t waitResponse(uint32_t timeout_ms, GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    String data;
    return waitResponse(timeout_ms, data, r1, r2, r3, r4, r5);
  }

  int8_t waitResponse(GsmConstStr r1 = GFP(GSM_OK),
                      GsmConstStr r2 = GFP(GSM_ERROR),
#if defined TINY_GSM_DEBUG
                      GsmConstStr r3 = GFP(GSM_CME_ERROR),
                      GsmConstStr r4 = GFP(GSM_CMS_ERROR),
#else
                      GsmConstStr r3 = NULL, GsmConstStr r4 = NULL,
#endif
                      GsmConstStr r5 = NULL) {
    return waitResponse(1000, r1, r2, r3, r4, r5);
  }

 public:
  Stream& stream;

 protected:
  GsmClientGL868* sockets[TINY_GSM_MUX_COUNT];
  const char*      gsmNL = GSM_NL;
};

#endif  // SRC_TINYGSMCLIENTGL868_H_
