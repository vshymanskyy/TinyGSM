/**
 * @file       TinyGsmAudio.tpp
 * @author     Volodymyr Shymanskyy
 * @license    LGPL-3.0
 * @copyright  Copyright (c) 2016 Volodymyr Shymanskyy
 * @date       Nov 2016
 */

#ifndef _TINY_GSM_AUDIO_TPP
#define _TINY_GSM_AUDIO_TPP

#include "TinyGsmCommon.h"



template <class modemType>
class TinyGSMAudio {
  /* =========================================== */
  /* =========================================== */
  /*
   * Define the interface
   */
 public:
  /**
   * @brief Set the speaker volume (0 to 100).
   *
   * @param level Volume level
   * @return true if successful
   */
  bool setSpeakerVolume(uint8_t level) {
    return thisModem().setSpeakerVolumeImpl(level);
  }

  /**
   * @brief Get the speaker volume.
   *
   * @return Speaker volume level, or -1 on failure
   */
  int getSpeakerVolume() {
    return thisModem().getSpeakerVolumeImpl();
  }

  /**
   * @brief Mute or unmute the microphone.
   *
   * @param mute true to mute, false to unmute
   * @return true if successful
   */
  bool setMicMute(bool mute) {
    return thisModem().setMicMuteImpl(mute);
  }

  /**
   * @brief Set the microphone gain (0 to 15).
   *
   * @param gain Gain level
   * @return true if successful
   */
  bool setMicGain(uint8_t gain) {
    return thisModem().setMicGainImpl(gain);
  }

  /**
   * @brief Set monitor speaker loudness (0 to 5).
   *
   * @param level Loudness level
   * @return true if successful
   */
  bool setMonitorLoudness(uint8_t level) {
    return thisModem().setMonitorLoudnessImpl(level);
  }

  /**
   * @brief Enable or disable DTMF detection.
   *
   * @param enable true to enable, false to disable
   * @return true if successful
   */
  bool enableDTMFDetection(bool enable) {
    return thisModem().enableDTMFDetectionImpl(enable);
  }

 protected:
  inline const modemType& thisModem() const {
    return static_cast<const modemType&>(*this);
  }
  inline modemType& thisModem() {
    return static_cast<modemType&>(*this);
  }
  ~TinyGSMAudio() {}

  /* =========================================== */
  /* =========================================== */
  /*
   * Define the default function implementations
   */
 protected:
  bool setSpeakerVolumeImpl(uint8_t level) {
    if (level > 100) level = 100;
    thisModem().sendAT(GF("+CLVL="), level);
    return thisModem().waitResponse() == 1;
  }

  int getSpeakerVolumeImpl() {
    thisModem().sendAT(GF("+CLVL?"));
    if (thisModem().waitResponse(GF("+CLVL:")) != 1) return -1;
    int level = thisModem().streamGetIntBefore('\n');
    thisModem().waitResponse();
    return level;
  }

  bool setMicMuteImpl(bool mute) {
    thisModem().sendAT(GF("+CMUT="), mute ? 1 : 0);
    return thisModem().waitResponse() == 1;
  }

  bool setMicGainImpl(uint8_t gain) {
    if (gain > 15) gain = 15;
    thisModem().sendAT(GF("+QMIC=0,"), gain);
    return thisModem().waitResponse() == 1;
  }

  bool setMonitorLoudnessImpl(uint8_t level) {
    if (level > 5) level = 5;
    thisModem().sendAT(GF("+ATL="), level);
    return thisModem().waitResponse() == 1;
  }

  bool enableDTMFDetectionImpl(bool enable) {
    thisModem().sendAT(GF("+QTONEDET="), enable ? 1 : 0);
    return thisModem().waitResponse() == 1;
  }
};


#endif
