#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
    private:                                    
        uint8_t _pin;                           // pin connected
        bool _lastState;                        // last state read
        bool _stableState;
        unsigned long _lastDebouncetime;        // time when state last changed
        unsigned long _pressStartTime;
        unsigned long _debounceDelay = 50;

    public:
        Button(u_int8_t);                       // constructor
        void begin();                           // pinMode in INPUT_PULLUP
        bool isPressed();                       
        bool isLongPressed();
};

#endif