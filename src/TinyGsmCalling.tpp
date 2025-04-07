/**
 * @file       TinyGsmCalling.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef SRC_TINYGSMCALLING_H_
#define SRC_TINYGSMCALLING_H_

#include "TinyGsmCommon.h"

#define TINY_GSM_MODEM_HAS_CALLING

template <class modemType>
class TinyGsmCalling {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /*
   * Phone Call functions
   */

  /**
   * @brief Answer an incoming phone call
   *
   * @return *true* The call was answered.
   * @return *false* The call was not answered.
   */
  bool callAnswer() {
    return thisModem().callAnswerImpl();
  }

  /**
   * @brief Make a phone call
   *
   * @param number The number to call
   * @return *true* The call was successfully made
   * @return *false* The call wasn't made
   */
  bool callNumber(const String& number) {
    return thisModem().callNumberImpl(number);
  }

  /**
   * @brief Hang up an in-progress phone call
   *
   * @return *true* The call was hung up.
   * @return *false* The call was not hung up.
   */
  bool callHangup() {
    return thisModem().callHangupImpl();
  }

  /**
   * @brief Sent a DTMF (dual tone multi frequency) message over a phone call.
   *
   * @param cmd The command to send
   * @param duration_ms The tone duration
   * @return *true* The message was sent
   * @return *false* The message failed to send
   */
  bool dtmfSend(char cmd, int duration_ms = 100) {
    return thisModem().dtmfSendImpl(cmd, duration_ms);
  }
  
  int8_t getCallStatus() {
   return thisModem().getCallStatusImpl();
  }
  
  uint8_t getClccStatus() {
    return thisModem().getClccStatusImpl();
	}
	
  bool setVolume(uint8_t volume_level) {
    return thisModem().setVolumeImpl(volume_level);
  }
  
  uint8_t getVolume() {
    return thisModem().getVolumeImpl();
  }


  String Phonebook_Number() {
    return thisModem().getPhonebook_NumberImpl();
  }
  
  
  bool selectPhonebook() {
    return thisModem().selectPhonebookImpl();
  }
  
  /*
   * CRTP Helper
   */
 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }
  ~TinyGsmCalling() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */

  /*
   * Phone Call functions
   */
 protected:
  bool callAnswerImpl() {
    thisModem().sendAT(GF("A"));
    return thisModem().waitResponse() == 1;
  }

  // Returns true on pick-up, false on error/busy
  bool callNumberImpl(const String& number) {
    if (number == GF("last")) {
      thisModem().sendAT(GF("DL"));
    } else {
      thisModem().sendAT(GF("D"), number, ";");
    }
    int8_t status = thisModem().waitResponse(60000L, GF("OK"), GF("BUSY"),
                                             GF("NO ANSWER"), GF("NO CARRIER"));
    switch (status) {
      case 1: return true;
      case 2:
      case 3: return false;
      default: return false;
    }
  }

  bool callHangupImpl() {
    thisModem().sendAT(GF("H"));
    return thisModem().waitResponse() == 1;
  }

  // 0-9,*,#,A,B,C,D
  bool dtmfSendImpl(char cmd, int duration_ms = 100) {
    duration_ms = constrain(duration_ms, 100, 1000);

    thisModem().sendAT(GF("+VTD="),
                       duration_ms / 100);  // VTD accepts in 1/10 of a second
    thisModem().waitResponse();

    thisModem().sendAT(GF("+VTS="), cmd);
    return thisModem().waitResponse(10000L) == 1;
  }
  
//Return 0:Ready, 2: Unknown, 3:Ringing, 4:Call in progress    
  int8_t getCallStatusImpl() {
    thisModem().sendAT(GF("+CPAS"));
    if (thisModem().waitResponse(GF("+CPAS:")) != -1) { 
	   int8_t res = thisModem().streamGetIntBefore('\n');

    // Wait for final OK
    thisModem().waitResponse();
    return res;
	 } else if(thisModem().waitResponse(GF("+CPAS:")) != 1){
	   return 0;
  } else{
	   return 99;
	 }
	
  } 
  
  
  uint8_t getClccStatusImpl() {
    thisModem().sendAT(GF("+CLCC"));
    if (thisModem().waitResponse(GF("+CLCC:")) != 1) { return 9; }
    thisModem().streamSkipUntil(',');  // Skip battery charge status
    thisModem().streamSkipUntil(',');  // Skip battery charge level
    // return voltage in mV
    uint8_t res = thisModem().streamGetIntBefore(',');
    
    return res;
  }
  
  
  bool setVolumeImpl(uint8_t volume_level) {
	thisModem().sendAT(GF("+CLVL="), volume_level);
	return thisModem().waitResponse(10000L) == 1;
	}
	
  uint8_t getVolumeImpl() {
	if (thisModem().waitResponse(GF("+CLVL:")) != 1) { return 0; }
	uint8_t res = thisModem().streamGetIntBefore('\n');
	thisModem().waitResponse();
	return res;
	} 
	
  String getPhonebook_NumberImpl() {
	thisModem().sendAT(GF("+CPBR=1"));
	if (thisModem().waitResponse(GF("+CPBR:")) != 1) { return "0"; }
	thisModem().streamSkipUntil(',');  // skip the phonebook location
	thisModem().streamSkipUntil('"'); 
	String res = thisModem().stream.readStringUntil('"'); // Read saved number here
	thisModem().waitResponse();
	return res;
	} 
	
  bool selectPhonebookImpl() {
    thisModem().sendAT(GF("+CPBS=\"ME\""));
    return thisModem().waitResponse() == 1;
  }
  
  
};

#endif  // SRC_TINYGSMCALLING_H_
