/**
 * @file       TinyGsmClientSIM808.h
 * @author     Lukasz Wozniak
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2017 Lukasz Wozniak
 * @date       Jul 2017
 */
#define TinyGSM TinyGSM808
#define GsmClient GsmClient800
#include <TinyGsmClientSIM800.h>
#undef TinyGSM
#undef GsmClient

#ifndef TinyGsmClientSIM808_h
#define TinyGsmClientSIM808_h

class TinyGSM : TinyGSM808
{

}

class GsmClient : GsmClient800
{
public:
  virtual bool getGps() {
    return true;
  }
}
