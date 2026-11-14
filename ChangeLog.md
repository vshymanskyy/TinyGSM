# ChangeLog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and its stricter, better defined, brother [Common Changelog](https://common-changelog.org/).

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

***

## [Unreleased]

### Changed

### Added

### Removed

### Fixed

***

## [0.13.0-rc.1]

This release substantially updates TinyGSM's client, socket, SSL/TLS, parsing, and modem-support layers.
The documented single-modem entry point remains `#include <TinyGsmClient.h>`, but applications that use modem-specific headers, internal mixins, SSL configuration, registration-status values, or the changed return values below will need to be updated.

### Changed

- **BREAKING — SSL/TLS configuration and ownership:** Rebuilt the shared TCP and SSL layers.
  SSL settings now belong to reusable `GsmSecureContext` objects rather than individual secure clients.
  Applications must keep each modem, context, and attached client alive for as long as the dependent objects use it.
  - Functions that copy certificate data into a modem now use `load...` names; functions that select an already-loaded credential use `...Name` names.
  - Authentication mode, TLS version, CA certificate, client certificate and key, PSK, and PSK-table settings are now configured through secure contexts on supported modems.
  - Modem-specific secure client classes were standardized as `GsmClientSecureSim7000SSL`, `GsmClientSecureSim7080`, `GsmClientSecureSaraR4`, and `GsmClientSecureSaraR5`.
    The previous names remain as compatibility aliases, and the global `TinyGsmClientSecure` alias is unchanged.
- **BREAKING — scoped enums:** Date/time, registration-status, and SSL values now use scoped `enum class` types, so enum values must be qualified with their type.
  Registration-status type capitalization was also standardized; for example, `SIM800RegStatus` became `Sim800RegStatus`.
- **BREAKING — header locations:** Moved modem implementations into `src/modems` and internal CRTP mixins into `src/mixins`.
  The standard `#include <TinyGsmClient.h>` remains unchanged.
Direct modem includes must use paths such as `#include <modems/TinyGsmClientSIM7080.h>`.
- **BREAKING — failure values:** Standardized unsupported, missing, and invalid sensor results across modem families.
  `getBattVoltage()` returns `-9999`, `getBattPercent()` and `getBattChargeState()` return `-99`, and `getTemperature()` returns `-9999.0F` on failure.
- **BREAKING — u-blox radio access APIs:**
  Corrected `setRadioAccessTecnology()` and `getCurrentRadioAccessTecnology()` to `setRadioAccessTechnology()` and `getCurrentRadioAccessTechnology()`.
  SARA-R4 now uses `setRadioAccessTechnology()` instead of `setURAT()`, and the SARA-R5 setter argument types and getter result and implementation were corrected.
- **Potentially breaking — NTP result:**
  `NTPServerSync()` now consistently returns `bool`, with `true` meaning success.
  Code that interpreted the old numeric results from Espressif or BG96 modems may have its condition inverted.
- **Potentially breaking — typed configuration:**
  Replaced internal modem, TCP, SSL, and response macros, including `TINY_GSM_MUX_COUNT`, with public `ModemConfig`, `TcpConfig`, and `SslConfig` traits.
  These traits describe socket and SSL-context counts, static or dynamic mux assignment, receive behavior, send limits, timeouts, and response strings.
- Replaced the internal `TinyGsmGPRS` mixin with `TinyGsmCellularData` and added generic cellular-data terminology.
  The historical `gprsConnect()`, `gprsDisconnect()`, and `isGprsConnected()` APIs remain as aliases.
- Reworked client and context registration so objects can be default-created, explicitly initialized, safely attached or detached, and reassigned without leaving stale modem socket or SSL-context pointers.
- Split Espressif support into separate drivers for ESP32 AT firmware 3.2 or newer, ESP8266 [v2.2.1 firmware](https://github.com/espressif/esp-at/releases/tag/v2.2.1.0_esp8266), and the legacy ESP8266 Non-OS firmware.
- Moved bundled utilities from `tools` to `extras/tools`, the compile-test sketch to `extras/tools/test_build`, and download fixtures to `extras/download_tests` so the packaged library has a consistent layout.
- Made the default client receive buffer platform-aware: 1024 bytes on SAMD51, 256 bytes on other SAMD boards, and 64 bytes elsewhere.
  The `TINY_GSM_RX_BUFFER` override remains available.
- Reworked `waitResponse()` and timed stream parsing to use explicit overall and inter-character limits instead of repeated yields.
  The new `TINY_GSM_INTERCHAR_TIMEOUT_MS` and `TINY_GSM_RESPONSE_MAX_GRACE_MS` settings bound incomplete and continuously arriving responses.
- Increased the maximum autobaud rate to 921600 and made baud probing preserve the stream timeout.
- Standardized NTP timezone inputs as quarter-hour offsets and corrected date, time, timezone, and epoch handling across modem families.
- Aligned the A7672x cellular-data, socket, and SSL flows with the related SIM7600 implementation.

### Added

- Added modem-scoped `TinyGsmClient`, `TinyGsmClientSecure`, and `TinyGsmSecureContext` aliases for libraries and multi-modem sketches.
The existing global aliases remain available for single-modem sketches.
- Added public client/context lifecycle and lookup APIs, including `init()`, `isAttached()`, `detach()`, `getClient()`, `findFirstClient()`, and `findFirstIdleClient()`.
Static-mux clients can request strict attachment to a specific socket.
- Added `isSecure()`, `getMux()`, and `getConnectionID()` so applications can inspect a client and distinguish its TinyGSM socket slot from a modem's internal connection identifier.
- Added `beginWrite()` and `endWrite()` for sending a complete payload through a modem's direct-send path.
  Oversized writes are rejected, accepted byte counts can be checked where supported, and ordinary writes are split into modem-safe chunks.
  The API can be paired with PubSubClient's `beginPublish()` and `endPublish()`.
- Added generic `dataConnect()`, `dataDisconnect()`, and `isDataConnected()` names while retaining the historical `gprs*` aliases.
- Added compile-time modem capability traits for SSL, certificate management, networking, GPS, SMS, calling, battery, temperature, Bluetooth, time, and other features, plus a `CapabilityQuery` example.
- Added `getConfiguredModem()` to identify the configured modem without sending an AT command, a selected-modem `RegStatus` alias, and public aliases for the modem, TCP, and SSL configuration types.
- Added SSL version and authentication selection, PSK and PSK-table support, CA/client credential selection, stored-certificate conversion and printing, and AVR-safe loading of certificates from program memory on supported modems.
- Added SIM7600 SSL support from \@floBik.
- Added `getNetworkEpoch()` and public `waitForTimeSync()` support on capable modems.
- Added `setDefaultBaud()` for Espressif modems and `forceModemBaud()` for recovering a modem whose current baud rate is unknown or incorrect.
- Added the configurable `TINY_GSM_UNREAD_CHECK_MS` interval for polling modems that may lose incoming-data notifications.
- Added opt-in `TINY_GSM_USE_PUBLIC_DNS` support.
It is disabled by default so supported modems continue using their network-provided DNS servers.
- Added support for Digi XBee3 Cellular LTE-M3 modules.
- Added `powerOff()` as the preferred spelling and deprecated `poweroff()`.
- Added AWS IoT Core mutual-authentication, multiple-connection, and multiple-modem examples, plus an Espressif AT-firmware update tool.
- Added generated modem capability documentation, Doxygen coverage for the public API, example and source guides, and library-structure/release tooling.

### Removed

- **BREAKING:** Removed `ShowNTPError()`.
- Removed the `TINY_GSM_YIELD` and `TINY_GSM_YIELD_MS` configuration macros; timed stream helpers now bound parser waits directly.
- Removed modem-owned certificate-name arrays; secure contexts now own the credential names used by their connections.
- Removed the obsolete `AT_Spy` and Industruino sketches and legacy Makefile and archived Travis configuration.
- Stopped including the certificate-management SSL mixin on modems that provide only basic secure-client support.

### Fixed

- Fixed socket assignment, mux validation and translation, attachment and destruction, connection-state tracking, partial sends, stop behavior, and receive-buffer draining across modem families.
- Avoided unnecessary socket-status and available-byte queries, including repeated Espressif queries whose responses already describe every socket.
- Corrected per-modem TCP socket, secure-socket, and SSL-context counts, and separated shared socket limits from SSL configuration-profile limits.
- Hardened stream, integer, floating-point, IP-address, and hexadecimal parsing against malformed or partial data, invalid lengths, overflow, and underflow.
- Fixed SSL-context validation, credential pairing and attachment, certificate loading and conversion, connection setup, and failure cleanup across supported secure modems.
- Fixed SIM7070/SIM7080/SIM7090 parsing of the 18-field `+CGNSINF` GPS response, eliminating an unnecessary five-second wait.
- Fixed A7672x and SIM7600 cellular-data activation, socket initialization, command ordering, result handling, battery-voltage units, and SSL setup.
- Fixed Espressif socket handling, URCs, time synchronization, timezone conversion, certificate management, and long-domain lookup behavior.
- Fixed XBee command-mode transitions, socket setup and state tracking, DNS/IP handling, and SSL configuration.
- Fixed FIFO copying for element types larger than one byte, empty-buffer peeking, and receive-buffer initialization.
- Updated non-TLS HTTP examples to use a site that still accepts unencrypted connections, and regenerated the Arduino keywords list from the documented API.

***

## [0.12.0]

### Changed

- Moved bulk of waitResponse function to modem template and gave modems handleURCs fxn
- Moved option in waitResponse for verbose outputs.
- setBaud now returns a bool
- Replace new line consts with defines and renamed to AT_NL
- Renamed all RegStatus enums to be unique
- Replaced `NULL` with `nullptr` and replaced c-style casts.
- Moved setCertificates function and the certificate name matrix to the SSL template.
- Changed inputs for (unimplemented) SSL certificate functions.
- All modems will now return the pre-defined manufacturer and model in the name if the function to get the internal name fails.
- Cleaned up code for getting modem names.
- Made battery return types signed.

### Added

- Added support for SSL for the Quectel BG95 and BG96 from [Aurelien BOUIN](https://github.com/aurelihein) and [George O'Connor](https://github.com/georgeman93)
- Added support for UBLOX SARA-R5 from [Sebastian Bergner](https://github.com/sebastianbergner)
- Added support for SIMCOM A7672x from [Giovanni de Rosso Unruh](https://github.com/giovannirosso)
- Added SIM5320 GPS location from [Bengarman](https://github.com/Bengarman)
- Added functions `getModemSerialNumber`, `getModemModel`, and `getModemRevision`.
- Added deep debugging option
- Added documentation to the FIFO class

### Removed

- Removed non-functional factory reset from SIM70xx series

### Fixed

- Removed extra wait on SIM7000 from [Mikael Fredriksson](https://github.com/Gelemikke)
- Fix status returns on ESP8266/ESP32 AT commands
- Fix length of HEX for Sequans Monarch
- Fix SIM7600 password then user when cid is set from [github0013](https://github.com/github0013)
- Fix cardinal points in location by gps for SIM7600 from [Juxn3](https://github.com/Juxn3)
- Fix NTP server sync for SIM70xx models from [Gonzalo Brusco](https://github.com/gonzabrusco)
- Fixed SIM70xx inheritance

***

## [0.11.7]

### Added

- Added an XBee DNS check for confirming Wi-Fi network connectivity.

### Fixed

- Improved XBee retry behavior, Wi-Fi S6B compatibility, and connection stability with changes from [Neil Hancock](https://github.com/neilh10).
- Fixed Sequans Monarch compiler warnings and switch fallthrough handling.

***

## [0.11.5]

### Changed

- Switched Sequans Monarch outgoing data to hexadecimal mode.
- Updated Espressif configuration commands to try the current firmware syntax first and fall back to the `_CUR` syntax used by intermediate firmware versions.

### Added

- Added SIM7600 voice-call support from [LukasSkywalker](https://github.com/LukasSkywalker).
- Added stream `peek()` support from [IoTThinks](https://github.com/IoTThinks).
- Added SIM800-series audio functions from [wifixcort](https://github.com/wifixcort).

### Fixed

- Fixed invalid memory access in the Sequans Monarch driver.
- Declared the library's public headers in `library.json` for PlatformIO.

***

## [0.11.3]

### Fixed

- Fixed compiler warnings and an error in the u-blox driver.

***

## [0.11.2]

### Changed

- Updated u-blox support and added a radio-access-technology setter.

### Added

- Added `TinyGSM.h` for compatibility with Arduino CLI library discovery.

### Fixed

- Prevented a SIM7000SSL crash when checking available data on an invalid socket.
- Removed an unused-argument warning in the BG96 driver.

***

## [0.11.1]

### Changed

- Reduced unnecessary Digi XBee configuration writes by reading saved values before changing them.
- Improved Sequans Monarch operation and made BG96 registration checks include both GPRS and EPS status.
- Migrated continuous integration from Travis CI to GitHub Actions.

### Added

- Added a separate SSL-capable SIM7000 driver.
- Added support for SIM7070, SIM7080, and SIM7090 LTE-M/NB-IoT modules.
- Added shared NTP synchronization support.
- Added SIM808 Bluetooth support.
- Added GSM-location support to the SIM70xx family and root-certificate and SNI support to the SIM7000 SSL implementation.

### Fixed

- Fixed USSD responses longer than 127 characters.
- Fixed SIM7000 SSL socket-state and available-data handling.
- Fixed SIM7600 GPS handling and several XBee, MC60, and registration-status issues.

***

## [0.10.9]

### Added

- Added `setPhoneFunctionality()` for commands such as `AT+CFUN`.
- Added opt-in SIM800 SSL configuration through `AT+SSLOPT`.

### Fixed

- Fixed several MC60 issues.
- Corrected GPS output arguments to be passed by reference.
- Corrected SIM7600 connection-result handling.

***

## [0.10.7]

### Changed

- Configured Arduino's **Include Library** command to include only `TinyGsmClient.h`.
- Made Sequans Monarch deactivate all data contexts when disconnecting.

### Added

- Added SIM7500/SIM7600/SIM7800 GNSS-mode setters and getters.

### Fixed

- Fixed the internal integer and floating-point stream-length parsers.
- Made SIM7000 startup accept `SMS Ready` and reduced its reboot wait.

***

## [0.10.5]

### Changed

- Reduced the SIM7000 reboot wait so modules that omit a ready notification do not wait for a full minute.

### Fixed

- Accepted either `RDY` or `SMS Ready` as SIM7000 startup notifications.

***

## [0.10.3]

### Changed

- Made the underlying modem stream public again.

### Fixed

- Added missing null-pointer checks across modem and client operations.
- Checked both GPRS and EPS registration status on SIM7000 modules.
- Fixed SIM800 CCID parsing and battery-voltage reporting.
- Fixed temperature reporting on the Quectel M95.

***

## [0.10.1]

### Changed

- Reorganized shared modem functionality into CRTP templates inherited by the individual drivers.
- Reduced use of Arduino `String` objects, retaining them primarily for return values.
- Expanded the compile-test sketch to cover modem-specific functionality.

### Added

- Added support for SIM5320/SIM5360/SIM7100 modules.
- Added support for SIM7500/SIM7600/SIM7800 modules.
- Added network-time retrieval across all modem families, including parsed date/time output where supported.
- Added GPS accuracy data and consistent decimal-degree conversion to location APIs.
- Added IMSI retrieval and additional XBee firmware features.

### Fixed

- Corrected GPS, GSM-location, timezone, registration, socket, and connection handling across BG96, SIM7000, SARA-R4, Sequans Monarch, XBee, and other drivers.

***

## [0.7.9]

### Fixed

- Fixed receive-byte accounting, missed data notifications, and socket-state handling across BG96, SIM7000, SIM800, and other modem families.
- Corrected Quectel M95/MC60 command handling and SIM7000 operation.
- Fixed XBee SSL behavior and u-blox battery-percentage reporting.

> [!WARNING]
> Do not use releases 0.7.4 or 0.7.7; update to 0.7.9 or later.

***

## [0.7.4]

### Changed

- Removed the memory-intensive virtual modem base class and consolidated common behavior in preprocessor-based helpers.
- Implemented connection timeouts for every modem.
The client connection timeout is specified in seconds; most other timeouts use milliseconds.
- Added bounded waits to modem operations that previously could hang indefinitely.
- Split SARA-R4/SARA-N4 support from the generic u-blox driver.

### Added

- Added Sequans Monarch LTE-M support.
- Added chip-temperature and additional battery APIs where supported.

### Fixed

- Corrected timeout handling, connection results, receive lengths, socket closure, and XBee connection-state detection across multiple drivers.

***

## [0.6.2]

### Changed

- Added the `TinyGsmModem` parent interface so other libraries could use common modem functions without knowing the concrete modem type.
- Reorganized bundled AT-command manuals and expanded modem compile coverage.

### Added

- Added Quectel M95 support.
- Added Quectel MC60/MC60E support.
- Added SIM7000 support.

### Fixed

- Improved SARA-R4, XBee, SIM800, ESP8266, BG96, and other modem reset, socket, registration, and receive handling.

***

## [0.3.5]

### Changed

- Overrode `Client::write(const char*)` to optimize transfers on platforms such as STM32Duino.
- Renamed and expanded the generic u-blox driver.

### Added

- Added initial Quectel BG96 support.
- Added Hologram Dash, u-blox SARA-U201, Arduino MKR GSM 1400, and Industruino support.
- Added network-time retrieval for SIMCom modules.
- Added Arduino compatibility classes for non-Arduino client environments.

### Fixed

- Improved u-blox and XBee operation, examples, diagnostics, and build tests.

***

## [0.3.3]

### Fixed

- Fixed the SIM900 build.
- Clarified the README for new users and added answers to common questions.

***

## [0.3.2]

### Changed

- Renamed the misleading modem `autoBaud()` method to `testAT()` and added `setBaud()`.
- Made `gprsConnect()` credentials optional and exposed the modem stream.
- Added minimum and maximum baud-rate arguments to `TinyGsmAutoBaud()`.
- Moved library headers into `src` and expanded per-modem compile testing.

### Added

- Added Digi XBee cellular and Wi-Fi support.
- Added alpha support for u-blox SARA-U201.
- Added sleep control for SIM800 and M590, plus DTMF support for SIM800 and A6.
- Added ESP8266 modem information, signal quality, local IP, and initial SSL support.
- Added `isGprsConnected()` and `isNetworkConnected()`.

### Fixed

- Fixed local-IP and call handling for SIM800 and A6.
- Fixed socket closure, receive handling, and connection-state tracking across SIM800, ESP8266, XBee, and other drivers.

***

## [0.3.1]

### Changed

- Made `sendAT()` and `waitResponse()` public for applications that need to issue custom AT commands.

### Added

- Added HTTPS support for the SIM800 series.
- Added GPS support for SIM808/SIM868.
- Added calling and hang-up support for A6, A7, and SIMCom modules.
- Added 7-, 8-, and 16-bit USSD decoding, `sendUSSD()`, `localIP()`, `getModemInfo()`, `poweroff()`, and `TinyGsmAutoBaud()`.
- Added SIM808 SMS reading/deletion and new HTTP, HTTPS, and all-functions examples.

### Fixed

- Fixed false-positive A6 connection results, URC handling, and diagnostics for several modem types.

***

## [0.2.0]

### Changed

- Reorganized and grouped the public API and renamed the ESP8266 client header for consistency with the other modem drivers.

### Fixed

- Made major SIM800 stability improvements.
- Improved A6/A7 operation and ESP8266 watchdog handling.
- Fixed recursive response parsing and added yields to long modem operations.

***

## [0.1.7]

### Added

- Added beta support for A6 and A7 modems.
- Expanded diagnostics for all supported modem types.

### Fixed

- Fixed several A6, SIM800, M590, and ESP8266 issues.

***

## [0.1.6]

### Changed

- Split modem implementations into dedicated headers and introduced shared helpers in `TinyGsmCommon.h`.
- Improved diagnostics, examples, and compile tests.

### Added

- Added Neoway M590 support.
- Added ESP8266 AT-firmware support.

***

## [0.1.5]

### Fixed

- Prevented watchdog resets when the library runs on ESP8266 boards.

***

## [0.1.4]

### Changed

- Improved SIM900 compatibility and MUX handling.
- Updated the examples and renamed the SimpleTest sketch to Diagnostics.
- Improved documentation, build configuration, and network-wait behavior.

***

## [0.1.3]

### Changed

- Updated the public API, tools, and examples.

### Added

- Added automatic baud-rate detection.
- Added Travis CI, PlatformIO compile tests, and project artwork.

### Fixed

- Fixed builds on additional platforms and improved FIFO result reporting.

***

## [0.1.2]

### Changed

- Updated the supported-module list, keywords, package metadata, and README.
- Removed bundled AT-command manuals from the release package.

***

## [0.1.1]

### Changed

- Updated examples, tools, documentation, and package metadata.

### Added

- Added modem `init()` support and the SimpleTest diagnostic sketch.

***

## [0.1.0]

Initial release.

***

[Unreleased]: https://github.com/vshymanskyy/TinyGSM/compare/v0.13.0-rc.1...HEAD
[0.13.0-rc.1]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.13.0-rc.1
[0.12.0]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.12.0
[0.11.7]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.11.7
[0.11.5]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.11.5
[0.11.3]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.11.3
[0.11.2]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.11.2
[0.11.1]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.11.1
[0.10.9]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.10.9
[0.10.7]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.10.7
[0.10.5]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.10.5
[0.10.3]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.10.3
[0.10.1]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.10.1
[0.7.9]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.7.9
[0.7.4]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.7.4
[0.6.2]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.6.2
[0.3.5]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.3.5
[0.3.3]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.3.3
[0.3.2]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.3.2
[0.3.1]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.3.1
[0.2.0]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.2.0
[0.1.7]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.1.7
[0.1.6]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.1.6
[0.1.5]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.1.5
[0.1.4]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.1.4
[0.1.3]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.1.3
[0.1.2]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.1.2
[0.1.1]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.1.1
[0.1.0]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.1.0

<!-- cSpell:ignore Brusco Juxn3 Mikael Fredriksson Bengarman poweroff interchar urat tecnology STM32Duino LukasSkywalker IoTThinks wifixcort -->
