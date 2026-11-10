# ChangeLog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and its stricter, better defined, brother [Common Changelog](https://common-changelog.org/).

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

***

## [Unreleased]

### Changed

- **BREAKING** Rewrote SSL support module!
  - Certificate-loading and per-client credential-selection function names have changed.
    - Modem functions have been renamed to "load" instead of "set" to clarify that the certificate data is being loaded into the flash of the module rather than being used somewhere in the library code.
    - Client functions have been renamed to "setXX**Name**" to clarify that what you are selecting is the file name of a certificate that has been pre-loaded onto the module, not the certificate data itself.
    - New shared parent classes have been created for TCP and secure clients.
    - SSL settings are now owned by reusable `GsmSecureContext` objects rather than by individual clients.
- **BREAKING** Made date/time, registration-status, and SSL enums scoped enum classes, requiring the class name to be included when using their values. Registration-status type names were also standardized (for example, `SIM800RegStatus` became `Sim800RegStatus`).
- Renamed modem-specific secure client classes for consistent capitalization:
  - `GsmClientSecureSIM7000SSL` to `GsmClientSecureSim7000SSL`,
  - `GsmClientSecureSIM7080` to `GsmClientSecureSim7080`,
  - `GsmClientSecureR4` to `GsmClientSecureSaraR4`, and
  - `GsmClientSecureR5` to `GsmClientSecureSaraR5`.
  - The previous modem-specific names remain as compatibility aliases, and the common `TinyGsmClientSecure` alias is unchanged.
- **Potentially breaking** Replaced the internal `TinyGsmGPRS` mixin and `TinyGsmGPRS.tpp` header with `TinyGsmCellularData` and `TinyGsmCellularData.tpp`. The public `gprsConnect()`, `gprsDisconnect()`, and `isGprsConnected()` functions remain as compatibility aliases.
- **Potentially breaking** Replaced modem TCP, SSL, and response configuration macros, including `TINY_GSM_MUX_COUNT`, with typed `ModemConfig`, `TcpConfig`, and `SslConfig` traits that centralize socket counts, buffer modes, send limits, timeouts, response strings, and SSL-context counts.
- Separated Espressif into **3** modules:
  - the ESP32 (requiring AT firmware >= 3.2),
  - the ESP8266 using the final release of its AT firmware ([v2.2.1_esp8266](https://github.com/espressif/esp-at/releases/tag/v2.2.1.0_esp8266)),
  - and the ESP8266 using the original "non-OS" firmware that the first version of this library was written for.
- Created missing interface for CRTP for the client related modem functions
- Made adjustments and corrections to the A7672x based on similar functionality of the SIM7600.
- Fixed various compiler warnings, where possible
- Increased max baud rate for autobauding.
- Made the default receive buffer platform-aware: 1024 bytes on SAMD51, 256 bytes on other SAMD boards, and 64 bytes elsewhere, while retaining the `TINY_GSM_RX_BUFFER` override.
- Made `waitResponse()` use explicit overall and inter-character timeouts instead of delaying after each received character, configurable with `TINY_GSM_INTERCHAR_TIMEOUT_MS` and `TINY_GSM_RESPONSE_MAX_GRACE_MS`.
- Minor changes in notes and comments
- **BREAKING** Corrected the spelling of the function `setRadioAccessTecnology` to `setRadioAccessTechnology` and `getCurrentRadioAccessTecnology` to `getCurrentRadioAccessTechnology` for generic u-blox modules.
- **BREAKING** Renamed the function `setURAT` to `setRadioAccessTechnology` for u-blox SARA R4 modules.
- **BREAKING** Changed the inputs and outputs of `getCurrentRadioAccessTechnology` for the u-blox SARA R5 modules and made functional.
- **Potentially breaking** Changed the response type of NTPServerSync from a byte with inconsistent meaning to a boolean indicating success.  Using a boolean test against this may now return inverted results. Previously, Espressif and BG96 modules returned a falsy '0' byte indicating success, but SIMCom modules returned a truthy '1' byte for success.
- Standardized NTP timezone arguments as offsets in quarter-hours and corrected date, time, timezone, and epoch parsing across modem families.

### Added

- Added modem-scoped `TinyGsmClient`, `TinyGsmClientSecure`, and `TinyGsmSecureContext` aliases for libraries and multi-modem sketches while preserving the global single-modem aliases.
- Added support for selecting SSL version and authentication types on select modules
- Added support for SSL pre-shared keys and PSK tables on select modules.
- Added support for writing/loading certificates on select modules
- Added support for printing and converting stored certificates on select modules.
- Added AVR-safe loading of certificates stored in program memory.
- Added support for selecting specific certificates for specific connections on select modules
  - This includes both CA and client certs
- Added SSL support for the SIM7600 by \@floBik
- Added functions `beginWrite` and `endWrite` to directly write into the modem's send buffer to help ensure that an entire request is sent in one chunk.
  - `beginWrite()` rejects requests larger than the modem's configured single-send limit.
  - `endWrite()` can verify the number of bytes accepted on modems that report it.
  - These functions are directly analogous to the `beginPublish` and `endPublish` functions within [PubSubClient](https://github.com/knolleary/pubsubclient) and should be usable in combination with them.

```cpp
uint16_t mqtt_remaining_length = 2 + topic_len + msg_len;
uint16_t mqtt_packet_length    = 1 + mqtt_remaining_length;
do {
  mqtt_packet_length++;
  mqtt_remaining_length /= 128;
} while (mqtt_remaining_length > 0);
gsmClient.beginWrite(mqtt_packet_length);
mqttClient.beginPublish(topic, msg_len, false);
gsmClient.write((uint8_t*)message, strlen(message));
// or mqttClient.write(...) or stream.write(...)
// print/println functions should also work here
mqttClient.endPublish();
gsmClient.endWrite(mqtt_packet_length);
```

- Added an example connecting to AWS IoT Core with mutual authentication.
- Added an example using multiple connections at once
- Added an example using multiple modem types in one sketch.
- Added a tool for updating Espressif AT firmware.
- Added an "is_secure" property to all clients to help differentiate them
- Added a function (`setDefaultBaud(uint32_t baud)`) to set default baud rate, currently only available on Espressif modules
- Added a `forceModemBaud()` function to recover a modem with an unknown or incorrect baud rate.
- Added defines for the frequency of checking for lost incoming message notifications (`TINY_GSM_UNREAD_CHECK_MS`)
  - This can be used as an external build flag.
- Added per-modem typed configuration for socket counts, static or dynamic socket numbering, receive-buffer behavior, maximum send sizes, and connection, send, and stop timeouts.
  - All ordinary client writes are split into modem-safe chunks.
- Implemented a function (`moveCharsFromStreamToFifo`) for transferring a stream of characters from the modem stream into the fifo.
  - Implemented use of this function for all modules.
- Added compile-time modem capability detection for networking, GPS, SSL, SMS, calling, battery, temperature, Bluetooth, and more and an example demonstrating its use.
- Added a function getConfiguredModem() to return information about the compiled modem *without* communicating with the modem.
- Added generic `dataConnect()`, `dataDisconnect()`, and `isDataConnected()` names while retaining the historical `gprs*` names as aliases.
- Added `getNetworkEpoch()` and a public `waitForTimeSync()` function on supported modems.
- Added `getMux()` and `getConnectionID()` to distinguish the library socket index from modem-specific connection numbering.
- Added `RegStatus` aliases for the selected modem and public modem, TCP, and SSL configuration types.
- Added opt-in `TINY_GSM_USE_PUBLIC_DNS` support; it defaults to disabled so supported modems retain their network-assigned DNS servers unless public DNS is explicitly requested.
- Added support for Digi XBee3 Cellular LTE-M3 modules.
- Added complete Doxygen-style documentation to all functions and created scripts to update the internal lists of available functions for each modem.
- Added new `powerOff()` function as the preferred naming convention; deprecated `poweroff()` in favor of the new camelCase naming.

### Removed

- **BREAKING** Removed the ShowNTPError function.
- Removed the `TINY_GSM_YIELD` and `TINY_GSM_YIELD_MS` configuration macros; timed stream-read helpers now bound parser waits directly.
- Removed certificate arrays from the modem objects
- Only include the SSL template on the modules that support the SSL enhancements and certificate management instead of a bare flag for security.

### Fixed

- Don't force the maintain function to call modemGetAvailable if the sock_available is already non-zero
- Don't repeatedly call for sock_connected and sock_available for each socket on espressif modules when the response always includes all sockets.
- Fixes to stop logic on Espressif and SIM7080
- Fixed socket assignment, mux validation, connection-state tracking, partial sends, and receive-buffer draining across modem families.
- Corrected per-modem TCP socket, secure-socket, and SSL-context counts and separated shared socket limits from SSL configuration-profile limits.
- Hardened stream, numeric, IP-address, and hexadecimal parsing against malformed input, partial data, overflow, underflow, and invalid lengths.
- Fixed SSL context validation, credential attachment, certificate handling, and failure cleanup across supported secure modems.
- Fixed SIM7070/SIM7080/SIM7090 parsing of the 18-field `+CGNSINF` GPS response, eliminating the unnecessary five-second wait.
- Corrected A7672x and SIM7600 cellular-data and socket initialization, command ordering, and SSL connection setup.
- Corrected multiple Espressif connection, URC, time synchronization, timezone, certificate, and long-domain lookup behaviors.
- Corrected XBee socket, command-mode, connection-state, and SSL configuration behavior.
- Modified HTTP examples to attempt to connect to a site that doesn't require SSL.
  - \@vshymanskyy's site which displays the TinyGSM logo now requires SSL.
- The Arduino keywords file now contains all relevant Doxygen-derived keywords.

***

## [v0.12.0]

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
