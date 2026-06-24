#define TINY_GSM_MODEM_SIM800   // MC60 uses SIM800-like AT commands

#include <TinyGsmClient.h>
#include <SoftwareSerial.h>

// SIM card credentials
const char apn[]  = "your_apn"; // Not needed for SMS/Call, only for GPRS
const char user[] = "";
const char pass[] = "";

// Replace with your actual phone number
const char phoneNumber[] = "+1234567890";

// Use SoftwareSerial on most Arduino boards (or HardwareSerial if available)
SoftwareSerial SerialAT(7, 8); // RX, TX

TinyGsm modem(SerialAT);

void setup() {
  // Start debug serial
  Serial.begin(9600);
  delay(10);

  // Start communication with the modem
  SerialAT.begin(9600);
  delay(3000);

  Serial.println("Initializing modem...");
  modem.restart();  // or modem.init();

  // Optional: unlock SIM with PIN
  // modem.simUnlock("1234");

  // Wait for network
  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" failed");
    while (true);
  }
  Serial.println(" connected");

  // -------------------------
  // 📩 Send SMS
  // -------------------------
  Serial.println("Sending SMS...");
  if (modem.sendSMS(phoneNumber, "Hello from MC60!")) {
    Serial.println("SMS sent successfully!");
  } else {
    Serial.println("SMS failed to send.");
  }

  delay(5000); // Wait a bit before making the call

  // -------------------------
  // 📞 Make Voice Call
  // -------------------------
  Serial.print("Calling ");
  Serial.println(phoneNumber);
  modem.callNumber(phoneNumber);

  // Wait 20 seconds before hanging up
  delay(20000);
  modem.callHangup();
  Serial.println("Call ended.");
}

void loop() {
  // Nothing here — everything happens in setup()
}

