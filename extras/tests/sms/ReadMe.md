# SMS encoding regression

This host test calls the production SMS mixin using ArduinoCore-API's String and
Print implementations. The fake modem retains the data coding scheme selected by
CSMP across messages. It covers GSM, UTF-16, UTF-16 followed by a long or short GSM
message, and rejection of the message prompt. It does not test a physical modem,
serial transport, or cellular network.

From the TinyGSM repository on Linux with a C++11 compiler:

```sh
git clone https://github.com/arduino/ArduinoCore-API.git ../arduino-api
git -C ../arduino-api checkout 0f4e57ea193a00163ae59f0f0ff478feae7eb5db
g++ -std=c++11 -DARDUINO=100 -Iextras/tests/sms -I../arduino-api -Isrc \
  extras/tests/sms/sms_encoding.cpp \
  ../arduino-api/api/String.cpp ../arduino-api/api/Print.cpp \
  ../arduino-api/api/IPAddress.cpp ../arduino-api/test/src/dtostrf.cpp \
  ../arduino-api/test/src/itoa.cpp -o ../sms-encoding-test
../sms-encoding-test
```

On the unmodified library, the GSM message after UTF-16 leaves DCS set to 8 and
fails the regression. With the fix, the test prints its success message.
