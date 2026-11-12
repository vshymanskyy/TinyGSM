# Modem Implementations

This folder contains the modem-specific TinyGSM client implementations.
Most user sketches should define a specific modem type and then include either `TinyGsmClient.h` or `TinyGSM.h`.
Multi-modem sketches should **not** define a specific modem type.
They should include `TinyGSM.h` (**not** `TinyGsmClient.h`) and include the required modem headers from this folder directly.
