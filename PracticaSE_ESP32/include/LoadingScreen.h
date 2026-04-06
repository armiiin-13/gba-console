#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include "Screen.h"

class LoadingScreen : public Screen {
 private:
  GameCard* _game;
  uint32_t _startTime;
  bool _needsRedraw = true;

 public:
  LoadingScreen(GameCard* game);

  void enter() override;
  ConsoleState update() override;
  void draw(Adafruit_ST7735& tft) override;
  void exit() override;
  bool getNeedsRedraw() override;
};

#endif