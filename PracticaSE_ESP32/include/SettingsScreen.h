#ifndef SETTINGS_SCREEN_H
#define SETTINGS_SCREEN_H

#include "Screen.h"
#include "Button.h"

class SettingsScreen : public Screen {
 private:
  bool _needsRedraw = true;
  Button &_btnUp, &_btnDown, &_btnLeft, &_btnRight, &_btnA, &_btnB ;
  
  int _selectedOption = 0;
  const int _totalOptions = 3; // 0: Idioma, 1: Volumen, 2: Fecha/Hora

  // Variables de estado (Lo que modificamos)
  bool _isEnglish = false;
  bool _soundOn = true;

 public:
  SettingsScreen(Button& up, Button& down, Button& l, Button& r, Button& a, Button& b);
  void enter() override;
  ConsoleState update() override;
  void draw(Adafruit_ST7735& tft) override;
  void exit() override;
  bool getNeedsRedraw() override;
};
#endif