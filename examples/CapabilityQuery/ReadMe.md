# Capability Query<!--! {#example_capability_query} -->

This example shows how to query modem capabilities at compile time using capability detection traits.
This is particularly useful when working with multiple modem types in the same codebase.

## Benefits<!--! {#example_capability_query_benefits} -->

- Type-safe capability detection
- Works correctly with multiple modem types
- Compile-time evaluation (C++11 compatible)
- No runtime overhead

## Available Capability Detection Traits<!--! {#example_capability_query_available} -->

- `TinyGsmCapabilities::has_ssl` - SSL/TLS secure connections
- `TinyGsmCapabilities::can_specify_certs` - Certificate specification for SSL
- `TinyGsmCapabilities::can_load_certs` - Certificate loading for SSL
- `TinyGsmCapabilities::has_gps` - GPS/GNSS positioning
- `TinyGsmCapabilities::has_cellular_data` - Cellular data
- `TinyGsmCapabilities::has_wifi` - WiFi connectivity
- `TinyGsmCapabilities::has_sms` - SMS messaging
- `TinyGsmCapabilities::has_calling` - Voice calls
- `TinyGsmCapabilities::has_battery` - Battery status reporting
- `TinyGsmCapabilities::has_temperature` - Temperature sensor
- `TinyGsmCapabilities::has_ntp` - Network Time Protocol
- `TinyGsmCapabilities::has_time` - Clock/time functions
- `TinyGsmCapabilities::has_gsm_location` - GSM-based location
- `TinyGsmCapabilities::has_bluetooth` - Bluetooth connectivity
- `TinyGsmCapabilities::has_tcp` - TCP socket connections

_______

<!--! @section example_capability_query_code The Complete Code -->

<!--! @include{lineno} CapabilityQuery/CapabilityQuery.ino -->
