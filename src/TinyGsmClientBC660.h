/**
 * @file       TinyGsmClientBC660.h
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Apr 2018
 */

 #ifndef SRC_TINYGSMCLIENTBC660_H_
 #define SRC_TINYGSMCLIENTBC660_H_
 // #pragma message("TinyGSM:  TinyGsmClientBC660")
 
 // #define TINY_GSM_DEBUG Serial
 
 #define TINY_GSM_MUX_COUNT 5
 #define TINY_GSM_NO_MODEM_BUFFER
 
 #include "TinyGsmBattery.tpp"
 #include "TinyGsmGPRS.tpp"
 #include "TinyGsmModem.tpp"
 #include "TinyGsmTCP.tpp"
 #include "TinyGsmTime.tpp"
 #include "TinyGsmNTP.tpp"
 
 #ifdef AT_NL
 #undef AT_NL
 #endif
 #define AT_NL "\r\n"
 
 #if defined       TINY_GSM_DEBUG
 static const char GSM_CME_ERROR[] TINY_GSM_PROGMEM = AT_NL "+CME ERROR:";
 static const char GSM_CMS_ERROR[] TINY_GSM_PROGMEM = AT_NL "+CMS ERROR:";
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
 
 class TinyGsmBC660 : public TinyGsmModem<TinyGsmBC660>,
                     public TinyGsmGPRS<TinyGsmBC660>,
                     public TinyGsmTCP<TinyGsmBC660, TINY_GSM_MUX_COUNT>,
                     public TinyGsmNTP<TinyGsmBC660>,
                     public TinyGsmBattery<TinyGsmBC660> {
   friend class TinyGsmModem<TinyGsmBC660>;
   friend class TinyGsmGPRS<TinyGsmBC660>;
   friend class TinyGsmTCP<TinyGsmBC660, TINY_GSM_MUX_COUNT>;
   friend class TinyGsmNTP<TinyGsmBC660>;
   friend class TinyGsmBattery<TinyGsmBC660>;
 
   /*
    * Inner Client
    */
  public:
   class GsmClientBC660 : public GsmClient {
     friend class TinyGsmBC660;
 
    public:
     GsmClientBC660() {}
 
     explicit GsmClientBC660(TinyGsmBC660& modem, uint8_t mux = 0) {
       secure = false;
       init(&modem, mux);
     }
 
     bool init(TinyGsmBC660* modem, uint8_t mux = 0) {
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
       uint32_t startMillis = millis();
       dumpModemBuffer(maxWaitMs);
       if(secure){
         at->sendAT(GF("+QSSLCLOSE=0,"), mux);
         if (at->waitResponse((maxWaitMs - (millis() - startMillis))) == 1) { 
           if(at->waitResponse((maxWaitMs - (millis() - startMillis)), GF("+QSSLCLOSE:"))==1){
             at->streamSkipUntil(',');                  // Skip contextid
             at->streamSkipUntil(',');                  // Skip connection id
             int8_t err = at->streamGetIntBefore('\n'); // socket state
             //DBG("+QSSLCLOSE state:", err);
           }
         }
       }else{
         at->sendAT(GF("+QICLOSE="), mux);
         if (at->waitResponse((maxWaitMs - (millis() - startMillis))) == 1) { 
           at->waitResponse((maxWaitMs - (millis() - startMillis)), GF("CLOSE OK"));
         }
       }
       sock_connected = false;
     }
 
     void stop() override {
       stop(15000L);
     }
 
     /*
      * Extended API
      */
 
     String remoteIP() TINY_GSM_ATTR_NOT_IMPLEMENTED;
 
     protected:
     bool secure;
   };
 
   /*
    * Inner Secure Client
    */
 
   
   class GsmClientSecureBC660 : public GsmClientBC660
   {
   public:
     GsmClientSecureBC660() {
       secure = true;
     }
 
     GsmClientSecureBC660(TinyGsmBC660& modem, uint8_t mux = 0)
      : GsmClientBC660(modem, mux)
     {
       if(mux!=0){
         DBG(GF("### SSL Only supported on mux 0!"));
       }
       secure = true;
     }
 
 
   public:
     int connect(const char* host, uint16_t port, int timeout_s) override {
       stop();
       TINY_GSM_YIELD();
       rx.clear();
       sock_connected = at->modemConnect(host, port, mux, true, timeout_s);
       return sock_connected;
     }
     TINY_GSM_CLIENT_CONNECT_OVERRIDES
   };
   
 
   /*
    * Constructor
    */
  public:
   explicit TinyGsmBC660(Stream& stream) : stream(stream) {
     memset(sockets, 0, sizeof(sockets));
   }
 
   /*
    * Basic functions
    */
  protected:
   bool initImpl(const char* pin = NULL) {
     DBG(GF("### TinyGSM Version:"), TINYGSM_VERSION);
     DBG(GF("### TinyGSM Compiled Module:  TinyGsmClientBC660"));
 
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
 
     // disable sleep
     sleepEnable(false);
 
     // Disable time and time zone URC's
     sendAT(GF("+CTZR=0"));
     if (waitResponse(10000L) != 1) { return false; }
 
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
 
    String getModemNameImpl() {
     sendAT(GF("+CGMM"));
     String modelName;
     if (waitResponse(1000L, modelName) != 1) { return "unknown"; }
     modelName.replace("\r\nOK\r\n", "");
     modelName.replace("\rOK\r", "");
     modelName.trim();
     return modelName;
   }
 
   /*
    * Power functions
    */
  protected:
   bool restartImpl(const char* pin = NULL) {
     if (!testAT()) { return false; }
     sendAT("+QRST=1");
     waitResponse(5000L);
     return init(pin);
   }
 
   bool powerOffImpl() {
     sendAT(GF("+QPOWD=1"));
     return waitResponse(10000L, GF("OK")) == 1;
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
 
   bool setPhoneFunctionalityImpl(uint8_t fun) {
     sendAT(GF("+CFUN="), fun);
     return waitResponse(10000L, GF("OK")) == 1;
   }
 
   /*
    * Generic network functions
    */
  public:
   RegStatus getRegistrationStatus() {
     // Check first for EPS registration
     RegStatus epsStatus = (RegStatus)getRegistrationStatusXREG("CEREG");
 
     // If we're connected on EPS, great!
     if (epsStatus == REG_OK_HOME || epsStatus == REG_OK_ROAMING) {
       return epsStatus;
     } else {
       // Otherwise, check generic network status
       return (RegStatus)getRegistrationStatusXREG("CREG");
     }
   }
 
  protected:
   bool isNetworkConnectedImpl() {
     RegStatus s = getRegistrationStatus();
     return (s == REG_OK_HOME || s == REG_OK_ROAMING);
   }
 
   /*
    * GPRS functions
    */
  protected:
   bool gprsConnectImpl(const char* apn, const char* user = NULL,
                        const char* pwd = NULL) {
     gprsDisconnect();
 
     sendAT(GF("+CGDCONT=1,\"IP\",\""), apn, '"');
     waitResponse();
 
     if(!user || !pwd || strcmp(user, "")==0 || strcmp(pwd, "")==0){
       sendAT(GF("+CGAUTH=1,0"));
     }else{
       sendAT(GF("+CGAUTH=1,1,\""), user, GF("\",\""), pwd, GF("\""));
     }
     if (waitResponse(60000L) != 1) { return false; }
 
     sendAT(GF("+CGACT=1,1"));
     waitResponse(70000L);
     return true;
   }
 
   bool gprsDisconnectImpl() {
     sendAT(GF("+CGACT=0,1"));
     if (waitResponse(40000L) != 1) { return false; }
     return true;
   }
 
   /*
    * SIM card functions
    */
  protected:
   String getSimCCIDImpl() {
     sendAT(GF("+QCCID"));
     if (waitResponse(GF(AT_NL "+QCCID:")) != 1) { return ""; }
     String res = stream.readStringUntil('\n');
     waitResponse();
     // remove new lines
     res.trim();
     // remove trailing checksum
     res.remove(res.length()-1, 1);
     return res;
   }

     // This uses "CGSN" instead of "GSN"
    String getIMEIImpl() {
      sendAT(GF("+CGSN"));
      if (waitResponse(GF(AT_NL)) != 1) { return ""; }
      String res = stream.readStringUntil('\n');
      waitResponse();
      res.trim();
      return res;
    }
 
   /*
    * Battery functions
    */
  protected:
 
   // Use: float vBatt = modem.getBattVoltage() / 1000.0;
   uint16_t getBattVoltageImpl() {
     sendAT(GF("+CBC"));
     if (waitResponse(GF("+CBC:")) != 1) { return 0; }
     // return voltage in mV
     uint16_t res = streamGetIntBefore('\n');
     // Wait for final OK
     waitResponse();
     return res;
   }
 
   /*
    * NTP server functions
    */
   byte NTPServerSyncImpl(String server = "pool.ntp.org", byte = -5) {
     // Request network synchronization
     // AT+QNTP=<contextID>,<server>[,<port>][,<autosettime>]
     sendAT(GF("+QNTP=1,\""), server, '"');
     if (waitResponse(10000L, GF("+QNTP:"))) {
       String result = stream.readStringUntil(',');
       streamSkipUntil('\n');
       result.trim();
       if (TinyGsmIsValidNumber(result)) { return result.toInt(); }
     } else {
       return -1;
     }
     return -1;
   }
 
   String ShowNTPErrorImpl(byte error) TINY_GSM_ATTR_NOT_IMPLEMENTED;
 
   /*
    * Client related functions
    */
  protected:
   bool modemConnect(const char* host, uint16_t port, uint8_t mux,
                     bool ssl = false, int timeout_s = 150) {
 
     uint32_t timeout_ms = ((uint32_t)timeout_s) * 1000;
 
     // <PDPcontextID>(1-16), <connectID>(0-11),
     // "TCP/UDP/TCP LISTENER/UDPSERVICE", "<IP_address>/<domain_name>",
     // <remote_port>,<local_port>,<access_mode>(0-2; 0=buffer)
     if(ssl){
       sendAT(GF("+QSSLCFG=0,0,\"timeout\",300"));
       waitResponse();
 
       sendAT(GF("+QSSLOPEN=0,0,\""), host, GF("\","), port, GF(",0"));
       waitResponse();
 
       if (waitResponse(timeout_ms, GF(AT_NL "+QSSLOPEN:")) != 1) { return false; }
     }else{
       sendAT(GF("+QIOPEN=0,"), mux, GF(",\""), GF("TCP"), GF("\",\""), host,
                 GF("\","), port, GF(",0,1"));
       waitResponse();
       if (waitResponse(timeout_ms, GF(AT_NL "+QIOPEN:")) != 1) { return false; }
     }
 
     if (streamGetIntBefore(',') != mux) { return false; }
     // Read status
     return (0 == streamGetIntBefore('\n'));
   }
 
   int16_t modemSend(const void* buff, size_t len, uint8_t mux) {
      bool ssl = sockets[mux]->secure;
     if(ssl){
       sendAT(GF("+QSSLSEND=0,"), mux, ',', (uint16_t)len);
     }else{
       sendAT(GF("+QISEND="), mux, ',', (uint16_t)len);
     }
     if (waitResponse(GF(">")) != 1) { return 0; }
     stream.write(reinterpret_cast<const uint8_t*>(buff), len);
     stream.flush();
     if(ssl){
       if (waitResponse() != 1) { return 0; }
       if (waitResponse(GF("+QSSLSEND:")) != 1) { return 0; }
       streamSkipUntil(',');                  // Skip contextid
       streamSkipUntil(',');                  // Skip connection id
       int8_t err = streamGetIntBefore('\n'); // Send state
       //DBG("Send state:", err);
      if(err!=0) return 0;
     }else{
       if (waitResponse(GF(AT_NL "SEND OK")) != 1) { return 0; }
     }
     // TODO(?): Wait for ACK? AT+QISEND=id,0
     return len;
   }
 
   bool modemGetConnected(uint8_t mux) {
       return sockets[mux]->sock_connected;
   }
 
   /*
    * Utilities
    */
  public:

    // Handles unsolicited URCs from Quectel BC660, such as:
    // Example 1: +QSSLURC: "recv",0,0,4,"payload"
    // Example 2: +QSSLURC: "closed",0,0
    bool handleURCs(String& data) {
        // Check for relevant URCs from the stream
        if (data.endsWith(GF("+QSSLURC:")) || data.endsWith(GF("+QIURC:"))) {
        
          bool contextExpected = data.endsWith(GF("+QSSLURC:"));

          streamSkipUntil('\"');                         // Skip to the URC type (e.g., "recv" or "closed")
          String urc = stream.readStringUntil('\"');     // Read the URC type
      
          if (urc == "recv") {
              // Expected format: "recv",<contextID>,<connectID>,<length>,<binary_data>
              streamSkipUntil(',');                        // Skip comma
              if(contextExpected){
                int contextID  = streamGetIntBefore(',');     // Extract context ID
              }
              int mux  = streamGetIntBefore(',');           // Extract connect ID
              int16_t length = streamGetIntBefore(',');     // Extract data length
      
              if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
                  int len = length;
                  if (len > sockets[mux]->rx.free()) {
                      // If buffer is smaller than incoming data, adjust length
                      DBG("### Buffer overflow: ", len, "->", sockets[0]->rx.free());
                      len = sockets[mux]->rx.free();
                  }
          
                  // Skip until the start of the binary data (assumes it's after a '"')
                  while (stream.read() != '\"') {}            // Consume until starting '"'
          
                  // Read the binary data byte by byte into the RX FIFO
                  while (len--) {
                      moveCharFromStreamToFifo(0);
                  }

                  if (length != sockets[mux]->available()) {
                      DBG("### Different number of characters received than expected: ",
                      sockets[mux]->available(), " vs ", length);
                  }

                  // Skip the ending '"'
                  if (stream.peek() == '\"') stream.read();

                  DBG("### Got Data:", length, "on socket", mux);
              }
          } else if (urc == "closed") {
              // Expected format: "closed",<contextID>,<connectID>
              streamSkipUntil(',');                        // Skip comma
              if(contextExpected){
                int contextID  = streamGetIntBefore(',');     // Extract context ID
              }
              int mux = streamGetIntBefore('\n');          // Extract connect ID
      
              if (mux >= 0 && mux < TINY_GSM_MUX_COUNT && sockets[mux]) {
                  // Mark the socket as disconnected
                  sockets[0]->sock_connected = false;
                  DBG("### Closed: ", mux);
              }
              
          }
      
          data = "";  // Clear the input buffer
          return true;
        }
    
        return false;
    }
 
  public:
   Stream& stream;
 
  protected:
   GsmClientBC660* sockets[TINY_GSM_MUX_COUNT];
   const char*    gsmNL = AT_NL;
 };
 
 #endif  // SRC_TINYGSMCLIENTBC660_H_ 