#include "SoundManager.h"

SoundManager::SoundManager(int pin) : _pin(pin) {}

void SoundManager::begin(){
    digitalWrite(_pin, HIGH);
    pinMode(_pin, OUTPUT);
}

void SoundManager::setMuted(bool mute) {_muted = mute; }

void SoundManager::playNote(int frequency, int duration) {
    if (!consoleConfig.soundEnable) return;
    if (_muted) return;
    digitalWrite(_pin, LOW);
    tone(_pin, frequency, duration);
    digitalWrite(_pin, HIGH);
}

void SoundManager::playSelect(){
    playNote(1500, 50);
}

void SoundManager::playBootUp() {
    int melody[] = {262, 330, 392, 523};
    for (int note : melody) {
        playNote(note, 150);
        delay(160);
    }

}
