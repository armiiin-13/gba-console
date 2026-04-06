#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
    private:                                    // variables que solo pueden ser modificadas por la propia clase
        uint8_t _pin;                           // pin (placa) al que esta conectado el boton
        bool _lastState;                        // estado actual y ultimo para 
        bool _stableState;
        unsigned long _lastDebouncetime;        // guarda el tiempo de la ultima vez que el pin cambio de estado
        unsigned long _pressStartTime;
        unsigned long _debounceDelay = 50;

    public:
        Button(u_int8_t);                       // constructor
        void begin();                           // pinMode en INPUT_PULLUP
        bool isPressed();                       
        bool isLongPressed();
};

#endif