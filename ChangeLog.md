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

This release substantially updates TinyGSM's client, socket, SSL/TLS, parsing,
and modem-support layers. The documented single-modem entry point remains
`#include <TinyGsmClient.h>`, but applications that use modem-specific headers,
internal mixins, SSL configuration, registration-status values, or the changed
return values below will need to be updated.

### Changed

- **BREAKING — SSL/TLS configuration and ownership:** Rebuilt the shared TCP
  and SSL layers. SSL settings now belong to reusable `GsmSecureContext`
  objects rather than individual secure clients. Applications must keep each
  modem, context, and attached client alive for as long as the dependent
  objects use it.
  - Functions that copy certificate data into a modem now use `load...` names;
    functions that select an already-loaded credential use `...Name` names.
  - Authentication mode, TLS version, CA certificate, client certificate and
    key, PSK, and PSK-table settings are now configured through secure
    contexts on supported modems.
  - Modem-specific secure client classes were standardized as
    `GsmClientSecureSim7000SSL`, `GsmClientSecureSim7080`,
    `GsmClientSecureSaraR4`, and `GsmClientSecureSaraR5`. The previous names
    remain as compatibility aliases, and the global `TinyGsmClientSecure`
    alias is unchanged.
- **BREAKING — scoped enums:** Date/time, registration-status, and SSL values
  now use scoped `enum class` types, so enum values must be qualified with
  their type. Registration-status type capitalization was also standardized;
  for example, `SIM800RegStatus` became `Sim800RegStatus`.
- **BREAKING — header locations:** Moved modem implementations into
  `src/modems` and internal CRTP mixins into `src/mixins`. The standard
  `#include <TinyGsmClient.h>` remains unchanged. Direct modem includes must
  use paths such as `#include <modems/TinyGsmClientSIM7080.h>`.
- **BREAKING — failure values:** Standardized unsupported, missing, and invalid
  sensor results across modem families. `getBattVoltage()` returns `-9999`,
  `getBattPercent()` and `getBattChargeState()` return `-99`, and
  `getTemperature()` returns `-9999.0F` on failure.
- **BREAKING — u-blox radio access APIs:** Corrected
  `setRadioAccessTecnology()` and `getCurrentRadioAccessTecnology()` to
  `setRadioAccessTechnology()` and `getCurrentRadioAccessTechnology()`. SARA-R4
  now uses `setRadioAccessTechnology()` instead of `setURAT()`, and the SARA-R5
  setter argument types and getter result and implementation were corrected.
- **Potentially breaking — NTP result:** `NTPServerSync()` now consistently
  returns `bool`, with `true` meaning success. Code that interpreted the old
  numeric results from Espressif or BG96 modems may have its condition
  inverted.
- **Potentially breaking — typed configuration:** Replaced internal modem,
  TCP, SSL, and response macros, including `TINY_GSM_MUX_COUNT`, with public
  `ModemConfig`, `TcpConfig`, and `SslConfig` traits. These traits describe
  socket and SSL-context counts, static or dynamic mux assignment, receive
  behavior, send limits, timeouts, and response strings.
- Replaced the internal `TinyGsmGPRS` mixin with `TinyGsmCellularData` and added
  generic cellular-data terminology. The historical `gprsConnect()`,
  `gprsDisconnect()`, and `isGprsConnected()` APIs remain as aliases.
- Reworked client and context registration so objects can be default-created,
  explicitly initialized, safely attached or detached, and reassigned without
  leaving stale modem socket or SSL-context pointers.
- Split Espressif support into separate drivers for ESP32 AT firmware 3.2 or
  newer, ESP8266
  [v2.2.1 firmware](https://github.com/espressif/esp-at/releases/tag/v2.2.1.0_esp8266),
  and the legacy ESP8266 Non-OS firmware.
- Moved bundled utilities from `tools` to `extras/tools`, the compile-test
  sketch to `extras/tools/test_build`, and download fixtures to
  `extras/download_tests` so the packaged library has a consistent layout.
- Made the default client receive buffer platform-aware: 1024 bytes on SAMD51,
  256 bytes on other SAMD boards, and 64 bytes elsewhere. The
  `TINY_GSM_RX_BUFFER` override remains available.
- Reworked `waitResponse()` and timed stream parsing to use explicit overall
  and inter-character limits instead of repeated yields. The new
  `TINY_GSM_INTERCHAR_TIMEOUT_MS` and `TINY_GSM_RESPONSE_MAX_GRACE_MS` settings
  bound incomplete and continuously arriving responses.
- Increased the maximum autobaud rate to 921600 and made baud probing preserve
  the stream timeout.
- Standardized NTP timezone inputs as quarter-hour offsets and corrected date,
  time, timezone, and epoch handling across modem families.
- Aligned the A7672x cellular-data, socket, and SSL flows with the related
  SIM7600 implementation.

### Added

- Added modem-scoped `TinyGsmClient`, `TinyGsmClientSecure`, and
  `TinyGsmSecureContext` aliases for libraries and multi-modem sketches. The
  existing global aliases remain available for single-modem sketches.
- Added public client/context lifecycle and lookup APIs, including `init()`,
  `isAttached()`, `detach()`, `getClient()`, `findFirstClient()`, and
  `findFirstIdleClient()`. Static-mux clients can request strict attachment to
  a specific socket.
- Added `isSecure()`, `getMux()`, and `getConnectionID()` so applications can
  inspect a client and distinguish its TinyGSM socket slot from a modem's
  internal connection identifier.
- Added `beginWrite()` and `endWrite()` for sending a complete payload through
  a modem's direct-send path. Oversized writes are rejected, accepted byte
  counts can be checked where supported, and ordinary writes are split into
  modem-safe chunks. The API can be paired with PubSubClient's
  `beginPublish()` and `endPublish()`.
- Added generic `dataConnect()`, `dataDisconnect()`, and `isDataConnected()`
  names while retaining the historical `gprs*` aliases.
- Added compile-time modem capability traits for SSL, certificate management,
  networking, GPS, SMS, calling, battery, temperature, Bluetooth, time, and
  other features, plus a `CapabilityQuery` example.
- Added `getConfiguredModem()` to identify the configured modem without sending
  an AT command, a selected-modem `RegStatus` alias, and public aliases for the
  modem, TCP, and SSL configuration types.
- Added SSL version and authentication selection, PSK and PSK-table support,
  CA/client credential selection, stored-certificate conversion and printing,
  and AVR-safe loading of certificates from program memory on supported
  modems.
- Added SIM7600 SSL support from \@floBik.
- Added `getNetworkEpoch()` and public `waitForTimeSync()` support on capable
  modems.
- Added `setDefaultBaud()` for Espressif modems and `forceModemBaud()` for
  recovering a modem whose current baud rate is unknown or incorrect.
- Added the configurable `TINY_GSM_UNREAD_CHECK_MS` interval for polling modems
  that may lose incoming-data notifications.
- Added opt-in `TINY_GSM_USE_PUBLIC_DNS` support. It is disabled by default so
  supported modems continue using their network-provided DNS servers.
- Added support for Digi XBee3 Cellular LTE-M3 modules.
- Added `powerOff()` as the preferred spelling and deprecated `poweroff()`.
- Added AWS IoT Core mutual-authentication, multiple-connection, and
  multiple-modem examples, plus an Espressif AT-firmware update tool.
- Added generated modem capability documentation, Doxygen coverage for the
  public API, example and source guides, and library-structure/release tooling.

### Removed

- **BREAKING:** Removed `ShowNTPError()`.
- Removed the `TINY_GSM_YIELD` and `TINY_GSM_YIELD_MS` configuration macros;
  timed stream helpers now bound parser waits directly.
- Removed modem-owned certificate-name arrays; secure contexts now own the
  credential names used by their connections.
- Removed the obsolete `AT_Spy` and Industruino sketches and legacy Makefile
  and archived Travis configuration.
- Stopped including the certificate-management SSL mixin on modems that provide
  only basic secure-client support.

### Fixed

- Fixed socket assignment, mux validation and translation, attachment and
  destruction, connection-state tracking, partial sends, stop behavior, and
  receive-buffer draining across modem families.
- Avoided unnecessary socket-status and available-byte queries, including
  repeated Espressif queries whose responses already describe every socket.
- Corrected per-modem TCP socket, secure-socket, and SSL-context counts, and
  separated shared socket limits from SSL configuration-profile limits.
- Hardened stream, integer, floating-point, IP-address, and hexadecimal parsing
  against malformed or partial data, invalid lengths, overflow, and underflow.
- Fixed SSL-context validation, credential pairing and attachment, certificate
  loading and conversion, connection setup, and failure cleanup across
  supported secure modems.
- Fixed SIM7070/SIM7080/SIM7090 parsing of the 18-field `+CGNSINF` GPS response,
  eliminating an unnecessary five-second wait.
- Fixed A7672x and SIM7600 cellular-data activation, socket initialization,
  command ordering, result handling, battery-voltage units, and SSL setup.
- Fixed Espressif socket handling, URCs, time synchronization, timezone
  conversion, certificate management, and long-domain lookup behavior.
- Fixed XBee command-mode transitions, socket setup and state tracking, DNS/IP
  handling, and SSL configuration.
- Fixed FIFO copying for element types larger than one byte, empty-buffer
  peeking, and receive-buffer initialization.
- Updated non-TLS HTTP examples to use a site that still accepts unencrypted
  connections, and regenerated the Arduino keywords list from the documented
  API.

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

[Unreleased]: https://github.com/vshymanskyy/TinyGSM/compare/v0.13.0-rc.1...HEAD
[0.13.0-rc.1]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.13.0-rc.1
[0.12.0]: https://github.com/vshymanskyy/TinyGSM/releases/tag/v0.12.0

<!-- cSpell:ignore Brusco Juxn3 Mikael Fredriksson Bengarman poweroff interchar urat tecnology -->
