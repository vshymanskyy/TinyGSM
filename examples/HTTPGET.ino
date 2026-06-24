#define TINY_GSM_MODEM_SIM800   // MC60 is AT-compatible with SIM800
#define TINY_GSM_RX_BUFFER 512

#include <TinyGsmClient.h>
#include <SoftwareSerial.h>

// Define your GSM module serial connection
SoftwareSerial SerialAT(7, 8); // RX, TX

// Your SIM card credentials
const char apn[]  = "your_apn";     // e.g. "internet"
const char user[] = "";             // leave blank if not needed
const char pass[] = "";             // leave blank if not needed

TinyGsm modem(SerialAT);
TinyGsmClient client;

void setup() {
  // Start serial for debug
  Serial.begin(9600);
  delay(10);

  // Start communication with modem
  SerialAT.begin(9600);
  delay(3000);

  Serial.println("Initializing modem...");
  modem.restart();  // or modem.init();

  // Unlock SIM if needed
  // modem.simUnlock("1234");

  Serial.print("Connecting to network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" failed");
    while (true);
  }
  Serial.println(" connected");

  Serial.print("Connecting to GPRS...");
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println(" failed");
    while (true);
  }
  Serial.println(" connected");

  // Make HTTP request
  if (client.connect("example.com", 80)) {
    client.println("GET / HTTP/1.1");
    client.println("Host: example.com");
    client.println("Connection: close");
    client.println();
  }
}

void loop() {
  while (client.available()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);
  }

  if (!client.connected()) {
    Serial.println("Disconnecting...");
    client.stop();
    while (true);
  }
}




