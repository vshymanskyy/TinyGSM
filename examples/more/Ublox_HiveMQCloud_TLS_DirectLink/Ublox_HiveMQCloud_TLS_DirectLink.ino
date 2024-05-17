/* Tested on ESP32-S2 with Ublox Lara R280 modem*/

#define TINY_GSM_MODEM_UBLOX

#include <HardwareSerial.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include "certificates.h"

const char* broker = "your-mqtt-broker-hostname";
const uint16_t port = 8883;
const char* user = "your-mqtt-username";
const char* pass = "your-mqtt-password";
const char* topicLed = "GsmClientTest/led/tl";
const char* topicInit = "GsmClientTest/init/tl";

const char* apn = "your-apn";
const char* gprsUser = "your-gprs-username";
const char* gprsPass = "your-gprs-password";

HardwareSerial SerialAT(1);
TinyGsm modem(SerialAT);
TinyGsmClientSecure client(modem);
PubSubClient  mqtt(client);

uint32_t lastReconnectAttempt = 0;
uint32_t start_publish = millis();
uint32_t end_publish = millis();
uint32_t timer = millis();

uint8_t data[900];

void mqttCallback(char* topic, byte* payload, unsigned int len) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.write(payload, len);
  Serial.println();
}

boolean mqttConnect() {
  Serial.print("Connecting to ");
  Serial.print(broker);
  
  boolean status = mqtt.connect("tinygsmclienttest", user, pass);

  if (status == false) {
    Serial.printf(" fail, rc = %d \n", mqtt.state());
    return false;
  }
  Serial.println(" success");
  
  mqtt.subscribe(topicLed);
  return mqtt.connected();
}

// Add server CA cert to ublox modem memory
void addCACert() {
  // Search if server CA exists in modem memory
  Serial.println("Searching for hiveMQCA in modem CA storage...");
  modem.sendAT(GF("+USECMNG=3"));
  if (modem.waitResponse(GF("hiveMQCA")) == 1) {
    Serial.println("hiveMQCA found...");
    return;
  }

  // Server CA not found in memory, import now
  Serial.println("hiveMQCA not found...");
  Serial.println("Importing hiveMQCA...");
  
  int cert_size = sizeof(cert) - 1;
  modem.sendAT(GF("+USECMNG=0,0,\"hiveMQCA\","), cert_size);
  if (modem.waitResponse(GF(">")) != 1) {
    Serial.println("No response...");
    return;
  }
  Serial.print("Writing CA to modem... ");
  Serial.println(cert_size);
  for (int i = 0; i < cert_size; i++) {
    char c = pgm_read_byte(&cert[i]);
    modem.stream.write(c);
  }
  modem.stream.flush();
  if (modem.waitResponse(GF("OK")) != 1) {
    Serial.println("CA cert upload failed...");
    return;
  }
  Serial.println("CA cert upload completed...");
}

void setSSLProfile() {
  Serial.print("Reset SSL profile 0...");
  modem.sendAT(GF("+USECPRF=0"));
  if (modem.waitResponse(GF("OK")) != 1) {
    Serial.println(" fail");
    return;
  }
  Serial.println(" success");

  Serial.print("Set profile 0 certification to level 1...");
  modem.sendAT(GF("+USECPRF=0,0,1"));
  if (modem.waitResponse(GF("OK")) != 1) {
    Serial.println(" fail");
    return;
  }
  Serial.println(" success");

  Serial.print("Set profile 0 to use any validation TLS version...");
  modem.sendAT(GF("+USECPRF=0,1,0"));
  if (modem.waitResponse(GF("OK")) != 1) {
    Serial.println(" fail");
    return;
  }
  Serial.println(" success");

  Serial.print("Set profile 0 SNI to host...");
  modem.sendAT(GF("+USECPRF=0,10,\""), broker, "\"");
  if (modem.waitResponse(GF("OK")) != 1) {
    Serial.println(" fail");
    return;
  }
  Serial.println(" success");
  
  Serial.print("Set profile 0 root CA to hiveMQCA...");
  modem.sendAT(GF("+USECPRF=0,3,\"hiveMQCA\""));
  if (modem.waitResponse(GF("OK")) != 1) {
    Serial.println(" fail");
    return;
  }
  Serial.println(" success");
}

void setup() {
  Serial.begin(115200);
  delay(10);

  SerialAT.begin(115200);
  SerialAT.setPins(10, 9, 11, 12);
  SerialAT.setHwFlowCtrlMode();     // Required for direct link mode to prevent loss of data, as recommended by Ublox
  delay(10);

  Serial.println("Initializing modem...");
  if (!modem.init()) {
    Serial.println(" fail");
    delay(10000);
    return;
  }

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem Info: ");
  Serial.println(modemInfo);

  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isNetworkConnected()) { Serial.println("Network connected"); }

  Serial.print(F("Connecting to "));
  Serial.print(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    Serial.println(" fail");
    delay(10000);
    return;
  }
  Serial.println(" success");

  if (modem.isGprsConnected()) { Serial.println("GPRS connected"); }

  addCACert();
  setSSLProfile();
  modem.setSSLProfileID(0);     // set secure connection to use SSL profile 0

  mqtt.setServer(broker, port);
  mqtt.setCallback(mqttCallback);
  mqtt.setBufferSize(1024);

  mqttConnect();

  timer = millis();
  while (millis() - timer <= 3000) {
    mqtt.loop();
  }

  // some random data to be published to broker
  for (int j=0; j<9; j++) {
    for (int i=0; i<100; i++) {
      data[100 * j + i] = i;
    }
  }
}

void loop() {
  // Direct Link mode
  Serial.print("Enable direct link mode...");
  Serial.println(modem.enableDL());
  Serial.println("Publishing 36 kB data to broker...");     // Maximum 65535 bytes per transmission due to modem limitations?
  start_publish = millis();
  mqtt.beginPublish(topicInit, 36000, false);
  for (int i=0; i<40; i++) {
    mqtt.write((uint8_t*)&data, sizeof(data));
  }
  mqtt.endPublish();
  mqtt.flush();
  end_publish = millis();
  Serial.println("Publish ended...");
  Serial.print("Publish time used in Direct Link mode (ms): ");
  Serial.println(end_publish - start_publish);

  timer = millis();
  while (millis() - timer <= 10000) {
    // Network and GPRS connection check requires AT command mode, does not work in Direct Link mode

    // To check for host connection in Direct Link mode, client.read() must be called to catch the disconnection result code.
    // Modem will put back to AT command mode automatically when disconnected from host in Direct Link mode.
   
    // host connection check for other applications in Direct Link mode
    /*
    if (client.available()) {
      client.read();
      if (!client.connected()) {
        // client disconnected from host
      }
    }
    */
    
    // host connection check for mqtt broker in Direct Link mode using pubsubclient library
    if (!mqtt.connected()) {
      Serial.println("=== MQTT NOT CONNECTED ===");
      uint32_t t = millis();
      if (t - lastReconnectAttempt > 1000L) {
        lastReconnectAttempt = t;
        if (mqttConnect()) { lastReconnectAttempt = 0; }
      }
      delay(100);
      return;
    }
    mqtt.loop();
  }

  Serial.print("Disable direct link mode...");
  Serial.println(modem.disableDL());
  Serial.println();

  // AT command mode
  Serial.println("AT command mode");
  Serial.println("Publishing 36 kB data to broker...");
  start_publish = millis();
  mqtt.beginPublish(topicInit, 36000, false);
  for (int i=0; i<40; i++) {
    mqtt.write((uint8_t*)&data, sizeof(data));     // Maximum 1024 bytes packet per serial write with 50 ms delay per packet due AT command limitations
  }
  mqtt.endPublish();
  mqtt.flush();
  end_publish = millis();
  Serial.println("Publish ended...");
  Serial.print("Publish time used in AT command mode (ms): ");
  Serial.println(end_publish - start_publish);

  timer = millis();
  while (millis() - timer <= 10000) {
    // Network and GPRS check requires AT command mode, does not work in Direct Link mode
    
    if (!modem.isNetworkConnected()) {
      Serial.println("Network disconnected");
      if (!modem.waitForNetwork(180000L, true)) {
        Serial.println(" fail");
        delay(10000);
        return;
      }
      if (modem.isNetworkConnected()) {
        Serial.println("Network re-connected");
      }
  
      if (!modem.isGprsConnected()) {
        Serial.println("GPRS disconnected!");
        Serial.print(F("Connecting to "));
        Serial.print(apn);
        if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
          Serial.println(" fail");
          delay(10000);
          return;
        }
        if (modem.isGprsConnected()) { Serial.println("GPRS reconnected"); }
      }
    }
  
    if (!mqtt.connected()) {
      Serial.println("=== MQTT NOT CONNECTED ===");
      uint32_t t = millis();
      if (t - lastReconnectAttempt > 1000L) {
        lastReconnectAttempt = t;
        if (mqttConnect()) { lastReconnectAttempt = 0; }
      }
      delay(100);
      return;
    }
    mqtt.loop();
  }
  Serial.println();
}
