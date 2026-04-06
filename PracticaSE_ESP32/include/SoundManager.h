#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <Arduino.h>
#include "Definitions.h"

class SoundManager {
 private:
  int _pin;
  bool _muted = false;

 public:
  SoundManager(int pin);
  void begin();
  void setMuted(bool mute);

  void playSelect();
  void playBootUp();

  void playNote(int frecuency, int duration);
};

#endif