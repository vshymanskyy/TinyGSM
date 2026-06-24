/*
  Udp.h - Base class that provides UDP Client
  Copyright (c) 2011 Adrian McEwen.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef udp_h
#define udp_h
#include "Print.h"
#include "Stream.h"
#include "ArduinoCompat/IPAddress.h"

class UDP: public Stream {

    public:
        virtual uint8_t begin(uint16_t) = 0;
        virtual uint8_t beginMulticast(IPAddress, uint16_t) { return 0; }
        virtual void stop() = 0;
        virtual int beginPacket(IPAddress ip, uint16_t port) = 0;
        virtual int beginPacket(const char *host, uint16_t port) = 0;
        virtual int endPacket() = 0;
        virtual size_t write(uint8_t) = 0;
        virtual size_t write(const uint8_t *buffer, size_t size) = 0;
        virtual int parsePacket() = 0;
        virtual int available() = 0;
        virtual int read() = 0;
        virtual int read(unsigned char* buffer, size_t len) = 0;
        virtual int read(char* buffer, size_t len) = 0;
        virtual int peek() = 0;
        virtual void flush() = 0;
        virtual operator bool() = 0;
    protected:

        uint8_t* rawIPAddress(IPAddress& addr) {
            return addr.raw_address();
        }
};

#endif