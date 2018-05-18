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
Sketch uses 15022 bytes (46%) of program storage space. Maximum is 32256 bytes.
Global variables use 574 bytes (28%) of dynamic memory, leaving 1474 bytes for local variables. Maximum is 2048 bytes.
```
Arduino GSM library uses 15868 bytes (49%) of Flash and 1113 bytes (54%) of RAM in a similar scenario.  
TinyGSM also pulls data gently from the modem (whenever possible), so it can operate on very little RAM.  
**Now, you have more space for your experiments.**

## Features

Feature \ Modem              | SIM8xx | u-Blox | A6/A7/A20 | M590 | ESP8266 | XBee
---                          | ---    | ---    | ---       | ---  | ---     | ---
**Data connections**
TCP (HTTP, MQTT, Blynk, ...) | ✔      | ✔      | ✔         | ✔    | ✔       | ✔
UDP                          | ◌      | ◌      |           |      |         | ◌
SSL/TLS (HTTPS)              | ✔¹     | ✔      | 🅧        | 🅧    | ✔¹      | ✔¹
**USSD**
Sending USSD requests        | ✔      |        | ✔         | ✔    | 🅧       |
Decoding 7,8,16-bit response | ✔      |        | ✔         | ✔    | 🅧      |
**SMS**
Sending                      | ✔      | ✔      | ✔         | ✔    | 🅧      | ✔
Sending Unicode              | ✔      |        | ◌         | 🅧   | 🅧      |
Reading                      |        |        |           |      | 🅧      |
Incoming message event       |        |        |           | ?    | 🅧      |
**Calls**
Dial, hangup                 | ✔      |        | ✔         | 🅧   | 🅧      | 🅧
Receiving calls              | ✔      |        | ✔         | 🅧   | 🅧      | 🅧
Incoming event (RING)        | ◌      |        | ◌         | 🅧   | 🅧      | 🅧
DTMF sending                 | ✔      |        | ✔         | 🅧   | 🅧      | 🅧
DTMF decoding                | ◌      |        | 🅧        | 🅧   | 🅧      | 🅧
**Location**
GSM location service         | ✔      | ✔      | 🅧        | 🅧   | 🅧      | ✔
GPS/GNSS                     | ✔¹     | 🅧     | ◌¹        | 🅧   | 🅧      | 🅧

✔ - implemented  ◌ - planned  🅧 - not available on this modem  
¹ - only some device models or firmware revisions have this feature (SIM8xx R14.18, A7, etc.)  

## Supported modems

- SIMCom SIM800 series (SIM800A, SIM800C, SIM800L, SIM800H, SIM808, SIM868)
- SIMCom SIM900 series (SIM900A, SIM900D, SIM908, SIM968)
- AI-Thinker A6, A6C, A7, A20
- ESP8266 (AT commands interface, similar to GSM modems)
- Digi XBee WiFi and Cellular (using XBee command mode)
- Neoway M590
- u-blox Cellular Modems (LEON-G100, LISA-U2xx, SARA-G3xx, SARA-U2xx, TOBY-L2xx, LARA-R2xx, MPCI-L2xx)
- Quectel BG96 ***(alpha)***

### Supported boards/modules
- Arduino MKR GSM 1400
- GPRSbee
- Microduino GSM
- Adafruit FONA (Mini Cellular GSM Breakout)
- Adafruit FONA 800/808 Shield
- Industruino GSM
- RAK WisLTE ***(alpha)***
- ... other modules, based on supported modems. Some boards require [**special configuration**](https://github.com/vshymanskyy/TinyGSM/wiki/Board-configuration).

More modems may be supported later:
- [ ] Quectel M10, M35, M95, UG95, EC21
- [ ] Sequans Monarch LTE Cat M1/NB1
- [ ] SIMCom SIM5320, SIM5360, SIM5216, SIM7xxx
- [ ] Telit GL865
- [ ] ZTE MG2639
- [ ] Hi-Link HLK-RM04

Watch this repo for new updates! And of course, contributions are welcome ;)

## Donation

[![Donate BountySource](https://img.shields.io/badge/Donate-BountySource-149E5E.svg)](https://salt.bountysource.com/checkout/amount?team=tinygsm-dev)
[![Donate Bitcoin](https://img.shields.io/badge/Donate-Bitcoin-orange.svg)](http://tny.im/aen)

If you have found TinyGSM to be useful in your work, research or company, please consider making a donation to the project commensurate with your resources. Any amount helps!  
**All donations will be used strictly to fund the development of TinyGSM:**
- Covering cellular network expences
- Buying new hardware and modems for integration
- Bounty Budget (to reward other developers for their contributions)
- Implementing new features
- Quality Assurance

## Getting Started

  1. Using your phone:
    - Disable PIN code on the SIM card
    - Check your balance
    - Check that APN,User,Pass are correct and you have internet
  2. Ensure the SIM card is correctly inserted into the module
  3. Ensure that GSM antenna is firmly attached
  4. Check if serial connection is working (Hardware Serial is recommended)  
     Send an ```AT``` command using [this sketch](tools/AT_Debug/AT_Debug.ino)

If you have any issues:

  1. Read the whole README (you're looking at it!)
  2. Some boards require [**special configuration**](https://github.com/vshymanskyy/TinyGSM/wiki/Board-configuration).
  3. Try running the Diagnostics sketch
  4. Check for [**highlighted topics here**](https://github.com/vshymanskyy/TinyGSM/issues?utf8=%E2%9C%93&q=is%3Aissue+label%3A%22for+reference%22+)
  5. If you have a question, please post it in our [Gitter chat](https://gitter.im/tinygsm)

## How does it work?

Many GSM modems, WiFi and radio modules can be controlled by sending AT commands over Serial.  
TinyGSM knows which commands to send, and how to handle AT responses, and wraps that into standard Arduino Client interface.

## API Reference

For GPRS data streams, this library provides the standard [Arduino Client](https://www.arduino.cc/en/Reference/ClientConstructor) interface.  
For additional functions, please refer to [this example sketch](examples/AllFunctions/AllFunctions.ino)

## Troubleshooting

### Diagnostics sketch

Use this sketch to diagnose your SIM card and GPRS connection:  
  File -> Examples -> TynyGSM -> tools -> [Diagnostics](https://github.com/vshymanskyy/TinyGSM/blob/master/tools/Diagnostics/Diagnostics.ino)

### Ensure stable data & power connection

Most modules require up to 2A and specific voltage - according to the module documentation.
So this actually solves stability problems in **many** cases:
- Provide a good stable power supply. Read about [**powering your module**](https://github.com/vshymanskyy/TinyGSM/wiki/Powering-GSM-module).
- Keep your wires as short as possible
- Consider soldering them for a stable connection
- Do not put your wires next to noisy signal sources (buck converters, antennas, oscillators etc.)

### SoftwareSerial problems

When using ```SoftwareSerial``` (on Uno, Nano, etc), the speed **115200** may not work.  
Try selecting **57600**, **38400**, or even lower - the one that works best for you.  
In some cases **9600** is unstable, but using **38400** helps, etc.  
Be sure to set correct TX/RX pins in the sketch. Please note that not every Arduino pin can serve as TX or RX pin.  
**Read more about SoftSerial options and configuration [here](https://www.pjrc.com/teensy/td_libs_AltSoftSerial.html) and [here](https://www.arduino.cc/en/Reference/SoftwareSerial).**

### ESP32 HardwareSerial

When using ESP32 `HardwareSerial`, you may need to specify additional parameters to the `.begin()` call.
Please [refer to this comment](https://github.com/vshymanskyy/TinyGSM/issues/91#issuecomment-356024747).

### SAMD21

When using SAMD21-based boards, you may need to use a sercom uart port instead of `Serial1`.
Please [refer to this comment](https://github.com/vshymanskyy/TinyGSM/issues/102#issuecomment-345548941).

### Broken initial configuration

Sometimes (especially if you played with AT comands), your module configuration may become invalid.  
This may result in problems such as:

 * Can't connect to the GPRS network
 * Can't connect to the server
 * Sent/recieved data contains invalid bytes
 * etc.

To return module to **Factory Defaults**, use this sketch:  
  File -> Examples -> TinyGSM -> tools -> [FactoryReset](https://github.com/vshymanskyy/TinyGSM/blob/master/tools/FactoryReset/FactoryReset.ino)

### Goouuu Tech IOT-GA6 vs AI-Thinker A6 confusion

It turns out that **Goouuu Tech IOT-GA6** is not the same as **AI-Thinker A6**. Unfortunately IOT-GA6 is not supported out of the box yet. There are some hints that IOT-GA6 firmware may be updated to match A6... See [this topic](https://github.com/vshymanskyy/TinyGSM/issues/164).

__________

### License
This project is released under
The GNU Lesser General Public License (LGPL-3.0)
