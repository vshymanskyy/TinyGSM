# TinyGSM Modem Client Configuration Summary<!--! {#page_supported_module_configuration} -->

This table provides a comprehensive overview of TCP/SSL socket configuration and connection information for all supported modems.

| Modem          | TCP Sockets   | SSL Sockets | SSL Contexts | Socket Numbering | On-Module Buffer     | Max Send Size (bytes) | Connect Timeout (s) | Stop Timeout (s) | Min Free TX Buffer |
| -------------- | ------------- | ----------- | ------------ | ---------------- | -------------------- | --------------------- | ------------------- | ---------------- | ------------------ |
| A6             | 8             | N/A         | N/A          | Dynamic          | No Buffer            | 1024                  | 75                  | 1                | 1                  |
| A7672X         | 10            | 2           | 10           | Static           | Readable and Sizable | 1500                  | 75                  | 15               | 1                  |
| BG96           | 12 (combined) | —           | 6            | Static           | Readable and Sizable | 1460                  | 150                 | 15               | 1                  |
| ESP32          | 5 (combined)  | —           | 2            | Static           | Readable and Sizable | 2920                  | 75                  | 5                | 1                  |
| ESP8266        | 5 (combined)  | —           | 2            | Static           | No Buffer            | 1460                  | 75                  | 5                | 1                  |
| ESP8266 NonOS  | 5 (combined)  | —           | 2            | Static           | No Buffer            | 1460                  | 75                  | 5                | 1                  |
| M590           | 2             | N/A         | N/A          | Static           | No Buffer            | 2000                  | 75                  | 1                | 1                  |
| M95            | 6             | N/A         | N/A          | Static           | Readable             | 1460                  | 75                  | 75               | 1                  |
| MC60           | 6             | N/A         | N/A          | Static           | Readable             | 1460                  | 75                  | 75               | 1                  |
| SaraR4         | 7             | 4           | 5            | Dynamic          | Readable and Sizable | 1024                  | 120                 | 135              | 1                  |
| SaraR5         | 7 (combined)  | —           | 5            | Dynamic          | Readable and Sizable | 1024                  | 120                 | 15               | 1                  |
| SequansMonarch | 6 (combined)  | —           | 6            | Static           | Readable and Sizable | 750                   | 75                  | 15               | 1                  |
| SIM5360        | 10            | N/A         | N/A          | Static           | Readable and Sizable | 1500                  | 15                  | 15               | 1                  |
| SIM7000        | 8             | N/A         | N/A          | Static           | Readable and Sizable | 1500                  | 75                  | 15               | 1                  |
| SIM7000SSL     | 2 (combined)  | —           | 6            | Static           | Readable and Sizable | 1460                  | 75                  | 15               | 1                  |
| SIM7080        | 12 (combined) | —           | 6            | Static           | Readable and Sizable | 1360                  | 75                  | 15               | 1360               |
| SIM7600        | 10            | 2           | 10           | Static           | Readable and Sizable | 1500                  | 75                  | 15               | 1                  |
| SIM800         | 8             | 5           | —            | Static           | Readable and Sizable | 1500                  | 75                  | 15               | 1                  |
| SIM808         | 8             | 5           | —            | Static           | Readable and Sizable | 1500                  | 75                  | 15               | 1                  |
| UBLOX          | 7 (combined)  | —           | —            | Dynamic          | Readable and Sizable | 1024                  | 120                 | 15               | 1                  |
| XBee           | 1 (combined)  | —           | 1            | Static           | No Buffer            | 1500                  | 75                  | 5                | 1                  |

## Legend

- **—**: No separate value applies or no modem-specific value is defined
- **N/A**: The feature is not available
- **Modem**: The modem model or family
- **TCP Sockets**: Number of TCP-only socket connections (if separated from SSL) or combined secured and unsecured sockets
- **SSL Sockets**: Number of independent SSL socket connections (if separated from TCP)
- **SSL Contexts**: Number of SSL security profiles/contexts that can be configured
- **Socket Numbering**:
  - **Static**: User specifies the multiplexing channel number
  - **Dynamic**: Modem automatically assigns channel numbers
- **On-Module Buffer**:
  - **No Buffer**: Modem has no buffer; library must handle all buffering
  - **Readable**: Modem has a buffer but no command to check buffer size
  - **Readable and Sizable**: Modem has a buffer and provides size checking
- **Max Send Size**: Maximum bytes that can be sent in a single transmission
- **Connect Timeout**: Default connection timeout in seconds
- **Stop Timeout**: Default clean disconnection timeout
- **Min Free TX Buffer**: Minimum free transmit buffer space required before sending
