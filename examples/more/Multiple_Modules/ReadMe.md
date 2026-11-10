# Multiple Modules<!--! {#example_multiple_modules} -->

This example shows how to use two different modems together in the same code.
The key thing to notice is that instead of defining the modem type and including the TinyGsmClient.h file, it includes the specific modem headers.
The global aliases `TinyGsm`, `TinyGsmClient`, and `TinyGsmClientSecure` select only one modem type.
Use the modem classes and their scoped aliases, such as `TinyGsmSim7080::TinyGsmClientSecure`, when multiple modem headers are included.

_______

<!--! @section example_multiple_modules_code The Complete Code -->

<!--! @include{lineno} more/Multiple_Modules/Multiple_Modules.ino -->
