#ifndef MENUSCREEN_H
#define MENUSCREEN_H

#include "Button.h"
#include "Definitions.h"
#include "Screen.h"
#include "SoundManager.h"

class MenuScreen : public Screen {
 private:
  // Dependencias
  Button& _btnUp;
  Button& _btnDown;
  Button& _btnA;
  SoundManager& _sound;

  // Estado interno menu
  int _cursorMenu = 0;
  bool _needsRedraw = true;
  GameCard** _activeCardRef;

 public:
  MenuScreen(Button& up, Button& down, Button& a, GameCard** cardRef, SoundManager& sound);

  void enter() override;
  ConsoleState update() override;
  void draw(Adafruit_ST7735& tft) override;
  void exit() override;
  bool getNeedsRedraw() override;
};

#endif

/*
usamos & en Button& para que el constructor no cree una copia de
los botones pasados sino que use los botones originales referenciandolos
con la variable Button&
*/