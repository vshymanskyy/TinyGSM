<!--! @page page_the_tools TinyGSM Tools -->
# TinyGSM Tools

These tool sketches provide utilities for debugging, diagnosing, resetting, and validating TinyGSM modem support.

___

<!--! @if GITHUB -->

- [TinyGSM Tools](#tinygsm-tools)
  - [AT Debug](#at-debug)
  - [Diagnostics](#diagnostics)
  - [Factory Reset](#factory-reset)
  - [Update Espressif Firmware](#update-espressif-firmware)
  - [Test Build](#test-build)

<!--! @endif -->

<!--! @tableofcontents -->

<!--! @m_footernavigation -->

## AT Debug<!--! {#tools_at_debug} -->

This tool sketch tries to auto-detect the modem baud rate and provides direct AT command passthrough access.
<!--!
- [AT Debug](\ref extras/tools/AT_Debug) -->
- [The AT debug tool on GitHub](https://github.com/EnviroDIY/TinyGSM/tree/master/extras/tools/AT_Debug)

## Diagnostics<!--! {#tools_diagnostics} -->

This tool sketch runs a series of modem diagnostic checks and prints the results to help troubleshoot connectivity and configuration issues.
<!--!
- [Diagnostics](\ref extras/tools/Diagnostics) -->
- [The diagnostics tool on GitHub](https://github.com/EnviroDIY/TinyGSM/tree/master/extras/tools/Diagnostics)

## Factory Reset<!--! {#tools_factory_reset} -->

This tool sketch sends commands to reset a supported modem back to factory defaults.
<!--!
- [Factory Reset](\ref extras/tools/FactoryReset) -->
- [The factory reset tool on GitHub](https://github.com/EnviroDIY/TinyGSM/tree/master/extras/tools/FactoryReset)

## Update Espressif Firmware<!--! {#tools_update_espressif} -->

This sketch connects to WiFi and attempts to update the firmware for an Espressif module.
<!--!
- [Update Espressif Firmware](\ref extras/tools/EspressifUpdateFirmware) -->
- [The Espressif firmware update tool on GitHub](https://github.com/EnviroDIY/TinyGSM/tree/master/extras/tools/EspressifUpdateFirmware)

## Test Build<!--! {#tools_test_build} -->

This tool sketch is a compile-only validation target used to exercise API coverage and ensure builds succeed; it is not intended for normal runtime use.
<!--!
- [Test Build](\ref extras/tools/test_build) -->
- [The test build tool on GitHub](https://github.com/EnviroDIY/TinyGSM/tree/master/extras/tools/test_build)

<!--! @dir extras/tools
@brief Has helper and debugging sketches -->
