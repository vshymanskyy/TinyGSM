<!--! @page page_the_examples Examples -->
# TinyGSM Examples

These example programs demonstrate how to use the TinyGSM library.

___

<!--! @if GITHUB -->

- [TinyGSM Examples](#tinygsm-examples)
  - [Basic Connections](#basic-connections)
    - [HTTP Client](#http-client)
    - [HTTPS Client](#https-client)
    - [MQTT Client](#mqtt-client)
    - [Web Client](#web-client)
  - [More Complex Connections](#more-complex-connections)
    - [AWS IoT Core](#aws-iot-core)
    - [File Download](#file-download)
    - [Multiple Connections](#multiple-connections)
    - [Multiple Modules](#multiple-modules)
  - [Special Libraries](#special-libraries)
    - [Blynk Client](#blynk-client)
  - [Library Features](#library-features)
    - [All Functions](#all-functions)
    - [Capability Query](#capability-query)
  - [Other Use Cases](#other-use-cases)
    - [Hologram Dash](#hologram-dash)
    - [SIM800 SSL Set Cert](#sim800-ssl-set-cert)
  - [Other Tools](#other-tools)

<!--! @endif -->

<!--! @tableofcontents -->

<!--! @m_footernavigation -->

## Basic Connections<!--! {#examples_basic_connections} -->

### HTTP Client<!--! {#examples_http_client} -->

This example uses ArduinoHttpClient with TinyGSM to perform HTTP requests for REST-style API interactions.
<!--!
- [HTTP Client](\ref examples/HttpClient) -->
- [The HTTP client example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/HttpClient)

### HTTPS Client<!--! {#examples_https_client} -->

This example uses ArduinoHttpClient with TinyGSM to perform HTTPS requests over TLS on supported modems.
<!--!
- [HTTPS Client](\ref examples/HttpsClient) -->
- [The HTTPS client example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/HttpsClient)

### MQTT Client<!--! {#examples_mqtt_client} -->

This example connects to an MQTT broker and demonstrates publish/subscribe messaging using TinyGSM and PubSubClient.
<!--!
- [MQTT Client](\ref examples/MqttClient) -->
- [The MQTT client example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/MqttClient)

### Web Client<!--! {#examples_web_client} -->

This example builds and sends a raw HTTP request over a TinyGSM client socket to illustrate manual web request handling.
<!--!
- [Web Client](\ref examples/WebClient) -->
- [The web client example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/WebClient)

## More Complex Connections<!--! {#examples_complex_connections} -->

### AWS IoT Core<!--! {#examples_aws_iot_core} -->

This example shows how to provision certificates on supported modems and connect to AWS IoT Core over MQTT with TLS.
<!--!
- [AWS IoT Core](\ref examples/AWS_IoTCore) -->
- [The AWS IoT Core example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/AWS_IoTCore)

### File Download<!--! {#examples_file_download} -->

This example downloads a remote file in chunks and validates data integrity using CRC32 checksums.
<!--!
- [File Download](\ref examples/FileDownload) -->
- [The file download example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/FileDownload)

### Multiple Connections<!--! {#examples_multiple_connections} -->

This example opens and manages multiple simultaneous client connections to different servers in one sketch.
<!--!
- [Multiple Connections](\ref examples/more/Multiple_Connections) -->
- [The multiple connections example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/more/Multiple_Connections)

### Multiple Modules<!--! {#examples_multiple_modules} -->

This example shows how to use two different modems together in the same code.
The key thing to notice is that instead of defining the modem type and including the TinyGsmClient.h file, it includes the specific modem headers.
It uses modem-scoped client aliases, such as `TinyGsmSim7080::TinyGsmClientSecure`, so the client types cannot collide.
<!--!
- [Multiple Modules](\ref examples/more/Multiple_Modules) -->
- [The multiple modules example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/more/Multiple_Modules)

## Special Libraries<!--! {#examples_special_libraries} -->

### Blynk Client<!--! {#examples_blynk_client} -->

This example connects a TinyGSM modem to Blynk so a device can exchange data and be controlled from the Blynk app.
<!--!
- [Blynk Client](\ref examples/BlynkClient) -->
- [The Blynk client example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/BlynkClient)

## Library Features<!--! {#examples_library_features} -->

### All Functions<!--! {#examples_all_functions} -->

This example demonstrates and exercises a broad set of TinyGSM modem, network, client, SMS, call, and utility functions in a single sketch for capability testing.

- [The All Functions example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/AllFunctions)

### Capability Query<!--! {#examples_capability_query} -->

This example demonstrates compile-time modem capability detection using TinyGSM traits for multi-modem codebases.
<!--!
- [Capability Query](\ref examples/CapabilityQuery) -->
- [The capability query example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/CapabilityQuery)

## Other Use Cases<!--! {#examples_use_cases} -->

### Hologram Dash<!--! {#examples_hologram_dash} -->

This example targets Hologram Dash workflows by entering passthrough mode and sending network requests through TinyGSM.
<!--!
- [Hologram Dash](\ref examples/more/Hologram_Dash) -->
- [The Hologram Dash example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/more/Hologram_Dash)

### SIM800 SSL Set Cert<!--! {#examples_sim800_ssl_set_cert} -->

This example uploads SSL certificate material to SIM8xx modems so secure TLS connections can be established.
<!--!
- [SIM800 SSL Set Cert](\ref examples/more/SIM800_SslSetCert) -->
- [The SIM800 SSL set cert example on GitHub](https://github.com/vshymanskyy/TinyGSM/tree/master/examples/more/SIM800_SslSetCert)

## Other Tools<!--! {#examples_other_tools} -->

For other helpful tools, see the [tools](../extras/tools/ReadMe.md) page.

<!--! @dir examples
@brief Contains all of the example sketches -->
