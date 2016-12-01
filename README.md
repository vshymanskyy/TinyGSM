```
  _______ _____ __   _ __   __  ______ _______ _______
     |      |   | \  |   \_/   |  ____ |______ |  |  |
     |    __|__ |  \_|    |    |_____| ______| |  |  |

```

A small Arduino library for GPRS modules, that just works

Currently only SIM800/SIM900 are tested.  
More modules may be supported later.

## Getting started

  1. Using your phone:
    - Disable PIN code on the SIM card
    - Check your ballance
    - Check that APN,User,Pass are correct and you have internet
  2. Ensure the SIM card is correctly inserted into the module
  3. Provide a good, stable power supply (up to 2A, 4.0-4.2V or 5V according to your module documentation)
  4. Provide good, stable serial connection
     (Hardware Serial is recommended)
  5. Check if GSM antenna is attached

## Troubleshooting

Sometimes (especially if you played with AT comands), your module configuration may become invalid.
In this case, there may be different problems:

 * Can't connect to the GPRS network
 * Can't connect to the server
 * Sent/recieved data contains invalid bytes
 * etc.

To return module to **Factory Defaults**, use this sketch:  
  File -> Examples -> TynyGSM -> tools -> [FactoryReset](https://github.com/vshymanskyy/TinyGSM/blob/master/tools/FactoryReset/FactoryReset.ino)

__________

### License
This project is released under
The GNU Lesser General Public License (LGPL-3.0)
