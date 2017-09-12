![TinyGSM logo](https://cdn.rawgit.com/vshymanskyy/TinyGSM/d18e93dc51fe988a0b175aac647185457ef640b5/extras/logo.svg)

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

You can also join our chat:
[![Gitter](https://img.shields.io/gitter/room/vshymanskyy/TinyGSM.svg)](https://gitter.im/tinygsm)

### Arduino Client interface support
This library is easy to integrate with lots of sketches, which use Ethernet or WiFi.  
**PubSubClient ([MQTT](http://mqtt.org/))**, **[Blynk](http://blynk.cc)**, **HTTP Client** and **File Download** examples are provided.

![examples](/extras/examples.png)

### TinyGSM is tiny
The complete WebClient example for Arduino Uno (via Software Serial) takes little resources:
```
Sketch uses 14094 bytes (43%) of program storage space. Maximum is 32256 bytes.
Global variables use 625 bytes (30%) of dynamic memory, leaving 1423 bytes for local variables. Maximum is 2048 bytes.
```
Arduino GSM library uses 15868 bytes (49%) of Flash and 1113 bytes (54%) of RAM in a similar scenario.  
TinyGSM also pulls data gently from the modem (whenever possible), so it can operate on very little RAM.  
**Now, you have more space for your experiments.**

## Features

Feature \ Modem              | SIM800 | SIM8x8 | A6/A7/A20 | M590 | ESP8266
---                          | ---    | ---    | ---       | ---  | ---
**Data connections**
TCP (HTTP, MQTT, Blynk, ...) | ✔      | ✔      | ✔         | ✔    | ✔
UDP                          |        |        |           |      | 
SSL/TLS (HTTPS)              | ✔¹     | ✔¹     | 🅧         | 🅧   | ◌
**USSD**
Sending USSD requests        | ✔      | ✔      | ✔         | ✔    | 🅧
Decoding 7,8,16-bit response | ✔      | ✔      | ✔         | ✔    | 🅧
**SMS**
Sending                      | ✔      | ✔      | ✔         | ✔    | 🅧
Sending Unicode              | ✔      | ✔      | ◌         | 🅧   | 🅧
Receiving/Reading            |        |        |           |      | 🅧
**Calls**
Dial, hangup                 | ✔      | ✔      | ✔         | 🅧   | 🅧
Receiving calls              | ◌      | ◌      | ◌         | 🅧   | 🅧
DTMF decoding                |        |        | 🅧        | 🅧   | 🅧
**Location**
GSM location service         | ✔      | ✔      | 🅧        | 🅧   | 🅧
GPS/GNSS                     | 🅧     | ◌      | ◌¹        | 🅧   | 🅧

✔ - implemented  ◌ - planned  🅧 - not available for this modem  
¹ - supported only on some models or firmware revisions

## Supported modems

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
- [x] ... other modules based on supported modems

More modems may be supported later:
- [ ] Hi-Link HLK-RM04
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

## How does it work?

Many GSM modems, WiFi and radio modules can be controlled by sending AT commands over Serial.  
TinyGSM knows which commands to send, and how to handle AT responses, and wraps that into standard Arduino Client interface.

## API Reference

For GPRS data streams, this library provides the standard [Arduino Client](https://www.arduino.cc/en/Reference/ClientConstructor) interface.  
For additional functions, please refer to [this example sketch](examples/AllFunctions/AllFunctions.ino)

## Troubleshooting

### SoftwareSerial problems

When using ```SoftwareSerial``` (on Uno, Nano, etc), the speed **115200** may not work.  
Try selecting **57600**, **38400**, or even lower - the one that works best for you.  
Be sure to set correct TX/RX pins in the sketch. Please note that not every Arduino pin can serve as TX or RX pin.  
**Read more about SoftSerial options and configuration [here](https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html) and [here](https://www.arduino.cc/en/Reference/SoftwareSerial).**

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
