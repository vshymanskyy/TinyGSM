/** ============================================================================
 * @example{lineno} CapabilityQuery.ino
 *
 * @brief Demonstrates compile-time modem capability detection.
 *
 * This example shows how to query modem capabilities at compile time using
 * capability detection traits. This is particularly useful when working with
 * multiple modem types in the same codebase.
 *
 * @author  Sara Damiano
 * @license LGPL-3.0
 * @date    2026
 * ========================================================================== */

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM868
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_SIM7000
// #define TINY_GSM_MODEM_SIM7000SSL
// #define TINY_GSM_MODEM_SIM7080
// #define TINY_GSM_MODEM_SIM5360
// #define TINY_GSM_MODEM_SIM7600
// #define TINY_GSM_MODEM_A7672X
// #define TINY_GSM_MODEM_UBLOX
// #define TINY_GSM_MODEM_SARAR4
// #define TINY_GSM_MODEM_SARAR5
// #define TINY_GSM_MODEM_M95
// #define TINY_GSM_MODEM_BG95
// #define TINY_GSM_MODEM_BG96
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_MC60
// #define TINY_GSM_MODEM_MC60E
// #define TINY_GSM_MODEM_ESP32
// #define TINY_GSM_MODEM_ESP8266
// #define TINY_GSM_MODEM_ESP8266_NONOS
// #define TINY_GSM_MODEM_XBEE
// #define TINY_GSM_MODEM_SEQUANS_MONARCH

// Set serial for debug console (to the Serial Monitor)
#define SerialMon Serial

// Set serial for AT commands (to the module)
// Use Hardware Serial on Mega, Leonardo, Micro
#if !defined(__AVR_ATmega328P__) && !defined(SerialAT)
#define SerialAT Serial1

// or Software Serial on Uno, Nano
#elif !defined(SerialAT)
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(2, 3);  // RX, TX
#endif

#include <TinyGsmClient.h>
#include <TinyGsmCapabilities.h>

#if (defined(ARDUINO_NRF52840_FEATHER)) && !defined(ADAFRUIT_TINYUSB_H_)
#include <Adafruit_TinyUSB.h>  // for Serial
#endif

// Create the modem object
TinyGsm modem(SerialAT);

void setup() {
  SerialMon.begin(115200);
  while (!SerialMon && millis() < 10000L) {}  // wait for the port
  delay(10);

  SerialMon.println(F("\n\n==========================================="));
  SerialMon.println(F("TinyGSM Capability Query Example"));
  SerialMon.println(F("===========================================\n"));

  // Print the modem type
  SerialMon.print(F("Modem: "));
  SerialMon.print(modem.getConfiguredModem());
  SerialMon.println('\n');

  // Query capabilities using compile-time traits
  // Note: These traits are evaluated at compile time, even with C++11
  SerialMon.println(F("Compile-time capability detection:"));
  SerialMon.println(F("----------------------------------"));

  // SSL/TLS capabilities
  if (TinyGsmCapabilities::has_ssl<TinyGsm>::value) {
    SerialMon.println(F("✓ SSL/TLS support: YES"));
    if (TinyGsmCapabilities::can_specify_certs<TinyGsm>::value) {
      SerialMon.println(F("  ✓ Can specify certificates"));
    }
    if (TinyGsmCapabilities::can_load_certs<TinyGsm>::value) {
      SerialMon.println(F("  ✓ Can load certificates"));
    }
  } else {
    SerialMon.println(F("✗ SSL/TLS support: NO"));
  }

  // GPS capability
  if (TinyGsmCapabilities::has_gps<TinyGsm>::value) {
    SerialMon.println(F("✓ GPS/GNSS support: YES"));
  } else {
    SerialMon.println(F("✗ GPS/GNSS support: NO"));
  }

  // Cellular data capability
  if (TinyGsmCapabilities::has_cellular_data<TinyGsm>::value) {
    SerialMon.println(F("✓ Cellular data: YES"));
  } else {
    SerialMon.println(F("✗ Cellular data: NO"));
  }

  // WiFi capability
  if (TinyGsmCapabilities::has_wifi<TinyGsm>::value) {
    SerialMon.println(F("✓ WiFi support: YES"));
  } else {
    SerialMon.println(F("✗ WiFi support: NO"));
  }

  // SMS capability
  if (TinyGsmCapabilities::has_sms<TinyGsm>::value) {
    SerialMon.println(F("✓ SMS messaging: YES"));
  } else {
    SerialMon.println(F("✗ SMS messaging: NO"));
  }

  // Voice calling capability
  if (TinyGsmCapabilities::has_calling<TinyGsm>::value) {
    SerialMon.println(F("✓ Voice calling: YES"));
  } else {
    SerialMon.println(F("✗ Voice calling: NO"));
  }

  // Battery status capability
  if (TinyGsmCapabilities::has_battery<TinyGsm>::value) {
    SerialMon.println(F("✓ Battery status: YES"));
  } else {
    SerialMon.println(F("✗ Battery status: NO"));
  }

  // Temperature sensor capability
  if (TinyGsmCapabilities::has_temperature<TinyGsm>::value) {
    SerialMon.println(F("✓ Temperature sensor: YES"));
  } else {
    SerialMon.println(F("✗ Temperature sensor: NO"));
  }

  // Network Time Protocol capability
  if (TinyGsmCapabilities::has_ntp<TinyGsm>::value) {
    SerialMon.println(F("✓ NTP support: YES"));
  } else {
    SerialMon.println(F("✗ NTP support: NO"));
  }

  // Time/clock functions capability
  if (TinyGsmCapabilities::has_time<TinyGsm>::value) {
    SerialMon.println(F("✓ Time/clock functions: YES"));
  } else {
    SerialMon.println(F("✗ Time/clock functions: NO"));
  }

  // GSM-based location capability
  if (TinyGsmCapabilities::has_gsm_location<TinyGsm>::value) {
    SerialMon.println(F("✓ GSM location: YES"));
  } else {
    SerialMon.println(F("✗ GSM location: NO"));
  }

  // Bluetooth capability
  if (TinyGsmCapabilities::has_bluetooth<TinyGsm>::value) {
    SerialMon.println(F("✓ Bluetooth support: YES"));
  } else {
    SerialMon.println(F("✗ Bluetooth support: NO"));
  }

  // TCP sockets capability (should always be true)
  if (TinyGsmCapabilities::has_tcp<TinyGsm>::value) {
    SerialMon.println(F("✓ TCP sockets: YES"));
  } else {
    SerialMon.println(F("✗ TCP sockets: NO"));
  }

  SerialMon.println(F("\n===========================================\n"));
}

void loop() {
  // Nothing to do here
}
