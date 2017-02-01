![TinyGSM logo](https://cdn.rawgit.com/vshymanskyy/TinyGSM/ffac7710ec93ec36648ec336b08a5856dcba6154/extras/logo.svg)

A small Arduino library for GSM modules, that just works.
<!---
[![GitHub download](https://img.shields.io/github/downloads/vshymanskyy/TinyGSM/total.svg)](https://github.com/vshymanskyy/TinyGSM/releases/latest)--->
[![GitHub version](https://img.shields.io/github/release/vshymanskyy/TinyGSM.svg)](https://github.com/vshymanskyy/TinyGSM/releases/latest)
[![Build status](https://img.shields.io/travis/vshymanskyy/TinyGSM.svg)](https://travis-ci.org/vshymanskyy/TinyGSM)
[![GitHub issues](https://img.shields.io/github/issues/vshymanskyy/TinyGSM.svg)](https://github.com/vshymanskyy/TinyGSM/issues)
[![GitHub wiki](https://img.shields.io/badge/Wiki-available-brightgreen.svg)](https://github.com/vshymanskyy/TinyGSM/wiki)
[![GitHub stars](https://img.shields.io/github/stars/vshymanskyy/TinyGSM.svg)](https://github.com/vshymanskyy/TinyGSM/stargazers)
[![License](https://img.shields.io/badge/license-LGPL3-blue.svg)](https://github.com/vshymanskyy/TinyGSM/blob/master/LICENSE)

If you like **TinyGSM** - give it a star, or fork it and contribute! 
[![GitHub stars](https://img.shields.io/github/stars/vshymanskyy/TinyGSM.svg?style=social&label=Star)](https://github.com/vshymanskyy/TinyGSM/stargazers) 
[![GitHub forks](https://img.shields.io/github/forks/vshymanskyy/TinyGSM.svg?style=social&label=Fork)](https://github.com/vshymanskyy/TinyGSM/network)

### Arduino Client interface support
This library is easy to integrate with lots of sketches, which use Ethernet or WiFi.  
Examples for **PubSubClient ([MQTT](http://mqtt.org/))**, **[Blynk](http://blynk.cc)**, **Web Client** and **File Download** are provided.

![examples](/extras/examples.png)

### TinyGSM is tiny
WebClient example for Arduino Nano (with Software Serial) takes little resources:
```
Sketch uses 13,802 bytes (44%) of program storage space. Maximum is 30,720 bytes.
Global variables use 661 bytes (32%) of dynamic memory. Maximum is 2,048 bytes.
```
Now, you have more space for your experiments.  
TinyGSM also pulls data gently from the modem (whenever possible), so it can operate on very little RAM.

### Supported modem models
- [x] SIMCom SIM800 series (SIM800A, SIM800C, SIM800L, SIM800H, SIM808, SIM868)
- [x] SIMCom SIM900 series (SIM900A, SIM900D, SIM908, SIM968)
- [x] AI-Thinker A6, A6C, A7
- [x] Neoway M590
- [x] ESP8266 (AT commands interface, similar to GSM modems)

### Supported modules
- [x] GPRSbee
- [x] Microduino GSM
- [x] Adafruit FONA (Mini Cellular GSM Breakout)
- [x] Adafruit FONA 800/808 Shield

More modems may be supported later:
- [ ] Quectel M10, M95, UG95
- [ ] SIMCom SIM5320, SIM5216
- [ ] Telit GL865
- [ ] ZTE MG2639

Watch this repo for new updates! And of course, contributions are welcome ;)

## Getting Started

  1. Using your phone:
    - Disable PIN code on the SIM card
    - Check your ballance
    - Check that APN,User,Pass are correct and you have internet
  2. Ensure the SIM card is correctly inserted into the module
  3. Provide a good, [stable power supply](https://github.com/vshymanskyy/TinyGSM/wiki/Powering-GSM-module) (up to 2A and specific voltage according to your module documentation)
  4. Check if serial connection is working (Hardware Serial is recommended)  
     Send an ```AT``` command using [this sketch](tools/AT_Debug/AT_Debug.ino)
  5. Check if GSM antenna is attached

## Troubleshooting

### SoftwareSerial problems

When using ```SoftwareSerial``` (on Uno, Nano, etc), the speed **115200** may not work.  
Try selecting **57600**, **38400**, or even lower - the one that works best for you.  
Be sure to set correct TX/RX pins in the sketch.

### Diagnostics sketch

Use this sketch to diagnose your SIM card and GPRS connection:  
  File -> Examples -> TynyGSM -> tools -> [Diagnostics](https://github.com/vshymanskyy/TinyGSM/blob/master/tools/Diagnostics/Diagnostics.ino)

### Broken initial configuration

Sometimes (especially if you played with AT comands), your module configuration may become invalid.  
This may result in problems such as:

 * Can't connect to the GPRS network
 * Can't connect to the server
 * Sent/recieved data contains invalid bytes
 * etc.

To return module to **Factory Defaults**, use this sketch:  
  File -> Examples -> TynyGSM -> tools -> [FactoryReset](https://github.com/vshymanskyy/TinyGSM/blob/master/tools/FactoryReset/FactoryReset.ino)

__________

### License
This project is released under
The GNU Lesser General Public License (LGPL-3.0)
