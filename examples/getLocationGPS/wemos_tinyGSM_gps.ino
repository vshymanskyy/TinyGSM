// Select your modem:
#define TINY_GSM_MODEM_SIM800

// Increase RX buffer
#define TINY_GSM_RX_BUFFER 1030

#include <TinyGsmClient.h>

// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
// connected to D1, D2 of wemos
SoftwareSerial SerialAT(5, 4); // RX, TX

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

void setup() {
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(9600);
  delay(1000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();
  if (modem.poweronGPS()) {
    Serial.println("GPS On");
  } else {
    Serial.println("GPS Off");
  }
}

void loop() {
//GPS Position holen
  Serial.println(modem.getLocation());
  delay(1000);
}
