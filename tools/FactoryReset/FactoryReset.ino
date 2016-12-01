/**************************************************************
 *
 * To run this tool, you need to install StreamDebugger library:
 *   https://github.com/vshymanskyy/StreamDebugger
 *
 **************************************************************/

#include <TinyGsmClient.h>
#include <StreamDebugger.h>

StreamDebugger GsmSerial(Serial1, Serial);
GsmClient gsm(GsmSerial);

void setup() {
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  // Set GSM module baud rate
  GsmSerial.begin(115200);
  delay(3000);

  // Return to factory configuration
  gsm.factoryDefault();
}

void loop() {

}
