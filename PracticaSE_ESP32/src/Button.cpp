#include "Button.h"

Button::Button(uint8_t pin)
    : _pin(pin),
      _lastState(HIGH),  // debido al INPUT_PULLUP
      _stableState(HIGH),
      _lastDebouncetime(0) {}

void Button::begin() { pinMode(_pin, INPUT_PULLUP); }

bool Button::isPressed() {
  bool reading = digitalRead(_pin);
  bool pressed = false;

  // Si el estado cambió (ruido o pulsación) actualizamos cronometro
  // (Reseteador)
  if (reading != _lastState) _lastDebouncetime = millis();

  // Comprueba si pasado el tiempo de delay, no ha cambiado el estado
  // (significara que no es ruido) (Validador)
  if ((millis() - _lastDebouncetime) > _debounceDelay) {
    // Si la lectura es diferente al estado estable (el que sabemos que no es
    // ruido gracias al milis) significa que se ha pulsado el boton
    if (reading != _stableState) {
      _stableState = reading;

      if (_stableState == LOW) pressed = true;
    }
  }

  _lastState = reading;
  return pressed;
}

bool Button::isLongPressed() {
  bool reading = digitalRead(_pin);

  // 1. Detectamos el MOMENTO exacto en que se empieza a pulsar
  if (reading == LOW && _lastState == HIGH) {
    _pressStartTime = millis();
  }

  // 2. Si el botón SIGUE pulsado, comprobamos el cronómetro
  if (reading == LOW) {
    if (millis() - _pressStartTime >= 2000) {
      // Opcional: resetear _pressStartTime para no devolver true
      // infinitamente mientras siga pulsado
      _pressStartTime = millis();
      return true;
    }
  }

  return false;
}