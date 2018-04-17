/**
 * @file       TinyGsmClientM95.h
 * @author     Volodymyr Shymanskyy, Replicade Ltd.
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy, (c)2017 Replicade Ltd. <http://www.replicade.com>
 * @date       Nov 2016
 */

#ifndef TinyGsmClientM95_h
#define TinyGsmClientM95_h

#define TINY_GSM_DEBUG Serial
#define TINY_GSM_USE_HEX

#if !defined(TINY_GSM_RX_BUFFER)
  #define TINY_GSM_RX_BUFFER 64
#endif

#include <TinyGsmCommon.h>

#define GSM_NL "\r\n"
static const char GSM_OK[] TINY_GSM_PROGMEM = "OK" GSM_NL;
static const char GSM_ERROR[] TINY_GSM_PROGMEM = "ERROR" GSM_NL;

enum SimStatus {
  SIM_ERROR = 0,
  SIM_READY = 1,
  SIM_LOCKED = 2,
};

enum RegStatus {
  REG_UNREGISTERED = 0,
  REG_SEARCHING    = 2,
  REG_DENIED       = 3,
  REG_OK_HOME      = 1,
  REG_OK_ROAMING   = 5,
  REG_UNKNOWN      = 4,
};


class TinyGsm {

public:
    TinyGsm(Stream& stream) : stream(stream) {}

    class GsmClient : public Client {
        friend class TinyGsm;
        typedef TinyGsmFifo<uint8_t, TINY_GSM_RX_BUFFER> RxFifo;

    public:
        GsmClient() {}

        GsmClient(TinyGsm& modem, uint8_t mux = 1) {
            init(&modem, mux);
        }

        bool init(TinyGsm* modem, uint8_t mux = 1) {
            this->at = modem;
            this->mux = mux;
            sock_available = 0;
            sock_connected = false;

            at->sockets[mux] = this;

            return true;
        }

        virtual int connect(const char *host, uint16_t port) {
            TINY_GSM_YIELD();
            rx.clear();
            sock_connected = at->modemConnect(host, port, mux);
            return sock_connected;
        }

        virtual int connect(IPAddress ip, uint16_t port) {
            String host; host.reserve(16);
            host += ip[0];
            host += ".";
            host += ip[1];
            host += ".";
            host += ip[2];
            host += ".";
            host += ip[3];
            return connect(host.c_str(), port);
        }

        virtual void stop() {
            TINY_GSM_YIELD();
            at->sendAT(GF("+QICLOSE"));
            sock_connected = false;
            at->waitResponse(60000L, GF("CLOSED"), GF("CLOSE OK"), GF("ERROR"));
        }

        virtual size_t write(const uint8_t *buf, size_t size) {
            TINY_GSM_YIELD();
            at->maintain();
            return at->modemSend(buf, size, mux);
        }

        virtual size_t write(uint8_t c) {
            return write(&c, 1);
        }

        virtual int available() {
            TINY_GSM_YIELD();
            if (!rx.size()) {
              at->maintain();
            }
            return rx.size() + sock_available;
        }

        virtual int read(uint8_t *buf, size_t size) {
            TINY_GSM_YIELD();
            at->maintain();
            size_t cnt = 0;
            while (cnt < size) {
                size_t chunk = TinyGsmMin(size-cnt, rx.size());
                if (chunk > 0) {
                    rx.get(buf, chunk);
                    buf += chunk;
                    cnt += chunk;
                    continue;
                }
                // TODO: Read directly into user buffer?
                at->maintain();
                if (sock_available > 0) {
                    at->modemRead(rx.free(), mux);
                } else {
                    break;
                }
            }
            return cnt;
        }

        virtual int read() {
            uint8_t c;
            if (read(&c, 1) == 1) {
                return c;
            }
            return -1;
        }

        virtual int peek() { return -1; } //TODO
        virtual void flush() { at->stream.flush(); }

        virtual uint8_t connected() {
            if (available()) {
                return true;
            }
            return sock_connected;
        }
        virtual operator bool() { return connected(); }

    private:
        TinyGsm*      at;
        uint8_t       mux;
        uint16_t      sock_available;
        bool          sock_connected;
        RxFifo        rx;
    };

    // Basic functions
    bool begin() {
        return init();
    }

    bool init() {
        if (!autoBaud()) {
            return false;
        }
        sendAT(GF("&FZE0"));  // Factory + Reset + Echo Off
        if (waitResponse() != 1) {
            return false;
        }
        getSimStatus();
        return true;
    }

    bool autoBaud(unsigned long timeout = 10000L) {
        for (unsigned long start = millis(); millis() - start < timeout; ) {
            sendAT(GF(""));
            if (waitResponse(200) == 1) {
                delay(100);
                return true;
            }
            delay(100);
        }
        return false;
    }

    void maintain() {
        while (stream.available()) {
            waitResponse(10, NULL, NULL);
        }
    }

    bool factoryDefault() {
        sendAT(GF("&FZE0&W"));  // Factory + Reset + Echo Off + Write
        waitResponse();
        sendAT(GF("+IPR=0"));   // Auto-baud
        waitResponse();
        sendAT(GF("+IFC=0,0")); // No Flow Control
        waitResponse();
        sendAT(GF("+ICF=3,3")); // 8 data 0 parity 1 stop
        waitResponse();
        sendAT(GF("+CSCLK=0")); // Disable Slow Clock
        waitResponse();
        sendAT(GF("&W"));       // Write configuration
        return waitResponse() == 1;
    }

    // Power functions

    bool restart() {
        if (!autoBaud()) {
            return false;
        }
        sendAT(GF("+CFUN=0"));
        if (waitResponse(10000L, GF("NORMAL POWER DOWN"), GF("OK"), GF("FAIL")) == 3) {
            return false;
        }
        sendAT(GF("+CFUN=1"));
        if (waitResponse(10000L, GF("Call Ready"), GF("OK"), GF("FAIL")) == 3) {
            return false;
        }
        return init();
    }

    // SIM card & Networ Operator functions

    bool simUnlock(const char *pin) {
        sendAT(GF("+CPIN=\""), pin, GF("\""));
        return waitResponse() == 1;
    }

    String getSimCCID() {
        sendAT(GF("+ICCID"));
        if (waitResponse(GF(GSM_NL "+ICCID:")) != 1) {
            return "";
        }
        String res = stream.readStringUntil('\n');
        waitResponse();
        res.trim();
        return res;
    }

    String getIMEI() {
        sendAT(GF("+GSN"));
        if (waitResponse(GF(GSM_NL)) != 1) {
            return "";
        }
        String res = stream.readStringUntil('\n');
        waitResponse();
        res.trim();
        return res;
    }

    int getSignalQuality() {
        sendAT(GF("+CSQ"));
        if (waitResponse(GF(GSM_NL "+CSQ:")) != 1) {
            return 99;
        }
        int res = stream.readStringUntil(',').toInt();
        waitResponse();
        return res;
    }

    String getClock() {
        sendAT(GF("+QLTS"));
        if (waitResponse(GF(GSM_NL "+QLTS:")) != 1) {
            // Serial.println( F("No response from AT+QLTS") );
            return "";
        }
        String res = stream.readStringUntil('\n');
        waitResponse();
        return res;
    }

    SimStatus getSimStatus(unsigned long timeout = 10000L) {
        for (unsigned long start = millis(); millis() - start < timeout; ) {
            sendAT(GF("+CPIN?"));
            if (waitResponse(GF(GSM_NL "+CPIN:")) != 1) {
                delay(1000);
                continue;
            }
            int status = waitResponse(GF("READY"), GF("SIM PIN"), GF("SIM PUK"), GF("NOT INSERTED"));
            waitResponse();
            switch (status) {
                case 2:
                case 3:  return SIM_LOCKED;
                case 1:  return SIM_READY;
                default: return SIM_ERROR;
            }
        }
        return SIM_ERROR;
    }

    RegStatus getRegistrationStatus() {
        sendAT(GF("+CREG?"));
        if (waitResponse(GF(GSM_NL "+CREG:")) != 1) {
            return REG_UNKNOWN;
        }
        streamSkipUntil(','); // Skip format (0)
        int status = stream.readStringUntil('\n').toInt();
        waitResponse();
        return (RegStatus)status;
    }

    String getOperator() {
        sendAT(GF("+COPS?"));
        if (waitResponse(GF(GSM_NL "+COPS:")) != 1) {
            return "";
        }
        streamSkipUntil('"'); // Skip mode and format
        String res = stream.readStringUntil('"');
        waitResponse();
        return res;
    }

    bool waitForNetwork(unsigned long timeout = 60000L) {
        for (unsigned long start = millis(); millis() - start < timeout; ) {
            RegStatus s = getRegistrationStatus();
            if (s == REG_OK_HOME || s == REG_OK_ROAMING) {
                return true;
            } else if (s == REG_UNREGISTERED) {
                return false;
            }
            delay(1000);
        }
        return false;
    }

    void setHostFormat( bool useDottedQuad ) {
        if ( useDottedQuad ) {
            sendAT(GF("+QIDNSIP=0"));
        } else {
            sendAT(GF("+QIDNSIP=1"));
        }
        waitResponse();
    }

    // GPRS functions
    bool gprsConnect(const char* apn, const char* user, const char* pwd) {
        gprsDisconnect();

        sendAT(GF("+QIFGCNT=0"));
        waitResponse();

        sendAT(GF("+QICSGP=1,\""), apn, GF("\",\""), user, GF("\",\""), pwd, GF("\""));
        waitResponse();

        sendAT(GF("+QIREGAPP"));
        waitResponse();

        sendAT(GF("+QIACT"));
        waitResponse(10000L);

        return true;
    }

    bool gprsDisconnect() {
        sendAT(GF("+QIDEACT"));
        return waitResponse(60000L, GF("DEACT OK"), GF("ERROR")) == 1;
    }

    // Phone Call functions

    bool setGsmBusy(bool busy = true) {
        sendAT(GF("+GSMBUSY="), busy ? 1 : 0);
        return waitResponse() == 1;
    }

    bool callAnswer() {
        sendAT(GF("A"));
        return waitResponse() == 1;
    }

    bool callNumber(const String& number) {
        sendAT(GF("D"), number);
        return waitResponse() == 1;
    }

    bool callHangup(const String& number) {
        sendAT(GF("H"), number);
        return waitResponse() == 1;
    }

    // Messaging functions

    bool sendSMS(const String& number, const String& text) {
        sendAT(GF("+CMGF=1"));
        waitResponse();
        sendAT(GF("+CMGS=\""), number, GF("\""));
        if (waitResponse(GF(">")) != 1) {
            return false;
        }
        stream.print(text);
        stream.write((char)0x1A);
        return waitResponse(60000L) == 1;
    }

    bool sendSMS_UTF16(const String& number, const void* text, size_t len) {
        sendAT(GF("+CMGF=1"));
        waitResponse();
        sendAT(GF("+CSCS=\"HEX\""));
        waitResponse();
        sendAT(GF("+CSMP=17,167,0,8"));
        waitResponse();

        sendAT(GF("+CMGS=\""), number, GF("\""));
        if (waitResponse(GF(">")) != 1) {
            return false;
        }

        uint16_t* t = (uint16_t*)text;
        for (size_t i=0; i<len; i++) {
            uint8_t c = t[i] >> 8;
            if (c < 0x10) { stream.print('0'); }
            stream.print(c, HEX);
            c = t[i] & 0xFF;
            if (c < 0x10) { stream.print('0'); }
            stream.print(c, HEX);
        }
        stream.write((char)0x1A);
        return waitResponse(60000L) == 1;
    }

    void sendUSSD() {}

    void sendSMS() {}

    /** Delete all SMS */
    bool deleteAllSMS() {
        sendAT(GF("+QMGDA=6"));
        if (waitResponse(waitResponse(60000L, GF("OK"), GF("ERROR")) == 1) ) {
            return true;
        }
        return false;
    }

    // Location functions
    void getLocation() {}

    String getGsmLocation() {
        sendAT(GF("+CIPGSMLOC=1,1"));
        if (waitResponse(GF(GSM_NL "+CIPGSMLOC:")) != 1) {
            return "";
        }
        String res = stream.readStringUntil('\n');
        waitResponse();
        res.trim();
        return res;
    }

    // Battery functions

    // HTTP functions
    void httpGet( const char *url ) {
        int len = strlen( url );
        sendAT( GF("+QHTTPURL="), len);
        delay( 500 );
        streamWrite(url, GSM_NL);
        stream.flush();
        waitResponse(5000L, GF("OK"));

        sendAT( GF("+QHTTPGET"));
        waitResponse(30000L);

        String data;
        sendAT( GF("+QHTTPREAD"));
        waitResponse(30000L, data);
    }

protected:
    Stream&       stream;
    GsmClient*    sockets[5];

    int modemConnect(const char* host, uint16_t port, uint8_t mux) {
        sendAT(GF("+QIOPEN="), GF("\"TCP"), GF("\",\""), host, GF("\","), port);
        int rsp = waitResponse(75000L,
                               GF("CONNECT OK" GSM_NL),
                               GF("CONNECT FAIL" GSM_NL),
                               GF("ALREADY CONNECT" GSM_NL));
        if ( rsp != 1 ) {
            return 0;
        }
        return (1 == rsp);
    }

    int modemSend(const void* buff, size_t len, uint8_t mux) {
        sendAT(GF("+QIHEAD=1"));
        waitResponse(5000L);

        sendAT(GF("+QISEND="),len);
        if (waitResponse(GF(">")) != 1) {
            return -1;
        }
        stream.write((uint8_t*)buff, len);
        if (waitResponse(GF(GSM_NL "SEND OK")) != 1) {
            return -1;
        }

        bool allAcknowledged = false;
        // bool failed = false;
        while ( !allAcknowledged ) {
            sendAT( GF("+QISACK"));
            if (waitResponse(5000L, GF(GSM_NL "+QISACK:")) != 1) {
                return -1;
            } else {
                streamSkipUntil(','); /** Skip total */
                streamSkipUntil(','); /** Skip acknowledged data size */
                if ( stream.readStringUntil('\n').toInt() == 0 ) {
                    allAcknowledged = true;
                }
            }
        }
        waitResponse(5000L);

        // streamSkipUntil(','); // Skip mux
        // return stream.readStringUntil('\n').toInt();
        return 1;
    }

    size_t modemRead(size_t size, uint8_t mux) {
#ifdef TINY_GSM_USE_HEX
        sendAT(GF("+CIPRXGET=3,"), mux, ',', size);
        if (waitResponse(GF("+CIPRXGET:")) != 1) {
            return 0;
        }
#else
        sendAT(GF("+CIPRXGET=2,"), mux, ',', size);
        if (waitResponse(GF("+CIPRXGET:")) != 1) {
            return 0;
        }
#endif
        streamSkipUntil(','); // Skip mode 2/3
        streamSkipUntil(','); // Skip mux
        size_t len = stream.readStringUntil(',').toInt();
        sockets[mux]->sock_available = stream.readStringUntil('\n').toInt();

        for (size_t i=0; i<len; i++) {
#ifdef TINY_GSM_USE_HEX
            while (stream.available() < 2) {}
            char buf[4] = { 0, };
            buf[0] = stream.read();
            buf[1] = stream.read();
            char c = strtol(buf, NULL, 16);
#else
            while (!stream.available()) {}
            char c = stream.read();
#endif
            sockets[mux]->rx.put(c);
        }
        waitResponse();
        return len;
    }

    size_t modemGetAvailable(uint8_t mux) {
        sendAT(GF("+CIPRXGET=4,"), mux);
        size_t result = 0;
        if (waitResponse(GF("+CIPRXGET:")) == 1) {
            streamSkipUntil(','); // Skip mode 4
            streamSkipUntil(','); // Skip mux
            result = stream.readStringUntil('\n').toInt();
            waitResponse();
        }
        if (!result) {
            sockets[mux]->sock_connected = modemGetConnected(mux);
        }
        return result;
    }

    bool modemGetConnected(uint8_t mux) {
        sendAT(GF("+CIPSTATUS="), mux);
        int res = waitResponse(GF(",\"CONNECTED\""), GF(",\"CLOSED\""), GF(",\"CLOSING\""), GF(",\"INITIAL\""));
        waitResponse();
        return 1 == res;
    }

    // Utilities
    template<typename T>
    void streamWrite(T last) {
        stream.print(last);
    }

    template<typename T, typename... Args>
    void streamWrite(T head, Args... tail) {
        stream.print(head);
        streamWrite(tail...);
    }

    int streamRead() { return stream.read(); }

    bool streamSkipUntil(char c, int timeoutInMs=1000) {
        unsigned long startTime = millis();
        unsigned long t = millis();
        while ( t < (startTime + timeoutInMs) ) {
            while (!stream.available() && (t < (startTime + timeoutInMs)) ) {
                t = millis();
                delay( 250 );
            }
            if ( stream.available() ) {
                if (stream.read() == c) {
                    return true;
                }
            }
            t = millis();
        }
        return false;
    }

    template<typename... Args>
    void sendAT(Args... cmd) {
        streamWrite("AT", cmd..., GSM_NL);
        stream.flush();
        TINY_GSM_YIELD();
        DBG("### AT:", cmd...);
    }

    // TODO: Optimize this!
    uint8_t waitResponse(uint32_t timeout, String& data,
                         GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                         GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL) {
        /* String r1s(r1); r1s.trim();
        String r2s(r2); r2s.trim();
        String r3s(r3); r3s.trim();
        String r4s(r4); r4s.trim();
        String r5s(r5); r5s.trim();
        DBG("### ..:", r1s, ",", r2s, ",", r3s, ",", r4s, ",", r5s); */
        data.reserve(64);
        bool gotData = false;
        int mux = -1;
        int index = 0;
        unsigned long startMillis = millis();
        do {
            TINY_GSM_YIELD();
            while (stream.available() > 0) {
                int a = streamRead();
                if (a <= 0) continue; // Skip 0x00 bytes, just in case
                Serial.print((char)a);
                data += (char)a;
                if (r1 && data.endsWith(r1)) {
                    index = 1;
                    DBG("<<< Got1 '", String(r1), "'");
                    goto finish;
                } else if (r2 && data.endsWith(r2)) {
                    index = 2;
                    DBG("<<< Got2 '", String(r2), "'");
                    goto finish;
                } else if (r3 && data.endsWith(r3)) {
                    index = 3;
                    DBG("<<< Got3 '", String(r3), "'");
                    goto finish;
                } else if (r4 && data.endsWith(r4)) {
                    index = 4;
                    DBG("<<< Got4 '", String(r4), "'");
                    goto finish;
                } else if (r5 && data.endsWith(r5)) {
                    index = 5;
                    DBG("<<< Got5 '", String(r5), "'");
                    goto finish;
                } else if (data.endsWith(GF(GSM_NL "+CIPRXGET:"))) {
                    String mode = stream.readStringUntil(',');
                    if (mode.toInt() == 1) {
                        mux = stream.readStringUntil('\n').toInt();
                        gotData = true;
                        data = "";
                    } else {
                        data += mode;
                    }
                } else if (data.endsWith(GF("CLOSED" GSM_NL))) {
                    int nl = data.lastIndexOf(GSM_NL, data.length()-8);
                    int coma = data.indexOf(',', nl+2);
                    mux = data.substring(nl+2, coma).toInt();
                    if (mux) {
                        sockets[mux]->sock_connected = false;
                        data = "";
                    }
                }
            }
        } while (millis() - startMillis < timeout);
        finish:
            if (!index) {
                data.trim();
                if (data.length()) {
                    DBG("### Unhandled:", data);
                }
                data = "";
            }
            if (gotData) {
                sockets[mux]->sock_available = modemGetAvailable(mux);
            }
            return index;
    }

    uint8_t waitResponse(uint32_t timeout,
                         GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                         GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL) {
        String data;
        return waitResponse(timeout, data, r1, r2, r3, r4, r5);
    }

    uint8_t waitResponse(GsmConstStr r1=GFP(GSM_OK), GsmConstStr r2=GFP(GSM_ERROR),
                         GsmConstStr r3=NULL, GsmConstStr r4=NULL, GsmConstStr r5=NULL) {
        return waitResponse(1000, r1, r2, r3, r4, r5);
    }

    void printResponse(uint32_t timeout) {
        unsigned long startMillis = millis();
        do {
            TINY_GSM_YIELD();
            while (stream.available() > 0) {
                int a = streamRead();
                if (a <= 0) continue; // Skip 0x00 bytes, just in case
                Serial.print((char)a);
            }
        } while (millis() - startMillis < timeout);
    }

    uint16_t waitForHTTPResponse(uint32_t timeout, char * buffer, uint8_t bufferSize) {
        unsigned long startMillis = millis();
        uint8_t bufferIndex = 0;
        uint16_t httpCode = 0;
        do {
            TINY_GSM_YIELD();
            while (stream.available() > 0) {
                int a = streamRead();
                if (a <= 0) continue; // Skip 0x00 bytes, just in case
                Serial.print((char)a);
                if (bufferIndex < bufferSize-1) { // leave room to null-terminate
                    buffer[bufferIndex] = (char)a;
                    bufferIndex++;
                }
                if ((char)a == '\n') {
                    // null-terminate buffer!
                    buffer[bufferIndex] = '\0';
                    if (strstr((const char *)buffer, "CLOSED") != NULL) {
                        return httpCode;
                    }
                    if(httpCode == 0 && strstr((const char *)buffer, "HTTP/1.") != NULL) {
                        strtok((char *)buffer, " ");        /** Parse out HTTP/1.x */

                        char * codeString = strtok(NULL, " ");
                        httpCode = atoi((const char *)codeString);
                    }
                    bufferIndex = 0;
                }
            }
        } while (millis() - startMillis < timeout);

        return httpCode;
    }
};

typedef TinyGsm::GsmClient TinyGsmClient;

#endif
